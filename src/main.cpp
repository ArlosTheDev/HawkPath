
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

// Temporary control states

class HawkPathPopup : public geode::Popup {
protected:
    bool m_safeMode = true;
    bool m_emergencyStop = false;

    CCLabelBMFont* m_safeModeLabel = nullptr;
    CCLabelBMFont* m_stopLabel = nullptr;

    bool init() {
        if (!Popup::init(260.f, 180.f)) {
            return false;
        }

        this->setTitle("HawkPath Control Panel");

        // Pathfinder status
        auto status = CCLabelBMFont::create(
            "Pathfinder: Not started",
            "bigFont.fnt"
        );
        status->setScale(0.38f);
        m_mainLayer->addChildAtPosition(
            status,
            Anchor::Center,
            ccp(0, 38)
        );

        // Safe Mode status
        m_safeModeLabel = CCLabelBMFont::create(
            "Safe Mode: ON",
            "bigFont.fnt"
        );
        m_safeModeLabel->setScale(0.42f);
        m_mainLayer->addChildAtPosition(
            m_safeModeLabel,
            Anchor::Center,
            ccp(0, 12)
        );

        // Emergency Stop status
        m_stopLabel = CCLabelBMFont::create(
            "Emergency Stop: READY",
            "bigFont.fnt"
        );
        m_stopLabel->setScale(0.38f);
        m_mainLayer->addChildAtPosition(
            m_stopLabel,
            Anchor::Center,
            ccp(0, -14)
        );

        // Controls menu
        auto menu = CCMenu::create();

        auto safeButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Toggle Safe Mode"),
            this,
            menu_selector(HawkPathPopup::onToggleSafeMode)
        );

        auto stopButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Emergency Stop"),
            this,
            menu_selector(HawkPathPopup::onEmergencyStop)
        );

        // Make buttons smaller
        safeButton->setScale(0.65f);
        stopButton->setScale(0.65f);

        // Position relative to the menu center
        safeButton->setPosition(ccp(0, -48));
        stopButton->setPosition(ccp(0, -78));

        menu->addChild(safeButton);
        menu->addChild(stopButton);

        // IMPORTANT: Center the menu inside the popup
        m_mainLayer->addChildAtPosition(
            menu,
            Anchor::Center
        );

        return true;
    }

    void onToggleSafeMode(CCObject*) {
        m_safeMode = !m_safeMode;

        m_safeModeLabel->setString(
            m_safeMode ? "Safe Mode: ON" : "Safe Mode: OFF"
        );
    }

    void onEmergencyStop(CCObject*) {
        m_emergencyStop = true;
        m_stopLabel->setString("Emergency Stop: ACTIVE");
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

// Main menu integration
class $modify(HawkPathMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto button = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("HawkPath"),
            this,
            menu_selector(HawkPathMenuLayer::onHawkPath)
        );

        auto menu = CCMenu::create();
        menu->setPosition(ccp(0, 0));
        menu->addChild(button);

        button->setPosition(
            ccp(winSize.width - 65.f, winSize.height * 0.5f)
        );

        this->addChild(menu, 100);

        log::info("HawkPath Control Panel initialized!");

        return true;
    }

    void onHawkPath(CCObject*) {
        auto popup = HawkPathPopup::create();

        if (popup) {
            popup->show();
        }
    }
};
