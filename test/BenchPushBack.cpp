#include "Attractadore/TrivialVector.hpp"

#include <benchmark/benchmark.h>

using Attractadore::TrivialVector;

inline constexpr size_t final_size = 1 << 20;

void StdVectorReservePushBack(benchmark::State& state) {
    for (auto _: state) {
        std::vector<int> v1;
        v1.reserve(final_size);
        benchmark::DoNotOptimize(v1.data());
        for (auto i = 0; i < final_size; i++) {
            v1.push_back(i);
        }
        benchmark::ClobberMemory();
    }
}

void TrivialVectorReservePushBack(benchmark::State& state) {
    for (auto _: state) {
        TrivialVector<int> v1;
        v1.reserve(final_size);
        benchmark::DoNotOptimize(v1.data());
        for (auto i = 0; i < final_size; i++) {
            v1.push_back(i);
        }
        benchmark::ClobberMemory();
    }
}

void TrivialVectorReserveShoveBack(benchmark::State& state) {
    for (auto _: state) {
        TrivialVector<int> v1;
        v1.reserve(final_size);
        benchmark::DoNotOptimize(v1.data());
        for (int i = 0; i < final_size; i++) {
            v1.shove_back(i);
        }
        benchmark::ClobberMemory();
    }
}

void AllocAppend(benchmark::State& state) {
    for (auto _: state) {
        int* v1 = new int[final_size];
        size_t size = 0;
        benchmark::DoNotOptimize(v1);
        for (int i = 0; i < final_size; i++) {
            v1[size++] = i;
        }
        benchmark::ClobberMemory();
        delete[] v1;
    }
}

BENCHMARK(StdVectorReservePushBack);
BENCHMARK(TrivialVectorReservePushBack);
BENCHMARK(TrivialVectorReserveShoveBack);
BENCHMARK(AllocAppend);

BENCHMARK_MAIN();
