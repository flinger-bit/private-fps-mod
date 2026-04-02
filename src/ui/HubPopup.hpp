#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class HubPopup : public CCLayerColor {
protected:
    enum class Tab {
        FPS = 0,
        BOT = 1
    };

    Tab m_tab = Tab::FPS;

    CCLayerColor* m_panel = nullptr;
    CCLayerColor* m_header = nullptr;
    CCLayer* m_body = nullptr;

    CCScrollView* m_scroll = nullptr;
    CCLayer* m_scrollContent = nullptr;
    CCMenu* m_bodyMenu = nullptr;

    CCMenu* m_tabsMenu = nullptr;
    CCLabelBMFont* m_statusTop = nullptr;
    CCLabelBMFont* m_statusBottom = nullptr;

    bool m_dragging = false;
    CCPoint m_dragOffset = CCPointZero;

    bool init() override;
    void registerWithTouchDispatcher() override;

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;

    void update(float) override;

    void rebuild();
    void refresh();

public:
    static HubPopup* create();
    static void toggle();

    void onTabToggle(CCObject*);
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
