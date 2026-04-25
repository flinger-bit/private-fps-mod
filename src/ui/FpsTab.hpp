#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include "HubPopup.hpp"
#include "UiHelpers.hpp"

#include <string>

using namespace geode::prelude;

namespace hub::ui::fps {

inline void build(CCNode* content, CCMenu* menu, CCLabelBMFont*& statusTop, CCObject* target) {
    auto size = content->getContentSize();
    const float cx = size.width / 2.f;
    const float top = size.height;

    auto title = CCLabelBMFont::create("FPS Unlocker", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({ 0.5f, 1.f });
    title->setPosition({ cx, top - 10.f });
    content->addChild(title);

    statusTop = CCLabelBMFont::create("FPS: --", "goldFont.fnt");
    statusTop->setScale(0.55f);
    statusTop->setAnchorPoint({ 0.5f, 1.f });
    statusTop->setPosition({ cx, top - 50.f });
    content->addChild(statusTop);

    auto toggleRow = CCMenu::create();
    toggleRow->setContentSize({ size.width - 40.f, 44.f });
    toggleRow->setPosition({ cx, top - 110.f });
    toggleRow->setLayout(RowLayout::create()->setGap(12.f));
    content->addChild(toggleRow);

    toggleRow->addChild(hub::ui::makeBtn(
        "Toggle FPS", target, menu_selector(HubPopup::onFpsToggle), 0.7f, 110.f
    ));
    toggleRow->updateLayout();

    auto fpsRow = CCMenu::create();
    fpsRow->setContentSize({ size.width - 40.f, 44.f });
    fpsRow->setPosition({ cx, top - 175.f });
    fpsRow->setLayout(RowLayout::create()->setGap(10.f));
    content->addChild(fpsRow);

    fpsRow->addChild(hub::ui::makeBtn("-30", target, menu_selector(HubPopup::onFpsMinus), 0.7f, 50.f, "GJ_button_06.png"));
    fpsRow->addChild(hub::ui::makeBtn("Default", target, menu_selector(HubPopup::onFpsDefault), 0.6f, 80.f));
    fpsRow->addChild(hub::ui::makeBtn("+30", target, menu_selector(HubPopup::onFpsPlus), 0.7f, 50.f, "GJ_button_01.png"));
    fpsRow->updateLayout();

    auto note = CCLabelBMFont::create(
        "Range 60 - 360. Use the Geode settings\nfor the slider control.",
        "chatFont.fnt"
    );
    note->setScale(0.6f);
    note->setAnchorPoint({ 0.5f, 1.f });
    note->setAlignment(kCCTextAlignmentCenter);
    note->setPosition({ cx, top - 235.f });
    note->setColor({ 180, 200, 220 });
    content->addChild(note);
}

inline void refresh(CCLabelBMFont* statusTop) {
    if (!statusTop) return;

    bool enabled = hub::ui::getBool("fps_enabled", true);
    int target  = hub::ui::clampInt(hub::ui::getInt("fps_target", 240), 60, 360);

    std::string text = enabled ? "FPS: " : "FPS: OFF (";
    text += std::to_string(target);
    if (!enabled) text += ")";

    statusTop->setString(text.c_str());
    statusTop->setColor(enabled ? ccc3(120, 240, 160) : ccc3(220, 120, 120));
}

} // namespace hub::ui::fps
