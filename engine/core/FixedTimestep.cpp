#include "core/FixedTimestep.h"

namespace engine {

FixedTimestep::FixedTimestep(float stepSeconds, float maxFrameSeconds)
    : m_step(stepSeconds > 0.0f ? stepSeconds : kDefaultStep)
    , m_maxFrame(maxFrameSeconds > 0.0f ? maxFrameSeconds : kDefaultMaxFrame) {}

float FixedTimestep::clampFrame(float frameSeconds) const {
    // !(x > 0) hem negatif sayıları hem de NaN'ı yakalar: NaN ile <, >, <=, >=, == hep false
    // döner (yalnızca != true döner). NaN'ı içeri alsaydık biriktirici sonsuza dek NaN kalır
    // ve bir daha hiç adım üretmezdi.
    if (!(frameSeconds > 0.0f)) {
        return 0.0f;
    }
    // İki işe yarar:
    //  - Tek seferlik donma (breakpoint, pencere sürükleme): kaybedilen zamanın fazlası atılır;
    //    oyun en fazla maxFrame kadar ileri sıçrar.
    //  - Sürekli aşırı yük (bir sabit adımı hesaplamak, simüle ettiği süreden uzun sürüyorsa):
    //    kare başına adım sayısı sınırlı kalır; oyun donmak yerine ağır çekimde akar.
    //    Sınır olmasaydı her kare bir öncekinden daha çok adım isterdi: "ölüm sarmalı".
    if (frameSeconds > m_maxFrame) {
        return m_maxFrame;
    }
    return frameSeconds;
}

int FixedTimestep::advance(float frameSeconds) {
    m_accumulator += clampFrame(frameSeconds);

    int steps = 0;
    while (m_accumulator >= m_step) {
        m_accumulator -= m_step;
        ++steps;
    }
    return steps;
}

} // namespace engine
