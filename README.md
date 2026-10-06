# Learning Game Engine

C++ ile sıfırdan, **öğrenme amaçlı** yazılan bir oyun motoru. Amaç çok özellikli bir motor
değil; bir oyun motorunun parçalarının *nasıl* ve *neden* öyle tasarlandığını adım adım
anlamak. Her adım çalışır durumda bırakılır ve bu dosyada belgelenir.

## Kararlar

| Konu | Karar | Neden |
|---|---|---|
| Amaç | Kişisel öğrenme projesi | Okunabilirlik ve anlama, özellik sayısından önemli |
| Kapsam | Önce 2D, sonra 3D'ye genişleme | Mimariyi grafik matematiğinde boğulmadan öğrenmek |
| Pencere / Input | GLFW | Sadece pencere ve input; çizimi biz yazıyoruz |
| Grafik API | OpenGL 3.3 Core | 2D ve 3D'de aynı model; renderer'ın içini öğrenmek |
| Matematik | glm | Shader'larla uyumlu vektör/matris kütüphanesi |
| Derleme | CMake + Visual Studio 2022 (MSVC) | Bağımlılıklar FetchContent ile otomatik iner; görsel debugger |
| C++ seviyesi | Başlangıç | Modern C++ özellikleri ilk kullanıldıkları yerde açıklanır |

## Yol Haritası

Her bölüm sonunda çalıştırılabilir bir sonuç olur ve bir git etiketi (`bolum-XX`) alır.

- [ ] Bölüm 01 — Pencere ve oyun döngüsü (sabit zaman adımı, delta time)
- [ ] Bölüm 02 — Shader'lar ve ekrana ilk üçgen
- [ ] Bölüm 03 — Sprite renderer, doku yükleme, ortografik kamera
- [ ] Bölüm 04 — Input sistemi
- [ ] Bölüm 05 — Entity-Component sistemi
- [ ] Bölüm 06 — Basit fizik ve AABB çarpışma
- [ ] Bölüm 07 — Sahne yönetimi ve kaynak (asset) yöneticisi
- [ ] Bölüm 08 — Demo oyun: Breakout
- [ ] Bölüm 09 — 3D'ye geçiş: perspektif kamera, mesh, ışık

## Günlük

Yapılan her adım burada, en yenisi en altta olacak şekilde yazılır.

### Adım 0 — Proje kurulumu
- Hedefler ve teknoloji seçimleri belirlendi (yukarıdaki tablo).
- Geliştirme ortamı olarak Visual Studio 2022 Community ("C++ ile masaüstü geliştirme"
  iş yükü: MSVC derleyicisi, Windows SDK, CMake) seçildi; `winget` ile kuruluyor.
- Bu repo oluşturuldu.
- **Proje yapısı kararlaştırıldı:** motor bir kütüphane (`engine/`), oyunlar onu kullanan
  programlar (`examples/`, `games/`). Katmanlar tek yönlü bağımlı:
  `core` → `renderer` / `input` / `physics` → `ecs` / `scene`. Oyun OpenGL'e doğrudan
  dokunmaz; bu sayede 3D'ye geçişte sadece `renderer` büyür.
- Derleme klasörü OneDrive dışında (`C:\dev\build\learning-game-engine`) tutulacak;
  OneDrive'ın yüzlerce MB'lık derleme çıktısını senkronlamaya çalışması derlemeyi yavaşlatır.
- **Çekirdek tasarımı kararlaştırıldı:** `Application::run()` sabit zaman adımlı döngü
  çalıştırır (`onFixedUpdate` 1/60 sn — fizik; `onUpdate(dt)`; `onRender()`). `Window`
  GLFW'yi RAII ile sarmalar. Hatalar: kurtarılamazsa logla ve düzgün kapat; kurtarılabilirse
  (ör. eksik resim) uyar ve pembe-siyah "eksik doku" ile devam et. Exception hiyerarşisi yok.
- **Renderer tasarımı kararlaştırıldı:** alt katman OpenGL sarmalayıcıları (`Shader`,
  `VertexBuffer`/`IndexBuffer`/`VertexArray`, `Texture` — RAII, kopyalanamaz/taşınabilir);
  üst katman `Renderer2D` (önce naif, sonra batching ile tek draw call). `Camera` soyut taban:
  `OrthographicCamera` (2D), ileride `PerspectiveCamera` (3D). 3D'ye geçişte alt katman aynen
  kalır, yanına `Renderer3D` + `Mesh` eklenir.
