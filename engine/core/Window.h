#pragma once

#include <string>

// GLFW'nin pencere tipini yalnızca "ileri bildirim" (forward declaration) ile tanıtıyoruz.
// Böylece bu başlığı dahil eden herkes GLFW başlığını da dahil etmek zorunda kalmaz.
struct GLFWwindow;

namespace engine {

struct WindowConfig {
    std::string title = "Learning Game Engine";
    int width = 1280;
    int height = 720;
    bool vsync = true; // ekran yenileme hızına kilitle (genelde 60 FPS)
};

// Bir işletim sistemi penceresi ve ona bağlı OpenGL 3.3 Core bağlamı.
//
// RAII: kurucu (constructor) pencereyi açar, yıkıcı (destructor) kapatır. Nesne kapsam
// dışına çıktığında temizlik otomatik olur; "kapatmayı unutmak" mümkün değildir.
// Açılış başarısız olursa nesne yine oluşur ama isValid() false döner.
//
// Not: GLFW'yi de bu sınıf başlatıp kapatır; aynı anda tek bir Window olmalıdır.
class Window {
public:
    explicit Window(const WindowConfig& config);
    ~Window();

    // Kopyalama yasak: iki Window nesnesi aynı pencereyi gösterseydi ikisinin yıkıcısı da
    // onu kapatmaya çalışırdı. "= delete" derleyiciye bu fonksiyonları üretmemesini söyler.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool isValid() const { return m_handle != nullptr; }
    bool isOpen() const; // geçerli ve kullanıcı kapatmak istemedi
    void setTitle(const std::string& title);
    void swapAndPoll(); // çizilen kareyi göster, işletim sistemi olaylarını işle

private:
    GLFWwindow* m_handle = nullptr;
};

} // namespace engine
