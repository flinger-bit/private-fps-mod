#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace {
    constexpr int kOverlayTag = 0x504F;
    constexpr int kButtonTag = 0x5041;
    constexpr int kOverlayZ = 99999;

    template <typename T>
    T clampValue(T value, T low, T high) {
        return std::max(low, std::min(value, high));
    }

    std::string onOff(bool value) {
        return value ? "ON" : "OFF";
    }

    class HubOverlay;
    static HubOverlay* s_overlay = nullptr;

    class HubButtonLayer;
}

class HubOverlay : public CCLayer {
protected:
    CCLabelBMFont* m_fpsLabel = nullptr;
    CCLabelBMFont* m_straightLabel = nullptr;
    CCLabelBMFont* m_straightPowerLabel = nullptr;
    CCLabelBMFont* m_waveLabel = nullptr;
    CCLabelBMFont* m_wavePowerLabel = nullptr;

    CCMenuItemSpriteExtra* makeButton(
        char const* title,
        float scale,
        CCObject* target,
        SEL_MenuHandler callback
    ) {
        auto label = CCLabelBMFont::create(title, "bigFont.fnt");
        label->setScale(scale);

        auto item = CCMenuItemSpriteExtra::create(label, target, callback);
        return item;
    }

    void refreshLabels() {
        auto mod = Mod::get();
        if (!mod) return;

        bool bypass = mod->getSettingValue<bool>("bypass");
        int fps = clampValue(mod->getSettingValue<int>("fps"), 60, 360);

        bool straightEnabled = mod->getSettingValue<bool>("straight_enabled");
        int straightStrength = clampValue(mod->getSettingValue<int>("straight_strength"), 1, 50);

        bool waveEnabled = mod->getSettingValue<bool>("wave_enabled");
        int waveStrength = clampValue(mod->getSettingValue<int>("wave_strength"), 1, 50);

        m_fpsLabel->setString(
            (std::string("FPS Bypass: ") + onOff(bypass) + " | Target: " + std::to_string(fps)).c_str()
        );

        m_straightLabel->setString(
            (std::string("Straight Fly: ") + onOff(straightEnabled)).c_str()
        );
        m_straightPowerLabel->setString(
            (std::string("Straight Assist: ") + std::to_string(straightStrength)).c_str()
        );

        m_waveLabel->setString(
            (std::string("Wave Fly: ") + onOff(waveEnabled)).c_str()
        );
        m_wavePowerLabel->setString(
            (std::string("Wave Assist: ") + std::to_string(waveStrength)).c_str()
        );
    }

    template <typename T>
    void updateSetting(char const* key, T value) {
        auto mod = Mod::get();
        if (!mod) return;
        mod->setSettingValue<T>(key, value);
        refreshLabels();
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
        if (!CCLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto dim = CCLayerColor::create(ccc4(0, 0, 0, 150));
        this->addChild(dim);

        auto panel = CCLayerColor::create(ccc4(18, 18, 18, 240));
        panel->setContentSize(CCSizeMake(340.f, 390.f));
        panel->setPosition(CCPointMake((winSize.width - 340.f) / 2.f, (winSize.height - 390.f) / 2.f));
        this->addChild(panel);

        auto title = CCLabelBMFont::create("PRIVATE HUB", "bigFont.fnt");
        title->setScale(0.85f);
        title->setPosition(CCPointMake(170.f, 355.f));
        panel->addChild(title);

        m_fpsLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_fpsLabel->setScale(0.42f);
        m_fpsLabel->setPosition(CCPointMake(170.f, 310.f));
        panel->addChild(m_fpsLabel);

        m_straightLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_straightLabel->setScale(0.42f);
        m_straightLabel->setPosition(CCPointMake(170.f, 265.f));
        panel->addChild(m_straightLabel);

        m_straightPowerLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_straightPowerLabel->setScale(0.38f);
        m_straightPowerLabel->setPosition(CCPointMake(170.f, 238.f));
        panel->addChild(m_straightPowerLabel);

        m_waveLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_waveLabel->setScale(0.42f);
        m_waveLabel->setPosition(CCPointMake(170.f, 190.f));
        panel->addChild(m_waveLabel);

        m_wavePowerLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_wavePowerLabel->setScale(0.38f);
        m_wavePowerLabel->setPosition(CCPointMake(170.f, 163.f));
        panel->addChild(m_wavePowerLabel);

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        panel->addChild(menu);

        auto straightToggle = makeButton("Straight", 0.50f, this, menu_selector(HubOverlay::onStraightToggle));
        straightToggle->setPosition(CCPointMake(85.f, 110.f));
        menu->addChild(straightToggle);

        auto straightMinus = makeButton("-", 0.65f, this, menu_selector(HubOverlay::onStraightMinus));
        straightMinus->setPosition(CCPointMake(170.f, 110.f));
        menu->addChild(straightMinus);

        auto straightPlus = makeButton("+", 0.65f, this, menu_selector(HubOverlay::onStraightPlus));
        straightPlus->setPosition(CCPointMake(235.f, 110.f));
        menu->addChild(straightPlus);

        auto waveToggle = makeButton("Wave", 0.50f, this, menu_selector(HubOverlay::onWaveToggle));
        waveToggle->setPosition(CCPointMake(85.f, 60.f));
        menu->addChild(waveToggle);

        auto waveMinus = makeButton("-", 0.65f, this, menu_selector(HubOverlay::onWaveMinus));
        waveMinus->setPosition(CCPointMake(170.f, 60.f));
        menu->addChild(waveMinus);

        auto wavePlus = makeButton("+", 0.65f, this, menu_selector(HubOverlay::onWavePlus));
        wavePlus->setPosition(CCPointMake(235.f, 60.f));
        menu->addChild(wavePlus);

        auto fpsToggle = makeButton("FPS", 0.50f, this, menu_selector(HubOverlay::onFpsToggle));
        fpsToggle->setPosition(CCPointMake(85.f, 10.f));
        menu->addChild(fpsToggle);

        auto fpsMinus = makeButton("-", 0.65f, this, menu_selector(HubOverlay::onFpsMinus));
        fpsMinus->setPosition(CCPointMake(170.f, 10.f));
        menu->addChild(fpsMinus);

        auto fpsPlus = makeButton("+", 0.65f, this, menu_selector(HubOverlay::onFpsPlus));
        fpsPlus->setPosition(CCPointMake(235.f, 10.f));
        menu->addChild(fpsPlus);

        auto closeBtn = makeButton("Close", 0.52f, this, menu_selector(HubOverlay::onClose));
        closeBtn->setPosition(CCPointMake(170.f, -35.f));
        menu->addChild(closeBtn);

        this->refreshLabels();
        return true;
    }

    void onExit() override {
        if (s_overlay == this) {
            s_overlay = nullptr;
        }
        CCLayer::onExit();
    }

    void onClose(CCObject*) {
        if (s_overlay == this) {
            s_overlay = nullptr;
        }
        this->removeFromParentAndCleanup(true);
    }

    void onStraightToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        bool value = mod->getSettingValue<bool>("straight_enabled");
        updateSetting("straight_enabled", !value);
    }

    void onWaveToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        bool value = mod->getSettingValue<bool>("wave_enabled");
        updateSetting("wave_enabled", !value);
    }

    void onFpsToggle(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        bool value = mod->getSettingValue<bool>("bypass");
        updateSetting("bypass", !value);
    }

    void onStraightMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        int value = clampValue(mod->getSettingValue<int>("straight_strength") - 1, 1, 50);
        updateSetting("straight_strength", value);
    }

    void onStraightPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        int value = clampValue(mod->getSettingValue<int>("straight_strength") + 1, 1, 50);
        updateSetting("straight_strength", value);
    }

    void onWaveMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        int value = clampValue(mod->getSettingValue<int>("wave_strength") - 1, 1, 50);
        updateSetting("wave_strength", value);
    }

    void onWavePlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        int value = clampValue(mod->getSettingValue<int>("wave_strength") + 1, 1, 50);
        updateSetting("wave_strength", value);
    }

    void onFpsMinus(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        int value = clampValue(mod->getSettingValue<int>("fps") - 30, 60, 360);
        updateSetting("fps", value);
    }

    void onFpsPlus(CCObject*) {
        auto mod = Mod::get();
        if (!mod) return;
        int value = clampValue(mod->getSettingValue<int>("fps") + 30, 60, 360);
        updateSetting("fps", value);
    }

    static void toggle() {
        if (s_overlay && s_overlay->getParent()) {
            s_overlay->removeFromParentAndCleanup(true);
            s_overlay = nullptr;
            return;
        }

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;

        s_overlay = HubOverlay::create();
        scene->addChild(s_overlay, kOverlayZ);
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
        if (!CCLayer::init())
            return false;

        auto winSize = CCDirector::sharedDirector()->getWinSize();

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
        button->setTag(kButtonTag);

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        menu->addChild(button);

        button->setPosition(CCPointMake(winSize.width - 30.f, winSize.height * 0.62f));
        this->addChild(menu, 1);

        return true;
    }

    void onButton(CCObject*) {
        HubOverlay::toggle();
    }
};

class $modify(MyGameManager, GameManager) {
    void update(float dt) {
        GameManager::update(dt);

        auto mod = Mod::get();
        if (!mod) return;

        int fps = mod->getSettingValue<int>("fps");
        fps = clampValue(fps, 60, 360);

        bool bypass = mod->getSettingValue<bool>("bypass");
        if (!bypass) {
            fps = 60;
        }

        this->m_customFPSTarget = static_cast<float>(fps);
        CCDirector::sharedDirector()->setAnimationInterval(1.0 / static_cast<double>(fps));
    }
};

class $modify(MyPlayerObject, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        auto mod = Mod::get();
        if (!mod || this->m_isDead)
            return;

        if (this->m_isShip && mod->getSettingValue<bool>("straight_enabled")) {
            int strength = clampValue(mod->getSettingValue<int>("straight_strength"), 1, 50);
            float blend = static_cast<float>(strength) / 100.0f;

            float target = this->playerIsMovingUp() ? 7.0f : -7.0f;
            this->m_yVelocity += (target - static_cast<float>(this->m_yVelocity)) * blend;

            this->m_rotationSpeed = clampValue(this->m_rotationSpeed, -20.0f, 20.0f);
            this->m_rotateSpeed = clampValue(this->m_rotateSpeed, -20.0f, 20.0f);
        }

        if ((this->m_isBird || this->m_isDart) && mod->getSettingValue<bool>("wave_enabled")) {
            int strength = clampValue(mod->getSettingValue<int>("wave_strength"), 1, 50);
            float blend = static_cast<float>(strength) / 100.0f;

            float target = this->playerIsMovingUp() ? 8.5f : -8.5f;
            this->m_yVelocity += (target - static_cast<float>(this->m_yVelocity)) * blend;

            this->m_rotationSpeed = clampValue(this->m_rotationSpeed, -20.0f, 20.0f);
            this->m_rotateSpeed = clampValue(this->m_rotateSpeed, -20.0f, 20.0f);
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    void onEnterTransitionDidFinish() {
        PlayLayer::onEnterTransitionDidFinish();

        if (!this->getChildByTag(kOverlayTag)) {
            auto ui = HubButtonLayer::create();
            ui->setTag(kOverlayTag);
            this->addChild(ui, kOverlayZ);
        }
    }
};
