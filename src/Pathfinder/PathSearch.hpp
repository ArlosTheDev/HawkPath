#pragma once

#include "PhysicsModel.hpp"
#include "Route.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace hawk {

struct SearchConfig {
    std::size_t maxNodes = 20000;     // hard cap on simulated steps stored
    std::uint32_t maxFrames = 1200;   // search depth in frames
    std::size_t beamWidth = 64;       // states kept per frame
};

enum class SearchStatus {
    Found,
    ModelUncalibrated,
    ModelUnavailable,
    Exhausted,
    BudgetExceeded
};

struct SearchResult {
    SearchStatus status = SearchStatus::Exhausted;
    bool found = false;
    Route route;
    std::size_t nodesExpanded = 0;
    std::string message;
};

// Frame-by-frame beam search over {hold, release}. Correctness depends
// entirely on the supplied IPhysicsModel.
class PathSearch {
public:
    static SearchResult search(
        IPhysicsModel const& model,
        LevelGeometry const& level,
        PlayerState const& start,
        SearchConfig const& config
    );
};

} // namespace hawk
