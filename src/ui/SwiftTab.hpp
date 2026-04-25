#pragma once

#include <Geode/Geode.hpp>

#include "HubPopup.hpp"
#include "UiHelpers.hpp"

#include <string>

using namespace geode::prelude;

namespace hub::ui::swift {

inline void build(
    CCNode* content,
    CCMenu* menu,
    CCLabelBMFont*& statusTop,
    CCObject* target
) {
    auto size = content->getContentSize();
    const float cx = size.width / 2.f;
    const float top = size.height;

    auto title = CCLabelBMFont::create("Swift Clicks", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({ 0.5f, 1.f });
    title->setPosition({ cx, top - 10.f });
    content->addChild(title);

    statusTop = CCLabelBMFont::create("Swift: --", "goldFont.fnt");
    statusTop->setScale(0.55f);
    statusTop->setAnchorPoint({ 0.5f, 1.f });
    statusTop->setPosition({ cx, top - 50.f });
    content->addChild(statusTop);

    auto row1 = CCMenu::create();
    row1->setContentSize({ size.width - 30.f, 44.f });
    row1->setPosition({ cx, top - 110.f });
    row1->setLayout(RowLayout::create()->setGap(12.f));
    content->addChild(row1);

    row1->addChild(hub::ui::makeBtn("Toggle", target, menu_selector(HubPopup::onSwiftToggle), 0.7f, 110.f));
    row1->updateLayout();

    auto row2 = CCMenu::create();
    row2->setContentSize({ size.width - 30.f, 44.f });
    row2->setPosition({ cx, top - 175.f });
    row2->setLayout(RowLayout::create()->setGap(10.f));
    content->addChild(row2);

    row2->addChild(hub::ui::makeBtn("-1",     target, menu_selector(HubPopup::onSwiftMinus),   0.7f, 50.f, "GJ_button_06.png"));
    row2->addChild(hub::ui::makeBtn("Default",target, menu_selector(HubPopup::onSwiftDefault), 0.6f, 80.f));
    row2->addChild(hub::ui::makeBtn("+1",     target, menu_selector(HubPopup::onSwiftPlus),    0.7f, 50.f, "GJ_button_01.png"));
    row2->updateLayout();

    auto note = CCLabelBMFont::create(
        "Range 1 - 60. Use the Geode settings\nfor the slider control.",
        "chatFont.fnt"
    );
    note->setScale(0.55f);
    note->setAnchorPoint({ 0.5f, 1.f });
    note->setAlignment(kCCTextAlignmentCenter);
    note->setPosition({ cx, top - 235.f });
    note->setColor({ 170, 190, 210 });
    content->addChild(note);
}

inline void refresh(CCLabelBMFont* statusTop) {
    if (!statusTop) return;
    bool enabled = hub::ui::getBool("swift_enabled", true);
    int  count   = hub::ui::clampInt(hub::ui::getInt("swift_clicks", 20), 1, 60);

    std::string text = "Swift: ";
    text += enabled ? "ON" : "OFF";
    text += "  |  Count: ";
    text += std::to_string(count);

    statusTop->setString(text.c_str());
    statusTop->setColor(enabled ? ccc3(120, 240, 200) : ccc3(220, 120, 120));
}

} // namespace hub::ui::swift
