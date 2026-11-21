#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "Core/Log.hpp"
#include "Core/Runtime.hpp"
#include "Core/SafetyManager.hpp"
#include "UI/HawkPathPopup.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    hawk::initialize();
}

// ---------------------------------------------------------------------------
// Main menu launcher button
// ---------------------------------------------------------------------------
class $modify(HawkPathMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        // Never add a second launcher to the same MenuLayer.
        if (this->getChildByID("hawkpath-menu"_spr)) {
            return true;
        }

        auto const winSize = CCDirector::sharedDirector()->getWinSize();

        auto sprite = ButtonSprite::create("HAWKPATH");
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(HawkPathMenuLayer::onHawkPath)
        );
        button->setID("hawkpath-button"_spr);

        // Size the button relative to the window, not a fixed pixel size.
        auto const cs = button->getContentSize();
        if (cs.width > 0.f) {
            button->setScale(std::min(0.8f, winSize.width * 0.14f / cs.width));
        }

        auto menu = CCMenu::create();
        menu->setID("hawkpath-menu"_spr);
        menu->setPosition(ccp(0.f, 0.f));
        menu->addChild(button);

        float const halfW = cs.width * button->getScale() * 0.5f;
        button->setPosition(ccp(
            winSize.width - halfW - winSize.width * 0.02f,
            winSize.height * 0.5f
        ));

        this->addChild(menu, 100);
        hawk::logInfo("Main menu launcher added");
        return true;
    }

    void onHawkPath(CCObject*) {
        HawkPathPopup::open();
    }
};

// ---------------------------------------------------------------------------
// Gameplay hooks: attempt locking + per-frame upkeep
// ---------------------------------------------------------------------------
class $modify(HawkPlayLayer, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        hawk::SafetyManager::get().notifyAttemptStarted();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        hawk::SafetyManager::get().lockAttempt("player died");
        PlayLayer::destroyPlayer(player, object);
    }

    void levelComplete() {
        hawk::SafetyManager::get().lockAttempt("level completed");
        PlayLayer::levelComplete();
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        hawk::tick(this);
    }
};
