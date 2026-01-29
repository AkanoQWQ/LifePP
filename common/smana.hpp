#pragma once
#include <windows.h>
#include <string>
#include <cstring>
#include <chrono>

namespace Smana{

static const wchar_t* smanaPipeName = L"\\\\.\\pipe\\smana_control";

using SecondType = std::chrono::seconds::rep;

constexpr SecondType defaultRestTime = 60 * 5;// 5min
constexpr SecondType defaultAlarmStart = 60 * 40;// 40min
constexpr SecondType defaultAlarmEnd = 60 * 60;// 60min
constexpr SecondType defaultAlarmIntervalSlow = 60 * 5;// 5min
constexpr SecondType defaultAlarmIntervalQuick = 10;// 10s

struct SmanaMessage{
    constexpr static size_t BUFFER_SIZE = 64;
    char buffer[BUFFER_SIZE];
    enum class Message : uint8_t{
        status = 0,
        rest = 1
    }cmd;
    void EncodeRest(int32_t time){
        std::memcpy(buffer,&time,sizeof(time));
        return ;
    }
    int32_t DecodeRest()const{
        int32_t timeVal = 0;
        std::memcpy(&timeVal,buffer,sizeof(timeVal));
        return timeVal;
    }
};
struct SmanaResponse{
    constexpr static size_t BUFFER_SIZE = 64;
    char buffer[BUFFER_SIZE];
    void Encode(const std::string& str){
        size_t n = str.size();
        if(n >= BUFFER_SIZE)n = BUFFER_SIZE - 1;
        std::memcpy(buffer,str.data(),n);
        buffer[n] = '\0';
        return ;
    }
    std::string Decode(){
        size_t n = 0;
        while(n < BUFFER_SIZE && buffer[n] != '\0'){
            n++;
        }
        return std::string(buffer,buffer+n);
    }
};

SecondType GetSecond(){
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}
std::wstring Utf8ToWide(const std::string& s){
    if(s.empty())return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring ws(len - 1,L'\0');
    MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1, ws.data(), len);
    return ws;
}
void Alert(const std::string& title,const std::string& body){
    std::string cmd =
        "pwsh -WindowStyle Hidden -NoProfile -ExecutionPolicy Bypass -Command "
        "\"Import-Module BurntToast; "
        "New-BurntToastNotification -Text '" + title + "','" + body + "'\"";
    _wsystem(Utf8ToWide(cmd).c_str());
    return ;
}

class Clock{
private:
    SecondType startTime;
    SecondType alarmStart,alarmEnd;
    class Trigger{
    private:
        SecondType interval,lastTriggerTime;
    public:
        // 返回当前时间是否触发
        bool Update(SecondType now){
            if(lastTriggerTime == -1){
                lastTriggerTime = now;
                return false;
            }
            if(now - lastTriggerTime >= interval){
                lastTriggerTime = now;
                return true;
            }
            return false;
        }
        void SetInterval(SecondType _interval){
            interval = _interval;
            return ;
        }
        void Reset(){
            lastTriggerTime = -1;
            return ;
        }
    }counterSlow,counterRapid;
public:
    void SetStartTime(SecondType _startTime){
        startTime = _startTime;
        return ;
    }
    SecondType GetStartTime(){
        return startTime;
    }
    void SetClock(SecondType _alarmStart,SecondType _alarmEnd,SecondType _intervalSlow,SecondType _intervalRapid){
        alarmStart = _alarmStart;
        alarmEnd = _alarmEnd;
        counterSlow.SetInterval(_intervalSlow);
        counterRapid.SetInterval(_intervalRapid);
        return ;
    }
    void Update(){
        SecondType now = GetSecond();
        if(now - startTime > alarmEnd){
            // 急迫提示
            counterSlow.Reset();
            if(counterRapid.Update(now)){
                Alert("smana","现在应该去休息眼睛了!");
            }
        }else if(now - startTime > alarmStart){
            // 温和提示
            counterRapid.Reset();
            if(counterSlow.Update(now)){
                Alert("smana","应该去休息了");
            }
        }else{
            counterSlow.Reset();
            counterRapid.Reset();
        }
        return ;
    }
};

} // namespace Smana