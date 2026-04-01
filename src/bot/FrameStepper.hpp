#pragma once

namespace bot {

class FrameStepper {
public:
    void setEnabled(bool enabled) {
        m_enabled = enabled;
        if (!enabled) {
            m_pending = false;
        }
    }

    bool enabled() const {
        return m_enabled;
    }

    void requestStep() {
        m_pending = true;
    }

    bool consumeStep() {
        if (!m_enabled) {
            return true;
        }

        if (m_pending) {
            m_pending = false;
            return true;
        }

        return false;
    }

    void reset() {
        m_pending = false;
    }

private:
    bool m_enabled = false;
    bool m_pending = false;
};

}
