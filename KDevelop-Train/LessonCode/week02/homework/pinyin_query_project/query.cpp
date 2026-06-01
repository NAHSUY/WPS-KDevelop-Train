#define NOMINMAX
#include <windows.h>
#include <psapi.h>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cstdint>
#include <cstring>

#pragma pack(push, 1)
struct FixedRecord {
    char key[16];
    uint64_t start;
    uint64_t end;
};
#pragma pack(pop)

static inline void trim_spaces_and_line_end(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}

static bool split_csv_line(const std::string& line, std::string& pinyin, std::string& word) {
    auto pos = line.find(',');
    if (pos == std::string::npos) return false;
    pinyin = line.substr(0, pos);
    word = line.substr(pos + 1);
    trim_spaces_and_line_end(pinyin);
    trim_spaces_and_line_end(word);
    return true;
}

static int cmp_key16(const char key16[16], const std::string& target) {
    char t[16] = { 0 };
    size_t n = target.size() > 15 ? 15 : target.size();
    std::memcpy(t, target.data(), n);

    int c = std::memcmp(key16, t, 16);
    if (c < 0) return -1;
    if (c > 0) return 1;
    return 0;
}

static bool find_range_in_index(const std::string& idx_path, const std::string& target, uint64_t& start, uint64_t& end) {
    std::ifstream fin(idx_path, std::ios::binary);
    if (!fin) return false;

    char magic[8]{};
    uint32_t count = 0;

    fin.read(magic, 8);
    fin.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!fin) return false;
    if (std::memcmp(magic, "PYIDXV1\0", 8) != 0) return false;

    const std::streamoff base = 8 + 4;
    int64_t l = 0, r = static_cast<int64_t>(count) - 1;

    while (l <= r) {
        int64_t m = l + ((r - l) >> 1);
        std::streamoff pos = base + static_cast<std::streamoff>(m) * static_cast<std::streamoff>(sizeof(FixedRecord));
        fin.seekg(pos, std::ios::beg);
        if (!fin) return false;

        FixedRecord rec{};
        fin.read(reinterpret_cast<char*>(&rec), sizeof(rec));
        if (!fin) return false;

        int c = cmp_key16(rec.key, target);
        if (c == 0) {
            start = rec.start;
            end = rec.end;
            return true;
        }
        else if (c < 0) {
            l = m + 1;
        }
        else {
            r = m - 1;
        }
    }
    return false;
}

static SIZE_T working_set_bytes() {
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
}
static SIZE_T private_bytes() {
    PROCESS_MEMORY_COUNTERS_EX pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
        return pmc.PrivateUsage;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: query_fixed <dict.csv> <index_fixed.bin> <pinyin> <output.csv>\n";
        return 1;
    }

    const std::string csv_path = argv[1];
    const std::string idx_path = argv[2];
    std::string target = argv[3];
    const std::string out_path = argv[4];
    trim_spaces_and_line_end(target);

    SIZE_T ws_before = working_set_bytes();
    SIZE_T pv_before = private_bytes();

    auto t1 = std::chrono::steady_clock::now();

    uint64_t start = 0, end = 0;
    bool ok = find_range_in_index(idx_path, target, start, end);

    std::ofstream fout(out_path, std::ios::binary | std::ios::trunc);
    if (!fout) {
        std::cerr << "Failed to open output: " << out_path << "\n";
        return 1;
    }
    fout << "pinyin,word\n";

    size_t hit_count = 0;

    if (ok) {
        std::ifstream fin(csv_path, std::ios::binary);
        if (!fin) {
            std::cerr << "Failed to open csv: " << csv_path << "\n";
            return 1;
        }

        fin.seekg(static_cast<std::streamoff>(start), std::ios::beg);

        std::string line;
        while (std::getline(fin, line)) {
            std::streampos pos = fin.tellg();
            if (pos != std::streampos(-1) && static_cast<uint64_t>(pos) > end) break;

            trim_spaces_and_line_end(line);
            if (line.empty()) continue;

            std::string py, word;
            if (!split_csv_line(line, py, word)) continue;
            if (py == target) {
                fout << py << "," << word << "\n";
                ++hit_count;
            }
        }
    }

    fout.flush();

    auto t2 = std::chrono::steady_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    SIZE_T ws_after = working_set_bytes();
    SIZE_T pv_after = private_bytes();

    if (!ok) std::cout << "Not found: " << target << "\n";
    else     std::cout << "Found " << hit_count << " rows for: " << target << "\n";

    std::cout << "Output: " << out_path << "\n";
    std::cout << "Query time: " << us << " us\n";
    std::cout << "WorkingSet: " << (ws_after / 1024.0 / 1024.0) << " MB\n";
    std::cout << "PrivateUsage: " << (pv_after / 1024.0 / 1024.0) << " MB\n";
    std::cout << "Delta WorkingSet: " << ((ws_after - ws_before) / 1024.0 / 1024.0) << " MB\n";
    std::cout << "Delta PrivateUsage: " << ((pv_after - pv_before) / 1024.0 / 1024.0) << " MB\n";

    return 0;
}