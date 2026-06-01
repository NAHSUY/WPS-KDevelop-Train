#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstring>

// 固定索引格式（便于磁盘二分，低内存）
// Header:
//   magic[8] = "PYIDXV1\0"
//   u32 record_count
// Record (fixed 32 bytes):
//   char key[16]      // UTF-8 bytes, '\0' padded, max 15 bytes + '\0'
//   u64 start_offset
//   u64 end_offset

#pragma pack(push, 1)
struct FixedRecord {
    char key[16];
    uint64_t start;
    uint64_t end;
};
#pragma pack(pop)

struct TmpEntry {
    std::string key;
    uint64_t start = 0;
    uint64_t end = 0;
};

static inline void trim_line_end(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}
static inline void trim_spaces(std::string& s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}
static bool split_csv_line(const std::string& line, std::string& pinyin, std::string& word) {
    auto pos = line.find(',');
    if (pos == std::string::npos) return false;
    pinyin = line.substr(0, pos);
    word = line.substr(pos + 1);
    trim_line_end(pinyin);
    trim_line_end(word);
    trim_spaces(pinyin);
    trim_spaces(word);
    return !pinyin.empty();
}

static FixedRecord make_fixed_record(const TmpEntry& e) {
    FixedRecord r{};
    std::memset(&r, 0, sizeof(r));

    // 限制 key 最大 15 字节（最后 1 字节留 '\0'）
    // 你的拼音通常很短，足够覆盖；超长可按需增大 key[32]
    const size_t n = std::min<size_t>(15, e.key.size());
    std::memcpy(r.key, e.key.data(), n);
    r.key[n] = '\0';

    r.start = e.start;
    r.end = e.end;
    return r;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: build_index_fixed <dict.csv> <index_fixed.bin>\n";
        return 1;
    }

    const std::string csv_path = argv[1];
    const std::string idx_path = argv[2];

    std::ifstream fin(csv_path, std::ios::binary);
    if (!fin) {
        std::cerr << "Failed to open csv: " << csv_path << "\n";
        return 1;
    }

    std::string line;
    if (!std::getline(fin, line)) {
        std::cerr << "Empty csv\n";
        return 1;
    }

    std::vector<TmpEntry> entries;
    entries.reserve(700000);

    bool has_prev = false;
    std::string prev_key;
    uint64_t prev_start = 0;

    size_t total_data_lines = 0;
    size_t valid_lines = 0;
    size_t skipped_lines = 0;

    while (true) {
        std::streampos line_start = fin.tellg();
        if (!std::getline(fin, line)) break;

        ++total_data_lines;
        trim_line_end(line);
        if (line.empty()) { ++skipped_lines; continue; }

        std::string key, word;
        if (!split_csv_line(line, key, word)) { ++skipped_lines; continue; }

        ++valid_lines;
        uint64_t off = static_cast<uint64_t>(line_start);

        if (!has_prev) {
            prev_key = key;
            prev_start = off;
            has_prev = true;
        }
        else if (key != prev_key) {
            entries.push_back(TmpEntry{ prev_key, prev_start, off });
            prev_key = key;
            prev_start = off;
        }
    }

    // 收尾
    fin.clear();
    fin.seekg(0, std::ios::end);
    uint64_t file_end = static_cast<uint64_t>(fin.tellg());
    if (has_prev) {
        entries.push_back(TmpEntry{ prev_key, prev_start, file_end });
    }

    // 关键：按 key 排序（确保可二分）
    std::sort(entries.begin(), entries.end(),
        [](const TmpEntry& a, const TmpEntry& b) { return a.key < b.key; });

    // 若同 key 因无序源数据出现多个区间，合并成一个大区间（保证能找到）
    std::vector<TmpEntry> merged;
    merged.reserve(entries.size());
    for (const auto& e : entries) {
        if (merged.empty() || merged.back().key != e.key) {
            merged.push_back(e);
        }
        else {
            merged.back().start = std::min(merged.back().start, e.start);
            merged.back().end = std::max(merged.back().end, e.end);
        }
    }

    std::ofstream fout(idx_path, std::ios::binary | std::ios::trunc);
    if (!fout) {
        std::cerr << "Failed to open index output: " << idx_path << "\n";
        return 1;
    }

    const char magic[8] = { 'P','Y','I','D','X','V','1','\0' };
    uint32_t count = static_cast<uint32_t>(merged.size());
    fout.write(magic, 8);
    fout.write(reinterpret_cast<const char*>(&count), sizeof(count));

    for (const auto& e : merged) {
        FixedRecord r = make_fixed_record(e);
        fout.write(reinterpret_cast<const char*>(&r), sizeof(r));
    }

    fout.flush();

    std::cout << "Index built successfully.\n";
    std::cout << "CSV lines(data): " << total_data_lines << "\n";
    std::cout << "Valid lines: " << valid_lines << "\n";
    std::cout << "Skipped lines: " << skipped_lines << "\n";
    std::cout << "Unique keys(merged): " << merged.size() << "\n";
    std::cout << "Index file: " << idx_path << "\n";
    return 0;
}