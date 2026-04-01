#include <Geode/Geode.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(MyGameManager, GameManager) {
    void update(float dt) {
        GameManager::update(dt);

        if (auto mod = Mod::get(); mod && mod->getSettingValue<bool>("bypass")) {
            this->m_customFPSTarget = static_cast<float>(mod->getSettingValue<int>("fps"));
        }
    }
};

class $modify(MyPlayer, PlayerObject) {
    void update(float dt) {
        PlayerObject::update(dt);

        auto mod = Mod::get();
        if (!mod) return;

        if (mod->getSettingValue<bool>("ship") && this->m_isShip) {
            this->m_rotationSpeed = 0.0f;
            this->m_rotateSpeed = 0.0f;
            this->setYVelocity(0.0, 0);
        }

        // En Geode/2.2, el modo wave suele aparecer como dart en las bindings.
        if (mod->getSettingValue<bool>("wave") && (this->m_isBird || this->m_isDart)) {
            this->m_rotationSpeed = 0.0f;
            this->m_rotateSpeed = 0.0f;
            this->setYVelocity(0.0, 0);
        }
    }
};
