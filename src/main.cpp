#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

using namespace geode::prelude;

// =======================
// HUB POPUP SIMPLE
// =======================

class HubLayer : public CCLayer {
public:
    static HubLayer* create() {
        auto ret = new HubLayer();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() {
        if (!CCLayer::init()) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto bg = CCLayerColor::create(ccc4(0, 0, 0, 150));
        this->addChild(bg);

        auto panel = CCLayerColor::create(ccc4(20, 20, 20, 255));
        panel->setContentSize({300, 200});
        panel->setPosition({win.width/2 - 150, win.height/2 - 100});
        this->addChild(panel);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        panel->addChild(menu);

        auto close = CCMenuItemFont::create("X", this, menu_selector(HubLayer::onClose));
        close->setPosition({280, 180});
        menu->addChild(close);

        auto fpsUp = CCMenuItemFont::create("+ FPS", this, menu_selector(HubLayer::onFpsUp));
        fpsUp->setPosition({150, 120});
        menu->addChild(fpsUp);

        auto fpsDown = CCMenuItemFont::create("- FPS", this, menu_selector(HubLayer::onFpsDown));
        fpsDown->setPosition({150, 80});
        menu->addChild(fpsDown);

        return true;
    }

    void onClose(CCObject*) {
        this->removeFromParent();
    }

    void onFpsUp(CCObject*) {
        auto mod = Mod::get();
        int fps = mod->getSettingValue<int>("fps_target");
        mod->setSettingValue<int>("fps_target", std::min(fps + 30, 360));
    }

    void onFpsDown(CCObject*) {
        auto mod = Mod::get();
        int fps = mod->getSettingValue<int>("fps_target");
        mod->setSettingValue<int>("fps_target", std::max(fps - 30, 60));
    }
};

// =======================
// BOTÓN HUB
// =======================

void addButton(CCLayer* layer, CCObject* target, SEL_MenuHandler cb, CCPoint pos) {
    auto menu = CCMenu::create();
    menu->setPosition({0, 0});

    auto spr = CCSprite::create("GJ_plusBtn_001.png"); // seguro existente
    spr->setScale(0.8f);

    auto btn = CCMenuItemSpriteExtra::create(spr, target, cb);
    btn->setPosition(pos);

    menu->addChild(btn);
    layer->addChild(menu, 999);
}

// =======================
// MENU PRINCIPAL
// =======================

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();

        addButton(this, this, menu_selector(MyMenuLayer::onHub),
            {win.width - 30, win.height - 30});

        return true;
    }

    void onHub(CCObject*) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        scene->addChild(HubLayer::create(), 999);
    }
};

// =======================
// PAUSE (FIX CRASH)
// =======================

class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto win = CCDirector::sharedDirector()->getWinSize();

        addButton(this, this, menu_selector(MyPauseLayer::onHub),
            {win.width * 0.75f, win.height * 0.55f});
    }

    void onHub(CCObject*) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        scene->addChild(HubLayer::create(), 999);
    }
};

// =======================
// FPS REAL
// =======================

class $modify(MyGameManager, GameManager) {
    void update(float dt) {
        auto mod = Mod::get();

        int fps = mod->getSettingValue<int>("fps_target");

        this->m_customFPSTarget = fps;
        CCDirector::sharedDirector()->setAnimationInterval(1.0 / fps);

        GameManager::update(dt);
    }
};

// =======================
// FIX BOT / INPUT BASE
// =======================

class $modify(MyPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton btn) {
        return PlayerObject::pushButton(btn);
    }

    bool releaseButton(PlayerButton btn) {
        return PlayerObject::releaseButton(btn);
    }
};

// =======================
// PLAYLAYER SAFE
// =======================

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
    }
};
