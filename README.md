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

- [x] Bölüm 01 — Pencere ve oyun döngüsü (sabit zaman adımı, delta time) → [docs/bolum-01.md](docs/bolum-01.md)
- [ ] Bölüm 02 — Shader'lar ve ekrana ilk üçgen
- [ ] Bölüm 03 — Sprite renderer, doku yükleme, ortografik kamera
- [ ] Bölüm 04 — Input sistemi
- [ ] Bölüm 05 — Entity-Component sistemi
- [ ] Bölüm 06 — Basit fizik ve AABB çarpışma
- [ ] Bölüm 07 — Sahne yönetimi ve kaynak (asset) yöneticisi
- [ ] Bölüm 08 — Demo oyun: Breakout
- [ ] Bölüm 09 — 3D'ye geçiş: perspektif kamera, mesh, ışık

## Nasıl derlenir ve çalıştırılır

Gerekenler: Windows, Visual Studio 2022 ("C++ ile masaüstü geliştirme" iş yükü), Git.
Diğer bağımlılıklar (GLFW, doctest) ilk derlemede otomatik iner.

**Visual Studio ile:** *Dosya → Aç → Klasör* ile bu klasörü aç. Visual Studio
`CMakePresets.json`'u tanır. Üstteki listeden `example_01_window.exe`'yi seçip ▶ ile çalıştır.

**Komut satırıyla** ("Developer PowerShell for VS 2022" penceresinde, proje klasöründe):

```powershell
cmake --preset msvc            # configure (ilk seferde bağımlılıkları indirir)
cmake --build --preset debug   # derle
ctest --preset debug           # testleri çalıştır
C:\dev\build\learning-game-engine\msvc\examples\Debug\example_01_window.exe
```

Derleme çıktıları OneDrive dışında, `C:\dev\build\learning-game-engine\` altında oluşur.

## Günlük

Yapılan her adım burada, en yenisi en altta olacak şekilde yazılır.

### Adım 0 — Proje kurulumu
- Hedefler ve teknoloji seçimleri belirlendi (yukarıdaki tablo).
- Geliştirme ortamı olarak Visual Studio 2022 Community ("C++ ile masaüstü geliştirme"
  iş yükü: MSVC derleyicisi, Windows SDK, CMake) `winget` ile kuruldu
  (MSVC 14.44, CMake 3.31).
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
- **Input / ECS / fizik / sahne tasarımı kararlaştırıldı:** `Input` polling tabanlı
  (`isKeyDown`, `wasKeyPressed`). ECS sade: entity = sayı, component = sadece veri,
  system = fonksiyon; `Registry` her component türünü ayrı depoda tutar (`add<T>`, `view<A,B>`).
  Fizik: sabit adımda hız/yerçekimi + AABB çarpışma. `SceneManager` sahne geçişleri,
  `AssetManager` `shared_ptr` önbellekli kaynak yükleme. Sahneler kodda kurulur.
- **Test ve çalışma şekli kararlaştırıldı:** mantık kodu doctest ile test edilir (önce test);
  görsel kısımlar bölüm örnekleriyle doğrulanır. Her bölüm: doküman → kod + test →
  doğrulama → README → commit + push → `bolum-XX` etiketi.
- Tüm tasarım tek dosyada toplandı:
  [docs/superpowers/specs/2026-10-06-learning-game-engine-design.md](docs/superpowers/specs/2026-10-06-learning-game-engine-design.md)
- Bölüm 01'in adım adım uygulama planı yazıldı:
  [docs/superpowers/plans/2026-10-06-bolum-01-pencere-ve-oyun-dongusu.md](docs/superpowers/plans/2026-10-06-bolum-01-pencere-ve-oyun-dongusu.md)

### Adım 1.1 — Derleme altyapısı ve Log
- **CMake** kuruldu. `CMakePresets.json` derleme klasörünü OneDrive dışına
  (`C:\dev\build\learning-game-engine\msvc`) koyuyor; Visual Studio bu dosyayı otomatik tanır.
- **FetchContent:** GLFW 3.4 ve doctest v2.4.12 ilk configure sırasında GitHub'dan otomatik
  iniyor — elle kütüphane kurmak yok.
- **glad** (OpenGL 3.3 Core yükleyicisi) glad2 ile bir kez üretilip `external/glad/` altına
  konuldu. Böylece derleme Python gerektirmiyor. Eklentiler (extensions) dahil edilmedi:
  dosya küçük ve sade kalsın.
- **`Log`** (`engine/core/Log.h`): `LOG_INFO / LOG_WARN / LOG_ERROR`. Bunlar fonksiyon değil
  **makro**, çünkü çağrıldıkları yerin dosya adını (`__FILE__`) ve satırını (`__LINE__`)
  ancak bir makro yakalayabilir. Windows konsolu UTF-8'e alınıyor (Türkçe karakterler için)
  ve çıktı terminalse renkli yazılıyor.
- **İlk testler** (doctest): önce test yazıldı, derlemenin "Log.cpp bulunamadı" diye
  başarısız olduğu görüldü, sonra kod yazıldı ve 3 test geçti.

### Adım 1.2 — Sabit zaman adımı (`FixedTimestep`)
- **Problem:** Fizik "konum += hız × dt" ile ilerler. dt her karede farklıysa (30 FPS'te
  0.033 sn, 144 FPS'te 0.007 sn) sonuçlar bilgisayardan bilgisayara değişir; yavaş makinede
  top duvarın içinden geçebilir.
- **Çözüm — biriktirici:** Geçen süreyi bir kovada biriktir; kovada 1/60 sn'lik dolu bir
  "adım" oldukça fiziği tam 1/60 sn ilerlet. Bir kare 0, 1 veya birkaç sabit adım üretebilir.
- **Ölüm sarmalı koruması:** Program donarsa (breakpoint, pencere sürükleme) tek kare en
  fazla 0.25 sn sayılır; yoksa kaybedilen zamanı telafi etmeye çalışırken daha da geride kalırdık.
- **NaN tuzağı:** `NaN` ile her karşılaştırma `false` döner; biriktiriciye bir kez NaN
  girerse sonsuza dek NaN kalır. `!(x > 0)` kontrolü hem negatifleri hem NaN'ı ayıklar.
- 7 yeni test (toplam 10). Testlerde 0.25, 0.125 gibi float'ta tam temsil edilen sayılar
  kullanıldı ki yuvarlama hataları testleri rastgele bozmasın.

### Adım 1.3 — FPS sayacı (`FpsCounter`)
- Her karede `tick(dt)` çağrılır; 1 saniye dolunca o sürede çizilen kare sayısını raporlar
  ve sıfırdan başlar. Pencere başlığındaki FPS buradan gelecek.
- Bozuk (negatif/NaN) süreler zaman olarak sayılmıyor, ama kare yine de çizildiği için kare
  olarak sayılıyor. 4 yeni test (toplam 14).

### Adım 1.4 — Pencere ve oyun döngüsü
- **`Window`** GLFW penceresini ve OpenGL 3.3 Core bağlamını **RAII** ile yönetiyor: kurucu
  açar, yıkıcı kapatır. Kopyalama `= delete` ile yasak (iki nesne aynı pencereyi kapatmaya
  çalışmasın). Başlıkta GLFW yerine yalnızca `struct GLFWwindow;` **ileri bildirimi** var.
- **glad** pencere açıldıktan sonra OpenGL fonksiyonlarının adreslerini sürücüden yüklüyor.
- **`RenderCommand`** (`setClearColor`, `clear`): oyun kodu OpenGL'e doğrudan dokunmuyor.
- **`Application::run()`** döngüsü: (1) birikmiş süre kadar `onFixedUpdate`, (2) `onUpdate(dt)`
  ve `onRender()`, (3) `swapAndPoll()`. Oyunlar `Application`'dan türeyip bu kancaları yazıyor.
- **Örnek (`examples/01_window`)** çalıştırıldı:
  ```
  [INFO] Window.cpp:60 OpenGL 3.3.0 Core Profile Context ... | AMD Radeon(TM) Graphics
  [INFO] main.cpp:26 Son 1 saniyede 60 sabit adım çalıştı | FPS: 163
  [INFO] main.cpp:26 Son 1 saniyede 60 sabit adım çalıştı | FPS: 80
  [INFO] main.cpp:26 Son 1 saniyede 60 sabit adım çalıştı | FPS: 181
  ```
  FPS 80 ile 181 arasında oynarken sabit adım sayısı 52 saniye boyunca hep 60–62 kaldı.
  Pencere küçültme/geri açma ve yeniden boyutlandırmada çökme yok, renk tüm alanı kaplıyor;
  X ile kapatınca program 0 koduyla çıkıyor.

### Bölüm 01 tamamlandı ✔
- Ders dokümanı: [docs/bolum-01.md](docs/bolum-01.md). Oyun döngüsü, değişken dt'nin
  fizikte neden sorun olduğu (sayısal örnekle), ölüm sarmalı, NaN tuzağı, C++ köşesi
  (RAII, `= delete`, ileri bildirim, `virtual`/`override`, makrolar) ve 5 alıştırma içeriyor.
- README'ye "Nasıl derlenir ve çalıştırılır" bölümü eklendi.
- Git etiketi: `bolum-01` (`git checkout bolum-01` ile bu ana dönebilirsin).
