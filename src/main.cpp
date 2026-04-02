#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include "ui/HubPopup.hpp"

using namespace geode::prelude;

namespace {
    void addHubButtonToLayer(
        CCLayer* layer,
        CCObject* target,
        SEL_MenuHandler callback,
        CCPoint position,
        float scale = 0.24f
    ) {
        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);

        auto sprite = CCSprite::create("icon.png"_spr);
        if (!sprite) {
            log::error("Missing sprite resource: resources/icon.png");
            return;
        }

        sprite->setScale(scale);

        auto button = CCMenuItemSpriteExtra::create(sprite, target, callback);
        button->setPosition(position);
        menu->addChild(button);

        layer->addChild(menu, 99999);
    }
}

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto win = CCDirector::sharedDirector()->getWinSize();

        addHubButtonToLayer(
            this,
            this,
            menu_selector(MyPauseLayer::onHub),
            CCPointMake(win.width * 0.77f, win.height * 0.58f),
            0.24f
        );
    }

    void onHub(CCObject*) {
        HubPopup::toggle();
    }
};
