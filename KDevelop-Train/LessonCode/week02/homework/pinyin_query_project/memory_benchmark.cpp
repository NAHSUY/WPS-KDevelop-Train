#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <chrono>

using namespace std;

static inline void trim_cr(string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
}

static bool parse_csv_line(const string& line, string& pinyin, string& word) {
    auto pos = line.find(',');
    if (pos == string::npos) return false;
    pinyin = line.substr(0, pos);
    word   = line.substr(pos + 1);
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
        getline(fin, line); // skip header

        index_.reserve(700000);
        size_t total = 0, valid = 0;

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
        cout << "Memory index build done.\n";
        cout << "CSV total lines: " << total << " valid lines: " << valid << "\n";
        cout << "Unique pinyin: " << index_.size() << "\n";
        cout << "Build time(ms): " << ms << "\n";
        return true;
    }

    size_t find_count(const string& key) const {
        auto it = index_.find(key);
        if (it == index_.end()) return 0;
        return it->second.size();
    }

private:
    unordered_map<string, vector<string>> index_;
};

static bool load_queries(const string& path, vector<string>& queries) {
    ifstream fin(path, ios::binary);
    if (!fin) return false;
    string line;
    while (getline(fin, line)) {
        trim_cr(line);
        if (!line.empty()) queries.push_back(line);
    }
    return !queries.empty();
}

static long long percentile_us(vector<long long> v, double p) {
    if (v.empty()) return 0;
    sort(v.begin(), v.end());
    size_t idx = static_cast<size_t>(p * (v.size() - 1));
    return v[idx];
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: memory_benchmark <dict.csv> <queries.txt> <result.csv>\n";
        return 1;
    }

    const string csv_path    = argv[1];
    const string queries_txt = argv[2];
    const string result_csv  = argv[3];

    MemoryIndex index;
    if (!index.load(csv_path)) return 1;

    vector<string> queries;
    if (!load_queries(queries_txt, queries)) {
        cerr << "Failed to load queries: " << queries_txt << "\n";
        return 1;
    }

    vector<long long> lat_us;
    lat_us.reserve(queries.size());

    size_t hit = 0, miss = 0;
    auto t_all_1 = chrono::high_resolution_clock::now();

    for (const auto& q : queries) {
        auto t1 = chrono::high_resolution_clock::now();
        size_t cnt = index.find_count(q);
        auto t2 = chrono::high_resolution_clock::now();

        long long us = chrono::duration_cast<chrono::microseconds>(t2 - t1).count();
        lat_us.push_back(us);

        if (cnt > 0) ++hit;
        else ++miss;
    }

    auto t_all_2 = chrono::high_resolution_clock::now();
    auto total_us = chrono::duration_cast<chrono::microseconds>(t_all_2 - t_all_1).count();

    double avg = lat_us.empty() ? 0.0 : (double)total_us / (double)lat_us.size();
    long long p50 = percentile_us(lat_us, 0.50);
    long long p95 = percentile_us(lat_us, 0.95);
    long long p99 = percentile_us(lat_us, 0.99);

    ofstream fout(result_csv, ios::binary | ios::trunc);
    if (!fout) {
        cerr << "Failed to open result file: " << result_csv << "\n";
        return 1;
    }

    fout << "queries,hit,miss,avg_us,p50_us,p95_us,p99_us\n";
    fout << queries.size() << "," << hit << "," << miss << ","
         << avg << "," << p50 << "," << p95 << "," << p99 << "\n";

    cout << "Benchmark done.\n";
    cout << "queries: " << queries.size() << " hit: " << hit << " miss: " << miss << "\n";
    cout << "avg(us): " << avg << " p50: " << p50 << " p95: " << p95 << " p99: " << p99 << "\n";
    cout << "result: " << result_csv << "\n";
    return 0;
}