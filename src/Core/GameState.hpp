#pragma once

#include <string>

namespace hawk {

enum class GameStateKind {
    MainMenu,
    LevelLoading,
    ActiveGameplay,   // in a level, Practice Mode OFF (progress can be saved)
    PracticeMode,     // in a level, Practice Mode ON (non-saving)
    Editor,
    Paused,
    Dead,
    LevelCompleted,
    Unknown
};

struct GameStateSnapshot {
    GameStateKind kind = GameStateKind::Unknown;

    // True only when `kind` was derived from fields that the game itself
    // exposes (PlayLayer, LevelEditorLayer, MenuLayer, PauseLayer ...).
    bool verified = false;

    // True only when the game reports that this state does not save
    // progress (currently: Practice Mode, read from PlayLayer).
    // This describes the game's own flag; HawkPath does not add or
    // enforce any save protection beyond refusing to act when it is false.
    bool nonSaving = false;

    std::string detail;
};

class GameStateDetector {
public:
    // Reads the current game state. Safe to call at any time (UI thread).
    static GameStateSnapshot detect();

    static char const* name(GameStateKind kind);
};

} // namespace hawk
