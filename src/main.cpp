#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MyGameManager, GameManager) {
    void setFPS(float fps) {
        auto mod = Mod::get();
        if (mod && mod->getSettingValue<bool>("bypass")) {
            fps = static_cast<float>(mod->getSettingValue<int64_t>("fps"));
        }
        GameManager::setFPS(fps);
    }
};

class $modify(MyPlayer, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        auto mod = Mod::get();
        if (!mod) return;

        if (mod->getSettingValue<bool>("ship") && this->m_isShip) {
            this->m_rotation = 0.0f;
            this->m_velocity.y = 0.0f;
        }

        if (mod->getSettingValue<bool>("wave") && this->m_isBird) {
            this->m_rotation = 0.0f;
            this->m_velocity.y = 0.0f;
        }
    }
};
