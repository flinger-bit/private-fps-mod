#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace hub::ui {

// ---- Theme palette (inspired by EclipseMenu's cocos renderer) -------------
namespace theme {
    inline cocos2d::ccColor3B accent()        { return {  70, 220, 220 }; } // cyan border
    inline cocos2d::ccColor3B panelOuter()    { return {  44, 60,  74 }; } // outline tint
    inline cocos2d::ccColor3B panelMain()     { return {  28, 34,  44 }; } // main bg
    inline cocos2d::ccColor3B panelInner()    { return {  18, 22,  30 }; } // sidebar/content bg
    inline cocos2d::ccColor3B tabActiveBg()   { return {  78, 200, 200 }; } // selected tab
    inline cocos2d::ccColor3B tabInactiveBg() { return {  68, 80,  98 }; } // unselected tab
    inline cocos2d::ccColor3B tabActiveFg()   { return {  20,  30,  40 }; } // dark text on cyan
    inline cocos2d::ccColor3B tabInactiveFg() { return { 220, 230, 240 }; } // light text on grey
}

inline int clampInt(int v, int lo, int hi) {
    return std::max(lo, std::min(v, hi));
}

inline bool getBool(char const* key, bool fallback = false) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) return fallback;
    return mod->getSettingValue<bool>(key);
}

inline int getInt(char const* key, int fallback = 0) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) return fallback;
    return mod->getSettingValue<int>(key);
}

// Robust CCScale9Sprite factory: prefers `square02b_001.png` (sprite frame in
// GJ_GameSheet01.plist — the same one EclipseMenu uses), with sensible
// fallbacks so we never end up with a null background on Android.
inline cocos2d::extension::CCScale9Sprite* makeNineSlice(
    cocos2d::CCSize const& size,
    cocos2d::ccColor3B color = theme::panelMain(),
    GLubyte opacity = 235
) {
    using cocos2d::extension::CCScale9Sprite;
    auto sp = CCScale9Sprite::createWithSpriteFrameName("square02b_001.png");
    if (!sp) sp = CCScale9Sprite::create("GJ_square02.png");
    if (!sp) sp = CCScale9Sprite::create("GJ_square01.png");
    if (!sp) return nullptr;
    sp->setContentSize(size);
    sp->setColor(color);
    sp->setOpacity(opacity);
    return sp;
}

inline CCMenuItemSpriteExtra* makeBtn(
    char const* text,
    CCObject* target,
    SEL_MenuHandler cb,
    float scale = 0.7f,
    float width = 90.f,
    char const* sprite = "GJ_button_01.png"
) {
    auto btn = ButtonSprite::create(text, static_cast<int>(width), false, "bigFont.fnt", sprite, 28.f, scale);
    return CCMenuItemSpriteExtra::create(btn, target, cb);
}

// Sidebar tab button — Eclipse `TabButton` style: a NineSlice background with
// a centred label that re-tints when selected. Returned as a
// CCMenuItemSpriteExtra so the parent CCMenu routes touches correctly.
inline CCMenuItemSpriteExtra* makeSidebarBtn(
    char const* text,
    int tag,
    bool active,
    CCObject* target,
    SEL_MenuHandler cb
) {
    constexpr float kBtnW = 96.f;
    constexpr float kBtnH = 28.f;

    auto container = CCNode::create();
    container->setContentSize({ kBtnW, kBtnH });
    container->setAnchorPoint({ 0.5f, 0.5f });

    auto bg = makeNineSlice(
        { kBtnW, kBtnH },
        active ? theme::tabActiveBg() : theme::tabInactiveBg(),
        active ? 235 : 200
    );
    if (bg) {
        bg->setPosition({ kBtnW / 2.f, kBtnH / 2.f });
        container->addChild(bg, 0);
    }

    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->limitLabelWidth(kBtnW - 12.f, 0.5f, 0.25f);
    label->setColor(active ? theme::tabActiveFg() : theme::tabInactiveFg());
    label->setPosition({ kBtnW / 2.f, kBtnH / 2.f });
    container->addChild(label, 1);

    auto item = CCMenuItemSpriteExtra::create(container, target, cb);
    item->setTag(tag);
    return item;
}

} // namespace hub::ui
