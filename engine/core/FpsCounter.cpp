#include "core/FpsCounter.h"

#include <cmath>

namespace engine {

bool FpsCounter::tick(float frameSeconds) {
    if (!(frameSeconds > 0.0f)) { // negatif ve NaN süreleri yok say (bkz. FixedTimestep)
        frameSeconds = 0.0f;
    }
    ++m_frames;
    m_elapsed += frameSeconds;

    if (m_elapsed < 1.0f) {
        return false;
    }
    m_fps = static_cast<int>(std::lround(m_frames / m_elapsed));
    m_frames = 0;
    m_elapsed = 0.0f;
    return true;
}

} // namespace engine
