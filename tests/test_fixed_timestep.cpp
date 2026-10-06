#include <doctest/doctest.h>

#include <limits>

#include "core/FixedTimestep.h"

using engine::FixedTimestep;

// Not: Testlerde 0.25, 0.125, 0.625 (= 5/8) gibi paydası 2'nin kuvveti olan kesirler
// kullanıyoruz. Bunlar float'ta tam olarak temsil edilir; böylece yuvarlama hataları
// testleri rastgele bozmaz.

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

TEST_CASE("clampFrame: onUpdate'e verilecek kare süresi de aynı kurallarla sınırlanır") {
    FixedTimestep ts(0.25f, 0.5f);
    CHECK(ts.clampFrame(0.125f) == 0.125f); // normal süre olduğu gibi kalır
    CHECK(ts.clampFrame(10.0f) == 0.5f);    // uzun donma en fazla maxFrame sayılır
    CHECK(ts.clampFrame(-1.0f) == 0.0f);
    CHECK(ts.clampFrame(std::numeric_limits<float>::quiet_NaN()) == 0.0f);
}

TEST_CASE("Varsayılan ayarlarla 1/60 sn'lik kare tam bir adım üretir") {
    FixedTimestep ts;
    CHECK(ts.advance(1.0f / 60.0f) == 1);
}
