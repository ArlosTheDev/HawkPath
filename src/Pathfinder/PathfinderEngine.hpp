#pragma once

#include "LevelAnalyzer.hpp"
#include "PathSearch.hpp"
#include "PhysicsModel.hpp"
#include "Route.hpp"
#include "RouteValidator.hpp"

#include <memory>
#include <string>

class PlayLayer;

namespace hawk {

enum class PathfinderState {
    Idle,
    Analyzing,
    Searching,
    Validating,
    RouteReady,
    Failed
};

// Orchestrates analyze -> search -> validate. Runs synchronously on the main
// thread, bounded by SearchConfig. Every start goes through SafetyManager.
class PathfinderEngine {
public:
    static PathfinderEngine& get();

    // Returns true only if a validated route is ready.
    bool start(PlayLayer* layer);
    void stop();

    PathfinderState state() const { return m_state; }
    std::string const& statusText() const { return m_status; }
    Route const& route() const { return m_route; }
    LevelGeometry const& geometry() const { return m_geometry; }
    ValidationReport const& report() const { return m_report; }
    IPhysicsModel const& model() const { return *m_model; }
    SearchConfig const& config() const { return m_config; }

    // Extension point: install a real, calibrated model.
    void setModel(std::unique_ptr<IPhysicsModel> model);

    static char const* stateName(PathfinderState state);

private:
    PathfinderEngine();

    void fail(std::string const& why);
    void setState(PathfinderState next);

    PathfinderState m_state = PathfinderState::Idle;
    std::string m_status = "Idle";
    std::unique_ptr<IPhysicsModel> m_model;
    SearchConfig m_config;
    LevelGeometry m_geometry;
    Route m_route;
    ValidationReport m_report;
};

} // namespace hawk
