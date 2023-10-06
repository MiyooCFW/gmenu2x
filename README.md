# Info

This GMenu2X is a fork of [GMenuNX](https://github.com/pingflood/GMenuNX/) and is developed for the MiyooCFW, released under the GNU GPL license v2.

View changelog in [changelog](https://github.com/MiyooCFW/gmenu2x/blob/master/ChangeLog.md) file.

## Installation

Replace the ``gmenu2x`` binary with the latest one from action builds.

(Re)Boot your device and enjoy new GMenu2X


## Building

### MiyooCFW:

Set up your environment with latest SDK (use [MiyooCFW/buildroot](https://github.com/miyoocfw/buildroot/)). Generated toolchain should be placed at `/opt/miyoo/` directory.
- build gmenu2x binary:
```
make -f Makefile.miyoo
```
- build GMenu2X distribution package:
```
make -f Makefile.miyoo dist
```
you can find both outputs in `/dist/miyoo/` directory.
### PC (Linux):
This is primarily useful for development/testing. 
First, install the dependencies. This should work for Debian/Ubuntu systems, use the appropriate package manager for other systems:
```sh
sudo apt-get install -y build-essential libsdl-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev libboost-all-dev libfreetype6-dev libbz2-dev libmpg123-dev
```
Compile with:
```sh
make -f Makefile.linux dist
```
Then run it from the `dist/linux/` directory:
```sh
cd dist/linux
./gmenu2x
```

## Translations

The list of available translations with examples are stored in `gmenu2x/translations/` dir of your GMenu2X distribution package.  

### Dictionary
The current in use messages from US English language can be read from [dictionary.txt](https://github.com/MiyooCFW/gmenu2x/blob/master/dictionary.txt) file.  

To generate list of available strings' aliases you can run from src:  
```
(grep -o -rn . -P -e "\ttr\["[^]]*"\]" ; grep -o -rn . -e '>tr\["[^]]*"\]\|\+tr\["[^]]*"\]\|\ tr\["[^]]*"\]\|,tr\["[^]]*"\]') | sed 's/.*\[\(.*\)\].*/\1/' | sed 's/\"\(.*\)\"/\1=/' | tr -d '\\' |sort | uniq
```

## Controls

* A: Accept / Launch selected link / Confirm action;
* B: Back / Cancel action;
* X: Goes up one directory in file browser;
* X (hold): Bring up Date&Time quick dialog;
* Y: Bring up the manual/readme;
* L1, R1: Switch between sections / PageUp/PageDown on lists;
* L2, R2: Fast Decrement/Increment of selected value;
* START: GMenu2X settings;
* START (hold):  Toggle Suspend mode;
* SELECT: Bring up the contextual menu;
* SELECT (hold): Disconnect TV-out;
* SELECT + L: Volume control;
* SELECT + R, BRIGHTNESS: Change screen brightness;
* SELECT + START: Take a screenshot;
* POWER: Bring up Poweroff dialog;
* TV-Out: If the device supports, (dis)connect the TV-out jack to toggle TV-out signal.

In settings:

* A, LEFT, RIGHT: Accept, select or toggle setting;
* B: Back or cancel edits and return;
* START: Save edits and return;
* SELECT: Clear or set default value.

## AutoStart
* In Settings - enable AUTOSTART feature.
* Start app/game you would like to be launched every time at first after boot/reboot.
* Every time you launch a device you will be welcomed with AutoStart selection menu.
* Next previously launched app will start with all the settings you provide it with from GMenu2X linkapp config.
* When you disable AutoStart msgbox there are only 2 ways to turn OFF this feature:
	* press "Y" at startup during GMenu2X loading screen
	* discard `dialogAutoStart=1` line in gmenu2x.conf file
	* remove gmenu2x.conf file entirely

## Parameters

You can pass useful variables (which can be treated as arguments for cmd) before launching app in the editLink menu under "Parameters" option: 
- **[selPath]** - this string will be replaced by the selector path.  
- **[selFile]** - this string will be replaced by the selected file's name, in the selector, without its extension.  
- **[selExt]** - this string will be replaced by the selected file's extension.  
- **[selFileFull]** - this is a shortcut value that means [selFile][selExt]  
- **[selFullPath]** - this is a shortcut value that means [selPath][selFile][selExt]  

## [How to have previews in Selector Browser](http://boards.dingoonity.org/ingenic-jz4760-devices/gmenunext-let's-make-gmenu-great-again!/msg177392/#msg177392)

* Select the link you want to edit and press "menu";
* Edit the link;
* Configure the link to match your directory structure. Important fields:
	* Selector Directory: The directory of your roms
	* Selector Browser: Enable selector before launching the app
	* Selector Filter: Filter extensions to be shown in the selector. Separe multiple extensions with commas.
	* Selector Screenshots: The directory of the screenshots/preview of your roms. It can be different than your roms directory.
* The name of the file of rom and preview have to be exactly the same. Suported image types are .png or .jpg;

## Contacts

GMenu2X Copyright (c) 2006-2010 [Massimiliano Torromeo](mailto:massimiliano.torromeo@gmail.com);  
GMenuNX 2018-2019 by [@pingflood](https://github.com/pingflood);  
GMenu2X 2022 (modded GMenuNX) by [@Apaczer](https://github.com/Apaczer);

## Credits

### Contributors
NoidZ for his gp2x' buttons graphics;  
Pickle for the initial Wiz and Caanoo ports;  
Steward-Fu for the initial RetroGame ports;  
TonyJih for the new RetroFW features;  
Fontes for the RetroFW graphics;  

### Beta testers
Goemon4, PokeParadox, PSyMastR and Tripmonkey_uk (GP2X);  
Yann Vaillant (WIZ);  
msx, jbanes, jutley and scooterpsu (RetroFW);  
salvacam, SolidOne (MiyooCFW).  

### Translators
Chinese (CN): 蔡蔡哥, simpleasy, KungfuPanda;  
Chinese (TW): wentao, TonyJih;  
Danish: claus;  
Dutch: superfly;  
English: Massimiliano;  
Finnish: Jontte Atte;  
French: Yodaz;  
German: fusion_power, johnnysnet, Waldteufel;  
Italian: Massimiliano;  
Korean: haven-jeon;  
Norwegian: cowai;  
Polish: Macmmm81;  
Portuguese (Brazil): pingflood;  
Portuguese (Portugal): NightShadow;  
Russian: XaMMaX90;  
Slovak: Jozef;  
Spanish: pedator;  
Swedish: Esslan Micket;  

### Donors
EvilDragon (www.gp2x.de), 
Tecnologie Creative (www.tecnologiecreative.it), 
TelcoLou, 
gaterooze, 
deepmenace, 
superfly, 
halo9, 
sbock, 
b._.o._.b, 
Jacopastorius, 
lorystorm90.

and all the anonymous colaborators...