// This file was developed by Thomas Müller <thomas94@gmx.net>.
// It is published under the BSD 3-Clause License within the LICENSE file.

#pragma once

#include <tinyformat.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#   define NOMINMAX
#   include <Windows.h>
#   undef NOMINMAX
#   pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#   pragma warning(disable : 4244) // warning C4244: conversion from X to Y, possible loss of data
#endif

// Needs to be included _after_ Windows.h to ensure NOMINMAX has an effect
#include <filesystem/path.h>

// A macro is used such that external tools won't end up indenting entire files,
// resulting in wasted horizontal space.
#define TEV_NAMESPACE_BEGIN namespace tev {
#define TEV_NAMESPACE_END }

#ifdef __GNUC__
#   define LIKELY(condition) __builtin_expect(static_cast<bool>(condition), 1)
#   define UNLIKELY(condition) __builtin_expect(static_cast<bool>(condition), 0)
#else
#   define LIKELY(condition) condition
#   define UNLIKELY(condition) condition
#endif

#define TEV_ASSERT(cond, description, ...) \
    if (UNLIKELY(!(cond))) \
        throw std::runtime_error{tfm::format(description, ##__VA_ARGS__)};

struct NVGcontext;

TEV_NAMESPACE_BEGIN

inline int codePointLength(char first) {
    if ((first & 0xf8) == 0xf0) {
        return 4;
    } else if ((first & 0xf0) == 0xe0) {
        return 3;
    } else if ((first & 0xe0) == 0xc0) {
        return 2;
    } else {
        return 1;
    }
}

#ifdef _WIN32
inline std::wstring nativeString(const filesystem::path& path) {
    return path.wstr();
}
#else
inline std::string nativeString(const filesystem::path& path) {
    return path.str();
}
#endif

#ifdef _WIN32
inline FILE* cfopen(const filesystem::path& path, std::string mode) {
    int size = MultiByteToWideChar(CP_UTF8, 0, &mode[0], (int)mode.size(), NULL, 0);
    std::wstring wmode(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &mode[0], (int)mode.size(), &wmode[0], size);
    return _wfopen(path.wstr().c_str(), wmode.c_str());
}
#else
inline FILE* cfopen(const filesystem::path& path, std::string mode) {
    return fopen(path.str().c_str(), mode.c_str());
}
#endif

class ScopeGuard {
public:
    ScopeGuard(const std::function<void(void)>& callback) : mCallback{callback} {}
    ~ScopeGuard() { mCallback(); }
private:
    std::function<void(void)> mCallback;
};

template <typename T>
T clamp(T value, T min, T max) {
    TEV_ASSERT(max >= min, "Minimum (%f) may not be larger than maximum (%f).", min, max);
    return std::max(std::min(value, max), min);
}

template <typename T>
T round(T value, T decimals) {
    auto precision = std::pow(static_cast<T>(10), decimals);
    return std::round(value * precision) / precision;
}

template <typename T>
std::string join(const T& components, const std::string& delim) {
    std::ostringstream s;
    for (const auto& component : components) {
        if (&components[0] != &component) {
            s << delim;
        }
        s << component;
    }

    return s.str();
}

std::vector<std::string> split(std::string text, const std::string& delim);

std::string toLower(std::string str);
std::string toUpper(std::string str);

bool matches(std::string text, std::string filter, bool isRegex);

void drawTextWithShadow(NVGcontext* ctx, float x, float y, std::string text, float shadowAlpha = 1.0f);

inline float toSRGB(float linear, float gamma = 2.4f) {
    static const float a = 0.055f;
    if (linear <= 0.0031308f) {
        return 12.92f * linear;
    } else {
        return (1 + a) * pow(linear, 1 / gamma) - a;
    }
}

inline float toLinear(float sRGB, float gamma = 2.4f) {
    static const float a = 0.055f;
    if (sRGB <= 0.04045f) {
        return sRGB / 12.92f;
    } else {
        return pow((sRGB + a) / (1 + a), gamma);
    }
}

int lastError();
int lastSocketError();
std::string errorString(int errorId);

filesystem::path homeDirectory();

void toggleConsole();

enum ETonemap : int {
    SRGB = 0,
    Gamma,
    FalseColor,
    PositiveNegative,

    // This enum value should never be used directly.
    // It facilitates looping over all members of this enum.
    NumTonemaps,
};

ETonemap toTonemap(std::string name);

enum EMetric : int {
    Error = 0,
    AbsoluteError,
    SquaredError,
    RelativeAbsoluteError,
    RelativeSquaredError,

    // This enum value should never be used directly.
    // It facilitates looping over all members of this enum.
    NumMetrics,
};

EMetric toMetric(std::string name);

enum EDirection {
    Forward,
    Backward,
};

TEV_NAMESPACE_END
