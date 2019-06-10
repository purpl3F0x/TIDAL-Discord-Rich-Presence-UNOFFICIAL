//
// Created by Stavros Avramidis on 2019-06-08.
//

#pragma once

#include <cstdlib>
#include <iostream>
#include <wchar.h>
#include <vector>
#include <Carbon/Carbon.h>
#include <string>
#include <cstring>
#include <chrono>
#include <regex>
#include <codecvt>
#include <sstream>

std::string safeWstringToString(const std::wstring& wstr) {
  std::ostringstream os;
  for (auto& i: wstr) {
    if (i <= 255)
      os << (char) i;
    else if (i==0x201C || i==0x201D)    //  replace “ and ”
      os << "\"";
  }
  return os.str();
}

std::string rawWstringToString(const std::wstring& wstr) {
  return std::string(wstr.begin(), wstr.end());
}

enum status { error, closed, opened, playing };

std::wstring ctow(const char* src) {
  std::vector<wchar_t> dest(strlen(src) + 1);
  int i = mbstowcs(&dest[0], src, strlen(src));
  return std::wstring(&dest[0]);
}

status tidalInfo(std::wstring& song, std::wstring& artist) {
  char buffer[512] = "";
  int layer = 0;
  status result = closed;
  CFArrayRef windowList = CGWindowListCopyWindowInfo(kCGWindowListOptionAll, kCGNullWindowID);
  CFIndex numWindows = CFArrayGetCount(windowList);
  const std::wregex rgx(L"(.+) - (?!\\{)(.+)");

  song = L"";
  artist = L"";

  for (int i = 0; i < (int) numWindows; i++) {
    auto info = (CFDictionaryRef) CFArrayGetValueAtIndex(windowList, i);
    auto appName = (CFStringRef) CFDictionaryGetValue(info, kCGWindowOwnerName);

    CFNumberGetValue((CFNumberRef) CFDictionaryGetValue(info, kCGWindowLayer), kCFNumberIntType, &layer);

    if (appName!=nullptr) {
      CFStringGetCString(appName, buffer, 512, kCFStringEncodingUTF8);
      if (layer==0 && std::strcmp(buffer, "TIDAL")==0) {
        char title[512] = "";
        auto windowTitle = (CFStringRef) CFDictionaryGetValue(info, kCGWindowName);
        if (windowTitle!=nullptr) {
          CFStringGetCString(windowTitle, title, sizeof title, kCFStringEncodingUTF8);

          result = opened;
          std::wsmatch matches;
          auto wtitle = std::wstring(title, title + strlen(title));

          if (std::regex_search(wtitle, matches, rgx)) {
            song = matches[1].str();
            artist = matches[2].str();
            result = playing;
            goto _exit;
          }
        }
      }
    }
  }
  _exit:
  return result;
}

inline char* getLocale() noexcept {
  char* tmp = new char[2];
  auto localeID = (CFStringRef) CFLocaleGetValue(CFLocaleCopyCurrent(), kCFLocaleCountryCode);
  CFStringGetCString(localeID, tmp, 16, kCFStringEncodingUTF8);
  return tmp;
}
