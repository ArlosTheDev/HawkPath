#include "PathfinderEngine.hpp"

#include "../Core/Log.hpp"
#include "../Core/SafetyManager.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace hawk {

PathfinderEngine& PathfinderEngine::get() {
    static PathfinderEngine instance;
    return instance;
}

PathfinderEngine::PathfinderEngine()
    : m_model(std::make_unique<UncalibratedPhysicsModel>()) {}

char const* PathfinderEngine::stateName(PathfinderState state) {
    switch (state) {
        case PathfinderState::Idle:       return "Idle";
        case PathfinderState::Analyzing:  return "Analyzing";
        case PathfinderState::Searching:  return "Searching";
        case PathfinderState::Validating: return "Validating";
        case PathfinderState::RouteReady: return "Route ready";
        case PathfinderState::Failed:     return "Failed";
    }
    return "Unknown";
}

void PathfinderEngine::setState(PathfinderState next) {
    if (m_state != next) {
        logInfo(std::string("Pathfinder: ") + stateName(m_state) + " -> " +
                stateName(next));
        m_state = next;
    }
}

void PathfinderEngine::fail(std::string const& why) {
    m_status = why;
    setState(PathfinderState::Failed);
    logWarn("Pathfinder failed: " + why);
}

void PathfinderEngine::setModel(std::unique_ptr<IPhysicsModel> model) {
    if (model) {
        m_model = std::move(model);
        logInfo(std::string("Physics model set: ") + m_model->name());
    }
}

void PathfinderEngine::stop() {
    if (m_state != PathfinderState::Idle) {
        logInfo("Pathfinder stopped");
    }
    m_route = Route{};
    m_report = ValidationReport{};
    m_status = "Idle";
    m_state = PathfinderState::Idle;
}

bool PathfinderEngine::start(PlayLayer* layer) {
    m_route = Route{};
    m_report = ValidationReport{};

    auto decision = SafetyManager::get().check(
        SafetyAction::StartPathfinder, "PathfinderEngine::start"
    );
    if (!decision.allowed) {
        m_status = "Blocked: " + decision.text;
        m_state = PathfinderState::Idle;
        return false;
    }
    if (!layer || !layer->m_player1) {
        fail("No active player to plan from");
        return false;
    }

    // 1. Analyze
    setState(PathfinderState::Analyzing);
    m_geometry = LevelAnalyzer::analyze(layer);
    if (!m_geometry.valid) {
        fail("Level analysis failed: " + m_geometry.note);
        return false;
    }
    logInfo("Level analysis: " + std::to_string(m_geometry.solidCount) +
            " solid, " + std::to_string(m_geometry.hazardCount) +
            " hazard, " + std::to_string(m_geometry.otherObjectCount) +
            " unmodelled objects");

    // 2. Search
    setState(PathfinderState::Searching);
    PlayerState start;
    start.x = layer->m_player1->getPositionX();
    start.y = layer->m_player1->getPositionY();
    start.frame = 0;

    SearchResult const result =
        PathSearch::search(*m_model, m_geometry, start, m_config);
    if (!result.found) {
        fail(result.message);
        return false;
    }

    // 3. Validate
    setState(PathfinderState::Validating);
    m_report = RouteValidator::validate(
        result.route, *m_model, m_geometry, start
    );
    if (!m_report.valid) {
        fail(m_report.summary);
        return false;
    }

    m_route = result.route;
    m_status = m_report.summary;
    setState(PathfinderState::RouteReady);
    return true;
}

} // namespace hawk
