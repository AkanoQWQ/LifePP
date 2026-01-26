#pragma once
#include <windows.h>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <optional>
#include <cstring>
#include <cstdint>
#include <type_traits>

using PipeBufferType = uint8_t;

template<typename MessageType,typename ResponseType,size_t BUFFER_SIZE = 256>
class NamedPipeServer{
private:
    using Handler = std::function<ResponseType(const MessageType&)>;
    std::wstring pipeName;
    std::thread worker;
    std::atomic<bool> running{false};
    Handler handler = nullptr;
    PipeBufferType buffer[BUFFER_SIZE] = {0};
public:
    static_assert(sizeof(MessageType) < sizeof(buffer),
        "Buffer too small!");
    static_assert(std::is_trivially_copyable_v<MessageType>,
        "MessageType must be trivially copyable!");
    static_assert(std::is_trivially_copyable_v<ResponseType>,
        "ResponseType must be trivially copyable!");

    explicit NamedPipeServer(const std::wstring& pipeName)
        : pipeName(pipeName){}
    ~NamedPipeServer(){Stop();}

    inline void SetHandler(const Handler& h){
        handler = h;
        return ;
    }
    inline void Start(){
        if(running.exchange(true)){
            return ;
        }
        worker = std::thread([this]{this->Loop();});
        return ;
    }
    inline void Stop(){
        if(!running.exchange(false)){
            return ;
        }
        CallToWake();// 触发一次连接以解除阻塞
        if(worker.joinable())worker.join();
        return ;
    }
    inline bool IsRunning(){
        return running.load();
    }
private:
    void Loop(){
        while(running){
            HANDLE hPipe = CreateNamedPipeW(
                pipeName.c_str(),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                1, 0, 0, 0, nullptr
            );
            if(hPipe == INVALID_HANDLE_VALUE){
                // 创建失败，略过一次
                continue;
            }
            BOOL connected = ConnectNamedPipe(hPipe, nullptr) ? TRUE :
                                (GetLastError() == ERROR_PIPE_CONNECTED);
            if(connected){
                DWORD read = 0,written = 0;
                ReadFile(hPipe,buffer,sizeof(buffer),&read,nullptr);
                MessageType msg;
                ResponseType reply;
                memcpy(&msg,buffer,sizeof(MessageType));
                
                if(handler != nullptr){
                    reply = handler(msg);
                }else{
                    std::wcout<<L"还没有添加handler!"<<std::endl;
                }

                WriteFile(hPipe, &reply,
                            (DWORD)(sizeof(reply)), &written, nullptr);
                DisconnectNamedPipe(hPipe);
            }
            CloseHandle(hPipe);
        }
        return ;
    }

    // 连接自身管道以唤醒阻塞的 ConnectNamedPipe
    inline void CallToWake(){
        HANDLE h = CreateFileW(pipeName.c_str(), GENERIC_READ|GENERIC_WRITE, 0, nullptr,
                                OPEN_EXISTING, 0, nullptr);
        if(h != INVALID_HANDLE_VALUE){
            CloseHandle(h);
        }
        return ;
    }
};

template<typename MessageType,typename ResponseType,size_t BUFFER_SIZE = 256>
class NamedPipeClient{
private:
    PipeBufferType buffer[BUFFER_SIZE];
    std::wstring pipeName;
public:
    static_assert(sizeof(ResponseType) < sizeof(buffer),
        "Buffer too small!");
    static_assert(std::is_trivially_copyable_v<MessageType>,
        "MessageType must be trivially copyable!");
    static_assert(std::is_trivially_copyable_v<ResponseType>,
        "ResponseType must be trivially copyable!");
    explicit NamedPipeClient(const std::wstring& pipeName)
        : pipeName(pipeName){}
     ~NamedPipeClient() = default;

    std::optional<ResponseType> Send(const MessageType& msg){
        HANDLE hPipe = CreateFileW(
            pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, 0, nullptr
        );
        if(hPipe == INVALID_HANDLE_VALUE){
            return std::nullopt;
        }
        DWORD written = 0,read = 0;
        ResponseType reply;
        WriteFile(hPipe, &msg,
                    (DWORD)(sizeof(msg)), &written, nullptr);
        if(ReadFile(hPipe,buffer,sizeof(buffer),&read,nullptr)){
            memcpy(&reply,buffer,sizeof(ResponseType));
        }else{
            CloseHandle(hPipe);
            return std::nullopt;
        }
        CloseHandle(hPipe);
        return reply;
    }
};