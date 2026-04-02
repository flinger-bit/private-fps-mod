#pragma once

#include <Geode/Geode.hpp>
#include "HubPopup.hpp"
#include "../bot/BotManager.hpp"

#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace hub::ui::bot {

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

inline CCMenuItemSpriteExtra* makeButton(char const* text, CCObject* target, SEL_MenuHandler cb, float scale = 0.22f) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(scale);
    return CCMenuItemSpriteExtra::create(label, target, cb);
}

inline void build(
    CCLayer* content,
    CCMenu* menu,
    CCLabelBMFont*& statusTop,
    CCLabelBMFont*& statusBottom,
    float contentHeight,
    CCObject* target
) {
    const float cx = 160.f;

    auto title = CCLabelBMFont::create("BOT", "bigFont.fnt");
    title->setScale(0.30f);
    title->setPosition(CCPointMake(cx, contentHeight - 52.f));
    content->addChild(title);

    auto hint = CCLabelBMFont::create("Record / play / step", "bigFont.fnt");
    hint->setScale(0.16f);
    hint->setPosition(CCPointMake(cx, contentHeight - 76.f));
    content->addChild(hint);

    statusTop = CCLabelBMFont::create("", "bigFont.fnt");
    statusTop->setScale(0.19f);
    statusTop->setPosition(CCPointMake(cx, contentHeight - 28.f));
    content->addChild(statusTop);

    statusBottom = CCLabelBMFont::create("", "bigFont.fnt");
    statusBottom->setScale(0.16f);
    statusBottom->setPosition(CCPointMake(cx, contentHeight - 48.f));
    content->addChild(statusBottom);

    auto rec = makeButton("REC", target, menu_selector(HubPopup::onRecord), 0.22f);
    rec->setPosition(CCPointMake(80.f, contentHeight - 128.f));
    menu->addChild(rec);

    auto play = makeButton("PLAY", target, menu_selector(HubPopup::onPlay), 0.22f);
    play->setPosition(CCPointMake(160.f, contentHeight - 128.f));
    menu->addChild(play);

    auto stop = makeButton("STOP", target, menu_selector(HubPopup::onStop), 0.22f);
    stop->setPosition(CCPointMake(240.f, contentHeight - 128.f));
    menu->addChild(stop);

    auto save = makeButton("SAVE", target, menu_selector(HubPopup::onSave), 0.22f);
    save->setPosition(CCPointMake(80.f, contentHeight - 188.f));
    menu->addChild(save);

    auto load = makeButton("LOAD", target, menu_selector(HubPopup::onLoad), 0.22f);
    load->setPosition(CCPointMake(160.f, contentHeight - 188.f));
    menu->addChild(load);

    auto clear = makeButton("CLEAR", target, menu_selector(HubPopup::onClear), 0.20f);
    clear->setPosition(CCPointMake(240.f, contentHeight - 188.f));
    menu->addChild(clear);

    auto frame = makeButton("FRAME", target, menu_selector(HubPopup::onFrameStepToggle), 0.20f);
    frame->setPosition(CCPointMake(80.f, contentHeight - 248.f));
    menu->addChild(frame);

    auto step = makeButton("STEP", target, menu_selector(HubPopup::onStep), 0.22f);
    step->setPosition(CCPointMake(160.f, contentHeight - 248.f));
    menu->addChild(step);

    auto ignore = makeButton("IGNORE", target, menu_selector(HubPopup::onIgnoreToggle), 0.18f);
    ignore->setPosition(CCPointMake(240.f, contentHeight - 248.f));
    menu->addChild(ignore);

    auto swiftToggle = makeButton("SWIFT", target, menu_selector(HubPopup::onSwiftToggle), 0.20f);
    swiftToggle->setPosition(CCPointMake(62.f, contentHeight - 308.f));
    menu->addChild(swiftToggle);

    auto swiftMinus = makeButton("-", target, menu_selector(HubPopup::onSwiftMinus), 0.30f);
    swiftMinus->setPosition(CCPointMake(128.f, contentHeight - 308.f));
    menu->addChild(swiftMinus);

    auto swiftPlus = makeButton("+", target, menu_selector(HubPopup::onSwiftPlus), 0.30f);
    swiftPlus->setPosition(CCPointMake(192.f, contentHeight - 308.f));
    menu->addChild(swiftPlus);

    auto swiftDefault = makeButton("20", target, menu_selector(HubPopup::onSwiftDefault), 0.22f);
    swiftDefault->setPosition(CCPointMake(258.f, contentHeight - 308.f));
    menu->addChild(swiftDefault);

    auto note = CCLabelBMFont::create("Macro file is saved in mod storage", "bigFont.fnt");
    note->setScale(0.14f);
    note->setPosition(CCPointMake(cx, contentHeight - 372.f));
    content->addChild(note);
}

inline void refresh(CCLabelBMFont* statusTop, CCLabelBMFont* statusBottom) {
    auto& bot = BotManager::shared();

    if (statusTop) {
        std::string top = "BOT: ";
        top += bot.isRecording() ? "RECORDING" : (bot.isPlaying() ? "PLAYING" : "IDLE");
        top += " | Frames: ";
        top += std::to_string(bot.macro().size());
        statusTop->setString(top.c_str());
    }

    if (statusBottom) {
        std::string bottom = "Ignore: ";
        bottom += getBool("ignore_inputs", true) ? "ON" : "OFF";
        bottom += " | FrameStep: ";
        bottom += getBool("frame_stepper", false) ? "ON" : "OFF";
        bottom += " | Swift: ";
        bottom += getBool("swift_enabled", true) ? "ON" : "OFF";
        bottom += " | Cnt: ";
        bottom += std::to_string(clampValue(getInt("swift_clicks", 20), 1, 60));
        statusBottom->setString(bottom.c_str());
    }
}

} // namespace hub::ui::bot
