#include <windows.h>
#include <string>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <iostream>
#include <optional>
#include <io.h>
#include <fcntl.h>
#include <locale>

#include "smana.hpp"
#include "NamedPipe.hpp"


NamedPipe::NamedPipeClient<Smana::SmanaMessage,Smana::SmanaResponse> pipeClient(Smana::smanaPipeName);

int wmain(int argc, wchar_t* argv[]){
    _setmode(_fileno(stdout),_O_U16TEXT);
    std::wcout.imbue(std::locale(""));
    if (argc != 2) {
        std::wcout<<L"使用方法: smana <status|rest>\n";
        return 0;
    }
    std::wstring cmd = argv[1];
    std::optional<Smana::SmanaResponse> reply;
    if(cmd == L"status"){
        reply = pipeClient.Send({Smana::SmanaMessage::Message::status});
        if(reply.has_value() && reply.value().Decode().length() != 0){
            std::wcout<<L"status : "<<reply.value().Decode()<<L'\n';
        }else{
            std::wcout<<L"未获取到状态!\n";
        }
    }else if(cmd == L"rest"){
        reply = pipeClient.Send({Smana::SmanaMessage::Message::rest});
    }else{
        std::wcout<<L"未知指令!!\n";
    }
    return 0;
}
