#include "GameState.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace hawk {

char const* GameStateDetector::name(GameStateKind kind) {
    switch (kind) {
        case GameStateKind::MainMenu:       return "Main Menu";
        case GameStateKind::LevelLoading:   return "Level Loading";
        case GameStateKind::ActiveGameplay: return "Active Gameplay";
        case GameStateKind::PracticeMode:   return "Practice Mode";
        case GameStateKind::Editor:         return "Editor";
        case GameStateKind::Paused:         return "Paused";
        case GameStateKind::Dead:           return "Dead";
        case GameStateKind::LevelCompleted: return "Level Completed";
        case GameStateKind::Unknown:        return "Unknown";
    }
    return "Unknown";
}

GameStateSnapshot GameStateDetector::detect() {
    GameStateSnapshot snap; // defaults: Unknown, unverified, saving-unsafe

    auto director = CCDirector::sharedDirector();
    auto scene = director ? director->getRunningScene() : nullptr;
    if (!scene) {
        snap.detail = "No running scene";
        return snap;
    }

    // Editor. LevelEditorLayer::get() is only non-null while an editor
    // instance exists.
    if (LevelEditorLayer::get()) {
        snap.kind = GameStateKind::Editor;
        snap.verified = true;
        snap.nonSaving = true; // editing does not submit level progress
        snap.detail = "LevelEditorLayer is active";
        return snap;
    }

    if (auto pl = PlayLayer::get()) {
        bool const practice = pl->m_isPracticeMode;

        // Pause: either the pause layer is on the scene or the layer says so.
        bool const pauseLayerShown =
            scene->getChildByType<PauseLayer>(0) != nullptr;
        if (pauseLayerShown || pl->m_isPaused) {
            snap.kind = GameStateKind::Paused;
            snap.verified = true;
            snap.nonSaving = practice;
            snap.detail = practice ? "Paused (practice)" : "Paused (not practice)";
            return snap;
        }

        if (pl->m_levelEndAnimationStarted) {
            snap.kind = GameStateKind::LevelCompleted;
            snap.verified = true;
            snap.nonSaving = practice;
            snap.detail = "Level end animation started";
            return snap;
        }

        if (!pl->m_player1) {
            snap.kind = GameStateKind::LevelLoading;
            snap.verified = true;
            snap.nonSaving = false; // not established yet
            snap.detail = "PlayLayer exists but player is not created";
            return snap;
        }

        if (pl->m_player1->m_isDead) {
            snap.kind = GameStateKind::Dead;
            snap.verified = true;
            snap.nonSaving = practice;
            snap.detail = "Player 1 is dead";
            return snap;
        }

        snap.verified = true;
        if (practice) {
            snap.kind = GameStateKind::PracticeMode;
            snap.nonSaving = true;
            snap.detail = "PlayLayer::m_isPracticeMode is true";
        } else {
            snap.kind = GameStateKind::ActiveGameplay;
            snap.nonSaving = false;
            snap.detail = "In a level with Practice Mode off";
        }
        return snap;
    }

    if (scene->getChildByType<MenuLayer>(0)) {
        snap.kind = GameStateKind::MainMenu;
        snap.verified = true;
        snap.nonSaving = false;
        snap.detail = "MenuLayer is on the scene";
        return snap;
    }

    if (scene->getChildByType<LoadingLayer>(0)) {
        snap.kind = GameStateKind::LevelLoading;
        snap.verified = true;
        snap.nonSaving = false;
        snap.detail = "LoadingLayer is on the scene";
        return snap;
    }

    snap.detail = "Scene not recognised";
    return snap; // Unknown / unverified
}

} // namespace hawk
