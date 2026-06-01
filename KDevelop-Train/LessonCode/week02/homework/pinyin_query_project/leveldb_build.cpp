#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

static inline void trim(std::string& s) {
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
    trim(pinyin);
    trim(word);
    return !pinyin.empty();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: leveldb_build <dict.csv> <db_path>\n";
        return 1;
    }

    const std::string csv_path = argv[1];
    const std::string db_path  = argv[2];

    std::ifstream fin(csv_path, std::ios::binary);
    if (!fin) {
        std::cerr << "Failed to open csv: " << csv_path << "\n";
        return 1;
    }

    leveldb::DB* db = nullptr;
    leveldb::Options options;
    options.create_if_missing = true;
    options.error_if_exists = false;

    leveldb::Status st = leveldb::DB::Open(options, db_path, &db);
    if (!st.ok()) {
        std::cerr << "Open leveldb failed: " << st.ToString() << "\n";
        return 1;
    }

    std::string line;
    // skip header
    std::getline(fin, line);

    size_t total = 0, valid = 0;
    leveldb::WriteBatch batch;
    const size_t BATCH_LIMIT = 5000;
    size_t batch_cnt = 0;

    while (std::getline(fin, line)) {
        ++total;
        trim(line);
        if (line.empty()) continue;

        std::string py, wd;
        if (!split_csv_line(line, py, wd)) continue;
        ++valid;

        std::string old_val;
        st = db->Get(leveldb::ReadOptions(), py, &old_val);

        std::string new_val;
        if (st.ok()) {
            // 用 '\n' 拼接多个词，查询端再按行拆
            new_val = old_val;
            new_val.push_back('\n');
            new_val += wd;
        } else {
            new_val = wd;
        }

        batch.Put(py, new_val);
        ++batch_cnt;

        if (batch_cnt >= BATCH_LIMIT) {
            st = db->Write(leveldb::WriteOptions(), &batch);
            if (!st.ok()) {
                std::cerr << "Batch write failed: " << st.ToString() << "\n";
                delete db;
                return 1;
            }
            batch.Clear();
            batch_cnt = 0;
        }
    }

    if (batch_cnt > 0) {
        st = db->Write(leveldb::WriteOptions(), &batch);
        if (!st.ok()) {
            std::cerr << "Final batch write failed: " << st.ToString() << "\n";
            delete db;
            return 1;
        }
    }

    delete db;

    std::cout << "LevelDB build done.\n";
    std::cout << "CSV total lines: " << total << "\n";
    std::cout << "Valid lines: " << valid << "\n";
    std::cout << "DB path: " << db_path << "\n";
    return 0;
}