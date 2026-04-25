#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class HubPopup : public CCLayerColor {
public:
    enum class Tab {
        FPS    = 0,
        BOT    = 1,
        FRAME  = 2,
        SWIFT  = 3,
        ABOUT  = 4
    };

protected:
    Tab m_tab = Tab::FPS;

    cocos2d::extension::CCScale9Sprite* m_panel = nullptr;
    CCDrawNode*    m_border       = nullptr;
    CCNode*        m_sidebar      = nullptr;
    CCMenu*        m_sidebarMenu  = nullptr;
    CCNode*        m_content      = nullptr;
    CCMenu*        m_contentMenu  = nullptr;
    CCLabelBMFont* m_title        = nullptr;
    CCLabelBMFont* m_statusTop    = nullptr;
    CCLabelBMFont* m_statusBottom = nullptr;

    bool    m_dragging   = false;
    CCPoint m_dragOffset = CCPointZero;

    bool init() override;
    void registerWithTouchDispatcher() override;

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;

    void update(float) override;

    void buildSidebar();
    void buildContent();
    void rebuild();
    void refresh();

public:
    static HubPopup* create();
    static void toggle();

    Tab currentTab() const { return m_tab; }

    void onSidebarTab(CCObject*);
    void onClose(CCObject*);

    void onFpsToggle(CCObject*);
    void onFpsPlus(CCObject*);
    void onFpsMinus(CCObject*);
    void onFpsDefault(CCObject*);

    void onRecord(CCObject*);
    void onPlay(CCObject*);
    void onStop(CCObject*);
    void onSave(CCObject*);
    void onLoad(CCObject*);
    void onClear(CCObject*);

    void onFrameStepToggle(CCObject*);
    void onStep(CCObject*);
    void onIgnoreToggle(CCObject*);

    void onSwiftToggle(CCObject*);
    void onSwiftPlus(CCObject*);
    void onSwiftMinus(CCObject*);
    void onSwiftDefault(CCObject*);
};
