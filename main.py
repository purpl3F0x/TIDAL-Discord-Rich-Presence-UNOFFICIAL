"""
:filename:  main.py
:author:    Stavros Avramidis
:version:
"""

from os import path
from time import sleep, time

import psutil
import pylast
from pypresence import Presence

# configuration file containing API keys (provide your own)
from config import *

SESSION_KEY_FILE = path.join(path.expanduser("~"), ".session_key")

last_output = None
user_name = ''


def ask_for_username():
    """

    Asks user for username
    :return:
    """
    from tkinter import Tk
    from tkinter import ttk

    def exit():
        global user_name
        user_name = txt.get()
        window.quit()

    window = Tk()
    if path.exists('icon.ico'):
        window.iconbitmap('icon.ico')

    window.title("Enter your Last.fm username")
    window.resizable(0, 0)
    window.geometry(
        "{}x{}+{}+{}".format(350, 180, window.winfo_screenwidth() // 2 - 175, window.winfo_screenheight() // 2 - 90))
    window.attributes('-topmost', True)

    lbl = ttk.Label(window, text="Enter your Last.fm username", width=25)
    lbl.place(x=100, y=40)

    txt = ttk.Entry(window, width=25)
    txt.place(x=100, y=70)

    btn = ttk.Button(window, text="Ok", width=25, command=exit)
    btn.place(x=100, y=120)

    window.mainloop()
    window.destroy()
    return


if __name__ == "__main__":
    network = pylast.LastFMNetwork(API_KEY, API_SECRET)

    if not path.exists(SESSION_KEY_FILE):
        skg = pylast.SessionKeyGenerator(network)
        url = skg.get_web_auth_url()

        print(f"Please authorize the scrobbler to scrobble to your account: {url}\n")
        import webbrowser
        webbrowser.open(url)

        while True:
            try:
                session_key = skg.get_web_auth_session_key(url)
                fp = open(SESSION_KEY_FILE, "w")
                ask_for_username()
                fp.writelines([user_name, '\n', session_key])
                fp.close()
                exit()
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

    playing_track = None

    client_id = '584458858731405315'  # Discord BOT id, put your real one here
    RPC = Presence(client_id)  # Initialize the client class
    RPC.connect()  # Start the handshake loop

    while True:
        try:
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
            # check if song is played
            elif not new_track and playing_track and "TIDAL.exe" in (p.name() for p in psutil.process_iter()):
                print('song paused ?')

            # clear status
            elif not new_track:
                RPC.clear()

        except Exception as e:
            print("Error: %s" % repr(e))

        sleep(2)
