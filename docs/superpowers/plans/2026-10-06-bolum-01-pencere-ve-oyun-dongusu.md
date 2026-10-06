# Bölüm 01 — Pencere ve Oyun Döngüsü Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Derleme altyapısını kurmak ve rengi zamanla değişen, başlığında FPS gösteren, sabit zaman adımlı oyun döngüsüyle çalışan bir pencere açmak.

**Architecture:** `engine` statik kütüphanesi (`core/` + küçük bir `renderer/RenderCommand`) ve onu kullanan `example_01_window` programı. Ekran gerektirmeyen mantık (`FixedTimestep`, `FpsCounter`, `formatLogLine`) doctest ile önce-test (TDD) yazılır; pencere kısmı programı çalıştırarak doğrulanır.

**Tech Stack:** C++17, CMake 3.31 (VS ile gelen), MSVC 14.44, GLFW 3.4 (FetchContent), glad2 2.0.8 ile üretilmiş OpenGL 3.3 Core yükleyicisi (repoya eklenir), doctest v2.4.12 (FetchContent).

**Spec:** `docs/superpowers/specs/2026-10-06-learning-game-engine-design.md`

## Global Constraints

- C++17; `CMAKE_CXX_EXTENSIONS OFF`.
- CMake en az 3.21 (CMakePresets v3 için; spec'teki 3.20 bu nedenle 3.21'e çıkarıldı).
- MSVC'de `/utf-8` zorunlu (Türkçe yorumlar ve string'ler), motor ve testlerde `/W4 /permissive-`.
- Derleme klasörü OneDrive dışında: `C:/dev/build/learning-game-engine/<preset>`.
- Bağımlılık kuralı: `core` hiçbir motor modülüne bağlı değildir; örnek programlar OpenGL'i doğrudan çağırmaz (`RenderCommand` üzerinden).
- Kod tanımlayıcıları İngilizce, yorumlar ve dokümanlar Türkçe; namespace `engine`.
- Include kökü `engine/` klasörüdür: `#include "core/Log.h"`.
- GLFW her yerde `GLFW_INCLUDE_NONE` tanımlanarak dahil edilir; OpenGL başlıkları yalnızca `<glad/gl.h>` ile gelir.
- Exception hiyerarşisi yok; kurtarılamaz hata → `LOG_ERROR` + `run()` 1 döndürür.
- Her task sonunda README "Günlük" güncellenir, commit + `git push` yapılır (kullanıcı kuralı).

Komutlarda kullanılan değişkenler (PowerShell):

```powershell
$cmake = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$ctest = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe"
```

## Review Focus

1. **NaN veya negatif kare süresi** (`glfwGetTime` tuhaflığı, saat geri gitmesi) → `FixedTimestep` kilitlenmemeli, sonraki normal karede çalışmaya devam etmeli. (Task 2 testi)
2. **Çok uzun kare** (debugger breakpoint'i, Windows'ta pencereyi sürüklerken döngünün donması) → fizik yüzlerce adım atmaya çalışıp "ölüm sarmalına" girmemeli; kare 0.25 sn ile sınırlanmalı. (Task 2 testi)
3. **Sıfır/negatif sabit adım ayarı** → `while (acc >= step)` sonsuz döngüye girmemeli; varsayılan 1/60'a dönmeli. (Task 2 testi)
4. **Türkçe karakterler** (log mesajları, pencere başlığı "Bölüm") → konsolda ve başlıkta bozulmadan görünmeli. (`/utf-8` + `SetConsoleOutputCP`; Task 1 testi + Task 4 manuel kontrol)
5. **Pencere küçültme (minimize) / yeniden boyutlandırma** → program çökmemeli, viewport yeni boyuta uymalı, geri açılınca çizim sürmeli. (Task 4 manuel kontrol)

---

## Dosya yapısı (bu bölüm sonunda)

```
CMakeLists.txt                 kök: proje ayarları, bağımlılıklar, alt klasörler
CMakePresets.json              msvc configure + debug/release build + debug test preset'leri
external/glad/                 glad2 ile üretilmiş OpenGL 3.3 Core yükleyici
  CMakeLists.txt
  include/glad/gl.h, include/KHR/khrplatform.h, src/gl.c
engine/CMakeLists.txt          "engine" statik kütüphanesi
engine/core/Log.h/.cpp         LOG_INFO/WARN/ERROR, formatLogLine
engine/core/FixedTimestep.h/.cpp  sabit zaman adımı biriktiricisi
engine/core/FpsCounter.h/.cpp  saniyelik FPS sayacı
engine/core/Window.h/.cpp      GLFW penceresi + OpenGL bağlamı (RAII)
engine/core/Application.h/.cpp oyun döngüsü
engine/renderer/RenderCommand.h/.cpp  setClearColor, clear
examples/CMakeLists.txt
examples/01_window/main.cpp    Bölüm 01 demosu
tests/CMakeLists.txt
tests/test_main.cpp, test_log.cpp, test_fixed_timestep.cpp, test_fps_counter.cpp
docs/bolum-01.md               Bölüm 01 açıklaması
```

---

### Task 1: Derleme altyapısı, glad ve Log

**Files:**
- Create: `CMakeLists.txt`, `CMakePresets.json`
- Create: `external/glad/CMakeLists.txt` + üretilen `external/glad/include/**`, `external/glad/src/gl.c`
- Create: `engine/CMakeLists.txt`, `engine/core/Log.h`, `engine/core/Log.cpp`
- Create: `tests/CMakeLists.txt`, `tests/test_main.cpp`, `tests/test_log.cpp`
- Create: `examples/CMakeLists.txt` (bu task'te boş yorum satırı)
- Modify: `README.md` (Günlük)

**Interfaces:**
- Produces:
  - `enum class engine::LogLevel { Info, Warn, Error };`
  - `std::string engine::formatLogLine(LogLevel level, std::string_view file, int line, std::string_view message);` → `"[WARN] Window.cpp:42 mesaj"`
  - `void engine::logMessage(LogLevel level, const char* file, int line, std::string_view message);`
  - Makrolar: `LOG_INFO(msg)`, `LOG_WARN(msg)`, `LOG_ERROR(msg)` (msg: `std::string` veya string literal)
  - CMake hedefleri: `glad`, `engine`, `engine_tests`; fonksiyon `lge_set_warnings(target)`

- [ ] **Step 1: glad yükleyicisini üret**

```powershell
$scratch = "<scratchpad dizini>"
python -m venv "$scratch\gladenv"
& "$scratch\gladenv\Scripts\pip.exe" install glad2==2.0.8
& "$scratch\gladenv\Scripts\glad.exe" --api gl:core=3.3 --out-path external/glad c
Get-ChildItem -Recurse external/glad -Name
```
Expected: `include\glad\gl.h`, `include\KHR\khrplatform.h`, `src\gl.c`.

- [ ] **Step 2: `external/glad/CMakeLists.txt` yaz**

```cmake
# glad: OpenGL fonksiyonlarının adreslerini çalışma anında ekran kartı sürücüsünden yükler.
# Bu dosyalar glad2 2.0.8 ile üretildi: glad --api gl:core=3.3 --out-path external/glad c
add_library(glad STATIC src/gl.c)
target_include_directories(glad PUBLIC include)
```

- [ ] **Step 3: Kök `CMakeLists.txt` ve `CMakePresets.json` yaz**

`CMakeLists.txt`:
```cmake
cmake_minimum_required(VERSION 3.21)
project(LearningGameEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Kaynak dosyalar UTF-8 (Türkçe yorumlar/string'ler); MSVC'ye bunu açıkça söylüyoruz.
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Kendi kodumuz için sıkı uyarılar. Üçüncü parti kütüphanelere uygulamıyoruz.
function(lge_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# --- Dış bağımlılıklar: ilk configure sırasında otomatik indirilir ---
include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.12
    GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(glfw doctest)

add_subdirectory(external/glad)

# --- Kendi kodumuz ---
add_subdirectory(engine)
add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)
```

`CMakePresets.json`:
```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "msvc",
      "displayName": "Visual Studio 2022 (x64)",
      "generator": "Visual Studio 17 2022",
      "architecture": "x64",
      "binaryDir": "C:/dev/build/learning-game-engine/${presetName}"
    }
  ],
  "buildPresets": [
    { "name": "debug", "configurePreset": "msvc", "configuration": "Debug" },
    { "name": "release", "configurePreset": "msvc", "configuration": "Release" }
  ],
  "testPresets": [
    {
      "name": "debug",
      "configurePreset": "msvc",
      "configuration": "Debug",
      "output": { "outputOnFailure": true }
    }
  ]
}
```

`examples/CMakeLists.txt`:
```cmake
# Her bölümün örnek programı buraya eklenir.
```

- [ ] **Step 4: Log için başarısız testi yaz**

`tests/test_main.cpp`:
```cpp
// doctest'in main() fonksiyonunu burada, tek bir kez üretiyoruz.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
```

`tests/test_log.cpp`:
```cpp
#include <doctest/doctest.h>

#include "core/Log.h"

using engine::formatLogLine;
using engine::LogLevel;

TEST_CASE("Log satırı seviye, dosya adı, satır ve mesajı içerir") {
    CHECK(formatLogLine(LogLevel::Info, "main.cpp", 7, "merhaba") == "[INFO] main.cpp:7 merhaba");
}

TEST_CASE("Dosya yolunun yalnızca dosya adı kısmı yazılır") {
    CHECK(formatLogLine(LogLevel::Warn, "C:\\proje\\engine\\core\\Window.cpp", 42, "uyarı")
          == "[WARN] Window.cpp:42 uyarı");
    CHECK(formatLogLine(LogLevel::Error, "/home/ali/engine/Window.cpp", 1, "hata")
          == "[ERROR] Window.cpp:1 hata");
}

TEST_CASE("Türkçe karakterler olduğu gibi korunur") {
    CHECK(formatLogLine(LogLevel::Info, "a.cpp", 3, "çğıöşü ÇĞİÖŞÜ") == "[INFO] a.cpp:3 çğıöşü ÇĞİÖŞÜ");
}
```

`tests/CMakeLists.txt`:
```cmake
add_executable(engine_tests
    test_main.cpp
    test_log.cpp)
target_link_libraries(engine_tests PRIVATE engine doctest::doctest)
lge_set_warnings(engine_tests)

add_test(NAME engine_tests COMMAND engine_tests)
```

`engine/CMakeLists.txt` (henüz Log.cpp yokken derleme hatası görmek için):
```cmake
add_library(engine STATIC
    core/Log.cpp)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine PUBLIC glfw glad)
lge_set_warnings(engine)
```

- [ ] **Step 5: Configure + build, başarısızlığı gör**

```powershell
& $cmake --preset msvc
& $cmake --build --preset debug
```
Expected: configure başarılı (GLFW ve doctest indirilir); build `Log.cpp` / `core/Log.h` bulunamadığı için FAIL.

- [ ] **Step 6: Log'u yaz**

`engine/core/Log.h`:
```cpp
#pragma once

#include <string>
#include <string_view>

namespace engine {

enum class LogLevel { Info, Warn, Error };

// "[WARN] Window.cpp:42 mesaj" biçiminde tek bir satır üretir (renk kodu olmadan).
// Dosya yolunun yalnızca dosya adı kısmı kullanılır.
std::string formatLogLine(LogLevel level, std::string_view file, int line, std::string_view message);

// Satırı konsola renkli yazar. Error seviyesi stderr'e, diğerleri stdout'a gider.
// Doğrudan çağırmak yerine aşağıdaki LOG_ makrolarını kullan.
void logMessage(LogLevel level, const char* file, int line, std::string_view message);

} // namespace engine

// Makrolar, çağrıldıkları yerin dosya adını (__FILE__) ve satırını (__LINE__)
// otomatik ekler; bir fonksiyon bunu kendi başına bilemezdi.
#define LOG_INFO(msg)  ::engine::logMessage(::engine::LogLevel::Info,  __FILE__, __LINE__, (msg))
#define LOG_WARN(msg)  ::engine::logMessage(::engine::LogLevel::Warn,  __FILE__, __LINE__, (msg))
#define LOG_ERROR(msg) ::engine::logMessage(::engine::LogLevel::Error, __FILE__, __LINE__, (msg))
```

`engine/core/Log.cpp`:
```cpp
#include "core/Log.h"

#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace engine {

namespace {

const char* levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "?";
}

// ANSI renk kodları: terminale "bundan sonrasını şu renkte yaz" der.
const char* levelColor(LogLevel level) {
    switch (level) {
        case LogLevel::Info:  return "\x1b[37m"; // beyaz
        case LogLevel::Warn:  return "\x1b[33m"; // sarı
        case LogLevel::Error: return "\x1b[31m"; // kırmızı
    }
    return "";
}

std::string_view fileNameOnly(std::string_view path) {
    const auto slash = path.find_last_of("/\\");
    return slash == std::string_view::npos ? path : path.substr(slash + 1);
}

// Konsol ayarlarını yalnızca ilk log çağrısında bir kez yapar.
struct ConsoleSetup {
    bool colorStdout = true;
    bool colorStderr = true;

    ConsoleSetup() {
#ifdef _WIN32
        // Windows konsolu varsayılan olarak UTF-8 değildir; Türkçe karakterler bozulmasın.
        SetConsoleOutputCP(CP_UTF8);
        colorStdout = enableColors(STD_OUTPUT_HANDLE);
        colorStderr = enableColors(STD_ERROR_HANDLE);
#endif
    }

#ifdef _WIN32
    // Çıktı bir dosyaya yönlendirildiyse GetConsoleMode başarısız olur; o zaman renk kodu yazmayız.
    static bool enableColors(DWORD which) {
        HANDLE handle = GetStdHandle(which);
        DWORD mode = 0;
        if (handle == INVALID_HANDLE_VALUE || !GetConsoleMode(handle, &mode)) {
            return false;
        }
        return SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
    }
#endif
};

const ConsoleSetup& consoleSetup() {
    static const ConsoleSetup setup; // "static yerel değişken": ilk çağrıda bir kez oluşturulur
    return setup;
}

} // namespace

std::string formatLogLine(LogLevel level, std::string_view file, int line, std::string_view message) {
    std::string result = "[";
    result += levelName(level);
    result += "] ";
    result += fileNameOnly(file);
    result += ":";
    result += std::to_string(line);
    result += " ";
    result += message;
    return result;
}

void logMessage(LogLevel level, const char* file, int line, std::string_view message) {
    const ConsoleSetup& console = consoleSetup();
    const bool isError = (level == LogLevel::Error);
    std::FILE* out = isError ? stderr : stdout;
    const bool useColor = isError ? console.colorStderr : console.colorStdout;

    const std::string text = formatLogLine(level, file, line, message);
    if (useColor) {
        std::fprintf(out, "%s%s\x1b[0m\n", levelColor(level), text.c_str());
    } else {
        std::fprintf(out, "%s\n", text.c_str());
    }
    std::fflush(out);
}

} // namespace engine
```

- [ ] **Step 7: Build + test, geçtiğini gör**

```powershell
& $cmake --build --preset debug
& $ctest --preset debug
```
Expected: build uyarısız; `100% tests passed, 0 tests failed out of 1`.

- [ ] **Step 8: README Günlük + commit + push**

README "Günlük"e "Adım 1.1 — Derleme altyapısı ve Log" girdisi: CMakePresets/OneDrive dışı build, FetchContent, glad'ın neden repoya eklendiği (Python gerektirmesin diye), Log makrolarının neden makro olduğu (`__FILE__`/`__LINE__`), ilk testler.

```powershell
git add CMakeLists.txt CMakePresets.json external engine tests examples README.md
git commit -m "Bölüm 01: derleme altyapısı, glad ve Log"
git push
```

---

### Task 2: FixedTimestep (sabit zaman adımı)

**Files:**
- Create: `engine/core/FixedTimestep.h`, `engine/core/FixedTimestep.cpp`
- Create: `tests/test_fixed_timestep.cpp`
- Modify: `engine/CMakeLists.txt` (kaynak ekle), `tests/CMakeLists.txt` (test ekle), `README.md`

**Interfaces:**
- Produces:
  - `class engine::FixedTimestep`
    - `static constexpr float kDefaultStep = 1.0f / 60.0f;`
    - `static constexpr float kDefaultMaxFrame = 0.25f;`
    - `explicit FixedTimestep(float stepSeconds = kDefaultStep, float maxFrameSeconds = kDefaultMaxFrame);`
    - `int advance(float frameSeconds);` → bu karede çalıştırılacak sabit adım sayısı
    - `float step() const;`

- [ ] **Step 1: Başarısız testi yaz**

`tests/test_fixed_timestep.cpp`:
```cpp
#include <doctest/doctest.h>

#include <limits>

#include "core/FixedTimestep.h"

using engine::FixedTimestep;

// Not: Testlerde 0.25, 0.125 gibi 2'nin kuvveti olan kesirler kullanıyoruz. Bunlar float'ta
// tam olarak temsil edilir; böylece yuvarlama hataları testleri rastgele bozmaz.

TEST_CASE("Adımdan kısa kare hiç sabit adım üretmez") {
    FixedTimestep ts(0.25f, 1.0f);
    CHECK(ts.advance(0.125f) == 0);
}

TEST_CASE("Kısa kareler birikir; toplam bir adıma ulaşınca bir adım üretilir") {
    FixedTimestep ts(0.25f, 1.0f);
    CHECK(ts.advance(0.125f) == 0);
    CHECK(ts.advance(0.125f) == 1);
}

TEST_CASE("Uzun kare birden çok adım üretir ve artan süre bir sonraki kareye saklanır") {
    FixedTimestep ts(0.25f, 1.0f);
    CHECK(ts.advance(0.625f) == 2); // 0.5 sn kullanıldı, 0.125 sn kaldı
    CHECK(ts.advance(0.125f) == 1); // 0.125 + 0.125 = 0.25
}

TEST_CASE("Çok uzun kare sınırlanır (ölüm sarmalı koruması)") {
    FixedTimestep ts(0.125f, 0.5f);
    CHECK(ts.advance(10.0f) == 4); // 10 sn değil, en fazla 0.5 sn sayılır
}

TEST_CASE("Negatif ve NaN kare süresi yok sayılır, sayaç bozulmaz") {
    FixedTimestep ts(0.25f, 1.0f);
    CHECK(ts.advance(-1.0f) == 0);
    CHECK(ts.advance(std::numeric_limits<float>::quiet_NaN()) == 0);
    CHECK(ts.advance(0.25f) == 1);
}

TEST_CASE("Sıfır veya negatif adım varsayılan 1/60 sn'ye döner (sonsuz döngü koruması)") {
    CHECK(FixedTimestep(0.0f).step() == doctest::Approx(1.0f / 60.0f));
    CHECK(FixedTimestep(-1.0f).step() == doctest::Approx(1.0f / 60.0f));
}

TEST_CASE("Varsayılan ayarlarla 1/60 sn'lik kare tam bir adım üretir") {
    FixedTimestep ts;
    CHECK(ts.advance(1.0f / 60.0f) == 1);
}
```

`tests/CMakeLists.txt` içindeki kaynak listesine `test_fixed_timestep.cpp` ekle.

- [ ] **Step 2: Build, başarısızlığı gör**

```powershell
& $cmake --build --preset debug
```
Expected: FAIL — `core/FixedTimestep.h` bulunamadı.

- [ ] **Step 3: FixedTimestep'i yaz**

`engine/core/FixedTimestep.h`:
```cpp
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

    // Bu karenin süresini biriktirir ve şimdi çalıştırılması gereken sabit adım sayısını döndürür.
    // Negatif veya NaN süre 0 sayılır.
    int advance(float frameSeconds);

    float step() const { return m_step; }

private:
    float m_step;
    float m_maxFrame;
    float m_accumulator = 0.0f;
};

} // namespace engine
```

`engine/core/FixedTimestep.cpp`:
```cpp
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
```

`engine/CMakeLists.txt` kaynak listesine `core/FixedTimestep.cpp` ekle.

- [ ] **Step 4: Build + test, geçtiğini gör**

```powershell
& $cmake --build --preset debug
& $ctest --preset debug
```
Expected: `100% tests passed`.

- [ ] **Step 5: README Günlük + commit + push**

README'ye "Adım 1.2 — Sabit zaman adımı": problem (değişken dt'de fizik), biriktirici fikri, 0.25 sn sınırı, NaN tuzağı.

```powershell
git add engine tests README.md
git commit -m "Bölüm 01: FixedTimestep ve testleri"
git push
```

---

### Task 3: FpsCounter

**Files:**
- Create: `engine/core/FpsCounter.h`, `engine/core/FpsCounter.cpp`
- Create: `tests/test_fps_counter.cpp`
- Modify: `engine/CMakeLists.txt`, `tests/CMakeLists.txt`, `README.md`

**Interfaces:**
- Produces:
  - `class engine::FpsCounter`
    - `bool tick(float frameSeconds);` → 1 saniye dolduğunda `true`
    - `int fps() const;` → son tamamlanan saniyenin FPS'i (başta 0)

- [ ] **Step 1: Başarısız testi yaz**

`tests/test_fps_counter.cpp`:
```cpp
#include <doctest/doctest.h>

#include <limits>

#include "core/FpsCounter.h"

using engine::FpsCounter;

TEST_CASE("1 saniye dolmadan FPS raporlanmaz") {
    FpsCounter counter;
    CHECK_FALSE(counter.tick(0.5f));
    CHECK(counter.fps() == 0);
}

TEST_CASE("1 saniye dolunca o sürede çizilen kare sayısı raporlanır") {
    FpsCounter counter;
    CHECK_FALSE(counter.tick(0.25f));
    CHECK_FALSE(counter.tick(0.25f));
    CHECK_FALSE(counter.tick(0.25f));
    CHECK(counter.tick(0.25f));
    CHECK(counter.fps() == 4);
}

TEST_CASE("Rapordan sonra sayaç yeni saniye için sıfırdan başlar") {
    FpsCounter counter;
    CHECK(counter.tick(1.0f));
    CHECK(counter.fps() == 1);
    CHECK_FALSE(counter.tick(0.5f));
    CHECK(counter.tick(0.5f));
    CHECK(counter.fps() == 2);
}

TEST_CASE("Negatif ve NaN süre zaman olarak sayılmaz ama kare olarak sayılır") {
    FpsCounter counter;
    CHECK_FALSE(counter.tick(-1.0f));
    CHECK_FALSE(counter.tick(std::numeric_limits<float>::quiet_NaN()));
    CHECK(counter.tick(1.0f));
    CHECK(counter.fps() == 3);
}
```

`tests/CMakeLists.txt` kaynak listesine `test_fps_counter.cpp` ekle.

- [ ] **Step 2: Build, başarısızlığı gör**

```powershell
& $cmake --build --preset debug
```
Expected: FAIL — `core/FpsCounter.h` bulunamadı.

- [ ] **Step 3: FpsCounter'ı yaz**

`engine/core/FpsCounter.h`:
```cpp
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
```

`engine/core/FpsCounter.cpp`:
```cpp
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
```

`engine/CMakeLists.txt` kaynak listesine `core/FpsCounter.cpp` ekle.

- [ ] **Step 4: Build + test, geçtiğini gör**

```powershell
& $cmake --build --preset debug
& $ctest --preset debug
```
Expected: `100% tests passed`.

- [ ] **Step 5: README Günlük + commit + push**

```powershell
git add engine tests README.md
git commit -m "Bölüm 01: FpsCounter ve testleri"
git push
```

---

### Task 4: Window, RenderCommand, Application ve örnek program

**Files:**
- Create: `engine/core/Window.h`, `engine/core/Window.cpp`
- Create: `engine/renderer/RenderCommand.h`, `engine/renderer/RenderCommand.cpp`
- Create: `engine/core/Application.h`, `engine/core/Application.cpp`
- Create: `examples/01_window/main.cpp`
- Modify: `engine/CMakeLists.txt`, `examples/CMakeLists.txt`, `README.md`

**Interfaces:**
- Consumes: `LOG_*` (Task 1), `FixedTimestep` (Task 2), `FpsCounter` (Task 3)
- Produces:
  - `struct engine::WindowConfig { std::string title; int width = 1280; int height = 720; bool vsync = true; };`
  - `class engine::Window` — `explicit Window(const WindowConfig&)`, kopyalanamaz; `bool isValid() const`, `bool isOpen() const`, `void setTitle(const std::string&)`, `void swapAndPoll()`
  - `namespace engine::RenderCommand` — `void setClearColor(float r, float g, float b, float a = 1.0f)`, `void clear()`
  - `struct engine::ApplicationConfig { WindowConfig window; float fixedStep = FixedTimestep::kDefaultStep; };`
  - `class engine::Application` — `explicit Application(const ApplicationConfig& = {})`, `int run()` (0: normal, 1: pencere açılamadı); korumalı sanal kancalar `onInit()`, `onFixedUpdate(float step)`, `onUpdate(float dt)`, `onRender()`, `onShutdown()`; yardımcılar `float totalTime() const`, `int fps() const`

- [ ] **Step 1: Window'u yaz**

`engine/core/Window.h`:
```cpp
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
```

`engine/core/Window.cpp`:
```cpp
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
```

- [ ] **Step 2: RenderCommand'ı yaz**

`engine/renderer/RenderCommand.h`:
```cpp
#pragma once

// Oyun kodunun OpenGL'i doğrudan çağırmaması için en ince katman.
// İleride (Bölüm 09) 3D için de aynı komutlar kullanılacak.
namespace engine::RenderCommand {

void setClearColor(float r, float g, float b, float a = 1.0f);
void clear(); // renk ve derinlik tamponlarını temizler

} // namespace engine::RenderCommand
```

`engine/renderer/RenderCommand.cpp`:
```cpp
#include "renderer/RenderCommand.h"

#include <glad/gl.h>

namespace engine::RenderCommand {

void setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace engine::RenderCommand
```

- [ ] **Step 3: Application'ı yaz**

`engine/core/Application.h`:
```cpp
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
```

`engine/core/Application.cpp`:
```cpp
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
        const float dt = static_cast<float>(now - previousTime);
        previousTime = now;
        m_totalTime = static_cast<float>(now - startTime);

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

        if (m_fpsCounter.tick(dt)) {
            m_window.setTitle(m_baseTitle + " | FPS: " + std::to_string(m_fpsCounter.fps()));
        }
    }

    onShutdown();
    return 0;
}

} // namespace engine
```

`engine/CMakeLists.txt` son hali:
```cmake
add_library(engine STATIC
    core/Log.cpp
    core/FixedTimestep.cpp
    core/FpsCounter.cpp
    core/Window.cpp
    core/Application.cpp
    renderer/RenderCommand.cpp)
target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(engine PUBLIC glfw glad)
lge_set_warnings(engine)
```

- [ ] **Step 4: Örnek programı yaz**

`examples/01_window/main.cpp`:
```cpp
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
```

`examples/CMakeLists.txt`:
```cmake
# Her bölümün örnek programı buraya eklenir.
add_executable(example_01_window 01_window/main.cpp)
target_link_libraries(example_01_window PRIVATE engine)
lge_set_warnings(example_01_window)
```

- [ ] **Step 5: Build + test**

```powershell
& $cmake --build --preset debug
& $ctest --preset debug
```
Expected: build uyarısız, `100% tests passed`.

- [ ] **Step 6: Programı çalıştır ve doğrula (manuel)**

```powershell
$exe = "C:\dev\build\learning-game-engine\msvc\examples\Debug\example_01_window.exe"
$p = Start-Process $exe -PassThru -RedirectStandardOutput "$scratch\run01.txt" -RedirectStandardError "$scratch\run01err.txt"
# ~5 sn bekle (Monitor/until döngüsü), ekran görüntüsü al, sonra:
Stop-Process -Id $p.Id
Get-Content "$scratch\run01.txt"; Get-Content "$scratch\run01err.txt"
```
Expected:
- `[INFO] Window.cpp:.. OpenGL 3.3.0 ...` (veya daha yüksek bir sürüm) satırı.
- Her saniye `Son 1 saniyede 60 sabit adım çalıştı` (59–61 arası kabul).
- stderr boş.
- Ekran görüntüsü: renkli dolu pencere; başlıkta `Bölüm 01 - Pencere ve Oyun Döngüsü | FPS: NN` (Türkçe "ö" bozulmadan).
- Pencere küçültülüp geri açılınca ve yeniden boyutlandırılınca çökme yok, renk tüm pencereyi kaplar (Review Focus 5).
- Pencere X ile kapatılınca program 0 çıkış koduyla biter.

- [ ] **Step 7: README Günlük + commit + push**

README'ye "Adım 1.3 — Pencere ve oyun döngüsü": RAII, `= delete`, ileri bildirim, glad'ın görevi, döngünün 3 aşaması, örnek çıktı.

```powershell
git add engine examples README.md
git commit -m "Bölüm 01: Window, Application, RenderCommand ve pencere örneği"
git push
```

---

### Task 5: Bölüm 01 dokümanı, README ve etiket

**Files:**
- Create: `docs/bolum-01.md`
- Modify: `README.md` (yol haritasında Bölüm 01 işaretli, "Nasıl derlenir/çalıştırılır" bölümü, Günlük)

- [ ] **Step 1: `docs/bolum-01.md` yaz**

Şu başlıklar, her biri gerçek içerikle (bu plandaki kod ve yorumlardan beslenerek):
1. **Bu bölümde ne yaptık** — çalışan sonucun 2–3 cümlelik tarifi.
2. **Derleme altyapısı** — CMake nedir; preset'ler; FetchContent; neden derleme klasörü OneDrive dışında; glad neden repoda.
3. **Oyun döngüsü** — döngünün 3 aşamasını gösteren ASCII diyagram; değişken dt'nin fizikte neden sorun olduğu (somut sayısal örnek: 30 FPS vs 144 FPS'te zıplayan top); biriktirici; 0.25 sn sınırı ve "ölüm sarmalı"; NaN tuzağı.
4. **C++ köşesi** — RAII; `= delete` ile kopyalamayı yasaklamak; ileri bildirim (forward declaration); `virtual` ve `override`; neden `virtual ~Application()`; üye kurulum sırası; makrolar ve `__FILE__`/`__LINE__`; `static` yerel değişken.
5. **Kodda gezinti** — dosya listesi ve her birinin tek cümlelik sorumluluğu, okunma sırası önerisi.
6. **Testler** — TDD döngüsü (kırmızı → yeşil), testlerde neden 2'nin kuvveti kesirler.
7. **Alıştırmalar** — (a) `vsync`'i kapatıp FPS'in ve sabit adım sayısının nasıl değiştiğini gözle; (b) `fixedStep`'i 1/30 yap, konsoldaki sayının neden 30'a düştüğünü açıkla; (c) pencereyi 2 sn sürükleyip bırak, konsolda ne gördüğünü 0.25 sn sınırıyla açıkla; (d) `FixedTimestep` için kendi testini yaz: 3 kare × 0.1 sn, adım 0.25 → kaç adım?
8. **Sırada ne var** — Bölüm 02: shader'lar ve ilk üçgen.

- [ ] **Step 2: README güncelle**

- Yol haritasında `- [x] Bölüm 01 — ...` ve `docs/bolum-01.md` bağlantısı.
- Yeni "Nasıl derlenir ve çalıştırılır" bölümü: Visual Studio ile klasörü açma (CMake preset'i otomatik algılanır) ve komut satırı (`cmake --preset msvc`, `cmake --build --preset debug`, `ctest --preset debug`, örnek exe yolu).
- Günlük'e "Bölüm 01 tamamlandı" özeti.

- [ ] **Step 3: Commit, etiket, push**

```powershell
git add docs/bolum-01.md README.md
git commit -m "Bölüm 01: açıklama dokümanı ve README"
git tag -a bolum-01 -m "Bölüm 01 — Pencere ve oyun döngüsü"
git push
git push origin bolum-01
```
Expected: GitHub'da `bolum-01` etiketi görünür.
