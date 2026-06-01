#include <leveldb/db.h>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

static inline void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: leveldb_query <db_path> <pinyin> <output.csv>\n";
        return 1;
    }

    const std::string db_path = argv[1];
    std::string key = argv[2];
    const std::string out_csv = argv[3];
    trim(key);

    leveldb::DB* db = nullptr;
    leveldb::Options options;
    options.create_if_missing = false;

    leveldb::Status st = leveldb::DB::Open(options, db_path, &db);
    if (!st.ok()) {
        std::cerr << "Open DB failed: " << st.ToString() << "\n";
        return 1;
    }

    auto t1 = std::chrono::steady_clock::now();

    std::string val;
    st = db->Get(leveldb::ReadOptions(), key, &val);

    std::ofstream fout(out_csv, std::ios::binary | std::ios::trunc);
    fout << "pinyin,word\n";

    size_t hit = 0;
    if (st.ok()) {
        size_t start = 0;
        while (start <= val.size()) {
            size_t pos = val.find('\n', start);
            std::string word = (pos == std::string::npos) ? val.substr(start) : val.substr(start, pos - start);
            trim(word);
            if (!word.empty()) {
                fout << key << "," << word << "\n";
                ++hit;
            }
            if (pos == std::string::npos) break;
            start = pos + 1;
        }
    }

    auto t2 = std::chrono::steady_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    if (!st.ok()) std::cout << "Not found: " << key << "\n";
    else std::cout << "Found " << hit << " rows for: " << key << "\n";

    std::cout << "Output: " << out_csv << "\n";
    std::cout << "Query time: " << us << " us\n";

    delete db;
    return 0;
}