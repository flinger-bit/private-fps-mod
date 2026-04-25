#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace hub::ui {

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

inline CCMenuItemSpriteExtra* makeSidebarBtn(
    char const* text,
    int tag,
    bool active,
    CCObject* target,
    SEL_MenuHandler cb
) {
    char const* spr = active ? "GJ_button_02.png" : "GJ_button_05.png";
    auto btn = ButtonSprite::create(text, 110, false, "bigFont.fnt", spr, 28.f, 0.6f);
    auto item = CCMenuItemSpriteExtra::create(btn, target, cb);
    item->setTag(tag);
    return item;
}

} // namespace hub::ui
