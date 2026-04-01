#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

#include "bot/BotManager.hpp"

#include <algorithm>
#include <string>

using namespace geode::prelude;
using namespace hub;

namespace {
constexpr int kHubPopupTag = 0x5448;
constexpr int kHubButtonTag = 0x5449;

template <typename T>
T clampValue(T value, T low, T high) {
    return std::max(low, std::min(value, high));
}

CCMenuItemSpriteExtra* makeTextButton(char const* text, CCObject* target, SEL_MenuHandler callback, float scale = 0.32f) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(scale);
    return CCMenuItemSpriteExtra::create(label, target, callback);
}

CCMenuItemSpriteExtra* makeIconButton(CCObject* target, SEL_MenuHandler callback, float scale = 0.42f) {
    auto sprite = CCSprite::create("icon.png"_spr);
    if (!sprite) {
        log::error("Missing sprite resource: resources/icon.png");
        return nullptr;
    }

    sprite->setScale(scale);
    return CCMenuItemSpriteExtra::create(sprite, target, callback);
}

class HubPopup;
static HubPopup* s_popup = nullptr;

class HubPopup : public CCLayer {
protected:
    int m_tab = 0;
    CCLayer* m_content = nullptr;
    CCLabelBMFont* m_statusA = nullptr;
    CCLabelBMFont* m_statusB = nullptr;

    CCMenuItemSpriteExtra* button(char const* text, SEL_MenuHandler cb, float scale = 0.30f) {
        return makeTextButton(text, this, cb, scale);
    }

    void refreshLabels() {
        auto mod = Mod::get();
        if (!mod) {
            return;
        }

        if (m_tab == 0 && m_statusA) {
            bool enabled = mod->getSettingValue<bool>("fps_enabled");
            int target = clampValue(mod->getSettingValue<int>("fps_target"), 60, 360);
            m_statusA->setString(
                (std::string("FPS: ") + (enabled ? "ON" : "OFF") + " | Target: " + std::to_string(target)).c_str()
            );
        }

        if (m_tab == 1 && m_statusA && m_statusB) {
            auto& bot = BotManager::shared();
            std::string mode = "BOT: ";
            mode += bot.isRecording() ? "RECORDING" : (bot.isPlaying() ? "PLAYING" : "IDLE");
            mode += " | Frames: ";
            mode += std::to_string(bot.macro().size());

            std::string flags = "Ignore: ";
            flags += mod->getSettingValue<bool>("ignore_inputs") ? "ON" : "OFF";
            flags += " | FrameStep: ";
            flags += mod->getSettingValue<bool>("frame_stepper") ? "ON" : "OFF";
            flags += " | CBF: ";
            flags += std::to_string(clampValue(mod->getSettingValue<int>("cbf_boost"), 1, 10));

            m_statusA->setString(mode.c_str());
            m_statusB->setString(flags.c_str());
        }
    }

    void rebuild() {
        if (!m_content) {
            return;
        }

        m_content->removeAllChildrenWithCleanup(true);
        m_statusA = nullptr;
        m_statusB = nullptr;

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        m_content->addChild(menu);

        if (m_tab == 0) {
            buildFpsTab(menu);
        } else {
            buildBotTab(menu);
        }

        refreshLabels();
    }

    void buildFpsTab(CCMenu* menu) {
        m_statusA = CCLabelBMFont::create("", "bigFont.fnt");
        m_statusA->setScale(0.28f);
        m_statusA->setPosition(CCPointMake(180.f, 265.f));
        m_content->addChild(m_statusA);

        auto toggle = button("Toggle", menu_selector(HubPopup::onFpsToggle), 0.30f);
        toggle->setPosition(CCPointMake(180.f, 220.f));
        menu->addChild(toggle);

        auto minus = button("-", menu_selector(HubPopup::onFpsMinus), 0.36f);
        minus->setPosition(CCPointMake(115.f, 165.f));
        menu->addChild(minus);

        auto plus = button("+", menu_selector(HubPopup::onFpsPlus), 0.36f);
        plus->setPosition(CCPointMake(245.f, 165.f));
        menu->addChild(plus);

        auto def = button("Default", menu_selector(HubPopup::onFpsDefault), 0.28f);
        def->setPosition(CCPointMake(180.f, 115.f));
        menu->addChild(def);
    }

    void buildBotTab(CCMenu* menu) {
        m_statusA = CCLabelBMFont::create("", "bigFont.fnt");
        m_statusA->setScale(0.24f);
        m_statusA->setPosition(CCPointMake(180.f, 275.f));
        m_content->addChild(m_statusA);

        m_statusB = CCLabelBMFont::create("", "bigFont.fnt");
        m_statusB->setScale(0.22f);
        m_statusB->setPosition(CCPointMake(180.f, 250.f));
        m_content->addChild(m_statusB);

        auto rec = button("REC", menu_selector(HubPopup::onRecordToggle), 0.26f);
        rec->setPosition(CCPointMake(90.f, 200.f));
        menu->addChild(rec);

        auto play = button("PLAY", menu_selector(HubPopup::onPlayToggle), 0.26f);
        play->setPosition(CCPointMake(180.f, 200.f));
        menu->addChild(play);

        auto load = button("LOAD", menu_selector(HubPopup::onLoad), 0.26f);
        load->setPosition(CCPointMake(270.f, 200.f));
        menu->addChild(load);

        auto save = button("SAVE", menu_selector(HubPopup::onSave), 0.26f);
        save->setPosition(CCPointMake(90.f, 150.f));
        menu->addChild(save);

        auto ignore = button("IGNORE", menu_selector(HubPopup::onIgnoreToggle), 0.24f);
        ignore->setPosition(CCPointMake(180.f, 150.f));
        menu->addChild(ignore);

        auto frame = button("FRAME", menu_selector(HubPopup::onFrameStepToggle), 0.24f);
        frame->setPosition(CCPointMake(270.f, 150.f));
        menu->addChild(frame);

        auto step = button("STEP", menu_selector(HubPopup::onStepOneFrame), 0.26f);
        step->setPosition(CCPointMake(90.f, 100.f));
        menu->addChild(step);

        auto cbfMinus = button("CBF-", menu_selector(HubPopup::onCbfMinus), 0.24f);
        cbfMinus->setPosition(CCPointMake(180.f, 100.f));
        menu->addChild(cbfMinus);

        auto cbfPlus = button("CBF+", menu_selector(HubPopup::onCbfPlus), 0.24f);
        cbfPlus->setPosition(CCPointMake(270.f, 100.f));
        menu->addChild(cbfPlus);
    }

public:
    static HubPopup* create() {
        auto ret = new HubPopup();
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

        auto dim = CCLayerColor::create(ccc4(0, 0, 0, 150));
        addChild(dim);

        auto panel = CCLayerColor::create(ccc4(22, 22, 22, 245));
        panel->setContentSize(CCSizeMake(360.f, 390.f));
        panel->setPosition(CCPointMake((win.width - 360.f) / 2.f, (win.height - 390.f) / 2.f));
        addChild(panel);

        auto title = CCLabelBMFont::create("PRIVATE HUB", "bigFont.fnt");
        title->setScale(0.62f);
        title->setPosition(CCPointMake(180.f, 360.f));
        panel->addChild(title);

        auto tabs = CCMenu::create();
        tabs->setPosition(CCPointZero);
        panel->addChild(tabs);

        auto fpsTab = makeTextButton("FPS", this, menu_selector(HubPopup::onTabFps), 0.35f);
        fpsTab->setPosition(CCPointMake(90.f, 325.f));
        tabs->addChild(fpsTab);

        auto botTab = makeTextButton("BOT", this, menu_selector(HubPopup::onTabBot), 0.35f);
        botTab->setPosition(CCPointMake(270.f, 325.f));
        tabs->addChild(botTab);

        auto close = makeTextButton("X", this, menu_selector(HubPopup::onClose), 0.40f);
        close->setPosition(CCPointMake(332.f, 360.f));
        tabs->addChild(close);

        m_content = CCLayer::create();
        panel->addChild(m_content);

        rebuild();
        return true;
    }

    void onExit() override {
        if (s_popup == this) {
            s_popup = nullptr;
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

    void onClose(CCObject*) {
        if (s_popup) {
            s_popup->removeFromParentAndCleanup(true);
            s_popup = nullptr;
        }
    }

    void onFpsToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_enabled")) return;
        mod->setSettingValue<bool>("fps_enabled", !mod->getSettingValue<bool>("fps_enabled"));
        refreshLabels();
    }

    void onFpsMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) return;
        mod->setSettingValue<int>("fps_target", clampValue(mod->getSettingValue<int>("fps_target") - 30, 60, 360));
        refreshLabels();
    }

    void onFpsPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) return;
        mod->setSettingValue<int>("fps_target", clampValue(mod->getSettingValue<int>("fps_target") + 30, 60, 360));
        refreshLabels();
    }

    void onFpsDefault(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) return;
        mod->setSettingValue<int>("fps_target", 240);
        refreshLabels();
    }

    void onRecordToggle(CCObject*) {
        auto& bot = BotManager::shared();
        if (bot.isRecording()) {
            bot.stopRecording();
        } else {
            bot.startRecording();
        }
        refreshLabels();
    }

    void onPlayToggle(CCObject*) {
        auto& bot = BotManager::shared();
        if (bot.isPlaying()) {
            bot.stopPlaying();
        } else {
            bot.startPlaying();
        }
        refreshLabels();
    }

    void onLoad(CCObject*) {
        BotManager::shared().loadMacro();
        refreshLabels();
    }

    void onSave(CCObject*) {
        BotManager::shared().saveMacro();
        refreshLabels();
    }

    void onIgnoreToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("ignore_inputs")) return;
        mod->setSettingValue<bool>("ignore_inputs", !mod->getSettingValue<bool>("ignore_inputs"));
        refreshLabels();
    }

    void onFrameStepToggle(CCObject*) {
        BotManager::shared().toggleFrameStepper();
        refreshLabels();
    }

    void onStepOneFrame(CCObject*) {
        BotManager::shared().stepOneFrame();
        refreshLabels();
    }

    void onCbfMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("cbf_boost")) return;
        mod->setSettingValue<int>("cbf_boost", clampValue(mod->getSettingValue<int>("cbf_boost") - 1, 1, 10));
        refreshLabels();
    }

    void onCbfPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("cbf_boost")) return;
        mod->setSettingValue<int>("cbf_boost", clampValue(mod->getSettingValue<int>("cbf_boost") + 1, 1, 10));
        refreshLabels();
    }

    static void toggle() {
        if (s_popup && s_popup->getParent()) {
            s_popup->removeFromParentAndCleanup(true);
            s_popup = nullptr;
            return;
        }

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) {
            return;
        }

        s_popup = HubPopup::create();
        scene->addChild(s_popup, 99999);
    }
};

void addHubButtonToLayer(CCLayer* layer, CCObject* target, SEL_MenuHandler callback, CCPoint position) {
    auto menu = CCMenu::create();
    menu->setPosition(CCPointZero);

    auto button = makeIconButton(target, callback, 0.42f);
    if (!button) {
        return;
    }

    button->setPosition(position);
    menu->addChild(button);
    layer->addChild(menu, 99999);
}
}

class $modify(MyMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto win = CCDirector::sharedDirector()->getWinSize();
        addHubButtonToLayer(
            this,
            this,
            menu_selector(MyMenuLayer::onHub),
            CCPointMake(win.width - 30.f, win.height - 30.f)
        );

        return true;
    }

    void onHub(CCObject*) {
        HubPopup::toggle();
    }
};

class $modify(MyPauseLayer, PauseLayer) {
    bool init(bool unfocused) {
        if (!PauseLayer::init(unfocused)) {
            return false;
        }

        auto win = CCDirector::sharedDirector()->getWinSize();
        addHubButtonToLayer(
            this,
            this,
            menu_selector(MyPauseLayer::onHub),
            CCPointMake(win.width * 0.76f, win.height * 0.58f)
        );

        return true;
    }

    void onHub(CCObject*) {
        HubPopup::toggle();
    }
};

class $modify(MyGameManager, GameManager) {
    void update(float dt) {
        auto mod = Mod::get();
        if (mod && mod->hasSetting("fps_enabled") && mod->getSettingValue<bool>("fps_enabled")) {
            auto target = clampValue(mod->getSettingValue<int>("fps_target"), 60, 360);
            this->m_customFPSTarget = static_cast<float>(target);
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / static_cast<double>(target));
        } else {
            this->m_customFPSTarget = 60.0f;
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / 60.0);
        }

        GameManager::update(dt);
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void update(float dt) {
        BotManager::shared().update(dt);

        if (BotManager::shared().isFrameStepEnabled() && !BotManager::shared().allowGameplayFrame()) {
            return;
        }

        PlayLayer::update(dt);
    }

    void onEnterTransitionDidFinish() {
        PlayLayer::onEnterTransitionDidFinish();
        BotManager::shared().resetSession();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        BotManager::shared().resetSession();
    }

    void onExit() {
        HubPopup::toggle();
        BotManager::shared().resetSession();
        PlayLayer::onExit();
    }
};

class $modify(MyPlayerObject, PlayerObject) {
    void update(float dt) {
        BotManager::shared().attachPlayer(this);

        if (!BotManager::shared().allowGameplayFrame()) {
            return;
        }

        PlayerObject::update(dt);
    }

    bool pushButton(PlayerButton button) {
        auto& bot = BotManager::shared();

        if (bot.shouldIgnorePhysicalInput()) {
            return true;
        }

        if (bot.isRecording() && !bot.isSyntheticInput() && (bot.activePlayer() == nullptr || bot.activePlayer() == this)) {
            bot.recordEvent(static_cast<int>(button), true);
        }

        return PlayerObject::pushButton(button);
    }

    bool releaseButton(PlayerButton button) {
        auto& bot = BotManager::shared();

        if (bot.shouldIgnorePhysicalInput()) {
            return true;
        }

        if (bot.isRecording() && !bot.isSyntheticInput() && (bot.activePlayer() == nullptr || bot.activePlayer() == this)) {
            bot.recordEvent(static_cast<int>(button), false);
        }

        return PlayerObject::releaseButton(button);
    }
};
