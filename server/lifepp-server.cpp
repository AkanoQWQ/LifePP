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
        smanaClock.SetStartTime(Smana::GetSecond() + restTime);
    }else if(msg.cmd == Smana::SmanaMessage::Message::status){
        std::string str = std::to_string(Smana::GetSecond() - smanaClock.GetStartTime());
        str += " in ";
        str += std::to_string(alarmStart);
        str += "/";
        str += std::to_string(alarmEnd);
        ret.Encode(Smana::Utf8ToWide(str));
    }
    return ret;
};

Json ReadConfig(){
    auto/*opt<path>*/ appdata = lifepp::tools::GetEnv("APPDATA");
    if(appdata.has_value() == false){
        std::wcout<<L"没有找到APPDATA!"<<std::endl;
        return Json::object();
    }
    std::filesystem::path configFile = appdata.value() / "lifepp" / "config.json";
    std::ifstream instream(configFile);
    if(!instream.is_open()){
        std::wcout<<L"configFile无法打开!"<<std::endl;
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
    std::wcout<<L"read smana config : \n";
    std::wcout<<L"restTime : "<<restTime<<L"\n";
    std::wcout<<L"alarmStart : "<<alarmStart<<L"\n";
    std::wcout<<L"alarmEnd : "<<alarmEnd<<L"\n";
    std::wcout<<L"alarmIntervalSlow : "<<alarmIntervalSlow<<L"\n";
    std::wcout<<L"alarmIntervalQuick : "<<alarmIntervalQuick<<std::endl;
    return ;
}

int wmain(int argc, wchar_t* argv[]){
    _setmode(_fileno(stdout),_O_U16TEXT);
    try{
        std::wcout.imbue(std::locale("C.UTF-8"));
    }catch(const std::runtime_error&){
        std::wcout.imbue(std::locale::classic());
    }

    Json config = ReadConfig();
    SmanaConfig(config);

    smanaServer.SetHandler(smanaCallback);
    smanaServer.Start();
    while(true){
        smanaClock.Update();
    }
    return 0;
}


