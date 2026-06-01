#define NOMINMAX
#include <benchmark/benchmark.h>
#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

// -----------------------------
// 配置
// -----------------------------
static constexpr size_t KB = 1024;
static constexpr size_t MB = 1024 * 1024;
static constexpr size_t kReadChunk = 1 * MB;   // ReadFile块大小
static constexpr size_t kPageSize  = 4096;     // 随机访问粒度（页）

// 全局路径（放到可写目录）
static const char* kTestFileName = "benchmark_test_data.bin";

// 防止优化
static inline void ConsumeByte(uint8_t v) {
    benchmark::DoNotOptimize(v);
}

// 用于生成测试文件：至少 fileSize 字节
static bool EnsureTestFile(const std::string& path, size_t fileSize) {
    HANDLE hFile = CreateFileA(
            path.c_str(),
            GENERIC_WRITE | GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(hFile, &sz)) {
        CloseHandle(hFile);
        return false;
    }

    if (static_cast<size_t>(sz.QuadPart) >= fileSize) {
        CloseHandle(hFile);
        return true;
    }

    // 扩展文件并写入一些可预测内容
    std::vector<uint8_t> buf(1 * MB);
    for (size_t i = 0; i < buf.size(); ++i) buf[i] = static_cast<uint8_t>(i & 0xFF);

    size_t remaining = fileSize;
    SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);

    while (remaining > 0) {
        DWORD toWrite = static_cast<DWORD>(std::min(remaining, buf.size()));
        DWORD written = 0;
        if (!WriteFile(hFile, buf.data(), toWrite, &written, nullptr) || written != toWrite) {
            CloseHandle(hFile);
            return false;
        }
        remaining -= written;
    }

    FlushFileBuffers(hFile);
    CloseHandle(hFile);
    return true;
}

// -----------------------------
// 1) 顺序读：ReadFile
// -----------------------------
static void BM_ReadFile_Sequential(benchmark::State& state) {
    const size_t fileSize = static_cast<size_t>(state.range(0));
    const std::string path = kTestFileName;

    if (!EnsureTestFile(path, fileSize)) {
        state.SkipWithError("EnsureTestFile failed");
        return;
    }

    std::vector<uint8_t> buffer(kReadChunk);

    for (auto _ : state) {
        HANDLE hFile = CreateFileA(
                path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                nullptr);

        if (hFile == INVALID_HANDLE_VALUE) {
            state.SkipWithError("CreateFileA failed");
            return;
        }

        // 从头读
        SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);

        size_t totalRead = 0;
        uint64_t checksum = 0;

        while (totalRead < fileSize) {
            DWORD toRead = static_cast<DWORD>(std::min(kReadChunk, fileSize - totalRead));
            DWORD bytesRead = 0;
            BOOL ok = ReadFile(hFile, buffer.data(), toRead, &bytesRead, nullptr);
            if (!ok) {
                CloseHandle(hFile);
                state.SkipWithError("ReadFile failed");
                return;
            }
            if (bytesRead == 0) break;

            // 消费数据，防止优化
            for (DWORD i = 0; i < bytesRead; i += 4096) checksum += buffer[i];

            totalRead += bytesRead;
        }

        CloseHandle(hFile);
        ConsumeByte(static_cast<uint8_t>(checksum & 0xFF));
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(fileSize));
}

// -----------------------------
// 2) 顺序读：MapViewOfFile
// -----------------------------
static void BM_MMap_Sequential(benchmark::State& state) {
    const size_t fileSize = static_cast<size_t>(state.range(0));
    const std::string path = kTestFileName;

    if (!EnsureTestFile(path, fileSize)) {
        state.SkipWithError("EnsureTestFile failed");
        return;
    }

    for (auto _ : state) {
        HANDLE hFile = CreateFileA(
                path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

        if (hFile == INVALID_HANDLE_VALUE) {
            state.SkipWithError("CreateFileA failed");
            return;
        }

        HANDLE hMap = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!hMap) {
            CloseHandle(hFile);
            state.SkipWithError("CreateFileMappingA failed");
            return;
        }

        void* view = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, fileSize);
        if (!view) {
            CloseHandle(hMap);
            CloseHandle(hFile);
            state.SkipWithError("MapViewOfFile failed");
            return;
        }

        auto* p = static_cast<uint8_t*>(view);
        uint64_t checksum = 0;

        for (size_t i = 0; i < fileSize; i += 4096) checksum += p[i];

        ConsumeByte(static_cast<uint8_t>(checksum & 0xFF));
        benchmark::ClobberMemory();

        UnmapViewOfFile(view);
        CloseHandle(hMap);
        CloseHandle(hFile);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(fileSize));
}

// -----------------------------
// 3) 随机页访问：ReadFile（每次 seek + 读4KB）
// -----------------------------
static void BM_ReadFile_RandomPage(benchmark::State& state) {
    const size_t fileSize = static_cast<size_t>(state.range(0));
    const std::string path = kTestFileName;

    if (!EnsureTestFile(path, fileSize)) {
        state.SkipWithError("EnsureTestFile failed");
        return;
    }
    if (fileSize < kPageSize) {
        state.SkipWithError("file too small");
        return;
    }

    const size_t pageCount = fileSize / kPageSize;
    std::vector<size_t> pages(pageCount);
    for (size_t i = 0; i < pageCount; ++i) pages[i] = i;
    std::reverse(pages.begin(), pages.end()); // 固定“伪随机”次序，保证可复现

    std::vector<uint8_t> page(kPageSize);

    for (auto _ : state) {
        HANDLE hFile = CreateFileA(
                path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                nullptr);

        if (hFile == INVALID_HANDLE_VALUE) {
            state.SkipWithError("CreateFileA failed");
            return;
        }

        uint64_t checksum = 0;
        for (size_t idx : pages) {
            LARGE_INTEGER li{};
            li.QuadPart = static_cast<LONGLONG>(idx * kPageSize);
            if (!SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN)) {
                CloseHandle(hFile);
                state.SkipWithError("SetFilePointerEx failed");
                return;
            }

            DWORD bytesRead = 0;
            if (!ReadFile(hFile, page.data(), static_cast<DWORD>(kPageSize), &bytesRead, nullptr) ||
                bytesRead != kPageSize) {
                CloseHandle(hFile);
                state.SkipWithError("ReadFile random failed");
                return;
            }
            checksum += page[0];
        }

        CloseHandle(hFile);
        ConsumeByte(static_cast<uint8_t>(checksum & 0xFF));
        benchmark::ClobberMemory();
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(fileSize));
}

// -----------------------------
// 4) 随机页访问：MapViewOfFile（随机访存）
// -----------------------------
static void BM_MMap_RandomPage(benchmark::State& state) {
    const size_t fileSize = static_cast<size_t>(state.range(0));
    const std::string path = kTestFileName;

    if (!EnsureTestFile(path, fileSize)) {
        state.SkipWithError("EnsureTestFile failed");
        return;
    }
    if (fileSize < kPageSize) {
        state.SkipWithError("file too small");
        return;
    }

    const size_t pageCount = fileSize / kPageSize;
    std::vector<size_t> pages(pageCount);
    for (size_t i = 0; i < pageCount; ++i) pages[i] = i;
    std::reverse(pages.begin(), pages.end());

    for (auto _ : state) {
        HANDLE hFile = CreateFileA(
                path.c_str(),
                GENERIC_READ,
                FILE_SHARE_READ,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);

        if (hFile == INVALID_HANDLE_VALUE) {
            state.SkipWithError("CreateFileA failed");
            return;
        }

        HANDLE hMap = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!hMap) {
            CloseHandle(hFile);
            state.SkipWithError("CreateFileMappingA failed");
            return;
        }

        void* view = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, fileSize);
        if (!view) {
            CloseHandle(hMap);
            CloseHandle(hFile);
            state.SkipWithError("MapViewOfFile failed");
            return;
        }

        auto* p = static_cast<uint8_t*>(view);
        uint64_t checksum = 0;

        for (size_t idx : pages) {
            checksum += p[idx * kPageSize];
        }

        ConsumeByte(static_cast<uint8_t>(checksum & 0xFF));
        benchmark::ClobberMemory();

        UnmapViewOfFile(view);
        CloseHandle(hMap);
        CloseHandle(hFile);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(fileSize));
}

// 数据规模：64MB, 256MB, 1GB
#define FILE_SIZES ->Arg(64 * MB)->Arg(256 * MB)->Arg(1024 * MB)

BENCHMARK(BM_ReadFile_Sequential) FILE_SIZES;
BENCHMARK(BM_MMap_Sequential) FILE_SIZES;
BENCHMARK(BM_ReadFile_RandomPage) FILE_SIZES;
BENCHMARK(BM_MMap_RandomPage) FILE_SIZES;

BENCHMARK_MAIN();