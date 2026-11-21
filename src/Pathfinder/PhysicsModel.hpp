#pragma once

#include "LevelAnalyzer.hpp"

#include <cstdint>

namespace hawk {

enum class GameMode { Cube, Ship, Ball, Ufo, Wave, Robot, Spider, Swing };

struct PlayerState {
    double x = 0.0;
    double y = 0.0;
    double yVelocity = 0.0;
    bool gravityFlipped = false;
    bool onGround = false;
    GameMode mode = GameMode::Cube;
    std::uint32_t frame = 0;
};

enum class StepResult {
    Alive,
    Dead,
    Finished,
    Unavailable // the model cannot simulate (e.g. not calibrated)
};

// Extension point for physics. A real implementation must be derived from
// measured Geometry Dash 2.2 behaviour, then compared against the game
// before isCalibrated() may return true. HawkPath ships no invented
// constants: the only built-in model is UncalibratedPhysicsModel.
class IPhysicsModel {
public:
    virtual ~IPhysicsModel() = default;

    virtual char const* name() const = 0;
    virtual bool isCalibrated() const = 0;
    virtual char const* limitations() const = 0;

    // Advances `state` by exactly one simulation frame.
    virtual StepResult step(
        PlayerState& state, bool holdingJump, LevelGeometry const& level
    ) const = 0;
};

class UncalibratedPhysicsModel final : public IPhysicsModel {
public:
    char const* name() const override { return "Uncalibrated (placeholder)"; }
    bool isCalibrated() const override { return false; }
    char const* limitations() const override {
        return "No Geometry Dash 2.2 physics are implemented. "
               "Gravity, jump, speed portals and game modes are not simulated.";
    }
    StepResult step(PlayerState&, bool, LevelGeometry const&) const override {
        return StepResult::Unavailable;
    }
};

} // namespace hawk
