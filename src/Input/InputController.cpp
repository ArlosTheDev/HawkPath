#include "InputController.hpp"

#include "../Core/Log.hpp"
#include "../Core/SafetyManager.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace hawk {

InputController& InputController::get() {
    static InputController instance;
    return instance;
}

bool InputController::start(Route const& route) {
    if (!kExecutionEnabled) {
        logWarn("Input execution is disabled (experimental feature); no input will be sent");
        return false;
    }
    if (route.empty()) {
        logWarn("Input start rejected: empty route");
        return false;
    }
    auto decision = SafetyManager::get().check(
        SafetyAction::ExecuteInput, "InputController::start"
    );
    if (!decision.allowed) {
        return false;
    }

    m_route = route;
    m_nextEvent = 0;
    m_frame = 0;
    m_holding = false;
    m_paused = false;
    m_active = true;
    logInfo("Input execution started (" + std::to_string(route.events.size()) +
            " events)");
    return true;
}

void InputController::releaseIfHeld() {
    // Releasing can only REDUCE input, so it is allowed even when the safety
    // gate is closed. It is the one deliberate exception to check().
    if (!m_holding) {
        return;
    }
    if (auto layer = PlayLayer::get()) {
        layer->handleButton(false, 1, true);
    }
    m_holding = false;
}

void InputController::pause() {
    if (!m_active || m_paused) {
        return;
    }
    releaseIfHeld();
    m_paused = true;
    logInfo("Input execution paused");
}

bool InputController::resume() {
    if (!m_active || !m_paused) {
        return false;
    }
    auto decision = SafetyManager::get().check(
        SafetyAction::ExecuteInput, "InputController::resume"
    );
    if (!decision.allowed) {
        return false;
    }
    m_paused = false;
    logInfo("Input execution resumed");
    return true;
}

void InputController::cancel(std::string const& reason) {
    bool const wasActive = m_active;
    releaseIfHeld();
    m_active = false;
    m_paused = false;
    m_nextEvent = 0;
    m_frame = 0;
    m_route = Route{};
    if (wasActive) {
        logInfo("Input execution cancelled: " + reason);
    }
}

void InputController::tick(PlayLayer* layer) {
    if (!m_active || m_paused) {
        return;
    }
    if (!layer) {
        cancel("PlayLayer is gone");
        return;
    }

    auto decision = SafetyManager::get().check(
        SafetyAction::ExecuteInput, "InputController::tick"
    );
    if (!decision.allowed) {
        cancel("safety denied: " + decision.text);
        return;
    }

    while (m_nextEvent < m_route.events.size() &&
           m_route.events[m_nextEvent].frame <= m_frame) {
        bool const down = m_route.events[m_nextEvent].down;
        layer->handleButton(down, 1, true);
        m_holding = down;
        ++m_nextEvent;
    }

    ++m_frame;
    if (m_frame > m_route.totalFrames) {
        cancel("route finished");
    }
}

} // namespace hawk
