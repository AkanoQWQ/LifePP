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

#include "smana.hpp"
#include "NamedPipe.hpp"

NamedPipe::NamedPipeServer<Smana::SmanaMessage,Smana::SmanaResponse> smanaServer(Smana::smanaPipeName);
Smana::Clock smanaClock;
Smana::SecondType restTime = 60 * 5;
Smana::SecondType alarmStart = 60 * 40;
Smana::SecondType alarmEnd = 60 * 60;
Smana::SecondType AlarmInterval1 = 60 * 5;
Smana::SecondType AlarmInterval2 = 10;

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

int wmain(int argc, wchar_t* argv[]){
    _setmode(_fileno(stdout),_O_U16TEXT);
    std::wcout.imbue(std::locale(""));
    smanaClock.SetClock(alarmStart,alarmEnd,AlarmInterval1,AlarmInterval2);
    smanaClock.SetStartTime(Smana::GetSecond());
    smanaServer.SetHandler(smanaCallback);
    smanaServer.Start();
    while(true){
        smanaClock.Update();
    }
    return 0;
}


