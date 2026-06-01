// disk_thread_sync.cpp
// 模拟磁盘空间受限下的下载线程（A）与处理线程（B）协作
// 使用 std::thread, std::mutex, std::condition_variable 实现同步

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <chrono>

// 全局共享状态
std::mutex g_mutex;
std::condition_variable g_cond_var;
int g_disk_capacity = 0;          // 当前已用磁盘空间
const int MAX_DISK_CAPACITY = 3;  // 最大容量

// 用于安全输出的互斥锁
std::mutex g_print_mutex;

void safe_print(const std::string& msg) {
    std::lock_guard<std::mutex> lock(g_print_mutex);
    std::cout << msg << std::endl;
}

// 线程 A：下载文件（生产者）
void download_file() {
    int file_id = 1;
    while (true) {
        std::unique_lock<std::mutex> lock(g_mutex);
        // 等待：只有当磁盘未满时才下载
        g_cond_var.wait(lock, []() {
            return g_disk_capacity < MAX_DISK_CAPACITY;
            });

        // 下载一个文件
        g_disk_capacity++;
        std::ostringstream ss;
        ss << "[Download] File " << file_id++ << " downloaded. Disk used: "
            << g_disk_capacity << "/" << MAX_DISK_CAPACITY;
        safe_print(ss.str());

        if (g_disk_capacity == MAX_DISK_CAPACITY) {
            safe_print("[Download] Disk full! Pausing download, waking processor...");
        }

        lock.unlock();
        g_cond_var.notify_all(); // 唤醒处理线程
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟下载耗时
    }
}

// 线程 B：处理并删除文件（消费者）
void process_files() {
    while (true) {
        std::unique_lock<std::mutex> lock(g_mutex);
        // 等待：只有当磁盘有文件时才处理
        g_cond_var.wait(lock, []() {
            return g_disk_capacity > 0;
            });

        // 处理并删除一个文件
        g_disk_capacity--;
        std::ostringstream ss;
        ss << "[Process] File processed and deleted. Disk used: "
            << g_disk_capacity << "/" << MAX_DISK_CAPACITY;
        safe_print(ss.str());

        if (g_disk_capacity == 0) {
            safe_print("[Process] Disk empty! Pausing processor, waking downloader...");
        }

        lock.unlock();
        g_cond_var.notify_all(); // 唤醒下载线程
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟处理耗时
    }
}

// 主函数
int main() {
    safe_print("=== Starting Disk Synchronization Demo ===");
    safe_print("Max disk capacity: " + std::to_string(MAX_DISK_CAPACITY));
    safe_print("Starting download and processing threads...\n");

    std::thread downloader(download_file);
    std::thread processor(process_files);

    // 运行 20 秒后自动退出（避免无限运行）
    std::this_thread::sleep_for(std::chrono::seconds(20));

    safe_print("\n[Main] Time's up. Terminating threads...");
    // 注意：本例未实现优雅退出（如 atomic flag），仅用于演示
    // 实际项目中应使用标志位控制循环退出

    downloader.detach();  // 或 join，但需配合退出机制
    processor.detach();

    return 0;
}