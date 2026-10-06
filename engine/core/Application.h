#pragma once

#include <string>

#include "core/FixedTimestep.h"
#include "core/FpsCounter.h"
#include "core/Window.h"

namespace engine {

struct ApplicationConfig {
    WindowConfig window;
    float fixedStep = FixedTimestep::kDefaultStep;
};

// Bütün oyunların taban sınıfı. Oyun bundan türer, aşağıdaki "on..." fonksiyonlarını
// override eder ve run() çağırır; döngüyü motor yönetir.
class Application {
public:
    explicit Application(const ApplicationConfig& config = {});
    // Taban sınıfın yıkıcısı virtual olmalı: türetilmiş nesne taban sınıf işaretçisiyle
    // silinirse türetilmiş sınıfın yıkıcısı da çalışsın.
    virtual ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Oyun döngüsünü çalıştırır. Dönüş: 0 normal kapanış, 1 pencere açılamadı.
    int run();

protected:
    virtual void onInit() {}
    virtual void onFixedUpdate(float /*step*/) {} // sabit adımla: fizik, oyun kuralları
    virtual void onUpdate(float /*dt*/) {}        // her kare: animasyon, input tepkisi
    virtual void onRender() {}                    // her kare: çizim
    virtual void onShutdown() {}

    float totalTime() const { return m_totalTime; } // run() başladığından beri geçen saniye
    int fps() const { return m_fpsCounter.fps(); }

private:
    std::string m_baseTitle; // m_window'dan önce bildirilmeli: üyeler bu sırayla kurulur
    Window m_window;
    FixedTimestep m_timestep;
    FpsCounter m_fpsCounter;
    float m_totalTime = 0.0f;
};

} // namespace engine
