#pragma once

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace hawk {

enum class SessionState { Idle, Preparing, Running, Paused, Stopped, Failed };

class SessionManager {
public:
    static SessionManager& get();

    // Runs safety checks, then the pathfinder. Returns true only if the
    // session reached Running.
    bool start();

    // Always cancels automated input first, then ends the session.
    void stop(std::string const& reason);

    void pause();
    bool resume();

    // Per-frame upkeep (cheap). Stops the session if safety is lost.
    void tick();

    SessionState state() const { return m_state; }
    bool isActive() const {
        return m_state == SessionState::Preparing ||
               m_state == SessionState::Running ||
               m_state == SessionState::Paused;
    }
    double durationSeconds() const;
    std::string const& lastMessage() const { return m_lastMessage; }
    std::vector<std::string> const& events() const { return m_events; }
    std::size_t startCount() const { return m_startCount; }

    static char const* stateName(SessionState state);

private:
    SessionManager() = default;

    void addEvent(std::string const& text);
    void finish(SessionState endState, std::string const& message);

    SessionState m_state = SessionState::Idle;
    std::string m_lastMessage = "No session yet";
    std::vector<std::string> m_events;
    std::size_t m_startCount = 0;
    std::chrono::steady_clock::time_point m_start{};
    std::chrono::steady_clock::time_point m_end{};
    bool m_hasStart = false;
    bool m_hasEnd = false;
};

} // namespace hawk
