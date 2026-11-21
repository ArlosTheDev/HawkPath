#pragma once

#include <cstddef>
#include <string>
#include <vector>

class PlayLayer;

namespace hawk {

struct Rect {
    float x = 0.f;      // left
    float y = 0.f;      // bottom
    float width = 0.f;
    float height = 0.f;
};

enum class ObstacleKind { Solid, Hazard };

struct Obstacle {
    Rect rect;
    ObstacleKind kind = ObstacleKind::Solid;
};

// Geometry extracted from the live level. It only contains what the game
// objects report (bounding boxes and the game's own object type); it does
// not interpret portals, triggers, pads, orbs, rotation, or hitbox shapes.
struct LevelGeometry {
    bool valid = false;
    std::string note;

    std::vector<Obstacle> obstacles;
    std::size_t solidCount = 0;
    std::size_t hazardCount = 0;
    std::size_t otherObjectCount = 0; // seen but NOT modelled
    float minX = 0.f;
    float maxX = 0.f;
    float minY = 0.f;
    float maxY = 0.f;
};

class LevelAnalyzer {
public:
    static LevelGeometry analyze(PlayLayer* layer);
};

} // namespace hawk
