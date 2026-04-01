#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <Geode/ui/BasedButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include "bot/BotManager.hpp"

#include <algorithm>
#include <string>

using namespace geode::prelude;
using namespace bot;

namespace {
constexpr int kHubButtonTag = 0x5001;

template <typename T>
T clampValue(T value, T low, T high) {
    return std::max(low, std::min(value, high));
}

class HubOverlay;
static HubOverlay* s_open = nullptr;

class HubOverlay : public CCLayer {
protected:
    int m_tab = 0;
    CCLayer* m_content = nullptr;
    CCLabelBMFont* m_fpsStatus = nullptr;
    CCLabelBMFont* m_botStatus = nullptr;
    CCLabelBMFont* m_straightStatus = nullptr;
    CCLabelBMFont* m_waveStatus = nullptr;
    CCLabelBMFont* m_cbfStatus = nullptr;

    CCMenuItemSpriteExtra* makeButton(char const* text, SEL_MenuHandler callback, float scale = 0.40f) {
        auto sprite = ButtonSprite::create(text);
        sprite->setScale(scale);
        return CCMenuItemSpriteExtra::create(sprite, this, callback);
    }

    void refresh() {
        auto mod = Mod::get();
        if (!mod) {
            return;
        }

        bool fpsEnabled = mod->hasSetting("fps_enabled") ? mod->getSettingValue<bool>("fps_enabled") : true;
        int fpsTarget = mod->hasSetting("fps_target") ? clampValue(mod->getSettingValue<int>("fps_target"), 60, 360) : 240;
        bool straightEnabled = mod->hasSetting("straight_enabled") ? mod->getSettingValue<bool>("straight_enabled") : true;
        int straightStrength = mod->hasSetting("straight_strength") ? clampValue(mod->getSettingValue<int>("straight_strength"), 1, 50) : 18;
        bool waveEnabled = mod->hasSetting("wave_enabled") ? mod->getSettingValue<bool>("wave_enabled") : true;
        int waveStrength = mod->hasSetting("wave_strength") ? clampValue(mod->getSettingValue<int>("wave_strength"), 1, 50) : 20;
        int cbfBoost = mod->hasSetting("cbf_boost") ? clampValue(mod->getSettingValue<int>("cbf_boost"), 1, 10) : 3;

        if (m_fpsStatus) {
            m_fpsStatus->setString(
                (std::string("FPS: ") + (fpsEnabled ? "ON" : "OFF") + " | Target: " + std::to_string(fpsTarget)).c_str()
            );
        }

        if (m_botStatus) {
            auto const& macro = BotManager::shared().macro();
            std::string state = "BOT: ";
            state += BotManager::shared().isRecording() ? "RECORDING" : (BotManager::shared().isPlaying() ? "PLAYING" : "IDLE");
            state += " | Frames: ";
            state += std::to_string(macro.size());
            state += " | Step: ";
            state += BotManager::shared().isFrameStepEnabled() ? "ON" : "OFF";
            m_botStatus->setString(state.c_str());
        }

        if (m_straightStatus) {
            m_straightStatus->setString(
                (std::string("Straight: ") + (straightEnabled ? "ON" : "OFF") + " | Power: " + std::to_string(straightStrength)).c_str()
            );
        }

        if (m_waveStatus) {
            m_waveStatus->setString(
                (std::string("Wave: ") + (waveEnabled ? "ON" : "OFF") + " | Power: " + std::to_string(waveStrength)).c_str()
            );
        }

        if (m_cbfStatus) {
            m_cbfStatus->setString(
                (std::string("CBF Boost: ") + std::to_string(cbfBoost)).c_str()
            );
        }
    }

    void rebuild() {
        if (!m_content) {
            return;
        }

        m_content->removeAllChildrenWithCleanup(true);

        m_fpsStatus = nullptr;
        m_botStatus = nullptr;
        m_straightStatus = nullptr;
        m_waveStatus = nullptr;
        m_cbfStatus = nullptr;

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        m_content->addChild(menu);

        if (m_tab == 0) {
            buildFpsTab(menu);
        }
        else {
            buildBotTab(menu);
        }

        refresh();
    }

    void buildFpsTab(CCMenu* menu) {
        m_fpsStatus = CCLabelBMFont::create("", "bigFont.fnt");
        m_fpsStatus->setScale(0.38f);
        m_fpsStatus->setPosition(CCPointMake(195.f, 520.f));
        m_content->addChild(m_fpsStatus);

        auto toggle = makeButton("Toggle", menu_selector(HubOverlay::onFpsToggle), 0.36f);
        toggle->setPosition(CCPointMake(195.f, 460.f));
        menu->addChild(toggle);

        auto minus = makeButton("-", menu_selector(HubOverlay::onFpsMinus), 0.45f);
        minus->setPosition(CCPointMake(135.f, 400.f));
        menu->addChild(minus);

        auto plus = makeButton("+", menu_selector(HubOverlay::onFpsPlus), 0.45f);
        plus->setPosition(CCPointMake(255.f, 400.f));
        menu->addChild(plus);

        auto def = makeButton("Default", menu_selector(HubOverlay::onFpsDefault), 0.32f);
        def->setPosition(CCPointMake(195.f, 340.f));
        menu->addChild(def);
    }

    void buildBotTab(CCMenu* menu) {
        m_botStatus = CCLabelBMFont::create("", "bigFont.fnt");
        m_botStatus->setScale(0.34f);
        m_botStatus->setPosition(CCPointMake(195.f, 530.f));
        m_content->addChild(m_botStatus);

        m_straightStatus = CCLabelBMFont::create("", "bigFont.fnt");
        m_straightStatus->setScale(0.30f);
        m_straightStatus->setPosition(CCPointMake(195.f, 500.f));
        m_content->addChild(m_straightStatus);

        m_waveStatus = CCLabelBMFont::create("", "bigFont.fnt");
        m_waveStatus->setScale(0.30f);
        m_waveStatus->setPosition(CCPointMake(195.f, 470.f));
        m_content->addChild(m_waveStatus);

        m_cbfStatus = CCLabelBMFont::create("", "bigFont.fnt");
        m_cbfStatus->setScale(0.30f);
        m_cbfStatus->setPosition(CCPointMake(195.f, 440.f));
        m_content->addChild(m_cbfStatus);

        auto record = makeButton("Record", menu_selector(HubOverlay::onRecord), 0.34f);
        record->setPosition(CCPointMake(90.f, 380.f));
        menu->addChild(record);

        auto play = makeButton("Play", menu_selector(HubOverlay::onPlay), 0.34f);
        play->setPosition(CCPointMake(195.f, 380.f));
        menu->addChild(play);

        auto stop = makeButton("Stop", menu_selector(HubOverlay::onStop), 0.34f);
        stop->setPosition(CCPointMake(300.f, 380.f));
        menu->addChild(stop);

        auto save = makeButton("Save", menu_selector(HubOverlay::onSave), 0.34f);
        save->setPosition(CCPointMake(90.f, 330.f));
        menu->addChild(save);

        auto load = makeButton("Load", menu_selector(HubOverlay::onLoad), 0.34f);
        load->setPosition(CCPointMake(195.f, 330.f));
        menu->addChild(load);

        auto step = makeButton("Step", menu_selector(HubOverlay::onStep), 0.34f);
        step->setPosition(CCPointMake(300.f, 330.f));
        menu->addChild(step);

        auto frameStep = makeButton("FrameStep", menu_selector(HubOverlay::onFrameStepToggle), 0.31f);
        frameStep->setPosition(CCPointMake(90.f, 280.f));
        menu->addChild(frameStep);

        auto straight = makeButton("Straight", menu_selector(HubOverlay::onStraightToggle), 0.32f);
        straight->setPosition(CCPointMake(195.f, 280.f));
        menu->addChild(straight);

        auto wave = makeButton("Wave", menu_selector(HubOverlay::onWaveToggle), 0.32f);
        wave->setPosition(CCPointMake(300.f, 280.f));
        menu->addChild(wave);

        auto straightMinus = makeButton("Straight-", menu_selector(HubOverlay::onStraightMinus), 0.28f);
        straightMinus->setPosition(CCPointMake(90.f, 230.f));
        menu->addChild(straightMinus);

        auto straightPlus = makeButton("Straight+", menu_selector(HubOverlay::onStraightPlus), 0.28f);
        straightPlus->setPosition(CCPointMake(195.f, 230.f));
        menu->addChild(straightPlus);

        auto cbfMinus = makeButton("CBF-", menu_selector(HubOverlay::onCbfMinus), 0.30f);
        cbfMinus->setPosition(CCPointMake(300.f, 230.f));
        menu->addChild(cbfMinus);

        auto cbfPlus = makeButton("CBF+", menu_selector(HubOverlay::onCbfPlus), 0.30f);
        cbfPlus->setPosition(CCPointMake(90.f, 180.f));
        menu->addChild(cbfPlus);

        auto clear = makeButton("Clear", menu_selector(HubOverlay::onClear), 0.34f);
        clear->setPosition(CCPointMake(195.f, 180.f));
        menu->addChild(clear);

        auto close = makeButton("Close", menu_selector(HubOverlay::onClose), 0.34f);
        close->setPosition(CCPointMake(300.f, 180.f));
        menu->addChild(close);
    }

public:
    static HubOverlay* create() {
        auto ret = new HubOverlay();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) {
            return false;
        }

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto bg = CCLayerColor::create(ccc4(0, 0, 0, 150));
        addChild(bg);

        auto panel = CCLayerColor::create(ccc4(20, 20, 20, 240), 390.f, 640.f);
        panel->setPosition(CCPointMake((win.width - 390.f) / 2.f, (win.height - 640.f) / 2.f));
        addChild(panel);

        auto title = CCLabelBMFont::create("PRIVATE HUB", "bigFont.fnt");
        title->setScale(0.65f);
        title->setPosition(CCPointMake(195.f, 610.f));
        panel->addChild(title);

        auto tabs = CCMenu::create();
        tabs->setPosition(CCPointZero);
        panel->addChild(tabs);

        auto fpsTab = makeButton("FPS", menu_selector(HubOverlay::onTabFps), 0.38f);
        fpsTab->setPosition(CCPointMake(95.f, 575.f));
        tabs->addChild(fpsTab);

        auto botTab = makeButton("BOT", menu_selector(HubOverlay::onTabBot), 0.38f);
        botTab->setPosition(CCPointMake(295.f, 575.f));
        tabs->addChild(botTab);

        m_content = CCLayer::create();
        m_content->setPosition(CCPointZero);
        panel->addChild(m_content);

        rebuild();
        return true;
    }

    void onExit() override {
        if (s_open == this) {
            s_open = nullptr;
        }
        CCLayer::onExit();
    }

    void onTabFps(CCObject*) {
        m_tab = 0;
        rebuild();
    }

    void onTabBot(CCObject*) {
        m_tab = 1;
        rebuild();
    }

    void onFpsToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_enabled")) return;
        mod->setSettingValue<bool>("fps_enabled", !mod->getSettingValue<bool>("fps_enabled"));
        refresh();
    }

    void onFpsMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) return;
        mod->setSettingValue<int>("fps_target", clampValue(mod->getSettingValue<int>("fps_target") - 30, 60, 360));
        refresh();
    }

    void onFpsPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) return;
        mod->setSettingValue<int>("fps_target", clampValue(mod->getSettingValue<int>("fps_target") + 30, 60, 360));
        refresh();
    }

    void onFpsDefault(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) return;
        mod->setSettingValue<int>("fps_target", 240);
        refresh();
    }

    void onRecord(CCObject*) { BotManager::shared().startRecording(); refresh(); }
    void onPlay(CCObject*) { BotManager::shared().startPlaying(); refresh(); }
    void onStop(CCObject*) { BotManager::shared().stopRecording(); BotManager::shared().stopPlaying(); refresh(); }
    void onSave(CCObject*) { BotManager::shared().saveMacro(); refresh(); }
    void onLoad(CCObject*) { BotManager::shared().loadMacro(); refresh(); }
    void onStep(CCObject*) { BotManager::shared().stepOneFrame(); refresh(); }
    void onFrameStepToggle(CCObject*) { BotManager::shared().toggleFrameStepper(); refresh(); }

    void onStraightToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("straight_enabled")) return;
        mod->setSettingValue<bool>("straight_enabled", !mod->getSettingValue<bool>("straight_enabled"));
        refresh();
    }

    void onWaveToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("wave_enabled")) return;
        mod->setSettingValue<bool>("wave_enabled", !mod->getSettingValue<bool>("wave_enabled"));
        refresh();
    }

    void onStraightMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("straight_strength")) return;
        mod->setSettingValue<int>("straight_strength", clampValue(mod->getSettingValue<int>("straight_strength") - 1, 1, 50));
        refresh();
    }

    void onStraightPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("straight_strength")) return;
        mod->setSettingValue<int>("straight_strength", clampValue(mod->getSettingValue<int>("straight_strength") + 1, 1, 50));
        refresh();
    }

    void onCbfMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("cbf_boost")) return;
        mod->setSettingValue<int>("cbf_boost", clampValue(mod->getSettingValue<int>("cbf_boost") - 1, 1, 10));
        refresh();
    }

    void onCbfPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("cbf_boost")) return;
        mod->setSettingValue<int>("cbf_boost", clampValue(mod->getSettingValue<int>("cbf_boost") + 1, 1, 10));
        refresh();
    }

    void onClear(CCObject*) {
        BotManager::shared().clearMacro();
        refresh();
    }

    void onClose(CCObject*) {
        HubOverlay::toggle();
    }

    static void toggle() {
        if (s_open && s_open->getParent()) {
            s_open->removeFromParentAndCleanup(true);
            s_open = nullptr;
            return;
        }

        auto play = PlayLayer::get();
        if (!play) return;

        s_open = HubOverlay::create();
        play->addChild(s_open, 99999);
    }
};

class HubButtonLayer : public CCLayer {
public:
    static HubButtonLayer* create() {
        auto ret = new HubButtonLayer();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) {
            return false;
        }

        auto win = CCDirector::sharedDirector()->getWinSize();

        auto icon = CCSprite::create("icon.png"_spr);
        if (!icon) {
            log::error("Missing resource sprite: resources/icon.png");
            return false;
        }

        icon->setScale(0.42f);

        auto button = CCMenuItemSpriteExtra::create(
            icon,
            this,
            menu_selector(HubButtonLayer::onButton)
        );

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        menu->addChild(button);

        button->setPosition(CCPointMake(win.width - 30.f, win.height * 0.62f));
        addChild(menu, 1);

        return true;
    }

    void onButton(CCObject*) {
        HubOverlay::toggle();
    }
};

class $modify(MyGameManager, GameManager) {
    void update(float dt) {
        auto mod = Mod::get();
        if (mod && mod->hasSetting("fps_enabled") && mod->getSettingValue<bool>("fps_enabled")) {
            auto target = clampValue(mod->hasSetting("fps_target") ? mod->getSettingValue<int>("fps_target") : 240, 60, 360);
            this->m_customFPSTarget = static_cast<float>(target);
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / static_cast<double>(target));
        } else {
            this->m_customFPSTarget = 60.0f;
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / 60.0);
        }

        GameManager::update(dt);
    }
};

class $modify(MyPlayerObject, PlayerObject) {
    void update(float dt) {
        auto& bot = BotManager::shared();
        bot.attachPlayer(this);

        if (bot.activePlayer() != this) {
            PlayerObject::update(dt);
            return;
        }

        if (!bot.allowPlayerUpdate()) {
            return;
        }

        bot.update(dt);
        PlayerObject::update(dt);

        auto mod = Mod::get();
        if (!mod || this->m_isDead) {
            return;
        }

        bool straightEnabled = mod->hasSetting("straight_enabled") ? mod->getSettingValue<bool>("straight_enabled") : true;
        bool waveEnabled = mod->hasSetting("wave_enabled") ? mod->getSettingValue<bool>("wave_enabled") : true;

        int straightStrength = mod->hasSetting("straight_strength") ? clampValue(mod->getSettingValue<int>("straight_strength"), 1, 50) : 18;
        int waveStrength = mod->hasSetting("wave_strength") ? clampValue(mod->getSettingValue<int>("wave_strength"), 1, 50) : 20;

        if (this->m_isShip && straightEnabled) {
            auto blend = static_cast<double>(straightStrength) / 100.0;
            auto target = this->playerIsMovingUp() ? 8.0 : -8.0;

            this->m_yVelocity += (target - this->m_yVelocity) * blend;
            this->setYVelocity(this->m_yVelocity, 0);

            this->m_rotationSpeed *= 0.96f;
            this->m_rotateSpeed *= 0.96f;
        }

        if ((this->m_isBird || this->m_isDart) && waveEnabled) {
            auto blend = static_cast<double>(waveStrength) / 100.0;
            auto target = this->playerIsMovingUp() ? 8.5 : -8.5;

            this->m_yVelocity += (target - this->m_yVelocity) * blend;
            this->setYVelocity(this->m_yVelocity, 0);

            this->m_rotationSpeed *= 0.96f;
            this->m_rotateSpeed *= 0.96f;
        }
    }

    bool pushButton(PlayerButton button) {
        auto& bot = BotManager::shared();
        if (bot.isRecording() && !bot.isSyntheticInput()) {
            bot.recordEvent(static_cast<int>(button), true);
        }
        return PlayerObject::pushButton(button);
    }

    bool releaseButton(PlayerButton button) {
        auto& bot = BotManager::shared();
        if (bot.isRecording() && !bot.isSyntheticInput()) {
            bot.recordEvent(static_cast<int>(button), false);
        }
        return PlayerObject::releaseButton(button);
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void onEnterTransitionDidFinish() {
        PlayLayer::onEnterTransitionDidFinish();

        BotManager::shared().resetSession();

        if (!this->getChildByTag(kHubButtonTag)) {
            auto layer = HubButtonLayer::create();
            layer->setTag(kHubButtonTag);
            this->addChild(layer, 99999);
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        BotManager::shared().resetSession();
    }

    void onExit() {
        HubOverlay::toggle();
        BotManager::shared().resetSession();
        PlayLayer::onExit();
    }
};
}
