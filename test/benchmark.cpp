#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <span>
#include <tuple>
#include <vector>

std::tuple<uint64_t, uint64_t> average_and_stddev(std::span<const std::chrono::nanoseconds> tests) {
    uint64_t sum = 0;
    for (auto t: tests) {
        sum += t.count();
    }
    uint64_t average = sum / tests.size();

    sum = 0;
    for (auto t: tests) {
        int64_t diff = average - t.count();
        sum += diff * diff;
    }

    uint64_t stddev = std::sqrt(sum / tests.size());

    return {average, stddev};
}

int main() {
    constexpr auto final_size = (1 << 25);
    constexpr auto takes = 16;

    std::array<std::chrono::nanoseconds, takes> tests;
    std::cout << "Test size: " << final_size << "\n";
{
    for (auto& t: tests) {
        auto t0 = std::chrono::steady_clock::now();
        std::vector<int> v1;
        for (auto i = 0; i < final_size; i++) {
            v1.push_back(i);
        }
        auto t1 = std::chrono::steady_clock::now();
        t = t1 - t0;
    }
    auto [average, stddev] = average_and_stddev(tests);
    std::cout << "Time for push_back: " << average << " +- " << stddev << "\n";
}


{
    for (auto& t: tests) {
        auto t0 = std::chrono::steady_clock::now();
        std::vector<int> v1;
        v1.reserve(final_size);
        for (auto i = 0; i < final_size; i++) {
            v1.push_back(i);
        }
        auto t1 = std::chrono::steady_clock::now();
        t = t1 - t0;
    }
    auto [average, stddev] = average_and_stddev(tests);
    std::cout << "Time for reserve + push_back: " << average << " +- " << stddev << "\n";
}

{
    for (auto& t: tests) {
        auto t0 = std::chrono::steady_clock::now();
        std::vector<int> v1(final_size);
        for (auto i = 0; i < final_size; i++) {
            v1[i] = i;
        }
        auto t1 = std::chrono::steady_clock::now();
        t = t1 - t0;
    }
    auto [average, stddev] = average_and_stddev(tests);
    std::cout << "Time for create + assign: " << average << " +- " << stddev << "\n";
}
}
