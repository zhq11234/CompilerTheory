#include"irgen.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>

// 复用时间函数
static std::string getCurrentISO8601Time() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
#if defined(_MSC_VER)
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

static std::string escapeJSON(const std::string& s) {
    std::string result;
    for (char c : s) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else result += c;
    }
    return result;
}

void writeJSON(const std::string& filename, const std::vector<Quadruple>& quads) {
    std::ofstream out(filename);
    if (!out.is_open()) return;

    out << "{\n";
    out << "  \"source\": \"source.src\",\n";
    out << "  \"timestamp\": \"" << getCurrentISO8601Time() << "\",\n";
    out << "  \"quads\": [\n";
    for (size_t i = 0; i < quads.size(); ++i) {
        const auto& q = quads[i];
        out << "    {\"op\": \"" << escapeJSON(q.op) << "\", "
            << "\"arg1\": \"" << escapeJSON(q.arg1.empty() ? "-" : q.arg1) << "\", "
            << "\"arg2\": \"" << escapeJSON(q.arg2.empty() ? "-" : q.arg2) << "\", "
            << "\"result\": \"" << escapeJSON(q.result.empty() ? "-" : q.result) << "\"}";
        if (i != quads.size() - 1) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}