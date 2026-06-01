#include <benchmark/benchmark.h>
#include <cstdlib>
#include <vector>

// 1) 只测 malloc：分配计时，释放不计时
static void BM_MallocOnly(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));

    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(1024);

        for (int i = 0; i < 1024; ++i) {
            void* p = std::malloc(sz);
            if (!p) {
                state.SkipWithError("malloc failed");
                return;
            }
            benchmark::DoNotOptimize(p);
            ptrs.push_back(p);
        }

        // 防止编译器错误优化
        benchmark::ClobberMemory();

        // cleanup 不计入本轮统计（本轮循环内仍会执行，但我们把释放成本分离到其他基准）
        for (void* p : ptrs) {
            std::free(p);
        }
    }

    state.SetBytesProcessed(
            static_cast<int64_t>(state.iterations()) * 1024 * static_cast<int64_t>(sz));
}

// 2) 只测 free：先分配，再只计时 free
static void BM_FreeOnly(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));

    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(1024);

        // 预分配（不希望重点测这一段）
        for (int i = 0; i < 1024; ++i) {
            void* p = std::malloc(sz);
            if (!p) {
                state.SkipWithError("malloc failed");
                return;
            }
            ptrs.push_back(p);
        }

        benchmark::ClobberMemory();

        // 释放（这里会计入 benchmark 迭代）
        for (void* p : ptrs) {
            std::free(p);
        }
    }

    state.SetBytesProcessed(
            static_cast<int64_t>(state.iterations()) * 1024 * static_cast<int64_t>(sz));
}

// 3) 成对 malloc/free：更接近业务场景
static void BM_MallocFreePair(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));

    for (auto _ : state) {
        for (int i = 0; i < 1024; ++i) {
            void* p = std::malloc(sz);
            if (!p) {
                state.SkipWithError("malloc failed");
                return;
            }
            benchmark::DoNotOptimize(p);
            std::free(p);
        }
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(
            static_cast<int64_t>(state.iterations()) * 1024 * static_cast<int64_t>(sz));
}

// 测试不同分配大小
BENCHMARK(BM_MallocOnly)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);
BENCHMARK(BM_FreeOnly)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);
BENCHMARK(BM_MallocFreePair)->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096);

BENCHMARK_MAIN();