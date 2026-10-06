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
