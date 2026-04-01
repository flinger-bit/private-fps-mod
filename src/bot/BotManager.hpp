#pragma once

#include <Geode/Geode.hpp>

#include <filesystem>

#include "FrameStepper.hpp"
#include "Macro.hpp"

namespace bot {

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
    bool allowPlayerUpdate();

    void recordEvent(int button, bool down);
    void update(float dt);

    void saveMacro();
    void loadMacro();

    bool isRecording() const;
    bool isPlaying() const;
    bool isFrameStepEnabled() const;

    bool isSyntheticInput() const;
    void beginSyntheticInput();
    void endSyntheticInput();

    Macro& macro();
    Macro const& macro() const;

private:
    BotManager() = default;

    std::filesystem::path saveFile() const;

    bool getSettingBool(char const* key, bool fallback = false) const;
    int getSettingInt(char const* key, int fallback = 0) const;
    void setSettingBool(char const* key, bool value);
    void setSettingInt(char const* key, int value);

    PlayerObject* m_player = nullptr;
    double m_time = 0.0;
    bool m_inSyntheticInput = false;
    Macro m_macro;
    FrameStepper m_stepper;
};

}
