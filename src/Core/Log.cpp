#include "Log.hpp"

#include <Geode/Geode.hpp>

#include <chrono>
#include <utility>

namespace hawk {

namespace {
    double nowSeconds() {
        static auto const start = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(
            std::chrono::steady_clock::now() - start
        ).count();
    }
}

EventLog& EventLog::get() {
    static EventLog instance;
    return instance;
}

void EventLog::push(LogLevel level, std::string text) {
    m_entries.push_back(LogEntry{level, nowSeconds(), std::move(text)});
    while (m_entries.size() > kMaxEntries) {
        m_entries.pop_front();
    }
}

std::vector<LogEntry> EventLog::recent(std::size_t count) const {
    std::vector<LogEntry> out;
    std::size_t total = m_entries.size();
    std::size_t begin = total > count ? total - count : 0;
    out.reserve(total - begin);
    for (std::size_t i = begin; i < total; ++i) {
        out.push_back(m_entries[i]);
    }
    return out;
}

void logInfo(std::string const& message) {
    geode::log::info("[HawkPath] {}", message);
    EventLog::get().push(LogLevel::Info, message);
}

void logWarn(std::string const& message) {
    geode::log::warn("[HawkPath] {}", message);
    EventLog::get().push(LogLevel::Warn, message);
}

void logError(std::string const& message) {
    geode::log::error("[HawkPath] {}", message);
    EventLog::get().push(LogLevel::Error, message);
}

} // namespace hawk
