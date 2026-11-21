#pragma once

#include "GameState.hpp"

#include <chrono>
#include <functional>
#include <string>
#include <vector>

namespace hawk {

// Every automated action HawkPath can take must be requested through
// SafetyManager::check(). There is no other permission path.
enum class SafetyAction {
    StartSession,
    StartPathfinder,
    ExecuteInput
};

enum class BlockReason {
    None,
    EmergencyStop,
    SafeModeDisabled,
    GameStateUnknown,
    GameStateUnverified,
    NotNonSaving,
    NotInPractice,
    AttemptLocked
};

struct SafetyDecision {
    bool allowed = false;
    BlockReason reason = BlockReason::GameStateUnknown;
    std::string text;
};

// Fail-closed permission gate.
//
// What this class DOES guarantee: HawkPath will not start a session, run the
// pathfinder or send input unless (a) no emergency stop is active, (b) Safe
// Mode is on, (c) the game state was read from real game objects, (d) the
// game reports Practice Mode, and (for input) (e) the current attempt was
// unlocked by a verified new attempt.
//
// What it does NOT do: it does not intercept the game's own save logic.
// "Non-saving" here means "the game reports Practice Mode", nothing more.
class SafetyManager {
public:
    static SafetyManager& get();

    // ---- state -------------------------------------------------------
    bool isSafeMode() const { return m_safeMode; }
    bool isEmergencyStopped() const { return m_emergencyStop; }
    bool isAttemptLocked() const { return m_attemptLocked; }

    // ---- controls ----------------------------------------------------
    // Safe Mode off blocks all automation (it never clears Emergency Stop).
    void setSafeMode(bool enabled);

    // Immediately blocks everything and tells registered listeners
    // (session, input controller) to stop.
    void activateEmergencyStop(std::string const& reason);

    // Clears the stop flag only. Does not restart anything, and the attempt
    // stays locked until a verified new attempt.
    bool resetEmergencyStop();

    // Called from game hooks (death, level complete).
    void lockAttempt(std::string const& reason);

    // Called from the resetLevel hook. The lock is only released later, in
    // refreshState(), once the game state verifies as a live Practice attempt.
    void notifyAttemptStarted();

    // ---- state -------------------------------------------------------
    GameStateSnapshot const& refreshState();
    GameStateSnapshot const& lastState() const { return m_state; }

    // ---- permission --------------------------------------------------
    SafetyDecision check(SafetyAction action, std::string const& requester);

    // Short status for the dashboard header.
    std::string overallStatus();

    // Listeners are invoked (in order) when automation must stop immediately.
    using StopListener = std::function<void(std::string const& reason)>;
    void addStopListener(StopListener listener);

    static char const* actionName(SafetyAction action);
    static char const* reasonText(BlockReason reason);

private:
    SafetyManager() = default;

    BlockReason evaluate(SafetyAction action, GameStateSnapshot const& state) const;
    void notifyStop(std::string const& reason);

    // Conservative defaults. None of these are loaded from saved settings.
    bool m_safeMode = true;
    bool m_emergencyStop = false;
    bool m_attemptLocked = true;
    bool m_awaitingNewAttempt = false;

    GameStateSnapshot m_state;
    std::vector<StopListener> m_listeners;

    // Throttle for repeated identical denial logs.
    bool m_hasLastDeny = false;
    SafetyAction m_lastDenyAction = SafetyAction::StartSession;
    BlockReason m_lastDenyReason = BlockReason::None;
    std::chrono::steady_clock::time_point m_lastDenyTime{};
};

} // namespace hawk
