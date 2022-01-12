![GitHub All Releases](https://img.shields.io/github/downloads/purpl3F0x/TIDAL-Discord-Rich-Presence-UNOFFICIAL/total?style=flat-square)
![GitHub Releases](https://img.shields.io/github/downloads/purpl3F0x/TIDAL-Discord-Rich-Presence-UNOFFICIAL/latest/total?style=flat-square)

## TIDAL - Discord Rich Presence plug-in  (UNOFFICIAL)


Unofficial plug in to obtain Discord Rich Presence.

Feel free to report any bugs or make suggestions.

<br>

**If you like the project and want to buy me a glass of milk :)**

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=3XY7W2XHHUZF2&source=url)

## Example Screenshot

![alt text](./assets/screenshot.jpg)

When playing a song with master audio quality.

![alt text](./assets/highres.jpg)

or you have paused playback

![alt text](./assets/paused.jpg)



## Instructions
The new version doesn't require connection with last-fm, 'cause I reversed engineered TIDAL API,  get the data from the window name of the app.

1.  Download the latest release from [here](https://github.com/purpl3F0x/TIDAL-Discord-Rich-Presence-UNOFFICIAL/releases)
(windows and osx are supported).

2.  Run the binary, enjoy.

3.  *Optional*: Place the exe in windows start-up folder to start when computer starts. For OSX select that option from by right clicking the app on taskbar.


The program registers an icon on the taskbar, ~~where you can see the song playing and temporary disable *rich presence*~~.

![alt text](./assets/taskbar.jpg) ![alt text](./assets/taskbar_opened.png)

P.S. Remember to make sure you have Game Activity enabled!

![example of Game Activity tab inside of Discord Settings](https://user-images.githubusercontent.com/3516420/80171200-53356280-85af-11ea-8a51-66b3780250be.png)

Check that _Tidal - Discord RPC_ is running, and add it as a game. It might show as "TIDAL", or as the band you are currently listening to:

![Select Tidal - Discord RPC as a ](https://user-images.githubusercontent.com/12559216/149215040-98e81a25-7ba7-44c5-828b-74b44080d7ee.png)


## Build Instructions

To build the executable you'll need either msvc on windows or clang on osx. For windows I had problems with gcc either conflicting with discord lib on (debug) and http not have <mutex>.


### Disclaimer: This project is Unofficial and it's not published from TIDAL.com &/ Aspiro.

Kudos to:
+ https://github.com/nlohmann/json
+ https://github.com/yhirose/cpp-httplib

for their awesome work.



> ### Stavros Avramidis Never Settle & Keep Running
