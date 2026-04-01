#pragma once

#include <cstddef>
#include <vector>

namespace bot {

struct InputFrame {
    double time = 0.0;
    int button = 0;
    bool down = false;
    int sequence = 0;
};

class Macro {
public:
    void clear();

    void startRecording();
    void stopRecording();

    void startPlaying();
    void stopPlaying();

    bool isRecording() const;
    bool isPlaying() const;
    bool empty() const;
    bool finished() const;
    std::size_t size() const;

    void record(double time, int button, bool down, int boost = 1);
    bool next(double time, InputFrame& out);

    std::vector<InputFrame>& frames();
    std::vector<InputFrame> const& frames() const;

private:
    std::vector<InputFrame> m_frames;
    std::size_t m_playIndex = 0;
    bool m_recording = false;
    bool m_playing = false;
    double m_lastRecordedTime = -1.0;
};

}
