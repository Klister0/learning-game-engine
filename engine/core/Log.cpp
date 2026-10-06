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
