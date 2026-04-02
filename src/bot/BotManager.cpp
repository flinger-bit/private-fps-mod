#include "BotManager.hpp"

#include <Geode/utils/string.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <system_error>

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

std::filesystem::path BotManager::saveFile() const {
    auto mod = Mod::get();
    if (!mod) {
        return {};
    }

    auto dir = mod->getSaveDir();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        log::warn(
            "Could not create save directory {}: {}",
            geode::utils::string::pathToString(dir),
            ec.message()
        );
    }

    return dir / "macro.txt";
}

void BotManager::resetSession() {
    m_player = nullptr;
    m_time = 0.0;
    m_syntheticInput = false;
    m_allowGameplayFrame = true;
    m_recording = false;
    m_playing = false;
    m_stepperEnabled = false;
    m_stepPending = false;
    m_playIndex = 0;
}

void BotManager::clearMacro() {
    stopRecording();
    stopPlaying();
    m_macro.clear();
    m_playIndex = 0;
    m_time = 0.0;
    m_stepPending = false;
    m_allowGameplayFrame = true;
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
    m_recording = true;
    setSettingBool("bot_recording", true);
    setSettingBool("bot_playing", false);
    m_time = 0.0;
}

void BotManager::stopRecording() {
    m_recording = false;
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
    m_playing = true;
    setSettingBool("bot_playing", true);
    m_time = 0.0;
    m_playIndex = 0;
    m_stepPending = false;
    m_allowGameplayFrame = true;
}

void BotManager::stopPlaying() {
    m_playing = false;
    setSettingBool("bot_playing", false);
    m_syntheticInput = false;
}

void BotManager::toggleFrameStepper() {
    m_stepperEnabled = !m_stepperEnabled;
    setSettingBool("frame_stepper", m_stepperEnabled);

    if (!m_stepperEnabled) {
        m_stepPending = false;
        m_allowGameplayFrame = true;
    }
}

void BotManager::stepOneFrame() {
    if (!m_stepperEnabled) {
        m_stepperEnabled = true;
        setSettingBool("frame_stepper", true);
    }

    m_stepPending = true;
}

void BotManager::recordEvent(int button, bool down) {
    if (!m_recording || m_syntheticInput) {
        return;
    }

    int boost = clampValue(getSettingInt("cbf_boost", 3), 1, 10);
    int sequence = 0;

    if (!m_macro.empty()) {
        auto const& last = m_macro.back();
        auto const window = (1.0 / 60.0) / static_cast<double>(std::max(1, boost));
        if (std::abs(m_time - last.time) <= window) {
            sequence = last.sequence + 1;
        }
    }

    m_macro.push_back(InputFrame{
        .time = m_time,
        .button = button,
        .down = down,
        .sequence = sequence
    });
}

void BotManager::update(float dt) {
    m_allowGameplayFrame = true;

    if (m_stepperEnabled) {
        if (m_stepPending) {
            m_stepPending = false;
            m_allowGameplayFrame = true;
        } else {
            m_allowGameplayFrame = false;
            return;
        }
    }

    if (m_recording || m_playing) {
        m_time += dt;
    }

    if (!m_playing || !m_player) {
        return;
    }

    while (m_playIndex < m_macro.size()) {
        auto const frame = m_macro[m_playIndex];
        if (frame.time > m_time) {
            break;
        }

        ++m_playIndex;
        m_syntheticInput = true;

        auto const button = static_cast<PlayerButton>(frame.button);
        if (frame.down) {
            m_player->pushButton(button);
        } else {
            m_player->releaseButton(button);
        }

        m_syntheticInput = false;
    }

    if (m_playIndex >= m_macro.size()) {
        stopPlaying();
    }
}

void BotManager::saveMacro() {
    auto const path = saveFile();
    if (path.empty()) {
        log::error("Cannot save macro: invalid save path.");
        return;
    }

    std::ofstream out(geode::utils::string::pathToString(path), std::ios::trunc);
    if (!out.is_open()) {
        log::error("Failed to open macro file for writing: {}", geode::utils::string::pathToString(path));
        return;
    }

    out << std::setprecision(17);
    for (auto const& frame : m_macro) {
        out << frame.time << ' '
            << frame.button << ' '
            << frame.down << ' '
            << frame.sequence << '\n';
    }

    log::info("Saved {} frames to {}", m_macro.size(), geode::utils::string::pathToString(path));
}

void BotManager::loadMacro() {
    auto const path = saveFile();
    if (path.empty()) {
        log::error("Cannot load macro: invalid save path.");
        return;
    }

    std::ifstream in(geode::utils::string::pathToString(path));
    if (!in.is_open()) {
        log::warn("Macro file not found: {}", geode::utils::string::pathToString(path));
        return;
    }

    m_macro.clear();
    m_playIndex = 0;
    m_time = 0.0;

    double time = 0.0;
    int button = 0;
    bool down = false;
    int sequence = 0;

    while (in >> time >> button >> down >> sequence) {
        m_macro.push_back(InputFrame{
            .time = time,
            .button = button,
            .down = down,
            .sequence = sequence
        });
    }

    log::info("Loaded {} frames from {}", m_macro.size(), geode::utils::string::pathToString(path));
}

bool BotManager::isRecording() const {
    return getSettingBool("bot_recording", m_recording);
}

bool BotManager::isPlaying() const {
    return getSettingBool("bot_playing", m_playing);
}

bool BotManager::isFrameStepEnabled() const {
    return getSettingBool("frame_stepper", m_stepperEnabled);
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

std::vector<InputFrame>& BotManager::macro() {
    return m_macro;
}

std::vector<InputFrame> const& BotManager::macro() const {
    return m_macro;
}

} // namespace hub
