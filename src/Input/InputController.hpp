#pragma once

#include "../Pathfinder/Route.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

class PlayLayer;

namespace hawk {

// The only component allowed to send input. Input is sent through the game's
// own GJBaseGameLayer::handleButton, never by injecting OS-level events.
//
// EXPERIMENTAL: execution is compiled disabled (kExecutionEnabled). The frame
// counter here advances once per PlayLayer::postUpdate call, which has NOT been
// verified to line up with the simulation step, so route timing is unproven.
class InputController {
public:
    static constexpr bool kExecutionEnabled = false;

    static InputController& get();

    bool start(Route const& route);
    void pause();
    bool resume();
    void cancel(std::string const& reason);

    // Called once per frame from the PlayLayer hook.
    void tick(PlayLayer* layer);

    bool isActive() const { return m_active; }
    bool isPaused() const { return m_paused; }

private:
    InputController() = default;

    void releaseIfHeld();

    Route m_route;
    std::size_t m_nextEvent = 0;
    std::uint32_t m_frame = 0;
    bool m_active = false;
    bool m_paused = false;
    bool m_holding = false;
};

} // namespace hawk
