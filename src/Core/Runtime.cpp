#include "Runtime.hpp"

#include "Log.hpp"
#include "SafetyManager.hpp"
#include "SessionManager.hpp"
#include "../Input/InputController.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace hawk {

namespace {
    bool s_initialized = false;
    unsigned s_frameCounter = 0;
}

void initialize() {
    if (s_initialized) {
        return;
    }
    s_initialized = true;

    // Anything that must stop immediately (emergency stop, Safe Mode off)
    // goes through this listener.
    SafetyManager::get().addStopListener([](std::string const& reason) {
        SessionManager::get().stop(reason);
    });

    logInfo("Initialized (Safe Mode ON, Emergency Stop clear, attempt locked)");
    logInfo("Input execution is " +
            std::string(InputController::kExecutionEnabled ? "ENABLED" : "DISABLED") +
            "; physics model is uncalibrated by default");
}

void tick(PlayLayer* layer) {
    // Input first so a safety loss cancels it before anything else.
    InputController::get().tick(layer);
    SessionManager::get().tick();

    // Keep the attempt lock / game state fresh without logging per frame.
    if (++s_frameCounter % 15 == 0) {
        SafetyManager::get().refreshState();
    }
}

} // namespace hawk
