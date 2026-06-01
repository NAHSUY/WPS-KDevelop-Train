#include <benchmark/benchmark.h>
#include <windows.h>
#include <cstdlib>
#include <vector>

// 每轮操作次数（避免单次调用太短）
static constexpr int kOpsPerIteration = 1024;

// malloc/free 成对测试
static void BM_MallocFreePair(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));

    for (auto _ : state) {
        for (int i = 0; i < kOpsPerIteration; ++i) {
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

    state.SetItemsProcessed(state.iterations() * kOpsPerIteration);
    state.SetBytesProcessed(state.iterations() * kOpsPerIteration * static_cast<int64_t>(sz));
}

// HeapAlloc/HeapFree 成对测试（进程默认堆）
static void BM_HeapAllocFreePair(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));
    HANDLE heap = GetProcessHeap();
    if (!heap) {
        state.SkipWithError("GetProcessHeap failed");
        return;
    }

    for (auto _ : state) {
        for (int i = 0; i < kOpsPerIteration; ++i) {
            void* p = HeapAlloc(heap, 0, sz);
            if (!p) {
                state.SkipWithError("HeapAlloc failed");
                return;
            }
            benchmark::DoNotOptimize(p);
            if (!HeapFree(heap, 0, p)) {
                state.SkipWithError("HeapFree failed");
                return;
            }
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * kOpsPerIteration);
    state.SetBytesProcessed(state.iterations() * kOpsPerIteration * static_cast<int64_t>(sz));
}

// 只测 malloc（分配后统一释放）
static void BM_MallocOnly(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));

    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(kOpsPerIteration);

        for (int i = 0; i < kOpsPerIteration; ++i) {
            void* p = std::malloc(sz);
            if (!p) {
                state.SkipWithError("malloc failed");
                return;
            }
            benchmark::DoNotOptimize(p);
            ptrs.push_back(p);
        }

        benchmark::ClobberMemory();

        for (void* p : ptrs) {
            std::free(p);
        }
    }

    state.SetItemsProcessed(state.iterations() * kOpsPerIteration);
    state.SetBytesProcessed(state.iterations() * kOpsPerIteration * static_cast<int64_t>(sz));
}

// 只测 HeapAlloc（分配后统一释放）
static void BM_HeapAllocOnly(benchmark::State& state) {
    const size_t sz = static_cast<size_t>(state.range(0));
    HANDLE heap = GetProcessHeap();
    if (!heap) {
        state.SkipWithError("GetProcessHeap failed");
        return;
    }

    for (auto _ : state) {
        std::vector<void*> ptrs;
        ptrs.reserve(kOpsPerIteration);

        for (int i = 0; i < kOpsPerIteration; ++i) {
            void* p = HeapAlloc(heap, 0, sz);
            if (!p) {
                state.SkipWithError("HeapAlloc failed");
                return;
            }
            benchmark::DoNotOptimize(p);
            ptrs.push_back(p);
        }

        benchmark::ClobberMemory();

        for (void* p : ptrs) {
            HeapFree(heap, 0, p);
        }
    }

    state.SetItemsProcessed(state.iterations() * kOpsPerIteration);
    state.SetBytesProcessed(state.iterations() * kOpsPerIteration * static_cast<int64_t>(sz));
}

// 测试大小
#define BENCH_ARGS ->Arg(16)->Arg(64)->Arg(256)->Arg(1024)->Arg(4096)->Arg(65536)

BENCHMARK(BM_MallocOnly) BENCH_ARGS;
BENCHMARK(BM_HeapAllocOnly) BENCH_ARGS;
BENCHMARK(BM_MallocFreePair) BENCH_ARGS;
BENCHMARK(BM_HeapAllocFreePair) BENCH_ARGS;

BENCHMARK_MAIN();