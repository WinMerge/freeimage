// Copyright (C) 2026 The FreeImage Project
// Distributed under the FreeImage Public License or GNU GPL v2; see license-fi.txt and license-gplv2.txt.
// Internal multipage/cache path handling. No locale conversion of Windows paths.
#ifndef FREEIMAGE_MULTIBITMAP_PATH_H
#define FREEIMAGE_MULTIBITMAP_PATH_H
#include <string>
#include <cstdio>

class MultiBitmapPath {
    std::string narrow;
#ifdef _WIN32
    std::wstring wide;
#endif
public:
    MultiBitmapPath() {}
    MultiBitmapPath(const char *path) : narrow(path ? path : "") {}
    MultiBitmapPath(const std::string& path) : narrow(path) {}
#ifdef _WIN32
    MultiBitmapPath(const wchar_t *path) : wide(path ? path : L"") {}
#endif
    bool empty() const {
#ifdef _WIN32
        if (!wide.empty()) return false;
#endif
        return narrow.empty();
    }
    FILE *open(bool write) const {
#ifdef _WIN32
        if (!wide.empty()) return _wfopen(wide.c_str(), write ? L"w+b" : L"rb");
#endif
        return fopen(narrow.c_str(), write ? "w+b" : "rb");
    }
    int remove() const {
#ifdef _WIN32
        if (!wide.empty()) return _wremove(wide.c_str());
#endif
        return std::remove(narrow.c_str());
    }
    int renameTo(const MultiBitmapPath& destination) const {
#ifdef _WIN32
        if (!wide.empty()) return _wrename(wide.c_str(), destination.wide.c_str());
#endif
        return std::rename(narrow.c_str(), destination.narrow.c_str());
    }
    MultiBitmapPath withExtension(const char *extension) const {
        MultiBitmapPath result(*this);
#ifdef _WIN32
        if (!wide.empty()) {
            const size_t dot = wide.find_last_of(L'.');
            result.wide = (dot == std::wstring::npos ? wide + L"." : wide.substr(0, dot + 1));
            while (*extension) result.wide += static_cast<wchar_t>(*extension++);
            return result;
        }
#endif
        const size_t dot = narrow.find_last_of('.');
        result.narrow = (dot == std::string::npos ? narrow + "." : narrow.substr(0, dot + 1)) + extension;
        return result;
    }
};
#endif
