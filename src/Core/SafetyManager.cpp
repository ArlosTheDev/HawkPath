#include "SafetyManager.hpp"

#include "Log.hpp"


namespace hawk {

SafetyManager& SafetyManager::get() {
    static SafetyManager instance;
    return instance;
}

char const* SafetyManager::actionName(SafetyAction action) {
    switch (action) {
        case SafetyAction::StartSession:    return "start session";
        case SafetyAction::StartPathfinder: return "start pathfinder";
        case SafetyAction::ExecuteInput:    return "execute input";
    }
    return "unknown action";
}

char const* SafetyManager::reasonText(BlockReason reason) {
    switch (reason) {
        case BlockReason::None:                return "Allowed";
        case BlockReason::EmergencyStop:       return "Emergency Stop is active";
        case BlockReason::SafeModeDisabled:    return "Safe Mode is disabled";
        case BlockReason::GameStateUnknown:    return "Game state is unknown";
        case BlockReason::GameStateUnverified: return "Game state is unverified";
        case BlockReason::NotNonSaving:        return "Game does not report a non-saving state";
        case BlockReason::NotInPractice:       return "Not in a live Practice Mode attempt";
        case BlockReason::AttemptLocked:       return "Attempt is locked (waiting for verified new attempt)";
    }
    return "Blocked";
}

void SafetyManager::addStopListener(StopListener listener) {
    if (listener) {
        m_listeners.push_back(std::move(listener));
    }
}

void SafetyManager::notifyStop(std::string const& reason) {
    // Copy so a listener can't invalidate iteration.
    auto listeners = m_listeners;
    for (auto& listener : listeners) {
        listener(reason);
    }
}

void SafetyManager::setSafeMode(bool enabled) {
    if (m_safeMode == enabled) {
        return;
    }
    m_safeMode = enabled;
    logInfo(std::string("Safe Mode ") + (enabled ? "ENABLED" : "DISABLED") +
            (enabled ? "" : " - all automation is blocked"));
    if (!enabled) {
        notifyStop("Safe Mode disabled");
    }
    // Deliberately does not touch m_emergencyStop.
}

void SafetyManager::activateEmergencyStop(std::string const& reason) {
    bool const wasActive = m_emergencyStop;
    m_emergencyStop = true;
    m_attemptLocked = true;
    m_awaitingNewAttempt = false;
    if (!wasActive) {
        logWarn("EMERGENCY STOP activated: " + reason);
    }
    notifyStop("Emergency Stop: " + reason);
}

bool SafetyManager::resetEmergencyStop() {
    if (!m_emergencyStop) {
        return false;
    }
    m_emergencyStop = false;
    // A reset never restarts anything and never unlocks the attempt.
    m_attemptLocked = true;
    m_awaitingNewAttempt = false;
    logInfo("Emergency Stop reset (nothing restarted; attempt stays locked)");
    return true;
}

void SafetyManager::lockAttempt(std::string const& reason) {
    if (!m_attemptLocked) {
        logInfo("Attempt locked: " + reason);
    }
    m_attemptLocked = true;
    m_awaitingNewAttempt = false;
}

void SafetyManager::notifyAttemptStarted() {
    m_awaitingNewAttempt = true;
}

GameStateSnapshot const& SafetyManager::refreshState() {
    auto snap = GameStateDetector::detect();

    bool const changed = snap.kind != m_state.kind ||
                         snap.verified != m_state.verified ||
                         snap.nonSaving != m_state.nonSaving;
    if (changed) {
        logInfo(std::string("Game state: ") +
                GameStateDetector::name(m_state.kind) + " -> " +
                GameStateDetector::name(snap.kind) +
                " (verified=" + (snap.verified ? "yes" : "no") +
                ", non-saving=" + (snap.nonSaving ? "yes" : "no") + ")");
    }
    m_state = std::move(snap);

    bool const liveAttempt =
        m_state.verified && m_state.kind == GameStateKind::PracticeMode;

    // Fail closed: anything that is not a live practice attempt (or a pause
    // inside one) locks the attempt. This does not clear the awaiting flag,
    // so a reset that is still loading can unlock once it verifies.
    if (!liveAttempt && m_state.kind != GameStateKind::Paused) {
        m_attemptLocked = true;
    }

    // Only a verified new attempt (resetLevel hook + live practice state)
    // releases the lock.
    if (m_awaitingNewAttempt && liveAttempt && !m_emergencyStop) {
        m_attemptLocked = false;
        m_awaitingNewAttempt = false;
        logInfo("Verified new practice attempt: attempt unlocked");
    }

    return m_state;
}

BlockReason SafetyManager::evaluate(
    SafetyAction action, GameStateSnapshot const& state
) const {
    if (m_emergencyStop) return BlockReason::EmergencyStop;
    if (!m_safeMode) return BlockReason::SafeModeDisabled;
    if (state.kind == GameStateKind::Unknown) return BlockReason::GameStateUnknown;
    if (!state.verified) return BlockReason::GameStateUnverified;
    if (!state.nonSaving) return BlockReason::NotNonSaving;

    if (action == SafetyAction::ExecuteInput) {
        if (state.kind != GameStateKind::PracticeMode) {
            return BlockReason::NotInPractice;
        }
        if (m_attemptLocked) return BlockReason::AttemptLocked;
    } else {
        if (state.kind != GameStateKind::PracticeMode &&
            state.kind != GameStateKind::Paused) {
            return BlockReason::NotInPractice;
        }
    }
    return BlockReason::None;
}

SafetyDecision SafetyManager::check(
    SafetyAction action, std::string const& requester
) {
    auto const& state = refreshState();
    BlockReason const reason = evaluate(action, state);

    SafetyDecision decision;
    decision.reason = reason;
    decision.allowed = reason == BlockReason::None;
    decision.text = reasonText(reason);

    auto const now = std::chrono::steady_clock::now();
    if (!decision.allowed) {
        bool const repeated =
            m_hasLastDeny && m_lastDenyAction == action &&
            m_lastDenyReason == reason &&
            now - m_lastDenyTime < std::chrono::seconds(2);
        if (!repeated) {
            logWarn(std::string("DENIED ") + actionName(action) + " (" +
                    requester + "): " + decision.text);
            m_hasLastDeny = true;
            m_lastDenyAction = action;
            m_lastDenyReason = reason;
            m_lastDenyTime = now;
        }
    } else {
        m_hasLastDeny = false;
        // Input checks run every frame; only log the non-input grants.
        if (action != SafetyAction::ExecuteInput) {
            logInfo(std::string("ALLOWED ") + actionName(action) + " (" +
                    requester + ")");
        }
    }
    return decision;
}

std::string SafetyManager::overallStatus() {
    auto const& state = refreshState();
    if (m_emergencyStop) return "EMERGENCY STOP";
    if (!m_safeMode) return "SAFE MODE OFF - BLOCKED";
    BlockReason const reason = evaluate(SafetyAction::StartSession, state);
    if (reason == BlockReason::None) return "READY";
    return "BLOCKED";
}

} // namespace hawk
