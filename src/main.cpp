#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MyGameManager, GameManager) {
    void setFPS(float fps) {
        if (Mod::get()->getSettingValue<bool>("bypass")) {
            fps = Mod::get()->getSettingValue<int>("fps");
        }
        GameManager::setFPS(fps);
    }
};

class $modify(MyPlayer, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        if (Mod::get()->getSettingValue<bool>("ship") && this->m_isShip) {
            this->m_rotation = 0;
            this->m_velocity.y = 0;
        }

        if (Mod::get()->getSettingValue<bool>("wave") && this->m_isBird) {
            this->m_rotation = 0;
            this->m_velocity.y = 0;
        }
    }
};
