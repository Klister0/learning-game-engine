# Bölüm 01 — Pencere ve Oyun Döngüsü

![Bölüm 01 penceresi](images/bolum-01-pencere.png)

## 1. Bu bölümde ne yaptık

Bir pencere açtık, içine OpenGL 3.3 bağlamı kurduk ve rengi yumuşakça değişen bir arka plan
çizdik. Daha önemlisi, motorun kalbini kurduk: **oyun döngüsünü**. Döngü fiziği her
bilgisayarda aynı davranacak şekilde **sabit zaman adımıyla** ilerletiyor. Bunu konsolda
görebilirsin: FPS 80 ile 180 arasında oynarken "sabit adım" sayısı hep 60 civarında kalıyor.

Programı çalıştırmak için: README'deki "Nasıl derlenir ve çalıştırılır" bölümüne bak, sonra
`example_01_window.exe`'yi aç.

## 2. Derleme altyapısı

**CMake** bir derleme sistemi *üreticisidir*. Kodu kendisi derlemez; `CMakeLists.txt`
dosyalarını okuyup Visual Studio'nun anlayacağı proje dosyalarını üretir. Avantajı: aynı
`CMakeLists.txt` Linux'ta Makefile, Windows'ta Visual Studio projesi üretebilir.

Projede üç tür hedef (target) var:

| Hedef | Türü | Ne işe yarar |
|---|---|---|
| `engine` | statik kütüphane | Motorun kendisi. Tek başına çalışmaz, programlara "yapıştırılır". |
| `example_01_window` | program | Motoru kullanan Bölüm 01 örneği. |
| `engine_tests` | program | Birim testleri. |

**CMakePresets.json:** Configure ayarlarını bir dosyaya yazdık, böylece uzun komutları
ezberlemek gerekmiyor. Derleme klasörü `C:\dev\build\learning-game-engine\msvc`. Proje
OneDrive'da durduğu için derleme çıktılarını (yüzlerce MB) oraya koymuyoruz, yoksa OneDrive
her derlemede onları senkronlamaya çalışırdı.

**FetchContent:** GLFW ve doctest'i elle indirip kurmadık. CMake ilk configure sırasında
onları GitHub'dan belirli bir sürümle (`GIT_TAG 3.4`) kendisi indirdi. Herkes aynı sürümü
kullanır, "bende çalışıyor" sorunu olmaz.

**glad neden repoda?** OpenGL fonksiyonları (ör. `glClear`) Windows'un kendisinde değil,
ekran kartı sürücüsünün içinde yaşar. Programın, çalışırken bu fonksiyonların adreslerini
sürücüden sorması gerekir; glad bu işi yapan koddur. glad'ı bir Python aracıyla bir kez
ürettik ve `external/glad/` altına koyduk. Böylece derlemek için Python gerekmiyor.

## 3. Oyun döngüsü

Her oyun özünde şu döngüdür:

```
           ┌────────────────────────────────────────────┐
           │  dt = son kareden bu yana geçen süre       │
           │                                            │
           │  1) onFixedUpdate(1/60)  × (0, 1, 2… kez)  │  ← fizik, oyun kuralları
           │  2) onUpdate(dt)                           │  ← animasyon, input tepkisi
           │     onRender()                             │  ← çizim
           │  3) swapAndPoll()                          │  ← kareyi göster, olayları işle
           └──────────────────┬─────────────────────────┘
                              └── pencere açık olduğu sürece tekrar
```

### Değişken dt neden fizikte sorun?

Bir topu yerçekimiyle düşürelim (g = 10 m/s²). Her karede `hız += g·dt; konum += hız·dt`
yapıyoruz. 1 saniye sonra top ne kadar düşmüş olmalı? Gerçekte 5.00 m.

| FPS | dt | 1 sn sonra düşülen mesafe |
|---|---|---|
| 30 | 0.033 sn | 5.17 m |
| 144 | 0.007 sn | 5.03 m |

Aynı oyun, iki bilgisayarda farklı sonuç! Zıplama yüksekliği değişir. Yavaş bilgisayarda
büyük dt yüzünden top tek karede duvarın içinden geçebilir.

### Çözüm: biriktirici (accumulator)

Gerçek zamanı bir kovaya dök. Kovada 1/60 sn'lik dolu bir "adım" oldukça fiziği **tam
1/60 sn** ilerlet, sonra o adımı kovadan çıkar. Artan süre bir sonraki kareye kalır.

- 180 FPS'te (dt ≈ 0.0056 sn) çoğu karede kova dolmaz → 0 adım. Yaklaşık her 3 karede 1 adım.
- 30 FPS'te (dt ≈ 0.033 sn) her karede 2 adım.
- Her iki durumda da saniyede tam 60 adım çalışır ve fizik hep aynı sonucu verir.

Kod: `engine/core/FixedTimestep.cpp` → `advance()`.

### Ölüm sarmalı (spiral of death)

Program bir an donarsa ne olur? Mesela debugger'da breakpoint'te 10 saniye bekledin, ya da
Windows'ta pencereyi sürüklerken döngü durdu. Kovada 10 saniye birikir, yani 600 fizik adımı
çalıştırmak gerekir. Bu adımlar o kadar uzun sürer ki o sırada daha da çok süre birikir.
Oyun bir daha asla yetişemez ve donar.

Çözüm basit: tek bir karede en fazla **0.25 saniye** sayıyoruz. Kaybedilen zamanı telafi
etmeye çalışmıyoruz; oyun bir anlığına "yavaş çekim" olup devam ediyor.

### NaN tuzağı

`NaN` ("sayı değil") ile yapılan **her karşılaştırma `false`** döner: `NaN > 0`, `NaN < 0`,
hatta `NaN == NaN` bile `false`. Kovaya bir kez NaN girerse kova sonsuza dek NaN kalır,
`kova >= adım` hiç doğru olmaz ve oyun sessizce durur. Bu yüzden kontrolü
`if (dt < 0)` diye değil `if (!(dt > 0))` diye yazdık. İkincisi NaN'ı da yakalar.

## 4. C++ köşesi

**RAII (Resource Acquisition Is Initialization).** "Kaynağı kurucuda al, yıkıcıda bırak."
`Window` nesnesi oluştuğunda pencere açılır; nesne kapsamdan çıktığında (ör. `main` bitince)
yıkıcı `~Window()` otomatik çalışır ve pencereyi kapatır. "Kapatmayı unutmak" imkânsızdır.
Bu deseni motorun her yerinde (shader, texture, buffer) kullanacağız.

**`= delete` ile kopyalamayı yasaklamak.**
```cpp
Window(const Window&) = delete;
Window& operator=(const Window&) = delete;
```
`Window a(...); Window b = a;` yazılabilseydi `a` ve `b` aynı pencereyi gösterirdi. İkisinin
yıkıcısı da onu kapatmaya çalışır ve ikincisi zaten silinmiş bir pencereyi silerdi (çökme).
`= delete` derleyiciye "bu fonksiyonu üretme" der; kopyalama denemesi **derleme hatası** olur.

**İleri bildirim (forward declaration).** `Window.h` içinde `#include <GLFW/glfw3.h>` yerine
sadece `struct GLFWwindow;` yazdık. Derleyiciye "böyle bir tip var" demek, işaretçisini
(`GLFWwindow*`) tutmak için yeterli. Böylece `Window.h`'yi dahil eden herkes GLFW'nin dev
başlığını da dahil etmek zorunda kalmıyor ve derleme hızlanıyor.

**`virtual` ve `override`.** `Application` içindeki `virtual void onRender()` "türetilmiş
sınıf bunu değiştirebilir" demek. `WindowDemo` içindeki `void onRender() override` ise
"taban sınıftaki fonksiyonu değiştiriyorum" demek. İsmi yanlış yazarsan (`onRendr`)
`override` sayesinde derleyici hata verir.

**Neden `virtual ~Application()`?** Bir gün `Application* app = new Breakout(); delete app;`
yazarsak, yıkıcı virtual değilse yalnızca `~Application()` çalışır ve `Breakout`'un temizliği
atlanır. Taban sınıf olarak tasarlanan her sınıfın yıkıcısı virtual olmalıdır.

**Üye kurulum sırası.** Üyeler, kurucudaki yazılış sırasına göre değil, **sınıfta bildirildikleri
sırayla** kurulur. Bu yüzden `Application`'da `m_baseTitle`, `m_window`'dan önce bildirildi.

**Makrolar ve `__FILE__` / `__LINE__`.** `LOG_INFO("...")` bir fonksiyon değil makro.
Ön işlemci (preprocessor) onu çağrıldığı yerde metin olarak açar; `__FILE__` ve `__LINE__`
o noktanın dosya adı ve satır numarasıyla değiştirilir. Bir fonksiyon kendisini kimin
çağırdığını bu kadar kolay bilemezdi.

**`static` yerel değişken.** `Log.cpp`'deki `static const ConsoleSetup setup;` satırı fonksiyon
ilk çağrıldığında **bir kez** çalışır, sonraki çağrılarda aynı nesne kullanılır. Konsolu
UTF-8'e almayı bu şekilde yalnızca bir kez yapıyoruz.

## 5. Kodda gezinti

Okuma sırası önerisi:

1. `examples/01_window/main.cpp` — motoru kullanan taraf. Neye ihtiyacımız olduğunu görürsün.
2. `engine/core/Application.h/.cpp` — `run()` döngüsü.
3. `engine/core/FixedTimestep.h/.cpp` — biriktirici.
4. `engine/core/Window.h/.cpp` — GLFW, OpenGL bağlamı, glad.
5. `engine/renderer/RenderCommand.h/.cpp` — OpenGL'e açılan en ince kapı.
6. `engine/core/FpsCounter.h/.cpp`, `engine/core/Log.h/.cpp` — yardımcılar.
7. `tests/` — her sınıfın nasıl davranması gerektiğinin "yaşayan belgesi".

## 6. Testler

Ekran gerektirmeyen her şeyi (log biçimi, sabit adım, FPS) **önce test** yazarak geliştirdik:

1. **Kırmızı:** Test yazıldı ve derlendi, başarısız oldu (`FixedTimestep.h bulunamadı`).
2. **Yeşil:** Testi geçirecek en sade kod yazıldı ve testler geçti.

Önce kırmızıyı görmek önemli: hiç başarısız olmadığını gördüğün bir test, gerçekten bir şeyi
test ettiğini kanıtlamaz.

Testlerde 0.25, 0.125 gibi sayılar seçtik. Bunlar 2'nin kuvvetleri olduğu için `float`'ta
**tam** temsil edilir. 0.1 gibi bir sayı ise float'ta 0.100000001490116… olarak saklanır ve
toplamalarda küçük hatalar birikir.

Konsolda ilk satırda `FPS: 0` görmen de normal: örnek programın ilk "1 saniye" raporu,
FPS sayacının ilk ölçümünü tamamlamasından bir kare önce yazılıyor.

## 7. Alıştırmalar

a. `examples/01_window/main.cpp` içinde `config.window.vsync = false;` ekle. FPS ne oldu?
   Sabit adım sayısı değişti mi? Neden?

b. `config.fixedStep = 1.0f / 30.0f;` yap. Konsoldaki adım sayısı neden 30'a düştü?
   Bir yarış oyununda fiziği 30 Hz'de çalıştırmanın dezavantajı ne olurdu?

c. Programı çalıştır ve pencereyi başlığından tutup 2–3 saniye sürükle, sonra bırak. Konsolda
   o saniye için kaç adım gördün? Bunu 0.25 saniyelik sınırla açıkla.

d. `tests/test_fixed_timestep.cpp` dosyasına kendi testini yaz: adım 0.25 sn, art arda
   3 kare × 0.1 sn. Kaç adım bekliyorsun? Testi çalıştır.
   (İpucu: `ctest --preset debug` veya doğrudan `engine_tests.exe`.)

e. (Zor) `Application::run()` içinde `dt`'yi `onUpdate`'e vermeden önce 2 ile çarparak oyunu
   "2x hızlı" çalıştır. Hangi satırları değiştirmen gerekti? Sabit adım hâlâ doğru mu?

## 8. Sırada ne var

**Bölüm 02 — Shader'lar ve ilk üçgen.** GPU'ya köşe (vertex) verisi göndereceğiz, ilk vertex
ve fragment shader'larımızı yazacağız ve RAII'nin bir adım ötesine, *taşıma (move)
semantiğine* geçeceğiz.
