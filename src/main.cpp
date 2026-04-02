#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
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
constexpr float kPanelW = 320.f;
constexpr float kPanelH = 460.f;
constexpr int kPauseButtonTag = 0x5001;

template <typename T>
T clampValue(T value, T low, T high) {
    return std::max(low, std::min(value, high));
}

class HubPopup;
static HubPopup* s_open = nullptr;
static bool s_swiftProcessing = false;

bool getBool(char const* key, bool fallback = false) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return fallback;
    }
    return mod->getSettingValue<bool>(key);
}

int getInt(char const* key, int fallback = 0) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return fallback;
    }
    return mod->getSettingValue<int>(key);
}

CCMenuItemSpriteExtra* makeTextButton(char const* text, CCObject* target, SEL_MenuHandler callback, float scale = 0.25f) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(scale);
    return CCMenuItemSpriteExtra::create(label, target, callback);
}

CCMenuItemSpriteExtra* makeIconButton(CCObject* target, SEL_MenuHandler callback, float scale = 0.30f) {
    auto sprite = CCSprite::create("icon.png"_spr);
    if (!sprite) {
        log::error("Missing sprite resource: resources/icon.png");
        return nullptr;
    }

    sprite->setScale(scale);
    return CCMenuItemSpriteExtra::create(sprite, target, callback);
}

void addHubButtonToLayer(CCLayer* layer, CCObject* target, SEL_MenuHandler callback, CCPoint position, float scale = 0.30f) {
    auto menu = CCMenu::create();
    menu->setPosition(CCPointZero);
    menu->setTag(kPauseButtonTag);

    auto button = makeIconButton(target, callback, scale);
    if (!button) {
        return;
    }

    button->setPosition(position);
    menu->addChild(button);
    layer->addChild(menu, 99999);
}

void performSwiftDuplication(PlayerObject* player, PlayerButton button) {
    if (!player || s_swiftProcessing) {
        return;
    }

    if (!getBool("swift_enabled", true)) {
        return;
    }

    int clicks = clampValue(getInt("swift_clicks", 20), 1, 60);
    if (clicks <= 1) {
        return;
    }

    s_swiftProcessing = true;

    for (int i = 1; i < clicks; ++i) {
        player->releaseButton(button);
        player->pushButton(button);
    }

    s_swiftProcessing = false;
}

class HubPopup : public CCLayer {
protected:
    int m_tab = 0;
    CCLayer* m_content = nullptr;
    CCLabelBMFont* m_statusTop = nullptr;
    CCLabelBMFont* m_statusBottom = nullptr;

    void refresh() {
        if (m_tab == 0) {
            if (m_statusTop) {
                bool enabled = getBool("fps_enabled", true);
                int target = clampValue(getInt("fps_target", 240), 60, 360);

                std::string text = "FPS: ";
                text += enabled ? "ON" : "OFF";
                text += " | Target: ";
                text += std::to_string(target);
                m_statusTop->setString(text.c_str());
            }
            return;
        }

        if (m_statusTop && m_statusBottom) {
            auto& bot = BotManager::shared();

            std::string top = "BOT: ";
            top += bot.isRecording() ? "RECORDING" : (bot.isPlaying() ? "PLAYING" : "IDLE");
            top += " | Frames: ";
            top += std::to_string(bot.macro().size());
            m_statusTop->setString(top.c_str());

            std::string bottom = "Ignore: ";
            bottom += getBool("ignore_inputs", true) ? "ON" : "OFF";
            bottom += " | FrameStep: ";
            bottom += getBool("frame_stepper", false) ? "ON" : "OFF";
            bottom += " | Swift: ";
            bottom += getBool("swift_enabled", true) ? "ON" : "OFF";
            bottom += " | Cnt: ";
            bottom += std::to_string(clampValue(getInt("swift_clicks", 20), 1, 60));
            m_statusBottom->setString(bottom.c_str());
        }
    }

    void rebuild() {
        if (!m_content) {
            return;
        }

        m_content->removeAllChildrenWithCleanup(true);
        m_statusTop = nullptr;
        m_statusBottom = nullptr;

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        m_content->addChild(menu);

        if (m_tab == 0) {
            buildFpsTab(menu);
        } else {
            buildBotTab(menu);
        }

        refresh();
    }

    void buildFpsTab(CCMenu* menu) {
        m_statusTop = CCLabelBMFont::create("", "bigFont.fnt");
        m_statusTop->setScale(0.23f);
        m_statusTop->setPosition(CCPointMake(kPanelW / 2.f, 345.f));
        m_content->addChild(m_statusTop);

        auto toggle = makeTextButton("Toggle", this, menu_selector(HubPopup::onFpsToggle), 0.25f);
        toggle->setPosition(CCPointMake(kPanelW / 2.f, 290.f));
        menu->addChild(toggle);

        auto minus = makeTextButton("-", this, menu_selector(HubPopup::onFpsMinus), 0.30f);
        minus->setPosition(CCPointMake(96.f, 230.f));
        menu->addChild(minus);

        auto plus = makeTextButton("+", this, menu_selector(HubPopup::onFpsPlus), 0.30f);
        plus->setPosition(CCPointMake(160.f, 230.f));
        menu->addChild(plus);

        auto def = makeTextButton("Default", this, menu_selector(HubPopup::onFpsDefault), 0.22f);
        def->setPosition(CCPointMake(224.f, 230.f));
        menu->addChild(def);
    }

    void buildBotTab(CCMenu* menu) {
        m_statusTop = CCLabelBMFont::create("", "bigFont.fnt");
        m_statusTop->setScale(0.21f);
        m_statusTop->setPosition(CCPointMake(kPanelW / 2.f, 350.f));
        m_content->addChild(m_statusTop);

        m_statusBottom = CCLabelBMFont::create("", "bigFont.fnt");
        m_statusBottom->setScale(0.18f);
        m_statusBottom->setPosition(CCPointMake(kPanelW / 2.f, 326.f));
        m_content->addChild(m_statusBottom);

        auto rec = makeTextButton("REC", this, menu_selector(HubPopup::onRecord), 0.22f);
        rec->setPosition(CCPointMake(80.f, 275.f));
        menu->addChild(rec);

        auto play = makeTextButton("PLAY", this, menu_selector(HubPopup::onPlay), 0.22f);
        play->setPosition(CCPointMake(160.f, 275.f));
        menu->addChild(play);

        auto stop = makeTextButton("STOP", this, menu_selector(HubPopup::onStop), 0.22f);
        stop->setPosition(CCPointMake(240.f, 275.f));
        menu->addChild(stop);

        auto save = makeTextButton("SAVE", this, menu_selector(HubPopup::onSave), 0.22f);
        save->setPosition(CCPointMake(80.f, 220.f));
        menu->addChild(save);

        auto load = makeTextButton("LOAD", this, menu_selector(HubPopup::onLoad), 0.22f);
        load->setPosition(CCPointMake(160.f, 220.f));
        menu->addChild(load);

        auto clear = makeTextButton("CLEAR", this, menu_selector(HubPopup::onClear), 0.20f);
        clear->setPosition(CCPointMake(240.f, 220.f));
        menu->addChild(clear);

        auto frame = makeTextButton("FRAME", this, menu_selector(HubPopup::onFrameStepToggle), 0.20f);
        frame->setPosition(CCPointMake(80.f, 165.f));
        menu->addChild(frame);

        auto step = makeTextButton("STEP", this, menu_selector(HubPopup::onStep), 0.22f);
        step->setPosition(CCPointMake(160.f, 165.f));
        menu->addChild(step);

        auto ignore = makeTextButton("IGNORE", this, menu_selector(HubPopup::onIgnoreToggle), 0.18f);
        ignore->setPosition(CCPointMake(240.f, 165.f));
        menu->addChild(ignore);

        auto swiftToggle = makeTextButton("SWIFT", this, menu_selector(HubPopup::onSwiftToggle), 0.20f);
        swiftToggle->setPosition(CCPointMake(62.f, 108.f));
        menu->addChild(swiftToggle);

        auto swiftMinus = makeTextButton("-", this, menu_selector(HubPopup::onSwiftMinus), 0.30f);
        swiftMinus->setPosition(CCPointMake(128.f, 108.f));
        menu->addChild(swiftMinus);

        auto swiftPlus = makeTextButton("+", this, menu_selector(HubPopup::onSwiftPlus), 0.30f);
        swiftPlus->setPosition(CCPointMake(192.f, 108.f));
        menu->addChild(swiftPlus);

        auto swiftDefault = makeTextButton("20", this, menu_selector(HubPopup::onSwiftDefault), 0.22f);
        swiftDefault->setPosition(CCPointMake(258.f, 108.f));
        menu->addChild(swiftDefault);
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
        dim->setContentSize(win);
        dim->setAnchorPoint(CCPointZero);
        dim->setPosition(CCPointZero);
        addChild(dim);

        auto panel = CCLayerColor::create(ccc4(22, 22, 22, 240));
        panel->setContentSize(CCSizeMake(kPanelW, kPanelH));
        panel->setAnchorPoint(CCPointZero);
        panel->setPosition(CCPointMake((win.width - kPanelW) / 2.f, (win.height - kPanelH) / 2.f));
        addChild(panel);

        auto title = CCLabelBMFont::create("PRIVATE HUB", "bigFont.fnt");
        title->setScale(0.52f);
        title->setPosition(CCPointMake(kPanelW / 2.f, kPanelH - 18.f));
        panel->addChild(title);

        auto tabs = CCMenu::create();
        tabs->setPosition(CCPointZero);
        panel->addChild(tabs);

        auto fpsTab = makeTextButton("FPS", this, menu_selector(HubPopup::onTabFps), 0.28f);
        fpsTab->setPosition(CCPointMake(70.f, kPanelH - 52.f));
        tabs->addChild(fpsTab);

        auto botTab = makeTextButton("BOT", this, menu_selector(HubPopup::onTabBot), 0.28f);
        botTab->setPosition(CCPointMake(160.f, kPanelH - 52.f));
        tabs->addChild(botTab);

        auto close = makeTextButton("X", this, menu_selector(HubPopup::onClose), 0.30f);
        close->setPosition(CCPointMake(kPanelW - 18.f, kPanelH - 18.f));
        tabs->addChild(close);

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

    void onClose(CCObject*) {
        HubPopup::toggle();
    }

    void onFpsToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_enabled")) {
            return;
        }
        mod->setSettingValue<bool>("fps_enabled", !mod->getSettingValue<bool>("fps_enabled"));
        refresh();
    }

    void onFpsMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) {
            return;
        }
        mod->setSettingValue<int>("fps_target", clampValue(mod->getSettingValue<int>("fps_target") - 30, 60, 360));
        refresh();
    }

    void onFpsPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) {
            return;
        }
        mod->setSettingValue<int>("fps_target", clampValue(mod->getSettingValue<int>("fps_target") + 30, 60, 360));
        refresh();
    }

    void onFpsDefault(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("fps_target")) {
            return;
        }
        mod->setSettingValue<int>("fps_target", 240);
        refresh();
    }

    void onRecord(CCObject*) {
        BotManager::shared().startRecording();
        refresh();
    }

    void onPlay(CCObject*) {
        BotManager::shared().startPlaying();
        refresh();
    }

    void onStop(CCObject*) {
        BotManager::shared().stopRecording();
        BotManager::shared().stopPlaying();
        refresh();
    }

    void onSave(CCObject*) {
        BotManager::shared().saveMacro();
        refresh();
    }

    void onLoad(CCObject*) {
        BotManager::shared().loadMacro();
        refresh();
    }

    void onClear(CCObject*) {
        BotManager::shared().clearMacro();
        refresh();
    }

    void onFrameStepToggle(CCObject*) {
        BotManager::shared().toggleFrameStepper();
        refresh();
    }

    void onStep(CCObject*) {
        BotManager::shared().stepOneFrame();
        refresh();
    }

    void onIgnoreToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("ignore_inputs")) {
            return;
        }
        mod->setSettingValue<bool>("ignore_inputs", !mod->getSettingValue<bool>("ignore_inputs"));
        refresh();
    }

    void onSwiftToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("swift_enabled")) {
            return;
        }
        mod->setSettingValue<bool>("swift_enabled", !mod->getSettingValue<bool>("swift_enabled"));
        refresh();
    }

    void onSwiftMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("swift_clicks")) {
            return;
        }
        mod->setSettingValue<int>("swift_clicks", clampValue(mod->getSettingValue<int>("swift_clicks") - 1, 1, 60));
        refresh();
    }

    void onSwiftPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("swift_clicks")) {
            return;
        }
        mod->setSettingValue<int>("swift_clicks", clampValue(mod->getSettingValue<int>("swift_clicks") + 1, 1, 60));
        refresh();
    }

    void onSwiftDefault(CCObject*) {
        auto mod = Mod::get();
        if (!mod || !mod->hasSetting("swift_clicks")) {
            return;
        }
        mod->setSettingValue<int>("swift_clicks", 20);
        refresh();
    }

    static void toggle() {
        if (s_open && s_open->getParent()) {
            s_open->removeFromParentAndCleanup(true);
            s_open = nullptr;
            return;
        }

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) {
            return;
        }

        s_open = HubPopup::create();
        scene->addChild(s_open, 99999);
    }
};
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

class $modify(MyGameManager, GameManager) {
    void update(float dt) {
        auto mod = Mod::get();
        static int s_lastTarget = -1;

        int target = 60;
        if (mod && mod->hasSetting("fps_enabled") && mod->getSettingValue<bool>("fps_enabled")) {
            target = clampValue(mod->getSettingValue<int>("fps_target"), 60, 360);
        }

        if (s_lastTarget != target) {
            CCDirector::sharedDirector()->setAnimationInterval(1.0 / static_cast<double>(target));
            s_lastTarget = target;
        }

        this->m_customFPSTarget = static_cast<float>(target);
        GameManager::update(dt);
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void onEnterTransitionDidFinish() {
        PlayLayer::onEnterTransitionDidFinish();
        BotManager::shared().resetSession();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        BotManager::shared().resetSession();
    }

    void update(float dt) {
        auto& bot = BotManager::shared();
        bot.update(dt);

        if (bot.isFrameStepEnabled() && !bot.allowGameplayFrame()) {
            return;
        }

        PlayLayer::update(dt);
    }

    void onExit() {
        BotManager::shared().resetSession();
        PlayLayer::onExit();
    }
};

class $modify(MyPlayerObject, PlayerObject) {
    void update(float dt) {
        if (PlayLayer::get()) {
            BotManager::shared().attachPlayer(this);
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

        bool result = PlayerObject::pushButton(button);

        if (!s_swiftProcessing) {
            performSwiftDuplication(this, button);
        }

        return result;
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
