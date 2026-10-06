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
