#pragma once

#include "../pch.h"

namespace TinySpark
{

    class Timer
    {
    public:
        Timer();
        ~Timer();
        void StartClockNow();
        void StopClockNow();
        float TimeElapsedNanoSec();
        float TimeElapsedMilliSec();

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Begin;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_End;
        std::chrono::duration<float> m_Duration;
    };
} // namespace TinySpark

#define START_TIMER(X) X.StartClockNow();
#define TIME_ELAPSED(X) X.StopClockNow(); std::cout << "Time Elapsed " << X.TimeElapsedMilliSec() << " ms" << std::endl;