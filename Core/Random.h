#pragma once

#include <random>
#include <chrono>

int Random(int from, int to)
{
    static std::random_device rd;
    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> randomNumber(from, to);

    return randomNumber(generator);
}