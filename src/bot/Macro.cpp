#include "Macro.hpp"

#include <algorithm>
#include <cmath>

namespace bot {

void Macro::clear() {
    m_frames.clear();
    m_playIndex = 0;
    m_lastRecordedTime = -1.0;
}

void Macro::startRecording() {
    clear();
    m_recording = true;
    m_playing = false;
}

void Macro::stopRecording() {
    m_recording = false;
}

void Macro::startPlaying() {
    m_playIndex = 0;
    m_playing = true;
    m_recording = false;
}

void Macro::stopPlaying() {
    m_playIndex = 0;
    m_playing = false;
}

bool Macro::isRecording() const {
    return m_recording;
}

bool Macro::isPlaying() const {
    return m_playing;
}

bool Macro::empty() const {
    return m_frames.empty();
}

bool Macro::finished() const {
    return m_playing && m_playIndex >= m_frames.size();
}

std::size_t Macro::size() const {
    return m_frames.size();
}

void Macro::record(double time, int button, bool down, int boost) {
    if (!m_recording) {
        return;
    }

    int sequence = 0;

    if (!m_frames.empty()) {
        auto const& last = m_frames.back();
        auto const window = (1.0 / 60.0) * static_cast<double>(std::max(1, boost));

        if (std::abs(last.time - time) <= window) {
            sequence = last.sequence + 1;
        }
    }

    m_frames.push_back(InputFrame{
        .time = time,
        .button = button,
        .down = down,
        .sequence = sequence,
    });

    m_lastRecordedTime = time;
}

bool Macro::next(double time, InputFrame& out) {
    if (!m_playing || m_playIndex >= m_frames.size()) {
        return false;
    }

    auto const& frame = m_frames[m_playIndex];
    if (frame.time > time) {
        return false;
    }

    out = frame;
    ++m_playIndex;
    return true;
}

std::vector<InputFrame>& Macro::frames() {
    return m_frames;
}

std::vector<InputFrame> const& Macro::frames() const {
    return m_frames;
}

}
