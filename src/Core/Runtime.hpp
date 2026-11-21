#pragma once

class PlayLayer;

namespace hawk {

// One-time setup (called from $on_mod(Loaded)).
void initialize();

// Per-frame upkeep, called from the PlayLayer::postUpdate hook.
void tick(PlayLayer* layer);

} // namespace hawk
