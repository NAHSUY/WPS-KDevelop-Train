#include <benchmark/benchmark.h>  
#include <chrono>  
#include <vector>  
#include <random>  
  
// ... matrix_multiply 和 generate_random_matrix 函数的实现与上面的代码相同 ... 
// 简单的矩阵乘法函数  
std::vector<std::vector<int>> matrix_multiply(const std::vector<std::vector<int>>& a, const std::vector<std::vector<int>>& b) {
    int rows = a.size();
    int cols = b[0].size();
    int inner_dim = a[0].size();
    std::vector<std::vector<int>> result(rows, std::vector<int>(cols, 0));

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            for (int k = 0; k < inner_dim; ++k) {
                result[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    return result;
}

// 生成一个填充了随机值的矩阵  
std::vector<std::vector<int>> generate_random_matrix(int rows, int cols) {
    std::vector<std::vector<int>> matrix(rows, std::vector<int>(cols, 0));
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-10, 10);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = dis(gen);
        }
    }

    return matrix;

  }

// 自定义计时器函数，模拟测量特定于应用程序的执行时间  
double custom_timer(const std::vector<std::vector<int>>& a, const std::vector<std::vector<int>>& b) {  
    auto start_time = std::chrono::high_resolution_clock::now();  
    auto result = matrix_multiply(a, b);  
    auto end_time = std::chrono::high_resolution_clock::now();  
    std::chrono::duration<double> elapsed = end_time - start_time;  
  
    return elapsed.count();  
}  
  
static void BM_MatrixMultiply_CustomTimer(benchmark::State& state) {  
    int rows = state.range(0);  
    int cols = state.range(1);  
    int inner_dim = state.range(2);  
  
    auto a = generate_random_matrix(rows, inner_dim);  
    auto b = generate_random_matrix(inner_dim, cols);  
  
    for (auto _ : state) {  
        state.SetIterationTime(custom_timer(a, b));  
    }  
}  
  
// 使用自定义计时器测试矩阵乘法函数，使用 1 到 8 的范围内的行、列和内部维度  
BENCHMARK(BM_MatrixMultiply_CustomTimer)->Ranges({{1, 8}, {1, 8}, {1, 8}})->UseManualTime();  
  
BENCHMARK_MAIN();  