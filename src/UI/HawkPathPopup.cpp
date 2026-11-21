#include "HawkPathPopup.hpp"

#include "../Core/GameState.hpp"
#include "../Core/Log.hpp"
#include "../Core/SafetyManager.hpp"
#include "../Core/SessionManager.hpp"
#include "../Input/InputController.hpp"
#include "../Pathfinder/PathfinderEngine.hpp"

#include <algorithm>
#include <cstdio>

using namespace geode::prelude;

namespace {
    // palette
    ccColor4B const kPanel   = ccc4(14, 11, 26, 255);
    ccColor4B const kBorder  = ccc4(78, 56, 134, 255);
    ccColor4B const kCard    = ccc4(27, 21, 46, 255);
    ccColor3B const kAccent  = ccc3(172, 120, 255);
    ccColor3B const kText    = ccc3(232, 228, 248);
    ccColor3B const kDim     = ccc3(150, 144, 184);
    ccColor3B const kGood    = ccc3(110, 230, 160);
    ccColor3B const kWarn    = ccc3(255, 190, 80);
    ccColor3B const kBad     = ccc3(255, 100, 110);

    std::string formatSeconds(double s) {
        char buf[32];
        int total = static_cast<int>(s);
        std::snprintf(buf, sizeof(buf), "%02d:%02d", total / 60, total % 60);
        return buf;
    }
}

HawkPathPopup::~HawkPathPopup() {
    if (s_instance == this) {
        s_instance = nullptr;
    }
    hawk::logInfo("Dashboard closed");
}

void HawkPathPopup::open() {
    if (s_instance) {
        hawk::logInfo("Dashboard already open; duplicate open ignored");
        return;
    }
    auto popup = create();
    if (!popup) {
        hawk::logError("Failed to create dashboard");
        return;
    }
    s_instance = popup;
    popup->show();
    hawk::logInfo("Dashboard opened");
}

HawkPathPopup* HawkPathPopup::create() {
    auto ret = new HawkPathPopup();
    if (ret->initPopup()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

// ---------------------------------------------------------------------------
// layout helpers
// ---------------------------------------------------------------------------

HawkPathPopup::FitLabel HawkPathPopup::makeFitLabel(
    CCNode* parent, char const* font, CCPoint pos, CCPoint anchor,
    float maxW, float maxH, float maxScale, bool truncate
) {
    FitLabel fit;
    fit.label = CCLabelBMFont::create("", font);
    fit.label->setAnchorPoint(anchor);
    fit.label->setPosition(pos);
    fit.maxWidth = maxW;
    fit.maxHeight = maxH;
    fit.maxScale = maxScale;
    fit.truncate = truncate;
    parent->addChild(fit.label);
    return fit;
}

void HawkPathPopup::setFit(
    FitLabel& fit, std::string const& text, ccColor3B color
) {
    if (!fit.label) {
        return;
    }
    std::string shown = text;
    if (shown.empty()) {
        shown = " ";
    }
    fit.label->setString(shown.c_str());
    fit.label->setColor(color);

    auto fitScale = [&]() {
        auto cs = fit.label->getContentSize();
        float scale = fit.maxScale;
        if (cs.height > 0.f) {
            scale = std::min(scale, fit.maxHeight / cs.height);
        }
        return scale;
    };

    float scale = fitScale();
    auto cs = fit.label->getContentSize();
    if (cs.width * scale > fit.maxWidth && cs.width > 0.f) {
        if (fit.truncate && shown.size() > 4) {
            float const ratio = fit.maxWidth / (cs.width * scale);
            std::size_t keep = static_cast<std::size_t>(shown.size() * ratio);
            keep = keep > 2 ? keep - 2 : 1;
            shown = shown.substr(0, keep) + "..";
            fit.label->setString(shown.c_str());
            scale = fitScale();
            cs = fit.label->getContentSize();
        }
        if (cs.width * scale > fit.maxWidth && cs.width > 0.f) {
            scale = fit.maxWidth / cs.width;
        }
    }
    fit.label->setScale(scale);
}

CCNode* HawkPathPopup::makeCard(
    CCNode* parent, float x, float y, float w, float h, char const* title,
    float& titleHeightOut
) {
    auto border = CCLayerColor::create(kBorder, w, h);
    border->setPosition(ccp(x, y));
    parent->addChild(border);

    auto fill = CCLayerColor::create(kCard, w - 2.f, h - 2.f);
    fill->setPosition(ccp(1.f, 1.f));
    border->addChild(fill);

    auto content = CCNode::create();
    content->setContentSize(CCSize(w, h));
    content->setAnchorPoint(ccp(0.f, 0.f));
    content->setPosition(ccp(0.f, 0.f));
    border->addChild(content, 1);

    float const pad = std::max(4.f, w * 0.04f);
    titleHeightOut = std::min(h * 0.24f, 18.f + h * 0.02f);

    auto accent = CCLayerColor::create(
        ccc4(kAccent.r, kAccent.g, kAccent.b, 255), 2.f, titleHeightOut * 0.6f
    );
    accent->setPosition(ccp(pad * 0.5f, h - titleHeightOut * 0.8f));
    content->addChild(accent);

    FitLabel t = makeFitLabel(
        content, "bigFont.fnt",
        ccp(pad + 2.f, h - titleHeightOut * 0.5f), ccp(0.f, 0.5f),
        w - pad * 2.f, titleHeightOut * 0.6f, 0.4f, false
    );
    setFit(t, title, kAccent);
    return content;
}

HawkPathPopup::FitLabel HawkPathPopup::addRow(
    CCNode* card, CCSize size, float titleHeight, int index, int total,
    char const* key
) {
    float const pad = std::max(4.f, size.width * 0.04f);
    float const areaH = size.height - titleHeight - pad * 0.5f;
    float const rowH = areaH / static_cast<float>(total);
    float const y = size.height - titleHeight - rowH * (index + 0.5f);

    FitLabel k = makeFitLabel(
        card, "bigFont.fnt", ccp(pad, y), ccp(0.f, 0.5f),
        size.width * 0.36f, rowH * 0.62f, 0.3f, false
    );
    setFit(k, key, kDim);

    return makeFitLabel(
        card, "bigFont.fnt", ccp(size.width - pad, y), ccp(1.f, 0.5f),
        size.width - pad * 2.f - size.width * 0.38f, rowH * 0.62f, 0.3f, true
    );
}

CCMenuItemSpriteExtra* HawkPathPopup::addButton(
    CCMenu* menu, char const* text, char const* texture, CCPoint center,
    CCSize cell, SEL_MenuHandler handler
) {
    auto sprite = ButtonSprite::create(text, "goldFont.fnt", texture, 0.8f);
    auto button = CCMenuItemSpriteExtra::create(sprite, this, handler);
    auto cs = button->getContentSize();
    if (cs.width > 0.f && cs.height > 0.f) {
        float const scale = std::min(
            cell.width * 0.92f / cs.width, cell.height * 0.84f / cs.height
        );
        button->setScale(std::min(scale, 1.f));
    }
    button->setPosition(center);
    menu->addChild(button);
    return button;
}

// ---------------------------------------------------------------------------
// construction
// ---------------------------------------------------------------------------

bool HawkPathPopup::initPopup() {
    // Derive the popup size from the real logical window size.
    auto const win = CCDirector::sharedDirector()->getWinSize();
    float const width = std::clamp(win.width - win.width * 0.12f, 300.f, 540.f);
    float const height = std::clamp(win.height - win.height * 0.08f, 200.f, 310.f);

    if (!Popup::init(width, height)) {
        return false;
    }

    CCSize const S = m_size;
    float const pad = std::max(6.f, S.width * 0.015f);

    // Dark panel covering the default GD popup art.
    auto panelHolder = CCNode::create();
    panelHolder->setContentSize(S);
    panelHolder->setAnchorPoint(ccp(0.f, 0.f));
    panelHolder->setPosition(ccp(0.f, 0.f));
    m_mainLayer->addChild(panelHolder, 1);

    auto panelBorder = CCLayerColor::create(kBorder, S.width - 4.f, S.height - 4.f);
    panelBorder->setPosition(ccp(2.f, 2.f));
    panelHolder->addChild(panelBorder);
    auto panel = CCLayerColor::create(kPanel, S.width - 6.f, S.height - 6.f);
    panel->setPosition(ccp(1.f, 1.f));
    panelBorder->addChild(panel);

    // Keep the built-in close button above our panel.
    if (m_buttonMenu) {
        m_buttonMenu->setZOrder(100);
    }

    // ---- header ---------------------------------------------------------
    float const headerH = S.height * 0.14f;
    float const headerY = S.height - pad - headerH * 0.5f;
    float x = pad * 2.5f;

    if (auto logo = CCSprite::create("logo.png"_spr)) {
        float const target = headerH * 0.9f;
        auto ls = logo->getContentSize();
        if (ls.height > 0.f) {
            logo->setScale(target / ls.height);
        }
        logo->setAnchorPoint(ccp(0.f, 0.5f));
        logo->setPosition(ccp(x, headerY));
        panelHolder->addChild(logo, 2);
        x += ls.width * logo->getScale() + pad;
    }

    auto title = CCLabelBMFont::create("HAWKPATH", "bigFont.fnt");
    title->setAnchorPoint(ccp(0.f, 0.5f));
    {
        auto cs = title->getContentSize();
        float scale = std::min(0.8f, headerH * 0.62f / std::max(1.f, cs.height));
        title->setScale(scale);
        title->setPosition(ccp(x, headerY + headerH * 0.12f));
        title->setColor(kAccent);
        panelHolder->addChild(title, 2);

        FitLabel ver = makeFitLabel(
            panelHolder, "chatFont.fnt",
            ccp(x, headerY - headerH * 0.28f), ccp(0.f, 0.5f),
            S.width * 0.3f, headerH * 0.34f, 0.5f, false
        );
        setFit(ver,
               "v" + std::string(Mod::get()->getVersion().toVString(false)) +
                   "  CONTROL CENTER",
               kDim);
        ver.label->setZOrder(2);
    }

    m_headerStatus = makeFitLabel(
        panelHolder, "bigFont.fnt",
        ccp(S.width - pad * 2.f, headerY), ccp(1.f, 0.5f),
        S.width * 0.36f, headerH * 0.5f, 0.45f, false
    );
    m_headerStatus.label->setZOrder(2);

    // ---- columns --------------------------------------------------------
    float const contentTop = S.height - pad - headerH - pad * 0.5f;
    float const contentBottom = pad * 1.5f;
    float const contentH = contentTop - contentBottom;
    float const colW = (S.width - pad * 3.f) / 2.f;
    float const leftX = pad * 1.5f;
    float const rightX = leftX + colW + pad;
    float const gap = pad * 0.7f;

    // Left column: overview + controls
    {
        float const overviewH = (contentH - gap) * 0.5f;
        float const controlsH = contentH - gap - overviewH;

        float titleH = 0.f;
        float const overviewY = contentTop - overviewH;
        auto overview = makeCard(
            panelHolder, leftX, overviewY, colW, overviewH, "SYSTEM OVERVIEW", titleH
        );
        CCSize const oSize(colW, overviewH);
        m_ovGame    = addRow(overview, oSize, titleH, 0, 5, "Game state");
        m_ovSafe    = addRow(overview, oSize, titleH, 1, 5, "Safe Mode");
        m_ovStop    = addRow(overview, oSize, titleH, 2, 5, "Emergency Stop");
        m_ovPath    = addRow(overview, oSize, titleH, 3, 5, "Pathfinder");
        m_ovSession = addRow(overview, oSize, titleH, 4, 5, "Session");

        float controlsTitleH = 0.f;
        float const controlsY = contentBottom;
        auto controls = makeCard(
            panelHolder, leftX, controlsY, colW, controlsH, "MAIN CONTROLS",
            controlsTitleH
        );
        auto menu = CCMenu::create();
        menu->setContentSize(CCSize(colW, controlsH));
        menu->setAnchorPoint(ccp(0.f, 0.f));
        menu->setPosition(ccp(0.f, 0.f));
        controls->addChild(menu);

        float const gridH = controlsH - controlsTitleH - gap * 0.5f;
        CCSize const cell(colW / 2.f, gridH / 3.f);
        auto cellCenter = [&](int col, int row) {
            return ccp(
                cell.width * (col + 0.5f),
                gridH - cell.height * (row + 0.5f)
            );
        };

        addButton(menu, "START", "GJ_button_01.png", cellCenter(0, 0), cell,
                  menu_selector(HawkPathPopup::onStartPathfinder));
        addButton(menu, "STOP", "GJ_button_04.png", cellCenter(1, 0), cell,
                  menu_selector(HawkPathPopup::onStopPathfinder));
        addButton(menu, "SAFE MODE", "GJ_button_02.png", cellCenter(0, 1), cell,
                  menu_selector(HawkPathPopup::onToggleSafeMode));
        addButton(menu, "E-STOP", "GJ_button_06.png", cellCenter(1, 1), cell,
                  menu_selector(HawkPathPopup::onEmergencyStop));
        addButton(menu, "RESET STOP", "GJ_button_04.png", cellCenter(0, 2), cell,
                  menu_selector(HawkPathPopup::onResetEmergencyStop));
        addButton(menu, "REFRESH", "GJ_button_02.png", cellCenter(1, 2), cell,
                  menu_selector(HawkPathPopup::onRefreshState));
    }

    // Right column: session + pathfinder + log
    {
        float const usable = contentH - gap * 2.f;
        float const sessionH = usable * 0.22f;
        float const pathH = usable * 0.30f;
        float const logH = usable - sessionH - pathH;

        float titleH = 0.f;
        float y = contentTop - sessionH;
        auto session = makeCard(
            panelHolder, rightX, y, colW, sessionH, "SESSION", titleH
        );
        CCSize const sSize(colW, sessionH);
        m_sessState    = addRow(session, sSize, titleH, 0, 3, "State");
        m_sessDuration = addRow(session, sSize, titleH, 1, 3, "Duration");
        m_sessMessage  = addRow(session, sSize, titleH, 2, 3, "Last");

        y -= gap + pathH;
        auto path = makeCard(
            panelHolder, rightX, y, colW, pathH, "PATHFINDER (EXPERIMENTAL)", titleH
        );
        CCSize const pSize(colW, pathH);
        m_pfState  = addRow(path, pSize, titleH, 0, 4, "State");
        m_pfModel  = addRow(path, pSize, titleH, 1, 4, "Physics");
        m_pfExec   = addRow(path, pSize, titleH, 2, 4, "Execution");
        m_pfSearch = addRow(path, pSize, titleH, 3, 4, "Search");

        float logTitleH = 0.f;
        auto logCard = makeCard(
            panelHolder, rightX, contentBottom, colW, logH, "EVENT LOG", logTitleH
        );

        m_showLog = Mod::get()->getSettingValue<bool>("show-event-log");
        float const logPad = std::max(4.f, colW * 0.04f);
        float const logArea = logH - logTitleH - logPad * 0.5f;
        float const wantedLine = S.height * 0.045f;
        int const lines = std::clamp(
            static_cast<int>(logArea / std::max(1.f, wantedLine)), 1, 8
        );
        float const lineH = logArea / static_cast<float>(lines);

        if (m_showLog) {
            for (int i = 0; i < lines; ++i) {
                float const ly = logH - logTitleH - lineH * (i + 0.5f);
                m_logLines.push_back(makeFitLabel(
                    logCard, "chatFont.fnt", ccp(logPad, ly), ccp(0.f, 0.5f),
                    colW - logPad * 2.f, lineH * 0.9f, 0.5f, true
                ));
            }
        } else {
            m_logHidden = makeFitLabel(
                logCard, "chatFont.fnt", ccp(logPad, logH - logTitleH - lineH * 0.5f),
                ccp(0.f, 0.5f), colW - logPad * 2.f, lineH * 0.9f, 0.5f, true
            );
            setFit(m_logHidden, "Hidden (see mod settings)", kDim);
        }
    }

    refreshUI();

    float interval = static_cast<float>(
        Mod::get()->getSettingValue<double>("ui-refresh-interval")
    );
    interval = std::clamp(interval, 0.1f, 2.0f);
    this->schedule(schedule_selector(HawkPathPopup::onTick), interval);

    handleTouchPriority(this);
    return true;
}

// ---------------------------------------------------------------------------
// refresh
// ---------------------------------------------------------------------------

void HawkPathPopup::onTick(float) {
    refreshUI();
}

void HawkPathPopup::refreshUI() {
    auto& safety = hawk::SafetyManager::get();
    auto& session = hawk::SessionManager::get();
    auto& engine = hawk::PathfinderEngine::get();

    auto const& state = safety.refreshState();

    // header
    std::string const overall = safety.overallStatus();
    ccColor3B overallColor = kWarn;
    if (overall == "READY") overallColor = kGood;
    else if (overall == "EMERGENCY STOP") overallColor = kBad;
    setFit(m_headerStatus, overall, overallColor);

    // overview
    std::string game = hawk::GameStateDetector::name(state.kind);
    ccColor3B gameColor = kText;
    if (!state.verified) {
        game += " (unverified)";
        gameColor = kWarn;
    } else if (state.nonSaving) {
        game += " (non-saving)";
        gameColor = kGood;
    }
    setFit(m_ovGame, game, gameColor);

    setFit(m_ovSafe, safety.isSafeMode() ? "ON" : "OFF",
           safety.isSafeMode() ? kGood : kWarn);
    setFit(m_ovStop, safety.isEmergencyStopped() ? "ACTIVE" : "Clear",
           safety.isEmergencyStopped() ? kBad : kText);
    setFit(m_ovPath, hawk::PathfinderEngine::stateName(engine.state()),
           engine.state() == hawk::PathfinderState::Failed ? kWarn : kText);
    setFit(m_ovSession, hawk::SessionManager::stateName(session.state()),
           session.state() == hawk::SessionState::Running ? kGood : kText);

    // session
    setFit(m_sessState, hawk::SessionManager::stateName(session.state()), kText);
    setFit(m_sessDuration, formatSeconds(session.durationSeconds()), kText);
    setFit(m_sessMessage, session.lastMessage(), kDim);

    // pathfinder
    setFit(m_pfState, engine.statusText(), kText);
    bool const calibrated = engine.model().isCalibrated();
    setFit(m_pfModel, calibrated ? engine.model().name() : "NOT CALIBRATED",
           calibrated ? kGood : kWarn);
    bool const exec = hawk::InputController::kExecutionEnabled;
    setFit(m_pfExec, exec ? "Enabled" : "DISABLED", exec ? kGood : kWarn);
    auto const& cfg = engine.config();
    setFit(m_pfSearch,
           "beam " + std::to_string(cfg.beamWidth) + " / " +
               std::to_string(cfg.maxFrames) + "f",
           kDim);

    // log
    if (m_showLog && !m_logLines.empty()) {
        auto entries = hawk::EventLog::get().recent(m_logLines.size());
        for (std::size_t i = 0; i < m_logLines.size(); ++i) {
            if (i < entries.size()) {
                auto const& e = entries[i];
                char buf[16];
                std::snprintf(buf, sizeof(buf), "%4.0fs ", e.seconds);
                ccColor3B c = kText;
                if (e.level == hawk::LogLevel::Warn) c = kWarn;
                if (e.level == hawk::LogLevel::Error) c = kBad;
                setFit(m_logLines[i], std::string(buf) + e.text, c);
            } else {
                setFit(m_logLines[i], " ", kDim);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// callbacks
// ---------------------------------------------------------------------------

void HawkPathPopup::onStartPathfinder(CCObject*) {
    hawk::SessionManager::get().start();
    refreshUI();
}

void HawkPathPopup::onStopPathfinder(CCObject*) {
    hawk::SessionManager::get().stop("Stopped from dashboard");
    refreshUI();
}

void HawkPathPopup::onToggleSafeMode(CCObject*) {
    auto& safety = hawk::SafetyManager::get();
    safety.setSafeMode(!safety.isSafeMode());
    refreshUI();
}

void HawkPathPopup::onEmergencyStop(CCObject*) {
    hawk::SafetyManager::get().activateEmergencyStop("dashboard button");
    refreshUI();
}

void HawkPathPopup::onResetEmergencyStop(CCObject*) {
    if (!hawk::SafetyManager::get().resetEmergencyStop()) {
        hawk::logInfo("Reset ignored: Emergency Stop is not active");
    }
    refreshUI();
}

void HawkPathPopup::onRefreshState(CCObject*) {
    auto const& state = hawk::SafetyManager::get().refreshState();
    hawk::logInfo(std::string("Manual refresh: ") +
                  hawk::GameStateDetector::name(state.kind) + " - " + state.detail);
    refreshUI();
}
