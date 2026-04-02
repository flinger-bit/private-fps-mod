#pragma once

#include <Geode/Geode.hpp>

#include <filesystem>
#include <vector>

using namespace geode::prelude;

namespace hub {

struct InputFrame {
    double time = 0.0;
    int button = 0;
    bool down = false;
    int sequence = 0;
};

class BotManager {
public:
    static BotManager& shared();

    void resetSession();
    void clearMacro();

    void attachPlayer(PlayerObject* player);
    PlayerObject* activePlayer() const;

    void startRecording();
    void stopRecording();

    void startPlaying();
    void stopPlaying();

    void toggleFrameStepper();
    void stepOneFrame();

    void recordEvent(int button, bool down);
    void update(float dt);

    void saveMacro();
    void loadMacro();

    bool isRecording() const;
    bool isPlaying() const;
    bool isFrameStepEnabled() const;

    bool allowGameplayFrame() const;

    bool isSyntheticInput() const;
    void beginSyntheticInput();
    void endSyntheticInput();

    bool shouldIgnorePhysicalInput() const;

    std::vector<InputFrame>& macro();
    std::vector<InputFrame> const& macro() const;

private:
    BotManager() = default;

    std::filesystem::path saveFile() const;

    bool getSettingBool(char const* key, bool fallback = false) const;
    int getSettingInt(char const* key, int fallback = 0) const;

    void setSettingBool(char const* key, bool value);

    PlayerObject* m_player = nullptr;
    double m_time = 0.0;

    bool m_syntheticInput = false;
    bool m_allowGameplayFrame = true;

    bool m_recording = false;
    bool m_playing = false;

    bool m_stepperEnabled = false;
    bool m_stepPending = false;

    std::size_t m_playIndex = 0;
    std::vector<InputFrame> m_macro;
};

} // namespace hub
