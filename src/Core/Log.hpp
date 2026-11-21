#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

namespace hawk {

enum class LogLevel { Info, Warn, Error };

struct LogEntry {
    LogLevel level;
    double seconds; // seconds since the mod first logged something
    std::string text;
};

// Small in-memory ring buffer shown in the dashboard's event log.
// Everything pushed here is also written to Geode's log with a
// "[HawkPath]" prefix through logInfo / logWarn / logError.
class EventLog {
public:
    static EventLog& get();

    void push(LogLevel level, std::string text);
    std::vector<LogEntry> recent(std::size_t count) const;
    std::size_t size() const { return m_entries.size(); }

private:
    static constexpr std::size_t kMaxEntries = 200;
    std::deque<LogEntry> m_entries;
};

void logInfo(std::string const& message);
void logWarn(std::string const& message);
void logError(std::string const& message);

} // namespace hawk
