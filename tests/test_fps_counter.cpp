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
