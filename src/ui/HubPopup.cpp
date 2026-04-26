#include "HubPopup.hpp"
#include "UiHelpers.hpp"

#include "FpsTab.hpp"
#include "BotTab.hpp"
#include "FrameTab.hpp"
#include "SwiftTab.hpp"
#include "AboutTab.hpp"

#include "../bot/BotManager.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;
using cocos2d::extension::CCScale9Sprite;
using hub::BotManager;

namespace {
    // Tag used to find an existing popup attached to the running scene.
    // Avoids keeping a static raw pointer that goes stale across scene changes.
    constexpr int kHubTag = 0x70667048; // 'pfpH'

    constexpr float kPanelW   = 480.f;
    constexpr float kPanelH   = 280.f;
    constexpr float kSidebarW = 110.f;
    constexpr float kHeaderH  = 30.f;
    constexpr float kPadding  = 8.f;

    bool pointInRect(CCPoint const& p, CCRect const& r) {
        return p.x >= r.origin.x && p.x <= r.origin.x + r.size.width
            && p.y >= r.origin.y && p.y <= r.origin.y + r.size.height;
    }

    int tabId(HubPopup::Tab tab) {
        return static_cast<int>(tab);
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
    if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 140))) return false;

    auto win = CCDirector::sharedDirector()->getWinSize();
    setContentSize(win);
    setPosition(CCPointZero);
    setTouchEnabled(true);
    setTag(kHubTag);

    // ---- Panel: real GD CCScale9Sprite background, with fallbacks ----
    m_panel = CCScale9Sprite::create("GJ_square02.png");
    if (!m_panel) {
        m_panel = CCScale9Sprite::create("GJ_square01.png");
    }
    if (!m_panel) {
        m_panel = CCScale9Sprite::createWithSpriteFrameName("square02_001.png");
    }
    if (!m_panel) {
        log::error(
            "HubPopup: every CCScale9Sprite candidate returned null; aborting init"
        );
        return false;
    }
    m_panel->setContentSize({ kPanelW, kPanelH });
    m_panel->setPosition({ win.width / 2.f, win.height / 2.f });
    m_panel->setOpacity(235);
    addChild(m_panel, 1);

    // ---- Border drawn with CCDrawNode (cyan accent) ----
    m_border = CCDrawNode::create();
    m_border->drawRect(
        { -kPanelW / 2.f + 2.f, -kPanelH / 2.f + 2.f },
        {  kPanelW / 2.f - 2.f,  kPanelH / 2.f - 2.f },
        { 0, 0, 0, 0 },
        2.f,
        { 70.f / 255.f, 220.f / 255.f, 220.f / 255.f, 0.9f }
    );
    m_panel->addChild(m_border, 2);

    // ---- Header inside panel-local coordinates ----
    auto headerY = kPanelH - kHeaderH / 2.f - 4.f;

    m_title = CCLabelBMFont::create("Private FPS Pro", "bigFont.fnt");
    m_title->setScale(0.45f);
    m_title->setAnchorPoint({ 0.f, 0.5f });
    m_title->setPosition({ 14.f, headerY });
    m_panel->addChild(m_title, 5);

    auto closeMenu = CCMenu::create();
    closeMenu->setPosition({ kPanelW - 22.f, headerY });
    m_panel->addChild(closeMenu, 5);

    auto closeSprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    if (!closeSprite) {
        closeSprite = CCSprite::create("GJ_button_06.png");
    }
    if (closeSprite) {
        closeSprite->setScale(0.55f);
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeSprite, this, menu_selector(HubPopup::onClose)
        );
        if (closeBtn) closeMenu->addChild(closeBtn);
    } else {
        log::warn("HubPopup: no close sprite available");
    }

    auto headerSep = CCDrawNode::create();
    headerSep->drawSegment(
        { 8.f,            kPanelH - kHeaderH - 4.f },
        { kPanelW - 8.f,  kPanelH - kHeaderH - 4.f },
        0.6f,
        { 70.f / 255.f, 220.f / 255.f, 220.f / 255.f, 0.5f }
    );
    m_panel->addChild(headerSep, 4);

    // ---- Sidebar (left column with tab buttons) ----
    m_sidebar = CCNode::create();
    m_sidebar->setAnchorPoint({ 0.f, 0.f });
    m_sidebar->setContentSize({ kSidebarW, kPanelH - kHeaderH - 8.f });
    m_sidebar->setPosition({ kPadding, kPadding });
    m_panel->addChild(m_sidebar, 5);

    auto sidebarBg = CCDrawNode::create();
    sidebarBg->drawRect(
        { 0.f, 0.f },
        { kSidebarW, m_sidebar->getContentSize().height },
        { 0.07f, 0.09f, 0.13f, 0.55f },
        1.f,
        { 70.f / 255.f, 220.f / 255.f, 220.f / 255.f, 0.35f }
    );
    m_sidebar->addChild(sidebarBg, 0);

    m_sidebarMenu = CCMenu::create();
    m_sidebarMenu->setAnchorPoint({ 0.f, 0.f });
    m_sidebarMenu->setContentSize(m_sidebar->getContentSize());
    m_sidebarMenu->setPosition({ 0.f, 0.f });
    m_sidebarMenu->ignoreAnchorPointForPosition(true);
    m_sidebar->addChild(m_sidebarMenu, 1);

    // ---- Content area (right side) ----
    auto contentX = kPadding + kSidebarW + kPadding;
    auto contentW = kPanelW - contentX - kPadding;
    auto contentH = kPanelH - kHeaderH - 8.f;

    m_content = CCNode::create();
    m_content->setAnchorPoint({ 0.f, 0.f });
    m_content->setContentSize({ contentW, contentH });
    m_content->setPosition({ contentX, kPadding });
    m_panel->addChild(m_content, 5);

    auto contentBg = CCDrawNode::create();
    contentBg->drawRect(
        { 0.f, 0.f },
        { contentW, contentH },
        { 0.05f, 0.07f, 0.10f, 0.55f },
        1.f,
        { 70.f / 255.f, 220.f / 255.f, 220.f / 255.f, 0.30f }
    );
    m_content->addChild(contentBg, 0);

    m_contentMenu = CCMenu::create();
    m_contentMenu->setAnchorPoint({ 0.f, 0.f });
    m_contentMenu->setContentSize(m_content->getContentSize());
    m_contentMenu->setPosition({ 0.f, 0.f });
    m_contentMenu->ignoreAnchorPointForPosition(true);
    m_content->addChild(m_contentMenu, 1);

    rebuild();
    scheduleUpdate();
    return true;
}

void HubPopup::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 999, true);
}

bool HubPopup::ccTouchBegan(CCTouch* touch, CCEvent*) {
    if (!m_panel) return false;

    auto local = convertToNodeSpace(touch->getLocation());
    auto panelPos = m_panel->getPosition();
    auto headerRect = CCRectMake(
        panelPos.x - kPanelW / 2.f,
        panelPos.y + kPanelH / 2.f - kHeaderH,
        kPanelW,
        kHeaderH
    );
    if (pointInRect(local, headerRect)) {
        m_dragging = true;
        m_dragOffset = ccpSub(panelPos, local);
        return true;
    }
    return false;
}

void HubPopup::ccTouchMoved(CCTouch* touch, CCEvent*) {
    if (!m_dragging || !m_panel) return;

    auto win = CCDirector::sharedDirector()->getWinSize();
    auto newPos = ccpAdd(touch->getLocation(), m_dragOffset);
    newPos.x = std::max(kPanelW / 2.f, std::min(newPos.x, win.width  - kPanelW / 2.f));
    newPos.y = std::max(kPanelH / 2.f, std::min(newPos.y, win.height - kPanelH / 2.f));
    m_panel->setPosition(newPos);
}

void HubPopup::ccTouchEnded(CCTouch*, CCEvent*) {
    m_dragging = false;
}

void HubPopup::update(float) {
    refresh();
}

void HubPopup::buildSidebar() {
    if (!m_sidebarMenu) return;
    m_sidebarMenu->removeAllChildrenWithCleanup(true);

    struct TabDef { char const* label; Tab tab; };
    TabDef tabs[] = {
        { "FPS",    Tab::FPS    },
        { "Bot",    Tab::BOT    },
        { "Frame",  Tab::FRAME  },
        { "Swift",  Tab::SWIFT  },
        { "About",  Tab::ABOUT  },
    };

    auto sbSize = m_sidebar->getContentSize();

    // CCMenu only routes touches to CCMenuItem children that are *direct*
    // descendants. The previous version wrapped the buttons in an intermediate
    // CCNode ("column"), which silently swallowed every tap. Apply the layout
    // to m_sidebarMenu itself and parent the buttons directly to it.
    m_sidebarMenu->setContentSize(sbSize);
    m_sidebarMenu->setPosition({ sbSize.width / 2.f, sbSize.height / 2.f });
    m_sidebarMenu->setAnchorPoint({ 0.5f, 0.5f });

    m_sidebarMenu->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setGap(6.f)
            ->setAxisAlignment(AxisAlignment::Center)
            ->setCrossAxisAlignment(AxisAlignment::Center)
    );

    for (auto const& t : tabs) {
        bool active = (t.tab == m_tab);
        auto btn = hub::ui::makeSidebarBtn(
            t.label, tabId(t.tab), active,
            this, menu_selector(HubPopup::onSidebarTab)
        );
        m_sidebarMenu->addChild(btn);
    }

    m_sidebarMenu->updateLayout();
}

void HubPopup::buildContent() {
    if (!m_content || !m_contentMenu) return;

    m_contentMenu->removeAllChildrenWithCleanup(true);
    // Remove any non-menu, non-bg children we added on previous rebuild
    auto children = m_content->getChildren();
    if (children) {
        CCArray* toRemove = CCArray::create();
        for (int i = 0; i < (int)children->count(); ++i) {
            auto child = static_cast<CCNode*>(children->objectAtIndex(i));
            if (child == m_contentMenu) continue;
            if (dynamic_cast<CCDrawNode*>(child)) continue;
            toRemove->addObject(child);
        }
        for (int i = 0; i < (int)toRemove->count(); ++i) {
            auto child = static_cast<CCNode*>(toRemove->objectAtIndex(i));
            child->removeFromParentAndCleanup(true);
        }
    }

    m_statusTop = nullptr;
    m_statusBottom = nullptr;

    switch (m_tab) {
        case Tab::FPS:
            hub::ui::fps::build(m_content, m_contentMenu, m_statusTop, this);
            break;
        case Tab::BOT:
            hub::ui::bot::build(m_content, m_contentMenu, m_statusTop, m_statusBottom, this);
            break;
        case Tab::FRAME:
            hub::ui::frame::build(m_content, m_contentMenu, m_statusTop, this);
            break;
        case Tab::SWIFT:
            hub::ui::swift::build(m_content, m_contentMenu, m_statusTop, this);
            break;
        case Tab::ABOUT:
            hub::ui::about::build(m_content, m_contentMenu, this);
            break;
    }
}

void HubPopup::rebuild() {
    buildSidebar();
    buildContent();
    refresh();
}

void HubPopup::refresh() {
    switch (m_tab) {
        case Tab::FPS:   hub::ui::fps::refresh(m_statusTop); break;
        case Tab::BOT:   hub::ui::bot::refresh(m_statusTop, m_statusBottom); break;
        case Tab::FRAME: hub::ui::frame::refresh(m_statusTop); break;
        case Tab::SWIFT: hub::ui::swift::refresh(m_statusTop); break;
        case Tab::ABOUT: break;
    }
}

void HubPopup::toggle() {
    auto director = CCDirector::sharedDirector();
    if (!director) return;
    auto scene = director->getRunningScene();
    if (!scene) return;

    // Find any existing popup in the current scene by tag — this is safe across
    // scene transitions because we never dereference a stale raw pointer.
    if (auto existing = scene->getChildByTag(kHubTag)) {
        existing->removeFromParentAndCleanup(true);
        return;
    }

    auto popup = HubPopup::create();
    if (!popup) {
        log::error("HubPopup::toggle: HubPopup::create() returned null");
        return;
    }
    scene->addChild(popup, 99999);
}

void HubPopup::onSidebarTab(CCObject* sender) {
    auto item = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto next = static_cast<Tab>(item->getTag());
    if (next == m_tab) return;
    m_tab = next;
    rebuild();
}

void HubPopup::onClose(CCObject*) {
    // Removing ourselves is enough; the toggle()/tag-lookup path no longer
    // depends on a static pointer, so no extra bookkeeping is needed.
    if (this->getParent()) {
        this->removeFromParentAndCleanup(true);
    }
}

// ---------- Setting helpers (kept identical so BotManager / main keep working) ----------

void HubPopup::onFpsToggle(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_enabled")) return;
    mod->setSettingValue<bool>("fps_enabled", !mod->getSettingValue<bool>("fps_enabled"));
    refresh();
}

void HubPopup::onFpsPlus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_target")) return;
    mod->setSettingValue<int>(
        "fps_target",
        hub::ui::clampInt(mod->getSettingValue<int>("fps_target") + 30, 60, 360)
    );
    refresh();
}

void HubPopup::onFpsMinus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_target")) return;
    mod->setSettingValue<int>(
        "fps_target",
        hub::ui::clampInt(mod->getSettingValue<int>("fps_target") - 30, 60, 360)
    );
    refresh();
}

void HubPopup::onFpsDefault(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("fps_target")) return;
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

void HubPopup::onSave(CCObject*)  { BotManager::shared().saveMacro();  refresh(); }
void HubPopup::onLoad(CCObject*)  { BotManager::shared().loadMacro();  refresh(); }
void HubPopup::onClear(CCObject*) { BotManager::shared().clearMacro(); refresh(); }

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
    if (!mod || !mod->hasSetting("ignore_inputs")) return;
    mod->setSettingValue<bool>("ignore_inputs", !mod->getSettingValue<bool>("ignore_inputs"));
    refresh();
}

void HubPopup::onSwiftToggle(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_enabled")) return;
    mod->setSettingValue<bool>("swift_enabled", !mod->getSettingValue<bool>("swift_enabled"));
    refresh();
}

void HubPopup::onSwiftPlus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_clicks")) return;
    mod->setSettingValue<int>(
        "swift_clicks",
        hub::ui::clampInt(mod->getSettingValue<int>("swift_clicks") + 1, 1, 60)
    );
    refresh();
}

void HubPopup::onSwiftMinus(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_clicks")) return;
    mod->setSettingValue<int>(
        "swift_clicks",
        hub::ui::clampInt(mod->getSettingValue<int>("swift_clicks") - 1, 1, 60)
    );
    refresh();
}

void HubPopup::onSwiftDefault(CCObject*) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting("swift_clicks")) return;
    mod->setSettingValue<int>("swift_clicks", 20);
    refresh();
}
