# Learning Game Engine — Tasarım (Spec)

- **Tarih:** 2026-10-06
- **Durum:** Kullanıcı incelemesinde
- **Repo:** https://github.com/Klister0/learning-game-engine

## 1. Amaç ve başarı ölçütleri

**Amaç:** Kullanıcının (C++ başlangıç seviyesi) bir oyun motorunun parçalarını *nasıl* ve
*neden* öyle tasarlandığını öğrenmesi için, birlikte ve adım adım yazılan bir C++ oyun motoru.
Özellik zenginliği hedef değildir; okunabilirlik ve anlaşılabilirlik önceliklidir.

**Başarı ölçütleri:**
1. Her bölüm sonunda proje temiz derlenir, testler geçer ve bölümün örnek programı çalışır.
2. Her bölümün Türkçe açıklama dokümanı (`docs/bolum-XX.md`) vardır: kavram, kodun önemli
   yerleri ve alıştırmalar.
3. Bölüm 08 sonunda motor kullanılarak oynanabilir bir Breakout oyunu çalışır.
4. Bölüm 09 sonunda aynı motor 2D kodu bozulmadan basit bir 3D sahne (perspektif kamera,
   mesh, ışık) çizer.
5. Tüm ilerleme GitHub'da: her değişiklik commit + push, her bölüm `bolum-XX` etiketi,
   README "Günlük" bölümünde adım adım özet.

## 2. Teknoloji

| Konu | Seçim |
|---|---|
| Dil | C++17 |
| Pencere / input | GLFW |
| Grafik | OpenGL 3.3 Core, yükleyici: glad |
| Matematik | glm |
| Resim yükleme | stb_image |
| Test | doctest |
| Derleme | CMake ≥ 3.20; bağımlılıklar `FetchContent` ile otomatik indirilir |
| Ortam | Windows 11, Visual Studio 2022 Community (MSVC) |

Proje CMake sayesinde taşınabilir yazılır, ancak yalnızca Windows'ta test edilir.
**Derleme klasörü OneDrive dışındadır:** `C:\dev\build\learning-game-engine`
(CMakePresets.json ile otomatik). Ses ve ağ kapsam dışıdır.

## 3. Proje yapısı

```
learning-game-engine/
├── CMakeLists.txt, CMakePresets.json
├── engine/            motor kütüphanesi (statik kütüphane: "engine")
│   ├── core/          Application, Window, Time, Log
│   ├── renderer/      Shader, Texture, Buffer'lar, VertexArray, Renderer2D, Camera
│   ├── input/         Input
│   ├── ecs/           Entity, Registry, component'ler
│   ├── physics/       AABB, çarpışma çözümü
│   └── scene/         Scene, SceneManager, AssetManager
├── examples/          bölüm demoları (01_window, 02_triangle, ...)
├── games/breakout/    demo oyun
├── assets/            shader'lar ve resimler
├── tests/             doctest birim testleri
└── docs/              bolum-XX.md açıklamaları, spec ve planlar
```

**Bağımlılık kuralı (tek yönlü):** `core` hiçbir motor modülüne bağlı değildir.
`renderer`, `input`, `physics` yalnızca `core`'a bağlıdır. `ecs` ve `scene` en üst katmandır.
Oyun/örnek kodu yalnızca `engine` arayüzünü kullanır, OpenGL'i doğrudan çağırmaz.

## 4. Çekirdek (core)

- **`Application`**: oyunlar bundan türer; `onInit()`, `onFixedUpdate(float)`,
  `onUpdate(float dt)`, `onRender()`, `onShutdown()` sanal fonksiyonlarını yazar.
  `run()` döngüsü:
  ```
  while (pencere açık):
      dt = kare süresi (en fazla 0.25 sn ile sınırlanır — "ölüm sarmalı"nı önler)
      birikim += dt
      while birikim >= FIXED_DT (1/60 sn): onFixedUpdate(FIXED_DT); birikim -= FIXED_DT
      onUpdate(dt); onRender(); window.swapAndPoll()
  ```
- **`Window`**: GLFW penceresi + OpenGL 3.3 Core bağlamı; RAII (kurucu açar, yıkıcı kapatır);
  kopyalanamaz.
- **`Time`**: kare süresi, toplam süre, FPS.
- **`Log`**: `LOG_INFO`, `LOG_WARN`, `LOG_ERROR` makroları; dosya:satır bilgisiyle renkli çıktı.

**Hata yönetimi:**
- Kurtarılamaz (pencere/bağlam açılamadı, OpenGL 3.3 yok): `LOG_ERROR` + düzgün kapanış,
  sıfırdan farklı çıkış kodu.
- Kurtarılabilir (resim bulunamadı): `LOG_WARN` + pembe-siyah "eksik doku" ile devam.
- Shader derleme/bağlama hatası: OpenGL info log'u dosya adıyla loglanır; geçersiz shader
  çizimde kullanılmaz.
- Exception hiyerarşisi kullanılmaz.

## 5. Renderer

**Alt katman (2D ve 3D ortak):** `Shader` (dosyadan yükle/derle, `setInt/setFloat/setVec4/setMat4`),
`VertexBuffer`, `IndexBuffer`, `VertexArray` (vertex düzeni tanımı), `Texture` (stb_image).
Hepsi RAII; kopyalanamaz, taşınabilir (move constructor/assignment).

**Üst katman `Renderer2D`:** `beginScene(const Camera&)`, `drawQuad(pos, size, color)`,
`drawSprite(pos, size, texture, tint)`, `endScene()`. Önce naif (quad başına draw call) yazılır
ve ölçülür; sonra batching (tek dinamik vertex buffer, doku slotları, dolunca flush) eklenir.
Kare başına draw call ve quad sayısı istatistik olarak sunulur.

**Kamera:** soyut `Camera` (`getViewProjection()`); `OrthographicCamera` (Bölüm 03),
`PerspectiveCamera` (Bölüm 09). 3D için alt katman aynen kalır; `Renderer3D` + `Mesh` +
temel (Phong) ışık shader'ı eklenir.

**Kapsam dışı:** Vulkan/DirectX, render graph, post-processing, metin çizimi (gerekirse
Breakout skoru için basit bitmap font).

## 6. Input

Polling tabanlı: `Input::isKeyDown(Key)`, `Input::wasKeyPressed(Key)`,
`Input::wasKeyReleased(Key)`, `Input::isMouseButtonDown(...)`, `Input::mousePosition()`.
GLFW geri çağrıları (callback) "bu kare" tablosunu doldurur; her kare başında "önceki kare"
tablosuna kopyalanır.

## 7. ECS

- `Entity` = `uint32_t` kimlik.
- Component'ler yalnızca veri: `Transform{position, scale, rotation}`,
  `SpriteRenderer{texture, color}`, `RigidBody{velocity, gravityScale}`, `BoxCollider{size, isStatic}`
  ve oyuna özel etiketler (`Ball`, `Brick`, `Paddle`).
- `Registry`: `create()`, `destroy(e)`, `add<T>(e, T)`, `get<T>(e)`, `has<T>(e)`,
  `remove<T>(e)`, `view<A, B, ...>()`. Her component türü kendi deposunda
  (ilk sürüm: `std::unordered_map<Entity, T>`). Döngü sırasında silme güvenliği için
  `destroy` ertelenir (kare sonunda uygulanır).
- Sistemler serbest fonksiyonlar: `movementSystem`, `collisionSystem`, `renderSystem`.
- `docs/bolum-05.md` OOP "GameObject + virtual Update" modeliyle karşılaştırma içerir.

## 8. Fizik

`onFixedUpdate` içinde: hız += yerçekimi·dt; konum += hız·dt. AABB kesişim testi; çözüm en az
iç içe geçme eksenine göre itme ve hızı o eksende yansıtma/sıfırlama. Çarpışma olayları
oyuna bir liste olarak verilir (ör. top–tuğla → tuğlayı sil). Kapsam dışı: dönen kutular,
eklemler, sürtünme modelleri, sürekli çarpışma algılama.

## 9. Sahne ve kaynaklar

- `Scene`: bir `Registry` + `onEnter/onExit/onFixedUpdate/onUpdate/onRender`.
- `SceneManager`: aktif sahne; geçişler kare sonunda uygulanır (menü → oyun → oyun bitti).
- `AssetManager`: yol → `std::shared_ptr<Texture>/<Shader>` önbelleği; aynı dosya bir kez yüklenir.
- Sahneler kodda kurulur (dosyadan yükleme yok).

## 10. Bölümler

| # | Konu | Çalışır sonuç | Testler |
|---|---|---|---|
| 01 | Pencere, oyun döngüsü, Time, Log | Renk değiştiren pencere, başlıkta FPS | sabit adım hesabı |
| 02 | Shader, buffer'lar, RAII/move | Renkli üçgen ve kare | — |
| 03 | Texture, OrthographicCamera, Renderer2D (naif → batching) | 10.000 sprite, draw call sayacı | kamera matrisleri |
| 04 | Input | Klavye ile hareket eden sprite | input durum geçişleri |
| 05 | ECS | Entity'lerle kurulmuş sahne | Registry |
| 06 | Fizik, AABB | Zıplayan kutular | AABB, çözüm |
| 07 | Scene, SceneManager, AssetManager | Menü ↔ oyun geçişi | önbellek |
| 08 | Breakout | Oynanabilir oyun | — |
| 09 | 3D: PerspectiveCamera, Mesh, ışık | Dönen ışıklı küp | perspektif matris |

Her bölüm ayrı bir uygulama planı (`docs/superpowers/plans/`) ile yürütülür.

## 11. Test ve doğrulama

- doctest birim testleri ekran gerektirmeyen mantık kodu için; `ctest` ile çalıştırılır.
- Görsel kısımlar bölüm örnek programı çalıştırılıp ekran görüntüsüyle doğrulanır.
- Yeni mantık kodu için önce test yazılır (TDD).

## 12. Çalışma şekli

- Her bölüm: doküman → kod + test → derle/doğrula → README güncelle → commit + push → etiket.
- Her değişiklik commit'lenir ve GitHub'a gönderilir; README "Günlük" bölümü güncel tutulur.
- Bölüm geçişlerinde kullanıcıya soru/alıştırma molası verilir.
- Yorumlar ve dokümanlar Türkçe; kod tanımlayıcıları İngilizce.
