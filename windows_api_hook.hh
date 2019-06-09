
#pragma  once

#include <string>
#include <vector>

#include <windows.h>
#include <tlhelp32.h>
#include <Winuser.h>
#include <regex>

#include <codecvt>
#include <sstream>

#include <chrono>
#include <thread>

#pragma comment (lib, "User32.lib")


std::string safeWstringToString(const std::wstring& wstr){
    std::ostringstream os;
    for (auto &i: wstr){
        if (i <=255)
            os << (char)i;
        else if (i == 0x201C || i == 0x201D)    //  replace “ and ”
            os << "\"";
    }
    return os.str();
}

std::string rawWstringToString(const std::wstring& wstr){
    return  std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
}

enum status { error, closed, opened, playing };

struct EnumWindowsProcParam {
  std::vector<DWORD> &pids;
  std::wstring &song;
  std::wstring &artist;
  status tidalStatus = closed;

  EnumWindowsProcParam(std::vector<DWORD> &pids, std::wstring &song, std::wstring &artist) : pids(pids), song(song), artist(artist){}
};

inline bool isTIDAL(const PROCESSENTRY32W &entry) {        // inline that? y not?
    return wcscmp(entry.szExeFile, L"TIDAL.exe")==0;
}

BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) {

    static const std::wregex rgx(L"(.+) - (?!\\{)(.+)");
    auto &paramRe = *reinterpret_cast<EnumWindowsProcParam *>(lParam);

    DWORD winId;
    GetWindowThreadProcessId(hwnd, &winId);

    for (DWORD pid : (paramRe.pids)) {
        if (winId==pid) {
            paramRe.tidalStatus = opened;

            std::wstring title(GetWindowTextLength(hwnd) + 1, L'\0');
            GetWindowTextW(hwnd, &title[0], title.size()); //note: >=C++11

            std::regex_match(title, rgx);
            std::wsmatch matches;

            if (std::regex_search(title, matches, rgx)) {
//                paramRe.song = safeWstringToString(matches[1].str());
//                paramRe.artist = safeWstringToString(matches[2].str());
                paramRe.song = matches[1].str();
                paramRe.artist = matches[2].str();
                paramRe.tidalStatus = playing;
                return FALSE;
            }
        }
    }
    return TRUE;
}

status tidalInfo(std::wstring &song, std::wstring &artist) {

    static std::vector<DWORD> pids;

    static HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof entry;

    if (!Process32FirstW(snap, &entry))
        return error;

    if (pids.empty())
        do {
            if (isTIDAL(entry)) {
                pids.emplace_back(entry.th32ProcessID);
            }
        } while (Process32NextW(snap, &entry));

    song = L"";
    artist = L"";

   EnumWindowsProcParam param(pids, song, artist);

    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&param));

    if (param.tidalStatus==closed) {
        pids.clear();
        snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    }
    return param.tidalStatus;
}

inline char *getLocale() noexcept {
    static char buffer[3];
    GEOID myGEO = GetUserGeoID(GEOCLASS_NATION);
    int result = GetGeoInfoA(myGEO, GEO_ISO2, buffer, 3, 0);
    return buffer;
}