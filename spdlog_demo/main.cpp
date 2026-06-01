#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

int main()
{
    auto logger = spdlog::basic_logger_mt("file_logger", "text_log.txt");
    logger->info("Hello C++");
}