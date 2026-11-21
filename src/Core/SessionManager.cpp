#include "SessionManager.hpp"

#include "Log.hpp"
#include "SafetyManager.hpp"
#include "../Input/InputController.hpp"
#include "../Pathfinder/PathfinderEngine.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace hawk {

SessionManager& SessionManager::get() {
    static SessionManager instance;
    return instance;
}

char const* SessionManager::stateName(SessionState state) {
    switch (state) {
        case SessionState::Idle:      return "Idle";
        case SessionState::Preparing: return "Preparing";
        case SessionState::Running:   return "Running";
        case SessionState::Paused:    return "Paused";
        case SessionState::Stopped:   return "Stopped";
        case SessionState::Failed:    return "Failed";
    }
    return "Unknown";
}

void SessionManager::addEvent(std::string const& text) {
    m_events.push_back(text);
    constexpr std::size_t kMaxEvents = 50;
    if (m_events.size() > kMaxEvents) {
        m_events.erase(m_events.begin());
    }
    logInfo("Session: " + text);
}

double SessionManager::durationSeconds() const {
    if (!m_hasStart) {
        return 0.0;
    }
    auto const end = (isActive() || !m_hasEnd)
        ? std::chrono::steady_clock::now()
        : m_end;
    return std::chrono::duration<double>(end - m_start).count();
}

void SessionManager::finish(SessionState endState, std::string const& message) {
    m_state = endState;
    m_end = std::chrono::steady_clock::now();
    m_hasEnd = true;
    m_lastMessage = message;
    addEvent(std::string(stateName(endState)) + ": " + message);
}

bool SessionManager::start() {
    if (isActive()) {
        addEvent("Start ignored: a session is already active");
        return false;
    }

    auto decision = SafetyManager::get().check(
        SafetyAction::StartSession, "SessionManager::start"
    );
    if (!decision.allowed) {
        m_lastMessage = "Start denied: " + decision.text;
        addEvent(m_lastMessage);
        return false;
    }

    ++m_startCount;
    m_state = SessionState::Preparing;
    m_start = std::chrono::steady_clock::now();
    m_hasStart = true;
    m_hasEnd = false;
    m_lastMessage = "Preparing";
    addEvent("Preparing (running pathfinder)");

    auto& engine = PathfinderEngine::get();
    if (!engine.start(PlayLayer::get())) {
        finish(SessionState::Failed, engine.statusText());
        return false;
    }

    if (!InputController::get().start(engine.route())) {
        // Route exists but execution is disabled or was denied. Not a failure
        // of the pathfinder, and not a running session either.
        engine.stop();
        finish(SessionState::Stopped,
               "Route computed, but input execution is disabled (experimental)");
        return false;
    }

    m_state = SessionState::Running;
    m_lastMessage = "Running";
    addEvent("Running");
    return true;
}

void SessionManager::stop(std::string const& reason) {
    // Input is cancelled first and unconditionally.
    InputController::get().cancel(reason);
    PathfinderEngine::get().stop();

    if (isActive()) {
        finish(SessionState::Stopped, reason);
    }
}

void SessionManager::pause() {
    if (m_state != SessionState::Running) {
        return;
    }
    InputController::get().pause();
    m_state = SessionState::Paused;
    addEvent("Paused");
}

bool SessionManager::resume() {
    if (m_state != SessionState::Paused) {
        return false;
    }
    if (!InputController::get().resume()) {
        return false;
    }
    m_state = SessionState::Running;
    addEvent("Resumed");
    return true;
}

void SessionManager::tick() {
    if (m_state != SessionState::Running) {
        return;
    }
    if (!InputController::get().isActive()) {
        stop("input controller is no longer active");
    }
}

} // namespace hawk
