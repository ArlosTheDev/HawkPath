#pragma once

#include <cstdint>
#include <vector>

namespace hawk {

// One button transition at a given simulation frame (frame 0 = attempt start).
struct InputEvent {
    std::uint32_t frame = 0;
    bool down = false;
};

struct Route {
    std::vector<InputEvent> events;
    std::uint32_t totalFrames = 0;

    bool empty() const { return events.empty(); }
};

} // namespace hawk
