#pragma once

#include <Geode/Geode.hpp>

#include "HubPopup.hpp"
#include "UiHelpers.hpp"
#include "../bot/BotManager.hpp"

#include <string>

using namespace geode::prelude;

namespace hub::ui::bot {

inline void build(
    CCNode* content,
    CCMenu* menu,
    CCLabelBMFont*& statusTop,
    CCLabelBMFont*& statusBottom,
    CCObject* target
) {
    auto size = content->getContentSize();
    const float cx = size.width / 2.f;
    const float top = size.height;

    auto title = CCLabelBMFont::create("Bot / Macro", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({ 0.5f, 1.f });
    title->setPosition({ cx, top - 10.f });
    content->addChild(title);

    statusTop = CCLabelBMFont::create("BOT: IDLE", "goldFont.fnt");
    statusTop->setScale(0.50f);
    statusTop->setAnchorPoint({ 0.5f, 1.f });
    statusTop->setPosition({ cx, top - 50.f });
    content->addChild(statusTop);

    statusBottom = CCLabelBMFont::create("Frames: 0", "chatFont.fnt");
    statusBottom->setScale(0.65f);
    statusBottom->setAnchorPoint({ 0.5f, 1.f });
    statusBottom->setPosition({ cx, top - 80.f });
    statusBottom->setColor({ 200, 220, 240 });
    content->addChild(statusBottom);

    auto row1 = CCMenu::create();
    row1->setContentSize({ size.width - 30.f, 44.f });
    row1->setPosition({ cx, top - 130.f });
    row1->setLayout(RowLayout::create()->setGap(10.f));
    content->addChild(row1);

    row1->addChild(hub::ui::makeBtn("REC",  target, menu_selector(HubPopup::onRecord), 0.7f, 70.f, "GJ_button_06.png"));
    row1->addChild(hub::ui::makeBtn("PLAY", target, menu_selector(HubPopup::onPlay),   0.7f, 70.f, "GJ_button_01.png"));
    row1->addChild(hub::ui::makeBtn("STOP", target, menu_selector(HubPopup::onStop),   0.7f, 70.f, "GJ_button_03.png"));
    row1->updateLayout();

    auto row2 = CCMenu::create();
    row2->setContentSize({ size.width - 30.f, 44.f });
    row2->setPosition({ cx, top - 185.f });
    row2->setLayout(RowLayout::create()->setGap(10.f));
    content->addChild(row2);

    row2->addChild(hub::ui::makeBtn("Save",  target, menu_selector(HubPopup::onSave),  0.65f, 70.f));
    row2->addChild(hub::ui::makeBtn("Load",  target, menu_selector(HubPopup::onLoad),  0.65f, 70.f));
    row2->addChild(hub::ui::makeBtn("Clear", target, menu_selector(HubPopup::onClear), 0.65f, 70.f, "GJ_button_06.png"));
    row2->updateLayout();

    auto note = CCLabelBMFont::create(
        "Macro is saved into mod storage as macro.txt",
        "chatFont.fnt"
    );
    note->setScale(0.55f);
    note->setAnchorPoint({ 0.5f, 1.f });
    note->setPosition({ cx, top - 235.f });
    note->setColor({ 170, 190, 210 });
    content->addChild(note);
}

inline void refresh(CCLabelBMFont* statusTop, CCLabelBMFont* statusBottom) {
    auto& bot = BotManager::shared();

    if (statusTop) {
        char const* state = bot.isRecording() ? "RECORDING"
                          : bot.isPlaying()   ? "PLAYING"
                                              : "IDLE";
        std::string text = "BOT: ";
        text += state;
        statusTop->setString(text.c_str());

        if (bot.isRecording())      statusTop->setColor({ 240, 100, 100 });
        else if (bot.isPlaying())   statusTop->setColor({ 120, 240, 160 });
        else                        statusTop->setColor({ 230, 220, 120 });
    }

    if (statusBottom) {
        std::string text = "Frames: ";
        text += std::to_string(bot.macro().size());
        statusBottom->setString(text.c_str());
    }
}

} // namespace hub::ui::bot
