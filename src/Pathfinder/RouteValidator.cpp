#include "RouteValidator.hpp"

#include <cstddef>

namespace hawk {

char const* RouteValidator::confidenceText(Confidence c) {
    switch (c) {
        case Confidence::None:          return "None";
        case Confidence::SimulatedOnly: return "Simulated only (unverified in-game)";
    }
    return "None";
}

ValidationReport RouteValidator::validate(
    Route const& route,
    IPhysicsModel const& model,
    LevelGeometry const& level,
    PlayerState const& start
) {
    ValidationReport report;
    report.limitations.push_back(model.limitations());
    report.limitations.push_back(
        "Validation only proves the route works in HawkPath's own simulator, "
        "not in Geometry Dash itself.");

    if (route.empty()) {
        report.summary = "Rejected: route has no inputs";
        return report;
    }
    if (!model.isCalibrated()) {
        report.summary = "Rejected: physics model is not calibrated";
        return report;
    }

    PlayerState state = start;
    bool hold = false;
    std::size_t nextEvent = 0;

    for (std::uint32_t frame = 0; frame < route.totalFrames + 1; ++frame) {
        while (nextEvent < route.events.size() &&
               route.events[nextEvent].frame <= frame) {
            hold = route.events[nextEvent].down;
            ++nextEvent;
        }

        StepResult const r = model.step(state, hold, level);
        if (r == StepResult::Unavailable) {
            report.summary = "Rejected: model cannot simulate";
            return report;
        }
        if (r == StepResult::Dead) {
            report.summary = "Rejected: player dies at frame " +
                             std::to_string(frame);
            return report;
        }
        if (r == StepResult::Finished) {
            report.valid = true;
            report.confidence = Confidence::SimulatedOnly;
            report.summary = "Route completes in simulation at frame " +
                             std::to_string(frame);
            return report;
        }
    }

    report.summary = "Rejected: route ends before the level is completed";
    return report;
}

} // namespace hawk
