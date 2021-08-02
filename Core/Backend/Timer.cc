#include "Timer.h"

namespace TinySpark
{
    Timer::Timer() {}
    Timer::~Timer() {}
    void Timer::StartClockNow() { m_Begin = std::chrono::high_resolution_clock::now(); }
    void Timer::StopClockNow() { m_End = std::chrono::high_resolution_clock::now(); }
    float Timer::TimeElapsedNanoSec() { return (m_End - m_Begin).count(); }
    float Timer::TimeElapsedMilliSec() { return (m_End - m_Begin).count() * 0.000001f; }
    // float Timer::TimeElapsedSec();
} // namespace TinySpark