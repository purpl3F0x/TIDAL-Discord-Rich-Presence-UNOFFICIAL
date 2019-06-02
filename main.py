"""
:filename:  main.py
:author:    Stavros Avramidis
:version:
"""

import sys
from os import path
from time import sleep, time

import psutil
import pylast
from PyQt5.QtCore import QThread, Qt
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QAction, QApplication, QInputDialog, QMenu, QSystemTrayIcon
from pypresence import Presence

# configuration file containing API keys (provide your own)
from config import *

SESSION_KEY_FILE = path.join(path.expanduser("~"), ".session_key")

last_output = None
user_name = ''
user = None
isRunning = True
network = pylast.LastFMNetwork(API_KEY, API_SECRET)


def change_state():
    global isRunning
    isRunning = not isRunning
    is_running_action.setChecked(isRunning)
    is_running_action.setText("Running" if isRunning else "Disabled")
    print('Changed')


app = QApplication([])
icon = QSystemTrayIcon(QIcon('icon.ico'), parent=app)

title = QAction(QIcon('icon.ico'), "TIDAL RPC")

is_running_action = QAction("Running")
is_running_action.setCheckable(True)
is_running_action.setChecked(True)
is_running_action.triggered.connect(change_state)

quit_action = QAction("Exit")
quit_action.triggered.connect(lambda: sys.exit())

tray_menu = QMenu(parent=None)

tray_menu.addAction(title)
tray_menu.addAction(is_running_action)
tray_menu.addAction(quit_action)

icon.setContextMenu(tray_menu)
icon.show()


class AThread(QThread):
    def run(self):
        global isRunning, network, user
        playing_track = None

        client_id = '584458858731405315'  # Discord BOT id, put your real one here
        RPC = Presence(client_id)  # Initialize the client class
        RPC.connect()  # Start the handshake loop

        while True:
            try:
                if not isRunning:
                    RPC.clear()
                    continue
                new_track = user.get_now_playing()
                # A new, different track
                if new_track and new_track != playing_track:
                    playing_track = new_track
                    print(playing_track.get_name())
                    print(playing_track.get_artist())
                    dur = new_track.get_duration()

                    print(
                        RPC.update(
                            state=str(playing_track.get_name()),
                            details=str(playing_track.get_artist()),
                            large_image='fb_1200x627',
                            # small_image='tidalogo_0',
                            end=int(time()) + dur / 1000 if dur else None,
                            start=int(time()),
                        )
                    )  # Set the presence
                    title.setText(str(playing_track.get_name()))
                # check if song is played
                elif not new_track and playing_track and "TIDAL.exe" in (p.name() for p in psutil.process_iter()):
                    print('song paused ?')

                # clear status
                elif not new_track:
                    RPC.clear()

            except Exception as e:
                print("Error: %s" % repr(e))

            sleep(2)


########
if not path.exists(SESSION_KEY_FILE):
    skg = pylast.SessionKeyGenerator(network)
    url = skg.get_web_auth_url()

    print(f"Please authorize the scrobbler to scrobble to your account: {url}\n")
    import webbrowser

    webbrowser.open(url)

    while True:
        try:
            session_key = skg.get_web_auth_session_key(url)
            a = QInputDialog(None, Qt.WindowStaysOnTopHint)
            a.setWindowFlags(Qt.WindowStaysOnTopHint)
            user_name, ok = a.getText(None, 'TIDAL - Discord RPC', 'Enter your last.fm username')
            if not ok:
                sys.exit(0)

            fp = open(SESSION_KEY_FILE, "w")
            fp.writelines([user_name, '\n', session_key])
            fp.close()
            break
        except pylast.WSError:
            sleep(1)
else:
    try:
        with open(SESSION_KEY_FILE, 'r') as ss:
            user_name = ss.readline()[:-1]
            session_key = ss.readline()
            print('Found', user_name, session_key)
    except ...:
        print('Error reading session stage.')
        exit()

network.session_key = session_key
user = network.get_user(user_name)

print('Starting main app')
thread = AThread()
thread.start()

sys.exit(app.exec_())
