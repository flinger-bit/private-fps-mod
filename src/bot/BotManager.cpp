#include "BotManager.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace geode::prelude;

namespace hub {

namespace {
template <typename T>
T clampValue(T value, T low, T high) {
    return std::max(low, std::min(value, high));
}
}

BotManager& BotManager::shared() {
    static BotManager instance;
    return instance;
}

bool BotManager::getSettingBool(char const* key, bool fallback) const {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return fallback;
    }
    return mod->getSettingValue<bool>(key);
}

int BotManager::getSettingInt(char const* key, int fallback) const {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return fallback;
    }
    return mod->getSettingValue<int>(key);
}

void BotManager::setSettingBool(char const* key, bool value) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return;
    }
    mod->setSettingValue<bool>(key, value);
}

void BotManager::setSettingInt(char const* key, int value) {
    auto mod = Mod::get();
    if (!mod || !mod->hasSetting(key)) {
        return;
    }
    mod->setSettingValue<int>(key, value);
}

std::filesystem::path BotManager::saveFile() const {
    auto mod = Mod::get();
    if (!mod) {
        return {};
    }

    auto dir = mod->getSaveDir();
    std::filesystem::create_directories(dir);
    return dir / "macro.txt";
}

void BotManager::resetSession() {
    m_time = 0.0;
    m_player = nullptr;
    m_syntheticInput = false;
    m_allowGameplayFrame = true;
    m_stepper.reset();
}

void BotManager::clearMacro() {
    stopRecording();
    stopPlaying();
    m_macro.clear();
    resetSession();
}

void BotManager::attachPlayer(PlayerObject* player) {
    if (!m_player) {
        m_player = player;
    }
}

PlayerObject* BotManager::activePlayer() const {
    return m_player;
}

void BotManager::startRecording() {
    clearMacro();
    m_macro.startRecording();
    setSettingBool("bot_recording", true);
    setSettingBool("bot_playing", false);
    m_time = 0.0;
}

void BotManager::stopRecording() {
    m_macro.stopRecording();
    setSettingBool("bot_recording", false);
}

void BotManager::startPlaying() {
    if (m_macro.empty()) {
        loadMacro();
    }

    if (m_macro.empty()) {
        log::warn("No macro available to play.");
        stopPlaying();
        return;
    }

    stopRecording();
    m_macro.startPlaying();
    setSettingBool("bot_playing", true);
    m_time = 0.0;
    m_stepper.reset();
    m_allowGameplayFrame = true;
}

void BotManager::stopPlaying() {
    m_macro.stopPlaying();
    setSettingBool("bot_playing", false);
    m_syntheticInput = false;
}

void BotManager::toggleFrameStepper() {
    auto enabled = !getSettingBool("frame_stepper", false);
    setSettingBool("frame_stepper", enabled);
    m_stepper.setEnabled(enabled);
    if (!enabled) {
        m_stepper.reset();
        m_allowGameplayFrame = true;
    }
}

void BotManager::stepOneFrame() {
    if (!getSettingBool("frame_stepper", false)) {
        setSettingBool("frame_stepper", true);
        m_stepper.setEnabled(true);
    }

    m_stepper.requestStep();
}

void BotManager::recordEvent(int button, bool down) {
    if (!isRecording() || m_syntheticInput) {
        return;
    }

    auto const boost = clampValue(getSettingInt("cbf_boost", 3), 1, 10);
    m_macro.record(m_time, button, down, boost);
}

void BotManager::update(float dt) {
    m_allowGameplayFrame = m_stepper.advanceFrame();
    if (!m_allowGameplayFrame) {
        return;
    }

    if (isRecording() || isPlaying()) {
        m_time += dt;
    }

    if (!isPlaying() || !m_player) {
        return;
    }

    InputFrame frame;
    while (m_macro.next(m_time, frame)) {
        m_syntheticInput = true;

        auto const button = static_cast<PlayerButton>(frame.button);
        if (frame.down) {
            m_player->pushButton(button);
        } else {
            m_player->releaseButton(button);
        }

        m_syntheticInput = false;
    }

    if (m_macro.finished()) {
        stopPlaying();
    }
}

void BotManager::saveMacro() {
    auto const path = saveFile();
    if (path.empty()) {
        log::error("Cannot save macro: invalid save path.");
        return;
    }

    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        log::error("Failed to open macro file for writing: {}", path.string());
        return;
    }

    out << std::setprecision(17);
    for (auto const& frame : m_macro.frames()) {
        out << frame.time << ' '
            << frame.button << ' '
            << frame.down << ' '
            << frame.sequence << '\n';
    }

    log::info("Saved {} frames to {}", m_macro.size(), path.string());
}

void BotManager::loadMacro() {
    auto const path = saveFile();
    if (path.empty()) {
        log::error("Cannot load macro: invalid save path.");
        return;
    }

    std::ifstream in(path);
    if (!in.is_open()) {
        log::warn("Macro file not found: {}", path.string());
        return;
    }

    clearMacro();

    double time = 0.0;
    int button = 0;
    bool down = false;
    int sequence = 0;

    while (in >> time >> button >> down >> sequence) {
        m_macro.frames().push_back(InputFrame{
            .time = time,
            .button = button,
            .down = down,
            .sequence = sequence
        });
    }

    log::info("Loaded {} frames from {}", m_macro.size(), path.string());
}

bool BotManager::isRecording() const {
    return getSettingBool("bot_recording", false);
}

bool BotManager::isPlaying() const {
    return getSettingBool("bot_playing", false);
}

bool BotManager::isFrameStepEnabled() const {
    return getSettingBool("frame_stepper", false);
}

bool BotManager::allowGameplayFrame() const {
    return m_allowGameplayFrame;
}

bool BotManager::isSyntheticInput() const {
    return m_syntheticInput;
}

void BotManager::beginSyntheticInput() {
    m_syntheticInput = true;
}

void BotManager::endSyntheticInput() {
    m_syntheticInput = false;
}

bool BotManager::shouldIgnorePhysicalInput() const {
    return isPlaying() && getSettingBool("ignore_inputs", true) && !m_syntheticInput;
}

Macro& BotManager::macro() {
    return m_macro;
}

Macro const& BotManager::macro() const {
    return m_macro;
}

}
