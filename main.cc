/**
 * @file    main.cc
 * @authors Stavros Avramidis
 */


/* C++ libs */
#include <chrono>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <stack>
/* C libs */
#include <cstdio>
/* Qt */
#include <QApplication>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QCoreApplication>
/* local libs*/
#include "discord_rpc.h"
#include "discord_register.h"
#include "httplib.hh"
#include "json.hh"

#ifdef WIN32
#include <Windows.h>
#pragma comment(lib, "discord-rpc.lib")

#elif defined(__APPLE__) or defined(__MACH__)
#elif defined(__linux__)
#else
#   error "unkown target os"
#endif

#define API_KEY "0bd4a33cd0178d9b9d969a02c80bd8b8"

static std::string username("purple_runner");
static const char *APPLICATION_ID = "584458858731405315";
volatile bool isPresenceActive = true;

struct Song {
  int64_t runtime;
  std::string title;
  std::string artist;
  std::string album;
  std::string url;

  friend std::ostream &operator<<(std::ostream &out, const Song &song) {
      out << song.title << " of " << song.album << " from " << song.artist << "(" << song.runtime << ")";
      return out;
  }
};

static int prompt(char *line, size_t size) {
    int res;
    char *nl;
    printf("\n> ");
    fflush(stdout);
    res = fgets(line, (int) size, stdin) ? 1 : 0;
    line[size - 1] = 0;
    nl = strchr(line, '\n');
    if (nl) {
        *nl = 0;
    }
    return res;
}

static void updateDiscordPresence(const Song &song) {
    if (isPresenceActive) {
        DiscordRichPresence discordPresence;
        memset(&discordPresence, 0, sizeof(discordPresence));
        discordPresence.state = song.title.c_str();
        discordPresence.details = song.artist.c_str();
        discordPresence.startTimestamp = std::time(nullptr);
        if (song.runtime) discordPresence.endTimestamp = discordPresence.startTimestamp + song.runtime;
        std::cout << song.runtime << "\n";
        discordPresence.largeImageKey = "fb_1200x627";
        discordPresence.instance = 0;
        Discord_UpdatePresence(&discordPresence);
    } else {
        Discord_ClearPresence();
    }
}

static void handleDiscordReady(const DiscordUser *connectedUser) {
    printf("\nDiscord: connected to user %s#%s - %s\n",
           connectedUser->username,
           connectedUser->discriminator,
           connectedUser->userId);
}

static void handleDiscordDisconnected(int errcode, const char *message) {
    printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

static void handleDiscordError(int errcode, const char *message) {
    printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

static void discordInit() {
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = handleDiscordReady;
    handlers.disconnected = handleDiscordDisconnected;
    handlers.errored = handleDiscordError;
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

inline void rpcLoop() {

    using json = nlohmann::json;
    using string = std::string;
    httplib::Client cli("ws.audioscrobbler.com");

    char getLastSongBuf[128];
    char getSongInfoBuf[256];
    json j;

    Song curSong;

    sprintf(getLastSongBuf,
            "/2.0/?method=user.getRecentTracks&user=%s&api_key=%s&format=json&limit=1",
            username.c_str(),
            API_KEY);

    for (;;) {
        try {
            auto res = cli.Get(getLastSongBuf);

            if (res && res->status==200) {
                j = json::parse(res->body);
                auto first_track = j["recenttracks"]["track"][0];

                try {
                    first_track["@attr"]["nowplaying"].get<std::string>();
                } catch (std::exception &e) {
                    Discord_ClearPresence();
                    goto _continue;
                }

                if (first_track["@attr"]["nowplaying"].get<std::string>()=="true") {
                    if (curSong.title!=first_track["name"].get<std::string>()) {

                        std::clog << first_track["name"].get<std::string>();
                        std::clog << " - " << first_track["artist"]["#text"].get<std::string>() << "\n";
                        curSong.title = first_track["name"].get<std::string>();
                        curSong.artist = first_track["artist"]["#text"].get<std::string>();
                        curSong.album = first_track["album"]["#text"].get<std::string>();
                        curSong.runtime = 0;

                        auto songURL = first_track["url"].get<std::string>();

                        sprintf(getSongInfoBuf,
                                "/2.0/?method=track.getInfo&api_key=%s&format=json&artist=%s&track=%s",
                                API_KEY,
                                curSong.artist.c_str(),
                                curSong.title.c_str()
                        );
                        res = cli.Get(getSongInfoBuf);
                        j = json::parse(res->body);
                        curSong.runtime = (int64_t) stoi(j["track"]["duration"].get<std::string>())/1000;

                        updateDiscordPresence(curSong);

                    }
                }
            } else if (res && res->status==404)
                std::cerr << "User not found";
        }
        catch (std::exception &e) {
            std::cerr << e.what() << '\n';
        }
        _continue:
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    }
}

int main(int argc, char **argv) {
    using json = nlohmann::json;
    using string = std::string;

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("icon.ico"));

    QSystemTrayIcon tray(QIcon("icon.ico"), &app);

    QAction titleAction(QIcon("icon.ico"), "TIDAL - Discord RPC ", nullptr);

    QAction changePresenceStatusAction("Running", nullptr);
    changePresenceStatusAction.setCheckable(true);
    changePresenceStatusAction.setChecked(true);
    QObject::connect(&changePresenceStatusAction,
                     &QAction::triggered,
                     [&changePresenceStatusAction]() {
                       isPresenceActive = !isPresenceActive;
                       changePresenceStatusAction.setText(isPresenceActive ? "Running"
                                                                           : "Disabled (click to re-enable)");
                     }
    );

    QAction quitAction("Exit", nullptr);
    QObject::connect(&quitAction, &QAction::triggered, [&app]() {
      Discord_ClearPresence();
      app.quit();
    });

    QMenu trayMenu("TIDAL - RPC", nullptr);

    trayMenu.addAction(&titleAction);
    trayMenu.addAction(&changePresenceStatusAction);
    trayMenu.addAction(&quitAction);

    tray.setContextMenu(&trayMenu);

    tray.show();

    std::thread t1(rpcLoop);
    t1.detach();

    discordInit();

    return app.exec();

}