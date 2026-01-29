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
#include <charconv>
#include <system_error>

#include "smana.hpp"
#include "NamedPipe.hpp"


NamedPipe::NamedPipeClient<Smana::SmanaMessage,Smana::SmanaResponse> pipeClient(Smana::smanaPipeName);

int32_t GetRestTime(int argc,char* argv[]){
    if(argc < 3){
        std::cout<<"没有rest参数,传默认参数:"<<Smana::defaultRestTime<<std::endl;
        return Smana::defaultRestTime;
    }
    std::string timeParamStr = argv[2];
    int32_t val = 0;
    auto p = std::from_chars(timeParamStr.data(),timeParamStr.data()+timeParamStr.size(),val);
    if(p.ec != std::errc() || p.ptr != timeParamStr.data()+timeParamStr.size()){
        std::cout<<"传入的rest参数不是整数,传默认参数:"<<Smana::defaultRestTime<<std::endl;
        return Smana::defaultRestTime;
    }
    return val;
}

int main(int argc,char* argv[]){
    SetConsoleOutputCP(CP_UTF8);
    if (argc < 2) {
        std::cout<<"使用方法: smana <status|rest>\n";
        return 0;
    }
    std::string cmd = argv[1];
    std::optional<Smana::SmanaResponse> reply;
    Smana::SmanaMessage msg;
    if(cmd == "status"){
        msg.cmd = Smana::SmanaMessage::Message::status;
        reply = pipeClient.Send(msg);
        if(reply.has_value() && reply.value().Decode().length() != 0){
            std::cout<<"status : "<<reply.value().Decode()<<'\n';
        }else{
            std::cout<<"未获取到状态!\n";
        }
    }else if(cmd == "rest"){
        msg.cmd = Smana::SmanaMessage::Message::rest;
        msg.EncodeRest(GetRestTime(argc,argv));
        reply = pipeClient.Send(msg);
    }else{
        std::cout<<"未知指令!!\n";
    }
    return 0;
}
