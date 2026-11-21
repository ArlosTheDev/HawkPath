
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

// =====================================================
// HAWKPATH CONTROL CENTER
// =====================================================

class HawkPathPopup : public geode::Popup {
protected:
    bool m_safeMode = true;
    bool m_emergencyStop = false;

    CCLabelBMFont* m_safeModeStatus = nullptr;
    CCLabelBMFont* m_stopStatus = nullptr;

    bool init() {
        if (!Popup::init(380.f, 270.f)) {
            return false;
        }

        auto size = m_mainLayer->getContentSize();

        const float centerX = size.width * 0.5f;

        // ---------------------------------------------
        // COLORS
        // ---------------------------------------------

        const ccColor3B white = ccc3(240, 240, 255);
        const ccColor3B muted = ccc3(145, 150, 175);
        const ccColor3B purple = ccc3(160, 100, 255);

        // ---------------------------------------------
        // HEADER
        // ---------------------------------------------

        auto header = CCLabelBMFont::create(
            "HAWKPATH",
            "bigFont.fnt"
        );

        header->setScale(0.75f);
        header->setColor(purple);

        header->setPosition(
            ccp(centerX, size.height - 32.f)
        );

        m_mainLayer->addChild(header);

        auto subtitle = CCLabelBMFont::create(
            "// CONTROL CENTER",
            "goldFont.fnt"
        );

        subtitle->setScale(0.38f);
        subtitle->setColor(muted);

        subtitle->setPosition(
            ccp(centerX, size.height - 53.f)
        );

        m_mainLayer->addChild(subtitle);

        // ---------------------------------------------
        // STATUS CARD
        // ---------------------------------------------

        auto statusCard = CCLayerColor::create(
            ccc4(35, 32, 52, 255),
            size.width - 48.f,
            65.f
        );

        statusCard->setPosition(
            ccp(24.f, size.height - 135.f)
        );

        m_mainLayer->addChild(statusCard);

        auto statusTitle = CCLabelBMFont::create(
            "PATHFINDER STATUS",
            "goldFont.fnt"
        );

        statusTitle->setScale(0.42f);
        statusTitle->setColor(muted);

        statusTitle->setPosition(
            ccp(centerX, size.height - 86.f)
        );

        m_mainLayer->addChild(statusTitle);

        auto status = CCLabelBMFont::create(
            "NOT STARTED",
            "bigFont.fnt"
        );

        status->setScale(0.48f);
        status->setColor(white);

        status->setPosition(
            ccp(centerX, size.height - 112.f)
        );

        m_mainLayer->addChild(status);

        // ---------------------------------------------
        // CONTROL CARD
        // ---------------------------------------------

        auto controlCard = CCLayerColor::create(
            ccc4(35, 32, 52, 255),
            size.width - 48.f,
            95.f
        );

        controlCard->setPosition(
            ccp(24.f, 25.f)
        );

        m_mainLayer->addChild(controlCard);

        auto controlTitle = CCLabelBMFont::create(
            "SYSTEM CONTROLS",
            "goldFont.fnt"
        );

        controlTitle->setScale(0.42f);
        controlTitle->setColor(muted);

        controlTitle->setPosition(
            ccp(centerX, 101.f)
        );

        m_mainLayer->addChild(controlTitle);

        // ---------------------------------------------
        // SAFE MODE STATUS
        // ---------------------------------------------

        m_safeModeStatus = CCLabelBMFont::create(
            "SAFE MODE: ON",
            "bigFont.fnt"
        );

        m_safeModeStatus->setScale(0.42f);
        m_safeModeStatus->setColor(ccc3(100, 230, 150));

        m_safeModeStatus->setPosition(
            ccp(centerX, 82.f)
        );

        m_mainLayer->addChild(m_safeModeStatus);

        // ---------------------------------------------
        // EMERGENCY STOP STATUS
        // ---------------------------------------------

        m_stopStatus = CCLabelBMFont::create(
            "EMERGENCY STOP: READY",
            "bigFont.fnt"
        );

        m_stopStatus->setScale(0.36f);
        m_stopStatus->setColor(muted);

        m_stopStatus->setPosition(
            ccp(centerX, 65.f)
        );

        m_mainLayer->addChild(m_stopStatus);

        // ---------------------------------------------
        // BUTTONS
        // ---------------------------------------------

        auto menu = CCMenu::create();

        auto safeButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("TOGGLE SAFE MODE"),
            this,
            menu_selector(HawkPathPopup::onToggleSafeMode)
        );

        auto stopButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("EMERGENCY STOP"),
            this,
            menu_selector(HawkPathPopup::onEmergencyStop)
        );

        safeButton->setScale(0.65f);
        stopButton->setScale(0.65f);

        safeButton->setPosition(ccp(-82.f, 0.f));
        stopButton->setPosition(ccp(82.f, 0.f));

        menu->addChild(safeButton);
        menu->addChild(stopButton);

        menu->setPosition(
            ccp(centerX, 43.f)
        );

        m_mainLayer->addChild(menu);

        return true;
    }

    // ---------------------------------------------
    // SAFE MODE CALLBACK
    // ---------------------------------------------

    void onToggleSafeMode(CCObject*) {
        m_safeMode = !m_safeMode;

        m_safeModeStatus->setString(
            m_safeMode ? "SAFE MODE: ON" : "SAFE MODE: OFF"
        );

        m_safeModeStatus->setColor(
            m_safeMode
                ? ccc3(100, 230, 150)
                : ccc3(255, 190, 80)
        );
    }

    // ---------------------------------------------
    // EMERGENCY STOP CALLBACK
    // ---------------------------------------------

    void onEmergencyStop(CCObject*) {
        m_emergencyStop = true;

        m_stopStatus->setString(
            "EMERGENCY STOP: ACTIVE"
        );

        m_stopStatus->setColor(
            ccc3(255, 90, 90)
        );
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
// MAIN MENU INTEGRATION
// =====================================================

class $modify(HawkPathMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        // Get actual game UI dimensions
        auto winSize =
            CCDirector::sharedDirector()->getWinSize();

        // Create HawkPath launcher
        auto button = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("HAWKPATH"),
            this,
            menu_selector(HawkPathMenuLayer::onHawkPath)
        );

        button->setScale(0.8f);

        // Create menu
        auto menu = CCMenu::create();

        menu->setPosition(ccp(0.f, 0.f));
        menu->addChild(button);

        // Resolution-aware positioning
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
