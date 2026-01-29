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

int main(int argc,char* argv[]){
    SetConsoleOutputCP(CP_UTF8);
    if (argc != 2) {
        std::cout<<"使用方法: smana <status|rest>\n";
        return 0;
    }
    std::string cmd = argv[1];
    std::optional<Smana::SmanaResponse> reply;
    if(cmd == "status"){
        reply = pipeClient.Send({Smana::SmanaMessage::Message::status});
        if(reply.has_value() && reply.value().Decode().length() != 0){
            std::cout<<"status : "<<reply.value().Decode()<<'\n';
        }else{
            std::cout<<"未获取到状态!\n";
        }
    }else if(cmd == "rest"){
        reply = pipeClient.Send({Smana::SmanaMessage::Message::rest});
    }else{
        std::cout<<"未知指令!!\n";
    }
    return 0;
}
