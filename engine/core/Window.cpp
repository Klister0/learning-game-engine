#include "core/Window.h"

#include "core/Log.h"

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE // OpenGL başlıklarını glad'dan alıyoruz, GLFW eklemesin
#include <GLFW/glfw3.h>

namespace engine {

namespace {

void onGlfwError(int code, const char* description) {
    LOG_ERROR("GLFW hatası " + std::to_string(code) + ": " + description);
}

// Pencere boyutu değişince OpenGL'in çizim alanını (viewport) da yeni boyuta uydur.
// Küçültülmüş (minimize) pencerede boyut 0x0 gelir; glViewport bunu sorunsuz kabul eder.
void onFramebufferResize(GLFWwindow* /*window*/, int width, int height) {
    glViewport(0, 0, width, height);
}

std::string glString(GLenum name) {
    const GLubyte* text = glGetString(name);
    return text ? reinterpret_cast<const char*>(text) : "?";
}

} // namespace

Window::Window(const WindowConfig& config) {
    glfwSetErrorCallback(onGlfwError);
    if (!glfwInit()) {
        LOG_ERROR("GLFW başlatılamadı.");
        return;
    }

    // OpenGL 3.3 Core istiyoruz: eski (deprecated) fonksiyonlar olmadan, modern OpenGL.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    m_handle = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    if (!m_handle) {
        LOG_ERROR("Pencere açılamadı. Ekran kartın OpenGL 3.3'ü destekliyor mu? Sürücüyü güncellemeyi dene.");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_handle);

    // OpenGL fonksiyonları sürücünün içinde yaşar; adreslerini çalışma anında glad yükler.
    if (gladLoadGL(glfwGetProcAddress) == 0) {
        LOG_ERROR("OpenGL fonksiyonları yüklenemedi (glad).");
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
        glfwTerminate();
        return;
    }
    LOG_INFO("OpenGL " + glString(GL_VERSION) + " | " + glString(GL_RENDERER));

    glfwSwapInterval(config.vsync ? 1 : 0);
    glfwSetFramebufferSizeCallback(m_handle, onFramebufferResize);

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(m_handle, &width, &height);
    glViewport(0, 0, width, height);
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
    }
}

bool Window::isOpen() const {
    return m_handle != nullptr && !glfwWindowShouldClose(m_handle);
}

void Window::setTitle(const std::string& title) {
    if (m_handle) {
        glfwSetWindowTitle(m_handle, title.c_str());
    }
}

void Window::swapAndPoll() {
    if (!m_handle) {
        return;
    }
    glfwSwapBuffers(m_handle);
    glfwPollEvents();
}

} // namespace engine
