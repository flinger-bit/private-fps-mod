#pragma once

#include <Geode/Geode.hpp>

#include "HubPopup.hpp"
#include "UiHelpers.hpp"

using namespace geode::prelude;

namespace hub::ui::about {

inline void build(CCNode* content, CCMenu* /*menu*/, CCObject* /*target*/) {
    auto size = content->getContentSize();
    const float cx = size.width / 2.f;
    const float top = size.height;

    auto title = CCLabelBMFont::create("Private FPS Pro", "bigFont.fnt");
    title->setScale(0.55f);
    title->setAnchorPoint({ 0.5f, 1.f });
    title->setPosition({ cx, top - 10.f });
    content->addChild(title);

    auto version = CCLabelBMFont::create("v2.1.0  -  by fling", "goldFont.fnt");
    version->setScale(0.45f);
    version->setAnchorPoint({ 0.5f, 1.f });
    version->setPosition({ cx, top - 50.f });
    content->addChild(version);

    auto body = CCLabelBMFont::create(
        "FPS unlocker, bot recorder, frame stepper\n"
        "and swift clicks for Geometry Dash 2.2.\n\n"
        "Tabs use a Cocos sidebar layout with\n"
        "ButtonSprite + AxisLayout, drawn over a\n"
        "CCScale9Sprite panel.\n\n"
        "Sliders for the int settings live in the\n"
        "Geode settings page.",
        "chatFont.fnt"
    );
    body->setScale(0.6f);
    body->setAnchorPoint({ 0.5f, 1.f });
    body->setAlignment(kCCTextAlignmentCenter);
    body->setPosition({ cx, top - 90.f });
    body->setColor({ 200, 220, 240 });
    content->addChild(body);
}

} // namespace hub::ui::about
