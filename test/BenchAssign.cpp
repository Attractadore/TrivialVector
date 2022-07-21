#include "Attractadore/TrivialVector.hpp"

#include <benchmark/benchmark.h>

using Attractadore::TrivialVector;

inline constexpr size_t final_size = 1 << 15;

void StdVectorAssign(benchmark::State& state)
{
    for (auto _: state) {
        std::vector<int> v1(final_size);
        benchmark::DoNotOptimize(v1.data());
        for (auto i = 0; i < final_size; i++) {
            v1[i] = i;
        }
        benchmark::ClobberMemory();
    }
}

void TrivialVectorAssign(benchmark::State& state)
{
    for (auto _: state) {
        TrivialVector<int> v1(final_size);
        benchmark::DoNotOptimize(v1.data());
        for (auto i = 0; i < final_size; i++) {
            v1[i] = i;
        }
        benchmark::ClobberMemory();
    }
}

void AllocAssign(benchmark::State& state)
{
    for (auto _: state) {
        int* v1 = new int[final_size];
        benchmark::DoNotOptimize(v1);
        for (auto i = 0; i < final_size; i++) {
            v1[i] = i;
        }
        benchmark::ClobberMemory();
        delete[] v1;
    }
}

BENCHMARK(StdVectorAssign);
BENCHMARK(TrivialVectorAssign);
BENCHMARK(AllocAssign);

BENCHMARK_MAIN();
