#include <iostream>
#include <iomanip>
#include <chrono>

std::chrono::system_clock::duration calculateDelay(uint16_t delayDays, uint16_t minutesFromMidnight, uint16_t secondsFromMidnight);
void SetActionTime(uint16_t delayDays, uint16_t minutesFromMidnight,uint16_t secondsFromMidnight);

int main()
{
    auto   tnow = std::chrono::system_clock::now();
    time_t tnowTime = std::chrono::system_clock::to_time_t(tnow);
    tm*    tnowDate = std::localtime(&tnowTime);
    
    // 1 minute from now
    uint16_t delayDays = 0;
    uint16_t minutesFromMidnight = tnowDate->tm_hour * 60 + tnowDate->tm_min + 1;
    uint16_t secondsFromMidnight = tnowDate->tm_sec;
    
    SetActionTime(delayDays, minutesFromMidnight, secondsFromMidnight);

    return 0;
}

void SetActionTime(uint16_t delayDays, uint16_t minutesFromMidnight,uint16_t secondsFromMidnight)
{
    std::cout << "set action time delayDays:" << delayDays << ", minutesFromMidnight:" << minutesFromMidnight
              << ", secondsFromMidnight:" << secondsFromMidnight << std::endl;
    
    std::chrono::system_clock::duration actionTime = calculateDelay(delayDays, minutesFromMidnight, secondsFromMidnight);
    auto actionTimeout = std::chrono::duration_cast<std::chrono::seconds>(actionTime).count();
    
    std::cout << "\nactionTimeout:" << (uint64_t)actionTimeout << std::endl;
}

std::chrono::system_clock::duration calculateDelay(uint16_t delayDays, uint16_t minutesFromMidnight, uint16_t secondsFromMidnight)
{
    auto   tnow               = std::chrono::system_clock::now();
    
    std::time_t tnowTemp = std::chrono::system_clock::to_time_t(tnow);
    std::tm tmNow = *std::localtime(&tnowTemp);
    std::cout << "Date now:     " << std::put_time(&tmNow, "%c %Z") << std::endl;
    
    
    auto   triggerDay         = tnow + std::chrono::days(delayDays);
    time_t triggerDayAsTime = std::chrono::system_clock::to_time_t(triggerDay);
    tm*    date               = std::localtime(&triggerDayAsTime);
    date->tm_hour             = minutesFromMidnight / 60;
    date->tm_min              = minutesFromMidnight - (minutesFromMidnight / 60) * 60;
    date->tm_sec              = secondsFromMidnight;

    auto triggerDateAndHour = std::chrono::system_clock::from_time_t(std::mktime(date));    
    if(triggerDateAndHour <= tnow)
    {
        triggerDateAndHour = triggerDateAndHour + std::chrono::days(1);
    }
    
    std::time_t tmp = std::chrono::system_clock::to_time_t(triggerDateAndHour);
    std::tm tmptr = *std::localtime(&tmp);
    std::cout << "Trigger date: " << std::put_time(&tmptr, "%c %Z") << std::endl;

    return triggerDateAndHour - tnow;
}