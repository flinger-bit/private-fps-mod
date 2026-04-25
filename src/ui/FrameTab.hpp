#pragma once

#include <Geode/Geode.hpp>

#include "HubPopup.hpp"
#include "UiHelpers.hpp"
#include "../bot/BotManager.hpp"

#include <string>

using namespace geode::prelude;

namespace hub::ui::frame {

inline void build(
    CCNode* content,
    CCMenu* menu,
    CCLabelBMFont*& statusTop,
    CCObject* target
) {
    auto size = content->getContentSize();
    const float cx = size.width / 2.f;
    const float top = size.height;

    auto title = CCLabelBMFont::create("Frame Stepper", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({ 0.5f, 1.f });
    title->setPosition({ cx, top - 10.f });
    content->addChild(title);

    statusTop = CCLabelBMFont::create("Step: OFF", "goldFont.fnt");
    statusTop->setScale(0.55f);
    statusTop->setAnchorPoint({ 0.5f, 1.f });
    statusTop->setPosition({ cx, top - 50.f });
    content->addChild(statusTop);

    auto row = CCMenu::create();
    row->setContentSize({ size.width - 30.f, 44.f });
    row->setPosition({ cx, top - 120.f });
    row->setLayout(RowLayout::create()->setGap(12.f));
    content->addChild(row);

    row->addChild(hub::ui::makeBtn("Toggle", target, menu_selector(HubPopup::onFrameStepToggle), 0.7f, 90.f));
    row->addChild(hub::ui::makeBtn("Step",   target, menu_selector(HubPopup::onStep),            0.7f, 90.f, "GJ_button_01.png"));
    row->updateLayout();

    auto row2 = CCMenu::create();
    row2->setContentSize({ size.width - 30.f, 44.f });
    row2->setPosition({ cx, top - 180.f });
    row2->setLayout(RowLayout::create()->setGap(12.f));
    content->addChild(row2);

    row2->addChild(hub::ui::makeBtn("Ignore inputs", target, menu_selector(HubPopup::onIgnoreToggle), 0.55f, 140.f));
    row2->updateLayout();

    auto note = CCLabelBMFont::create(
        "When enabled, gameplay is paused\nuntil you press Step.",
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
    auto& bot = BotManager::shared();

    bool stepOn   = bot.isFrameStepEnabled();
    bool ignoreOn = hub::ui::getBool("ignore_inputs", true);

    std::string text = "Step: ";
    text += stepOn ? "ON" : "OFF";
    text += "  |  Ignore: ";
    text += ignoreOn ? "ON" : "OFF";

    statusTop->setString(text.c_str());
    statusTop->setColor(stepOn ? ccc3(120, 220, 255) : ccc3(220, 220, 220));
}

} // namespace hub::ui::frame
