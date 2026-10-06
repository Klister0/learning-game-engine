#pragma once

namespace engine {

// Saniyede kaç kare çizildiğini sayar. Her karede tick() çağrılır;
// 1 saniye dolduğunda true döner ve fps() yeni değeri verir.
class FpsCounter {
public:
    bool tick(float frameSeconds);
    int fps() const { return m_fps; }

private:
    float m_elapsed = 0.0f; // bu ölçüm penceresinde geçen süre
    int m_frames = 0;       // bu ölçüm penceresinde çizilen kare
    int m_fps = 0;          // son tamamlanan ölçümün sonucu
};

} // namespace engine
