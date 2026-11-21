
#pragma once

#include <Geode/Geode.hpp>

class SafetyManager {
public:
    enum class BlockReason {
        None,
        EmergencyStop,
        SafeModeDisabled,
        AttemptLocked,
        GameStateUnverified,
        SavingModeDetected,
        PathfinderNotRunning
    };

private:
    bool m_safeMode = true;
    bool m_emergencyStop = false;
    bool m_attemptLocked = false;

    bool m_gameStateVerified = false;
    bool m_nonSavingMode = false;

    bool m_pathfinderRunning = false;

public:
    // ---------------------------------------------
    // STATE
    // ---------------------------------------------

    bool isSafeModeEnabled() const {
        return m_safeMode;
    }

    bool isEmergencyStopped() const {
        return m_emergencyStop;
    }

    bool isAttemptLocked() const {
        return m_attemptLocked;
    }

    bool isPathfinderRunning() const {
        return m_pathfinderRunning;
    }

    bool isGameStateVerified() const {
        return m_gameStateVerified;
    }

    // ---------------------------------------------
    // SAFE MODE
    // ---------------------------------------------

    void setSafeMode(bool enabled) {
        m_safeMode = enabled;

        // Disabling Safe Mode immediately prevents
        // further pathfinder execution.
        if (!enabled) {
            m_pathfinderRunning = false;
        }
    }

    // ---------------------------------------------
    // GAME STATE VALIDATION
    // ---------------------------------------------

    // A future trusted game-state checker must call
    // this after verifying the current attempt.
    void setGameStateVerified(
        bool verified,
        bool nonSavingMode
    ) {
        m_gameStateVerified = verified;
        m_nonSavingMode = nonSavingMode;

        if (!verified || !nonSavingMode) {
            m_pathfinderRunning = false;
        }
    }

    // ---------------------------------------------
    // ATTEMPT LOCK
    // ---------------------------------------------

    void lockAttempt() {
        m_attemptLocked = true;
        m_pathfinderRunning = false;
    }

    // Only call when a genuinely new attempt begins
    // and its game state has been checked.
    bool beginNewAttempt() {
        if (m_emergencyStop || m_pathfinderRunning) {
            return false;
        }

        m_attemptLocked = false;
        return true;
    }

    // ---------------------------------------------
    // EMERGENCY STOP
    // ---------------------------------------------

    void activateEmergencyStop() {
        m_emergencyStop = true;
        m_pathfinderRunning = false;
    }

    bool resetEmergencyStop() {
        // Cannot reset while the pathfinder is running.
        if (m_pathfinderRunning) {
            return false;
        }

        m_emergencyStop = false;
        return true;
    }

    // ---------------------------------------------
    // PERMISSION CHECKS
    // ---------------------------------------------

    BlockReason getBlockReason() const {
        if (m_emergencyStop) {
            return BlockReason::EmergencyStop;
        }

        if (!m_safeMode) {
            return BlockReason::SafeModeDisabled;
        }

        if (m_attemptLocked) {
            return BlockReason::AttemptLocked;
        }

        if (!m_gameStateVerified) {
            return BlockReason::GameStateUnverified;
        }

        if (!m_nonSavingMode) {
            return BlockReason::SavingModeDetected;
        }

        return BlockReason::None;
    }

    bool canStartPathfinder() const {
        return getBlockReason() == BlockReason::None;
    }

    bool canExecuteActions() const {
        return m_pathfinderRunning &&
               canStartPathfinder();
    }

    // The pathfinder must call this before starting.
    bool startPathfinder() {
        if (!canStartPathfinder()) {
            return false;
        }

        m_pathfinderRunning = true;
        return true;
    }

    void stopPathfinder() {
        m_pathfinderRunning = false;
    }

    // ---------------------------------------------
    // STATUS TEXT
    // ---------------------------------------------

    const char* getBlockReasonText() const {
        switch (getBlockReason()) {
            case BlockReason::EmergencyStop:
                return "EMERGENCY STOP ACTIVE";

            case BlockReason::SafeModeDisabled:
                return "SAFE MODE DISABLED";

            case BlockReason::AttemptLocked:
                return "ATTEMPT LOCKED";

            case BlockReason::GameStateUnverified:
                return "WAITING FOR GAME CHECK";

            case BlockReason::SavingModeDetected:
                return "UNSAFE GAME STATE";

            default:
                return "SAFETY CHECKS PASSED";
        }
    }
};

// One shared safety manager for the mod.
inline SafetyManager g_safetyManager;
