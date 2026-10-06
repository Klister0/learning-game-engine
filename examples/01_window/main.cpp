// Bölüm 01 örneği: rengi zamanla değişen bir pencere.
//
// Konsola her saniye o saniyede kaç sabit adım (onFixedUpdate) çalıştığı yazılır.
// FPS ne olursa olsun bu sayı ~60 olmalıdır: sabit zaman adımının bütün amacı budur.

#include <cmath>
#include <string>

#include "core/Application.h"
#include "core/Log.h"
#include "renderer/RenderCommand.h"

class WindowDemo : public engine::Application {
public:
    explicit WindowDemo(const engine::ApplicationConfig& config) : engine::Application(config) {}

protected:
    void onFixedUpdate(float /*step*/) override {
        ++m_fixedStepsThisSecond;
    }

    void onUpdate(float dt) override {
        m_secondTimer += dt;
        if (m_secondTimer >= 1.0f) {
            LOG_INFO("Son 1 saniyede " + std::to_string(m_fixedStepsThisSecond) +
                     " sabit adım çalıştı | FPS: " + std::to_string(fps()));
            m_secondTimer = 0.0f;
            m_fixedStepsThisSecond = 0;
        }
    }

    void onRender() override {
        // Kırmızı, yeşil ve maviyi birbirinden 1/3 tur (2π/3 radyan) kaydırılmış
        // sinüs dalgalarıyla salındırıyoruz; renk yumuşakça döner.
        const float t = totalTime();
        const float r = 0.5f + 0.5f * std::sin(t);
        const float g = 0.5f + 0.5f * std::sin(t + 2.094f);
        const float b = 0.5f + 0.5f * std::sin(t + 4.189f);
        engine::RenderCommand::setClearColor(r, g, b);
        engine::RenderCommand::clear();
    }

private:
    int m_fixedStepsThisSecond = 0;
    float m_secondTimer = 0.0f;
};

int main() {
    engine::ApplicationConfig config;
    config.window.title = "Bölüm 01 - Pencere ve Oyun Döngüsü";

    WindowDemo app(config);
    return app.run();
}
