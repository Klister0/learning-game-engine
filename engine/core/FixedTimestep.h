#pragma once

namespace engine {

// "Fix Your Timestep" deseni: kare süreleri değişken olsa da oyun mantığını (özellikle fiziği)
// hep aynı sabit adımla ilerletmek için geçen süreyi biriktirir.
//
// Kullanım (her karede):
//     int steps = timestep.advance(frameSeconds);
//     for (int i = 0; i < steps; ++i) fixedUpdate(timestep.step());
class FixedTimestep {
public:
    static constexpr float kDefaultStep = 1.0f / 60.0f;
    static constexpr float kDefaultMaxFrame = 0.25f;

    // stepSeconds <= 0 ise kDefaultStep kullanılır (aksi halde advance sonsuz döngüye girerdi).
    // maxFrameSeconds: tek bir karenin en fazla ne kadar sayılacağı ("ölüm sarmalı" koruması).
    explicit FixedTimestep(float stepSeconds = kDefaultStep, float maxFrameSeconds = kDefaultMaxFrame);

    // Ham kare süresini güvenli aralığa çeker: negatif/NaN → 0, maxFrame'den uzun → maxFrame.
    // Application, onUpdate'e verdiği dt için de bunu kullanır; böylece uzun bir donmadan
    // sonra animasyonlar ve hareketler de birden sıçramaz.
    float clampFrame(float frameSeconds) const;

    // Bu karenin süresini (clampFrame'den geçirerek) biriktirir ve şimdi çalıştırılması
    // gereken sabit adım sayısını döndürür.
    int advance(float frameSeconds);

    float step() const { return m_step; }

private:
    float m_step;
    float m_maxFrame;
    float m_accumulator = 0.0f;
};

} // namespace engine
