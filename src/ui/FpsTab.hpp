#pragma once

#include <Geode/Geode.hpp>
#include "HubPopup.hpp"

#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace hub::ui::fps {

template <typename T>
inline T clampValue(T value, T low, T high) {
    return std::max(low, std::min(value, high));
}

inline bool getBool(char const* key, bool fallback = false) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return fallback;
    }
    return mod->getSettingValue<bool>(key);
}

inline int getInt(char const* key, int fallback = 0) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return fallback;
    }
    return mod->getSettingValue<int>(key);
}

inline CCMenuItemSpriteExtra* makeButton(char const* text, CCObject* target, SEL_MenuHandler cb, float scale = 0.24f) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(scale);
    return CCMenuItemSpriteExtra::create(label, target, cb);
}

inline void build(CCLayer* content, CCMenu* menu, CCLabelBMFont*& statusTop, float contentHeight, CCObject* target) {
    const float cx = 160.f;

    auto title = CCLabelBMFont::create("FPS", "bigFont.fnt");
    title->setScale(0.30f);
    title->setPosition(CCPointMake(cx, contentHeight - 52.f));
    content->addChild(title);

    auto hint = CCLabelBMFont::create("Compact controls", "bigFont.fnt");
    hint->setScale(0.16f);
    hint->setPosition(CCPointMake(cx, contentHeight - 76.f));
    content->addChild(hint);

    statusTop = CCLabelBMFont::create("", "bigFont.fnt");
    statusTop->setScale(0.20f);
    statusTop->setPosition(CCPointMake(cx, contentHeight - 28.f));
    content->addChild(statusTop);

    auto toggle = makeButton("Toggle", target, menu_selector(HubPopup::onFpsToggle), 0.24f);
    toggle->setPosition(CCPointMake(cx, contentHeight - 120.f));
    menu->addChild(toggle);

    auto minus = makeButton("-", target, menu_selector(HubPopup::onFpsMinus), 0.32f);
    minus->setPosition(CCPointMake(96.f, contentHeight - 188.f));
    menu->addChild(minus);

    auto plus = makeButton("+", target, menu_selector(HubPopup::onFpsPlus), 0.32f);
    plus->setPosition(CCPointMake(160.f, contentHeight - 188.f));
    menu->addChild(plus);

    auto def = makeButton("Default", target, menu_selector(HubPopup::onFpsDefault), 0.20f);
    def->setPosition(CCPointMake(228.f, contentHeight - 188.f));
    menu->addChild(def);

    auto note = CCLabelBMFont::create("FPS target is stored in settings", "bigFont.fnt");
    note->setScale(0.14f);
    note->setPosition(CCPointMake(cx, contentHeight - 246.f));
    content->addChild(note);
}

inline void refresh(CCLabelBMFont* statusTop) {
    if (!statusTop) {
        return;
    }

    bool enabled = getBool("fps_enabled", true);
    int target = clampValue(getInt("fps_target", 240), 60, 360);

    std::string text = "FPS: ";
    text += enabled ? "ON" : "OFF";
    text += " | Target: ";
    text += std::to_string(target);

    statusTop->setString(text.c_str());
}

} // namespace hub::ui::fps
