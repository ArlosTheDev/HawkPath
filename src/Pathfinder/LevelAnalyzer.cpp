#include "LevelAnalyzer.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <limits>

using namespace geode::prelude;

namespace hawk {

LevelGeometry LevelAnalyzer::analyze(PlayLayer* layer) {
    LevelGeometry out;

    if (!layer) {
        out.note = "No PlayLayer available";
        return out;
    }
    if (!layer->m_objects) {
        out.note = "PlayLayer has no object list";
        return out;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    for (auto obj : CCArrayExt<GameObject*>(layer->m_objects)) {
        if (!obj) {
            continue;
        }

        ObstacleKind kind;
        if (obj->m_objectType == GameObjectType::Solid) {
            kind = ObstacleKind::Solid;
            ++out.solidCount;
        } else if (obj->m_objectType == GameObjectType::Hazard) {
            kind = ObstacleKind::Hazard;
            ++out.hazardCount;
        } else {
            ++out.otherObjectCount;
            continue;
        }

        CCRect const r = obj->getObjectRect(); // copy of the game's own rect
        Obstacle ob;
        ob.kind = kind;
        ob.rect = Rect{
            r.origin.x, r.origin.y, r.size.width, r.size.height
        };
        out.obstacles.push_back(ob);

        minX = std::min(minX, r.origin.x);
        minY = std::min(minY, r.origin.y);
        maxX = std::max(maxX, r.origin.x + r.size.width);
        maxY = std::max(maxY, r.origin.y + r.size.height);
    }

    if (out.obstacles.empty()) {
        out.note = "No solid or hazard objects found";
        return out;
    }

    out.minX = minX;
    out.minY = minY;
    out.maxX = maxX;
    out.maxY = maxY;
    out.valid = true;
    out.note = "Extracted bounding boxes only (portals, triggers, pads, orbs "
               "and hitbox shapes are not modelled)";
    return out;
}

} // namespace hawk
