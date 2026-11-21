#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <vector>

// HawkPath control center. All positions are derived from the popup size,
// which itself is derived from the real logical window size.
class HawkPathPopup : public geode::Popup {
public:
    // Opens the dashboard unless one is already open.
    static void open();

    ~HawkPathPopup();

protected:
    // A label together with the limits it must fit inside.
    struct FitLabel {
        cocos2d::CCLabelBMFont* label = nullptr;
        float maxWidth = 0.f;
        float maxHeight = 0.f;
        float maxScale = 0.f;
        bool truncate = false;
    };

    static HawkPathPopup* create();
    bool initPopup();

    // layout helpers
    cocos2d::CCNode* makeCard(
        cocos2d::CCNode* parent, float x, float y, float w, float h,
        char const* title, float& titleHeightOut
    );
    FitLabel addRow(
        cocos2d::CCNode* card, cocos2d::CCSize size, float titleHeight,
        int index, int total, char const* key
    );
    FitLabel makeFitLabel(
        cocos2d::CCNode* parent, char const* font, cocos2d::CCPoint pos,
        cocos2d::CCPoint anchor, float maxW, float maxH, float maxScale,
        bool truncate
    );
    static void setFit(
        FitLabel& fit, std::string const& text, cocos2d::ccColor3B color
    );
    CCMenuItemSpriteExtra* addButton(
        cocos2d::CCMenu* menu, char const* text, char const* texture,
        cocos2d::CCPoint center, cocos2d::CCSize cell, cocos2d::SEL_MenuHandler handler
    );

    void refreshUI();
    void onTick(float dt);

    // callbacks
    void onStartPathfinder(cocos2d::CCObject*);
    void onStopPathfinder(cocos2d::CCObject*);
    void onToggleSafeMode(cocos2d::CCObject*);
    void onEmergencyStop(cocos2d::CCObject*);
    void onResetEmergencyStop(cocos2d::CCObject*);
    void onRefreshState(cocos2d::CCObject*);

    static inline HawkPathPopup* s_instance = nullptr;

    // header
    FitLabel m_headerStatus;

    // system overview
    FitLabel m_ovGame;
    FitLabel m_ovSafe;
    FitLabel m_ovStop;
    FitLabel m_ovPath;
    FitLabel m_ovSession;

    // session card
    FitLabel m_sessState;
    FitLabel m_sessDuration;
    FitLabel m_sessMessage;

    // pathfinder card
    FitLabel m_pfState;
    FitLabel m_pfModel;
    FitLabel m_pfExec;
    FitLabel m_pfSearch;

    // event log
    std::vector<FitLabel> m_logLines;
    FitLabel m_logHidden;
    bool m_showLog = true;
};
