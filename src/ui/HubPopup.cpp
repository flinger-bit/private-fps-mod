#include "HubPopup.hpp"

#include "FpsTab.hpp"
#include "BotTab.hpp"
#include "../bot/BotManager.hpp"

#include <Geode/utils/string.hpp>
#include <cocos-ext.h>

#include <algorithm>
#include <string>

using namespace geode::prelude;
using hub::BotManager;

namespace {
    static HubPopup* s_open = nullptr;

    constexpr float kPanelW = 320.f;
    constexpr float kPanelH = 460.f;
    constexpr float kHeaderH = 72.f;
    constexpr float kBodyH = kPanelH - kHeaderH;
    constexpr float kDragH = 12.f;

    template <typename T>
    T clampValue(T value, T low, T high) {
        return std::max(low, std::min(value, high));
    }

    CCMenuItemSpriteExtra* makeTextButton(char const* text, CCObject* target, SEL_MenuHandler callback, float scale = 0.25f) {
        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(scale);
        return CCMenuItemSpriteExtra::create(label, target, callback);
    }

    bool pointInRect(CCPoint const& p, CCRect const& r) {
        return p.x >= r.origin.x
            && p.x <= r.origin.x + r.size.width
            && p.y >= r.origin.y
            && p.y <= r.origin.y + r.size.height;
    }

    char const* tabLabelFor(HubPopup::Tab tab) {
        return tab == HubPopup::Tab::FPS ? "BOT" : "FPS";
    }
}

HubPopup* HubPopup::create() {
    auto ret = new HubPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool HubPopup::init() {
    if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 150))) {
        return false;
    }

    auto win = CCDirector::sharedDirector()->getWinSize();
    setContentSize(win);
    setPosition(CCPointZero);
    setTouchEnabled(true);

    auto panelX = (win.width - kPanelW) / 2.f;
    auto panelY = (win.height - kPanelH) / 2.f;

    m_panel = CCLayerColor::create(ccc4(20, 20, 20, 242));
    m_panel->setContentSize(CCSizeMake(kPanelW, kPanelH));
    m_panel->setAnchorPoint(CCPointZero);
    m_panel->setPosition(CCPointMake(panelX, panelY));
    addChild(m_panel, 1);

    m_body = CCLayer::create();
    m_body->setPosition(CCPointZero);
    m_body->setContentSize(CCSizeMake(kPanelW, kBodyH));
    m_panel->addChild(m_body, 1);

    m_header = CCLayerColor::create(ccc4(34, 34, 34, 255));
    m_header->setContentSize(CCSizeMake(kPanelW, kHeaderH));
    m_header->setAnchorPoint(CCPointZero);
    m_header->setPosition(CCPointMake(0.f, kBodyH));
    m_panel->addChild(m_header, 3);

    auto dragStrip = CCLayerColor::create(ccc4(255, 255, 255, 24));
    dragStrip->setContentSize(CCSizeMake(kPanelW, kDragH));
    dragStrip->setAnchorPoint(CCPointZero);
    dragStrip->setPosition(CCPointMake(0.f, kHeaderH - kDragH));
    m_header->addChild(dragStrip, 0);

    auto title = CCLabelBMFont::create("PRIVATE HUB", "bigFont.fnt");
    title->setScale(0.42f);
    title->setPosition(CCPointMake(104.f, 52.f));
    m_header->addChild(title, 1);

    auto hint = CCLabelBMFont::create("Drag the top strip", "bigFont.fnt");
    hint->setScale(0.12f);
    hint->setPosition(CCPointMake(104.f, 36.f));
    m_header->addChild(hint, 1);

    m_tabsMenu = CCMenu::create();
    m_tabsMenu->setPosition(CCPointZero);
    m_header->addChild(m_tabsMenu, 20);

    rebuild();
    scheduleUpdate();
    return true;
}

void HubPopup::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 999, true);
}

bool HubPopup::ccTouchBegan(CCTouch* touch, CCEvent*) {
    if (!m_panel) {
        return false;
    }

    auto point = convertToNodeSpace(touch->getLocation());
    auto panelPos = m_panel->getPosition();
    auto dragRect = CCRectMake(
        panelPos.x,
        panelPos.y + kBodyH + (kHeaderH - kDragH),
        kPanelW,
        kDragH
    );

    if (pointInRect(point, dragRect)) {
        m_dragging = true;
        m_dragOffset = ccpSub(m_panel->getPosition(), point);
        return true;
    }

    return false;
}

void HubPopup::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (!m_dragging || !m_panel) {
        return;
    }

    auto win = CCDirector::sharedDirector()->getWinSize();
    auto newPos = ccpAdd(touch->getLocation(), m_dragOffset);

    newPos.x = clampValue(newPos.x, 0.f, win.width - kPanelW);
    newPos.y = clampValue(newPos.y, 0.f, win.height - kPanelH);

    m_panel->setPosition(newPos);
}

void HubPopup::ccTouchEnded(CCTouch*, CCEvent*) {
    m_dragging = false;
}

void HubPopup::update(float) {
    refresh();
}

void HubPopup::rebuild() {
    if (!m_panel || !m_body || !m_tabsMenu) {
        return;
    }

    m_tabsMenu->removeAllChildrenWithCleanup(true);
    m_body->removeAllChildrenWithCleanup(true);

    m_scroll = nullptr;
    m_scrollContent = nullptr;
    m_bodyMenu = nullptr;
    m_statusTop = nullptr;
    m_statusBottom = nullptr;

    auto tabButton = makeTextButton(tabLabelFor(m_tab), this, menu_selector(HubPopup::onTabToggle), 0.30f);
    tabButton->setPosition(CCPointMake(84.f, 20.f));
    m_tabsMenu->addChild(tabButton);

    auto close = makeTextButton("X", this, menu_selector(HubPopup::onClose), 0.32f);
    close->setPosition(CCPointMake(282.f, 20.f));
    m_tabsMenu->addChild(close);

    float contentHeight = m_tab == Tab::FPS ? 390.f : 700.f;

    m_scrollContent = CCLayer::create();
    m_scrollContent->setAnchorPoint(CCPointZero);
    m_scrollContent->setPosition(CCPointZero);
    m_scrollContent->setContentSize(CCSizeMake(kPanelW, contentHeight));

    m_bodyMenu = CCMenu::create();
    m_bodyMenu->setPosition(CCPointZero);
    m_scrollContent->addChild(m_bodyMenu, 0);

    m_scroll = CCScrollView::create(CCSizeMake(kPanelW, kBodyH), m_scrollContent);
    m_scroll->setPosition(CCPointZero);
    m_scroll->setDirection(kCCScrollViewDirectionVertical);
    m_scroll->setBounceable(true);
    m_scroll->setTouchEnabled(true);

    if (contentHeight > kBodyH) {
        m_scroll->setContentOffset(m_scroll->minContainerOffset());
    } else {
        m_scroll->setContentOffset(CCPointZero);
    }

    m_body->addChild(m_scroll, 0);

    if (m_tab == Tab::FPS) {
        hub::ui::fps::build(m_scrollContent, m_bodyMenu, m_statusTop, contentHeight, this);
    } else {
        hub::ui::bot::build(m_scrollContent, m_bodyMenu, m_statusTop, m_statusBottom, contentHeight, this);
    }

    refresh();
}

void HubPopup::refresh() {
    if (m_tab == Tab::FPS) {
        hub::ui::fps::refresh(m_statusTop);
    } else {
        hub::ui::bot::refresh(m_statusTop, m_statusBottom);
    }
}

void HubPopup::toggle() {
    if (s_open && !s_open->getParent()) {
        s_open = nullptr;
    }

    if (s_open && s_open->getParent()) {
        s_open->removeFromParentAndCleanup(true);
        s_open = nullptr;
        return;
    }

    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene) {
        return;
    }

    s_open = HubPopup::create();
    scene->addChild(s_open, 99999);
}

void HubPopup::onTabToggle(CCObject*) {
    m_tab = (m_tab == Tab::FPS) ? Tab::BOT : Tab::FPS;
    rebuild();
}

void HubPopup::onClose(CCObject*) {
    if (s_open && s_open->getParent()) {
        s_open->removeFromParentAndCleanup(true);
        s_open = nullptr;
    }
}

void HubPopup::onFpsToggle(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_enabled")) {
        return;
    }
    mod->setSettingValue<bool>("fps_enabled", !mod->getSettingValue<bool>("fps_enabled"));
    refresh();
}

void HubPopup::onFpsPlus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_target")) {
        return;
    }
    mod->setSettingValue<int>(
        "fps_target",
        clampValue(mod->getSettingValue<int>("fps_target") + 30, 60, 360)
    );
    refresh();
}

void HubPopup::onFpsMinus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_target")) {
        return;
    }
    mod->setSettingValue<int>(
        "fps_target",
        clampValue(mod->getSettingValue<int>("fps_target") - 30, 60, 360)
    );
    refresh();
}

void HubPopup::onFpsDefault(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_target")) {
        return;
    }
    mod->setSettingValue<int>("fps_target", 240);
    refresh();
}

void HubPopup::onRecord(CCObject*) {
    BotManager::shared().startRecording();
    refresh();
}

void HubPopup::onPlay(CCObject*) {
    BotManager::shared().startPlaying();
    refresh();
}

void HubPopup::onStop(CCObject*) {
    BotManager::shared().stopRecording();
    BotManager::shared().stopPlaying();
    refresh();
}

void HubPopup::onSave(CCObject*) {
    BotManager::shared().saveMacro();
    refresh();
}

void HubPopup::onLoad(CCObject*) {
    BotManager::shared().loadMacro();
    refresh();
}

void HubPopup::onClear(CCObject*) {
    BotManager::shared().clearMacro();
    refresh();
}

void HubPopup::onFrameStepToggle(CCObject*) {
    BotManager::shared().toggleFrameStepper();
    refresh();
}

void HubPopup::onStep(CCObject*) {
    BotManager::shared().stepOneFrame();
    refresh();
}

void HubPopup::onIgnoreToggle(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("ignore_inputs")) {
        return;
    }
    mod->setSettingValue<bool>("ignore_inputs", !mod->getSettingValue<bool>("ignore_inputs"));
    refresh();
}

void HubPopup::onSwiftToggle(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_enabled")) {
        return;
    }
    mod->setSettingValue<bool>("swift_enabled", !mod->getSettingValue<bool>("swift_enabled"));
    refresh();
}

void HubPopup::onSwiftPlus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_clicks")) {
        return;
    }
    mod->setSettingValue<int>(
        "swift_clicks",
        clampValue(mod->getSettingValue<int>("swift_clicks") + 1, 1, 60)
    );
    refresh();
}

void HubPopup::onSwiftMinus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_clicks")) {
        return;
    }
    mod->setSettingValue<int>(
        "swift_clicks",
        clampValue(mod->getSettingValue<int>("swift_clicks") - 1, 1, 60)
    );
    refresh();
}

void HubPopup::onSwiftDefault(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_clicks")) {
        return;
    }
    mod->setSettingValue<int>("swift_clicks", 20);
    refresh();
}
