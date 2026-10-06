#include "core/Application.h"

#include "core/Log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace engine {

Application::Application(const ApplicationConfig& config)
    : m_baseTitle(config.window.title)
    , m_window(config.window)
    , m_timestep(config.fixedStep) {}

int Application::run() {
    if (!m_window.isValid()) {
        LOG_ERROR("Pencere açılamadığı için uygulama başlatılamıyor.");
        return 1;
    }

    onInit();

    const double startTime = glfwGetTime();
    double previousTime = startTime;

    while (m_window.isOpen()) {
        const double now = glfwGetTime();
        const float rawDt = static_cast<float>(now - previousTime);
        previousTime = now;
        m_totalTime = static_cast<float>(now - startTime);

        // Oyun koduna giden dt sınırlanır (en fazla 0.25 sn, NaN/negatif → 0);
        // FPS ölçümü ise gerçek süreyi kullanır.
        const float dt = m_timestep.clampFrame(rawDt);

        // 1) Sabit adımlar: bu kare için birikmiş süre kadar (0, 1 veya birkaç kez).
        const int steps = m_timestep.advance(dt);
        for (int i = 0; i < steps; ++i) {
            onFixedUpdate(m_timestep.step());
        }

        // 2) Değişken adım ve çizim: her karede tam bir kez.
        onUpdate(dt);
        onRender();

        // 3) Kareyi ekrana ver, klavye/fare/pencere olaylarını işle.
        m_window.swapAndPoll();

        if (m_fpsCounter.tick(rawDt)) {
            m_window.setTitle(m_baseTitle + " | FPS: " + std::to_string(m_fpsCounter.fps()));
        }
    }

    onShutdown();
    return 0;
}

} // namespace engine
