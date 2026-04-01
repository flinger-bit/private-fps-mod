#pragma once

namespace hub {

class FrameStepper {
public:
    void setEnabled(bool enabled) {
        m_enabled = enabled;
        if (!enabled) {
            m_pending = false;
            m_allowGameplayFrame = true;
        }
    }

    bool enabled() const {
        return m_enabled;
    }

    void requestStep() {
        m_pending = true;
    }

    bool advanceFrame() {
        if (!m_enabled) {
            m_allowGameplayFrame = true;
            return true;
        }

        if (m_pending) {
            m_pending = false;
            m_allowGameplayFrame = true;
            return true;
        }

        m_allowGameplayFrame = false;
        return false;
    }

    void reset() {
        m_pending = false;
        m_allowGameplayFrame = true;
    }

    bool allowGameplayFrame() const {
        return m_allowGameplayFrame;
    }

private:
    bool m_enabled = false;
    bool m_pending = false;
    bool m_allowGameplayFrame = true;
};

}
