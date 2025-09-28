#pragma once
#include <string>
#include <chrono>
#include <ctime>

struct Transaction {
    std::string timestamp;   // ISO 8601
    std::string type;        // DEPOSIT, WITHDRAW, TRANSFER_OUT, TRANSFER_IN
    double amount{0.0};
    double balance_after{0.0};
    std::string details;     // e.g., to/from user
};

inline std::string current_timestamp_iso() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    return std::string(buf);
}
