#include <leveldb/db.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>

static inline void trim(std::string& s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    if (i) s.erase(0, i);
}

static bool load_queries(const std::string& txt, std::vector<std::string>& qs) {
    std::ifstream fin(txt, std::ios::binary);
    if (!fin) return false;
    std::string line;
    while (std::getline(fin, line)) {
        trim(line);
        if (!line.empty()) qs.push_back(line);
    }
    return true;
}

static double percentile_us(std::vector<long long> v, double p) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    size_t idx = static_cast<size_t>(p * (v.size() - 1));
    return static_cast<double>(v[idx]);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: benchmark <db_path> <queries.txt> <result.csv>\n";
        return 1;
    }

    const std::string db_path = argv[1];
    const std::string queries_txt = argv[2];
    const std::string result_csv = argv[3];

    std::vector<std::string> queries;
    if (!load_queries(queries_txt, queries)) {
        std::cerr << "Failed to load queries: " << queries_txt << "\n";
        return 1;
    }
    if (queries.empty()) {
        std::cerr << "queries.txt is empty.\n";
        return 1;
    }

    leveldb::DB* db = nullptr;
    leveldb::Options options;
    options.create_if_missing = false;
    leveldb::Status st = leveldb::DB::Open(options, db_path, &db);
    if (!st.ok()) {
        std::cerr << "Open DB failed: " << st.ToString() << "\n";
        return 1;
    }

    std::vector<long long> costs;
    costs.reserve(queries.size());

    size_t hit = 0, miss = 0;
    long long total_us = 0;

    for (const auto& q : queries) {
        auto t1 = std::chrono::steady_clock::now();
        std::string val;
        st = db->Get(leveldb::ReadOptions(), q, &val);
        auto t2 = std::chrono::steady_clock::now();

        long long us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
        costs.push_back(us);
        total_us += us;

        if (st.ok()) ++hit;
        else ++miss;
    }

    delete db;

    double avg = static_cast<double>(total_us) / static_cast<double>(queries.size());
    double p50 = percentile_us(costs, 0.50);
    double p95 = percentile_us(costs, 0.95);
    double p99 = percentile_us(costs, 0.99);

    std::ofstream fout(result_csv, std::ios::binary | std::ios::trunc);
    fout << "query_count,hit,miss,avg_us,p50_us,p95_us,p99_us,total_us\n";
    fout << queries.size() << ","
         << hit << ","
         << miss << ","
         << avg << ","
         << p50 << ","
         << p95 << ","
         << p99 << ","
         << total_us << "\n";

    std::cout << "Benchmark done.\n";
    std::cout << "queries: " << queries.size() << " hit: " << hit << " miss: " << miss << "\n";
    std::cout << "avg(us): " << avg << " p50: " << p50 << " p95: " << p95 << " p99: " << p99 << "\n";
    std::cout << "result: " << result_csv << "\n";

    return 0;
}