#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>

static inline void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}

static bool parse_pinyin(const std::string& line, std::string& pinyin) {
    auto pos = line.find(',');
    if (pos == std::string::npos) return false;
    pinyin = line.substr(0, pos);
    trim(pinyin);
    return !pinyin.empty();
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: make_queries <dict.csv> <out_100.txt> <out_1000.txt>\n";
        return 1;
    }

    const std::string csv_path = argv[1];
    const std::string out100 = argv[2];
    const std::string out1000 = argv[3];

    std::ifstream fin(csv_path, std::ios::binary);
    if (!fin) {
        std::cerr << "Failed to open csv: " << csv_path << "\n";
        return 1;
    }

    std::string line;
    std::getline(fin, line); // skip header

    std::unordered_set<std::string> uniq;
    uniq.reserve(300000);

    size_t total = 0;
    while (std::getline(fin, line)) {
        ++total;
        trim(line);
        if (line.empty()) continue;

        std::string py;
        if (!parse_pinyin(line, py)) continue;
        uniq.insert(py);
    }

    std::vector<std::string> keys(uniq.begin(), uniq.end());
    if (keys.empty()) {
        std::cerr << "No valid pinyin keys found.\n";
        return 1;
    }

    // 固定随机种子，便于复现
    std::mt19937 rng(42);
    std::shuffle(keys.begin(), keys.end(), rng);

    size_t n100 = std::min<size_t>(100, keys.size());
    size_t n1000 = std::min<size_t>(1000, keys.size());

    {
        std::ofstream f100(out100, std::ios::binary | std::ios::trunc);
        for (size_t i = 0; i < n100; ++i) f100 << keys[i] << "\n";
    }

    {
        std::ofstream f1000(out1000, std::ios::binary | std::ios::trunc);
        for (size_t i = 0; i < n1000; ++i) f1000 << keys[i] << "\n";
    }

    std::cout << "Done.\n";
    std::cout << "Total CSV lines: " << total << "\n";
    std::cout << "Unique pinyin: " << keys.size() << "\n";
    std::cout << "Generated: " << out100 << " (" << n100 << ")\n";
    std::cout << "Generated: " << out1000 << " (" << n1000 << ")\n";
    return 0;
}