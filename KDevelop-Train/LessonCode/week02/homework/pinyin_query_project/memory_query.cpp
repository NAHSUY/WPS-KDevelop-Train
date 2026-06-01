#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

using namespace std;

struct Row {
    string pinyin;
    string word;
};

static inline void trim_cr(string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

static bool parse_csv_line(const string& line, string& pinyin, string& word) {
    auto pos = line.find(',');
    if (pos == string::npos) return false;
    pinyin = line.substr(0, pos);
    word = line.substr(pos + 1);
    trim_cr(pinyin);
    trim_cr(word);
    return !pinyin.empty() && !word.empty();
}

class MemoryIndex {
public:
    bool load(const string& csv_path) {
        ifstream fin(csv_path, ios::binary);
        if (!fin) {
            cerr << "Failed to open csv: " << csv_path << "\n";
            return false;
        }

        string line;
        // 跳过表头
        getline(fin, line);

        size_t total = 0, valid = 0;
        index_.reserve(700000);

        auto t1 = chrono::high_resolution_clock::now();

        while (getline(fin, line)) {
            ++total;
            trim_cr(line);
            if (line.empty()) continue;

            string py, wd;
            if (!parse_csv_line(line, py, wd)) continue;

            index_[py].push_back(wd);
            ++valid;
        }

        auto t2 = chrono::high_resolution_clock::now();
        auto ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

        cout << "Load done.\n";
        cout << "Total lines: " << total << ", valid: " << valid << "\n";
        cout << "Unique pinyin keys: " << index_.size() << "\n";
        cout << "Build memory index time: " << ms << " ms\n";
        return true;
    }

    vector<Row> query(const string& pinyin) const {
        vector<Row> out;
        auto it = index_.find(pinyin);
        if (it == index_.end()) return out;
        out.reserve(it->second.size());
        for (const auto& w : it->second) out.push_back({pinyin, w});
        return out;
    }

private:
    unordered_map<string, vector<string>> index_;
};

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: memory_query <dict.csv> <pinyin> <out.csv>\n";
        return 1;
    }

    const string csv_path = argv[1];
    const string key = argv[2];
    const string out_csv = argv[3];

    MemoryIndex idx;
    if (!idx.load(csv_path)) return 1;

    auto t1 = chrono::high_resolution_clock::now();
    auto rows = idx.query(key);
    auto t2 = chrono::high_resolution_clock::now();
    auto us = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();

    ofstream fout(out_csv, ios::binary | ios::trunc);
    if (!fout) {
        cerr << "Failed to open output: " << out_csv << "\n";
        return 1;
    }

    fout << "pinyin,word\n";
    for (const auto& r : rows) {
        fout << r.pinyin << "," << r.word << "\n";
    }

    cout << "Found " << rows.size() << " rows for: " << key << "\n";
    cout << "Output: " << out_csv << "\n";
    cout << "Query time: " << us << " us\n";
    return 0;
}