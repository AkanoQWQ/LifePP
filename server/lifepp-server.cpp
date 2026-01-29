#include <windows.h>
#include <string>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <iostream>
#include <io.h>
#include <fcntl.h>
#include <locale>
#include <format>
#include <fstream>

#include "nlohmannJson.hpp"
#include "smana.hpp"
#include "NamedPipe.hpp"
#include "lifeppTools.hpp"

using Json = nlohmann::json;

NamedPipe::NamedPipeServer<Smana::SmanaMessage,Smana::SmanaResponse> smanaServer(Smana::smanaPipeName);
Smana::Clock smanaClock;

Smana::SecondType restTime,alarmStart,alarmEnd;
Smana::SecondType alarmIntervalSlow,alarmIntervalQuick;

auto smanaCallback = [](const Smana::SmanaMessage& msg) -> Smana::SmanaResponse{
    Smana::SmanaResponse ret;
    if(msg.cmd == Smana::SmanaMessage::Message::rest){
        int32_t timeVal = msg.DecodeRest();
        std::cout<<"收到rest指令 : "<<timeVal<<"!"<<std::endl;
        smanaClock.SetStartTime(Smana::GetSecond() + timeVal);
    }else if(msg.cmd == Smana::SmanaMessage::Message::status){
        std::cout<<"查询status!"<<std::endl;
        std::string str = std::to_string(Smana::GetSecond() - smanaClock.GetStartTime());
        str += " in ";
        str += std::to_string(alarmStart);
        str += "/";
        str += std::to_string(alarmEnd);
        ret.Encode(str);
    }
    return ret;
};

Json ReadConfig(){
    auto/*opt<path>*/ appdata = lifepp::tools::GetEnv("APPDATA");
    if(appdata.has_value() == false){
        std::cout<<"没有找到APPDATA!"<<std::endl;
        return Json::object();
    }
    std::filesystem::path configFile = appdata.value() / "lifepp" / "config.json";
    std::ifstream instream(configFile);
    if(!instream.is_open()){
        std::cout<<"configFile无法打开!"<<std::endl;
        return Json::object();
    }
    Json json;
    instream>>json;
    return json;
}
void SmanaConfig(const Json& json){
    restTime = json.value("restTime",Smana::defaultRestTime);
    alarmStart = json.value("alarmStart",Smana::defaultAlarmStart);
    alarmEnd = json.value("alarmEnd",Smana::defaultAlarmEnd);
    alarmIntervalSlow = json.value("alarmIntervalSlow",Smana::defaultAlarmIntervalSlow);
    alarmIntervalQuick = json.value("alarmIntervalQuick",Smana::defaultAlarmIntervalQuick);
    smanaClock.SetClock(alarmStart,alarmEnd,alarmIntervalSlow,alarmIntervalQuick);
    smanaClock.SetStartTime(Smana::GetSecond());
    std::cout<<"read smana config : \n";
    std::cout<<"restTime : "<<restTime<<"\n";
    std::cout<<"alarmStart : "<<alarmStart<<"\n";
    std::cout<<"alarmEnd : "<<alarmEnd<<"\n";
    std::cout<<"alarmIntervalSlow : "<<alarmIntervalSlow<<"\n";
    std::cout<<"alarmIntervalQuick : "<<alarmIntervalQuick<<std::endl;
    return ;
}

int main(int argc,char* argv[]){
    SetConsoleOutputCP(CP_UTF8);
    Json config = ReadConfig();
    SmanaConfig(config);

    smanaServer.SetHandler(smanaCallback);
    smanaServer.Start();
    while(true){
        smanaClock.Update();
    }
    return 0;
}


