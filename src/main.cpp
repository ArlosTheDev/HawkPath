
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "SafetyManager.hpp"

using namespace geode::prelude;

// =====================================================
// HAWKPATH CONTROL CENTER
// =====================================================

class HawkPathPopup : public geode::Popup {
protected:
    CCLabelBMFont* m_safeModeStatus = nullptr;
    CCLabelBMFont* m_stopStatus = nullptr;
    CCLabelBMFont* m_readinessStatus = nullptr;

    void refreshUI() {
        // Safe Mode
        bool safe = g_safetyManager.isSafeModeEnabled();

        m_safeModeStatus->setString(
            safe ? "SAFE MODE: ON" : "SAFE MODE: OFF"
        );

        m_safeModeStatus->setColor(
            safe ? ccc3(100, 230, 150)
                 : ccc3(255, 190, 80)
        );

        // Emergency Stop
        bool stopped = g_safetyManager.isEmergencyStopped();

        m_stopStatus->setString(
            stopped
                ? "EMERGENCY STOP: ACTIVE"
                : "EMERGENCY STOP: READY"
        );

        m_stopStatus->setColor(
            stopped ? ccc3(255, 90, 90)
                    : ccc3(150, 155, 175)
        );

        // Pathfinder readiness
        m_readinessStatus->setString(
            g_safetyManager.getBlockReasonText()
        );

        m_readinessStatus->setColor(
            g_safetyManager.canStartPathfinder()
                ? ccc3(100, 230, 150)
                : ccc3(255, 190, 80)
        );
    }

    bool init() {
        if (!Popup::init(400.f, 310.f)) {
            return false;
        }

        auto size = m_mainLayer->getContentSize();
        float centerX = size.width * 0.5f;

        // ---------------------------------------------
        // HEADER
        // ---------------------------------------------

        auto header = CCLabelBMFont::create(
            "HAWKPATH",
            "bigFont.fnt"
        );

        header->setScale(0.75f);
        header->setColor(ccc3(170, 115, 255));
        header->setPosition(
            ccp(centerX, size.height - 34.f)
        );

        m_mainLayer->addChild(header);

        auto subtitle = CCLabelBMFont::create(
            "// CONTROL CENTER",
            "goldFont.fnt"
        );

        subtitle->setScale(0.38f);
        subtitle->setColor(ccc3(145, 150, 175));
        subtitle->setPosition(
            ccp(centerX, size.height - 56.f)
        );

        m_mainLayer->addChild(subtitle);

        // ---------------------------------------------
        // STATUS CARD
        // ---------------------------------------------

        auto statusCard = CCLayerColor::create(
            ccc4(35, 32, 52, 255),
            size.width - 48.f,
            115.f
        );

        statusCard->setPosition(
            ccp(24.f, size.height - 185.f)
        );

        m_mainLayer->addChild(statusCard);

        auto statusTitle = CCLabelBMFont::create(
            "SYSTEM STATUS",
            "goldFont.fnt"
        );

        statusTitle->setScale(0.43f);
        statusTitle->setColor(ccc3(145, 150, 175));
        statusTitle->setPosition(
            ccp(centerX, size.height - 88.f)
        );

        m_mainLayer->addChild(statusTitle);

        auto pathfinderStatus = CCLabelBMFont::create(
            "PATHFINDER: STOPPED",
            "bigFont.fnt"
        );

        pathfinderStatus->setScale(0.43f);
        pathfinderStatus->setColor(ccc3(230, 230, 245));
        pathfinderStatus->setPosition(
            ccp(centerX, size.height - 113.f)
        );

        m_mainLayer->addChild(pathfinderStatus);

        // ---------------------------------------------
        // SAFETY STATUS
        // ---------------------------------------------

        m_safeModeStatus = CCLabelBMFont::create(
            "",
            "bigFont.fnt"
        );

        m_safeModeStatus->setScale(0.42f);
        m_safeModeStatus->setPosition(
            ccp(centerX, size.height - 139.f)
        );

        m_mainLayer->addChild(m_safeModeStatus);

        m_stopStatus = CCLabelBMFont::create(
            "",
            "bigFont.fnt"
        );

        m_stopStatus->setScale(0.38f);
        m_stopStatus->setPosition(
            ccp(centerX, size.height - 160.f)
        );

        m_mainLayer->addChild(m_stopStatus);

        // ---------------------------------------------
        // READINESS CARD
        // ---------------------------------------------

        auto readinessCard = CCLayerColor::create(
            ccc4(35, 32, 52, 255),
            size.width - 48.f,
            42.f
        );

        readinessCard->setPosition(
            ccp(24.f, 78.f)
        );

        m_mainLayer->addChild(readinessCard);

        m_readinessStatus = CCLabelBMFont::create(
            "",
            "bigFont.fnt"
        );

        m_readinessStatus->setScale(0.38f);
        m_readinessStatus->setPosition(
            ccp(centerX, 99.f)
        );

        m_mainLayer->addChild(m_readinessStatus);

        // ---------------------------------------------
        // BUTTONS
        // ---------------------------------------------

        auto menu = CCMenu::create();

        auto safeButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("SAFE MODE"),
            this,
            menu_selector(HawkPathPopup::onToggleSafeMode)
        );

        auto stopButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("EMERGENCY STOP"),
            this,
            menu_selector(HawkPathPopup::onEmergencyStop)
        );

        auto resetButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("RESET STOP"),
            this,
            menu_selector(HawkPathPopup::onResetStop)
        );

        safeButton->setScale(0.58f);
        stopButton->setScale(0.58f);
        resetButton->setScale(0.58f);

        safeButton->setPosition(ccp(-95.f, 0.f));
        stopButton->setPosition(ccp(95.f, 0.f));
        resetButton->setPosition(ccp(0.f, -34.f));

        menu->addChild(safeButton);
        menu->addChild(stopButton);
        menu->addChild(resetButton);

        menu->setPosition(
            ccp(centerX, 55.f)
        );

        m_mainLayer->addChild(menu);

        refreshUI();

        return true;
    }

    // ---------------------------------------------
    // BUTTON CALLBACKS
    // ---------------------------------------------

    void onToggleSafeMode(CCObject*) {
        g_safetyManager.setSafeMode(
            !g_safetyManager.isSafeModeEnabled()
        );

        refreshUI();
    }

    void onEmergencyStop(CCObject*) {
        g_safetyManager.activateEmergencyStop();

        refreshUI();

        log::warn("HawkPath: Emergency Stop activated!");
    }

    void onResetStop(CCObject*) {
        if (!g_safetyManager.resetEmergencyStop()) {
            log::warn("HawkPath: Cannot reset while running!");
        } else {
            log::info("HawkPath: Emergency Stop reset.");
        }

        refreshUI();
    }

public:
    static HawkPathPopup* create() {
        auto ret = new HawkPathPopup();

        if (ret->init()) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};


// =====================================================
// GEOMETRY DASH MAIN MENU LAUNCHER
// =====================================================

class $modify(HawkPathMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        auto button = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("HAWKPATH"),
            this,
            menu_selector(HawkPathMenuLayer::onHawkPath)
        );

        button->setScale(0.8f);

        auto menu = CCMenu::create();
        menu->setPosition(ccp(0.f, 0.f));
        menu->addChild(button);

        button->setPosition(
            ccp(
                winSize.width - 75.f,
                winSize.height * 0.5f
            )
        );

        this->addChild(menu, 100);

        log::info("HawkPath Control Center initialized!");

        return true;
    }

    void onHawkPath(CCObject*) {
        auto popup = HawkPathPopup::create();

        if (popup) {
            popup->show();
        }
    }
};
