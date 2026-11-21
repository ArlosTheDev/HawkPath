#pragma once

#include "PhysicsModel.hpp"
#include "Route.hpp"

#include <string>
#include <vector>

namespace hawk {

enum class Confidence {
    None,         // nothing could be validated
    SimulatedOnly // passed OUR simulator; never checked against the real game
};

struct ValidationReport {
    bool valid = false;
    Confidence confidence = Confidence::None;
    std::string summary = "Not validated";
    std::vector<std::string> limitations;
};

class RouteValidator {
public:
    // Replays the route frame by frame through the model.
    static ValidationReport validate(
        Route const& route,
        IPhysicsModel const& model,
        LevelGeometry const& level,
        PlayerState const& start
    );

    static char const* confidenceText(Confidence c);
};

} // namespace hawk
