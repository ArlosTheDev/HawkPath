#include "PathSearch.hpp"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <vector>

namespace hawk {

namespace {

struct Node {
    PlayerState state;
    int parent = -1;
    bool hold = false;
};

std::uint64_t quantizedKey(PlayerState const& s) {
    auto q = [](double v, double scale) {
        return static_cast<std::int64_t>(std::llround(v * scale));
    };
    std::uint64_t h = 1469598103934665603ull;
    auto mix = [&](std::uint64_t v) {
        h ^= v;
        h *= 1099511628211ull;
    };
    mix(static_cast<std::uint64_t>(q(s.x, 4.0)));
    mix(static_cast<std::uint64_t>(q(s.y, 4.0)));
    mix(static_cast<std::uint64_t>(q(s.yVelocity, 8.0)));
    mix(s.gravityFlipped ? 1u : 0u);
    mix(s.onGround ? 1u : 0u);
    mix(static_cast<std::uint64_t>(s.mode));
    return h;
}

Route buildRoute(std::vector<Node> const& nodes, int goalIndex) {
    std::vector<bool> holds;
    for (int i = goalIndex; i > 0; i = nodes[i].parent) {
        holds.push_back(nodes[i].hold);
    }
    std::reverse(holds.begin(), holds.end());

    Route route;
    route.totalFrames = static_cast<std::uint32_t>(holds.size());
    bool previous = false;
    for (std::uint32_t f = 0; f < holds.size(); ++f) {
        if (holds[f] != previous) {
            route.events.push_back(InputEvent{f, holds[f]});
            previous = holds[f];
        }
    }
    return route;
}

} // namespace

SearchResult PathSearch::search(
    IPhysicsModel const& model,
    LevelGeometry const& level,
    PlayerState const& start,
    SearchConfig const& config
) {
    SearchResult result;

    if (!model.isCalibrated()) {
        result.status = SearchStatus::ModelUncalibrated;
        result.message = std::string("Physics model '") + model.name() +
                         "' is not calibrated; search was not run";
        return result;
    }

    std::vector<Node> nodes;
    nodes.reserve(std::min<std::size_t>(config.maxNodes, 1u << 16) + 1);
    nodes.push_back(Node{start, -1, false});

    std::vector<int> frontier{0};

    for (std::uint32_t frame = 0; frame < config.maxFrames; ++frame) {
        std::vector<int> next;
        std::unordered_set<std::uint64_t> seen;

        for (int parentIndex : frontier) {
            for (bool hold : {false, true}) {
                if (nodes.size() >= config.maxNodes) {
                    result.status = SearchStatus::BudgetExceeded;
                    result.message = "Node budget reached before a route was found";
                    return result;
                }

                PlayerState state = nodes[parentIndex].state; // copy
                StepResult const r = model.step(state, hold, level);
                ++result.nodesExpanded;

                if (r == StepResult::Unavailable) {
                    result.status = SearchStatus::ModelUnavailable;
                    result.message = "Physics model reported it cannot simulate";
                    return result;
                }
                if (r == StepResult::Dead) {
                    continue;
                }

                state.frame = nodes[parentIndex].state.frame + 1;
                nodes.push_back(Node{state, parentIndex, hold});
                int const index = static_cast<int>(nodes.size()) - 1;

                if (r == StepResult::Finished) {
                    result.status = SearchStatus::Found;
                    result.found = true;
                    result.route = buildRoute(nodes, index);
                    result.message = "Route found by simulation";
                    return result;
                }

                if (seen.insert(quantizedKey(state)).second) {
                    next.push_back(index);
                }
            }
        }

        if (next.empty()) {
            result.status = SearchStatus::Exhausted;
            result.message = "All simulated branches died";
            return result;
        }

        std::sort(next.begin(), next.end(), [&](int a, int b) {
            return nodes[a].state.x > nodes[b].state.x;
        });
        if (next.size() > config.beamWidth) {
            next.resize(config.beamWidth);
        }
        frontier = std::move(next);
    }

    result.status = SearchStatus::Exhausted;
    result.message = "Frame limit reached without finishing";
    return result;
}

} // namespace hawk
