#include "core/FixedTimestep.h"

namespace engine {

FixedTimestep::FixedTimestep(float stepSeconds, float maxFrameSeconds)
    : m_step(stepSeconds > 0.0f ? stepSeconds : kDefaultStep)
    , m_maxFrame(maxFrameSeconds > 0.0f ? maxFrameSeconds : kDefaultMaxFrame) {}

int FixedTimestep::advance(float frameSeconds) {
    // !(x > 0) hem negatif sayıları hem de NaN'ı yakalar: NaN ile her karşılaştırma false döner.
    // NaN'ı içeri alsaydık biriktirici sonsuza dek NaN kalır ve bir daha hiç adım üretmezdi.
    if (!(frameSeconds > 0.0f)) {
        frameSeconds = 0.0f;
    }
    // Program uzun süre donduysa (breakpoint, pencere sürükleme) kaybedilen zamanı
    // telafi etmeye çalışmayız; aksi halde yetişemeyip her karede daha da geride kalırdık.
    if (frameSeconds > m_maxFrame) {
        frameSeconds = m_maxFrame;
    }

    m_accumulator += frameSeconds;

    int steps = 0;
    while (m_accumulator >= m_step) {
        m_accumulator -= m_step;
        ++steps;
    }
    return steps;
}

} // namespace engine
