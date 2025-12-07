# Info

This GMenu2X is a fork of [GMenuNX](https://github.com/pingflood/GMenuNX/) and is developed for the MiyooCFW, released under the GNU GPL license v2.

View changelog in [changelog](https://github.com/MiyooCFW/gmenu2x/blob/master/ChangeLog.md) file & [release](https://github.com/MiyooCFW/gmenu2x/releases) page.

## Installation

#### A) Binary
1. Replace the ``gmenu2x`` binary with the latest one.
2. Additionaly for working directory with linux spefific file type partition (e.g.: EXT, BTRFS):
 ``chmod +x gmenu2x``
3. (Re)Boot your device and enjoy new GMenu2X
#### B) ZIP-bundle
1. Extract the `gmenu2x-*.zip` content to `$HOME/gmenu2x` directory on your device
2. Perform steps 2 & 3 from "Binary" installation process
#### C) IPK-package
1. Launch `gmenu2x-*.ipk` from within GMenu2X's Explorer.
2. Restart GMenu2X frontend or reboot device.

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
you can find both outputs in `./dist/miyoo/` directory.

### PC (Linux):
This is primarily useful for development/testing. 
First, install the dependencies. This should work for Debian/Ubuntu systems, use the appropriate package manager for other systems:
```sh
sudo apt-get install -y build-essential libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev libboost-all-dev libfreetype6-dev libbz2-dev libmpg123-dev
```
Compile with:
```sh
make dist
```
Then run it from the `dist/linux/` directory:
```sh
cd dist/linux
./gmenu2x
```

## Translations

The list of available translations with examples are stored in `gmenu2x/translations/` dir of your GMenu2X distribution package.  
The current in use messages from U.S. English language can be read from: [translate.txt](https://github.com/MiyooCFW/gmenu2x/blob/master/translate.txt). This does not include appLinks' Title&Description which can be added freely by user.

You can run `tools/gen_translate.sh` to generate above list providing argument in form of txt file with list of #define for your platform and target platform name. Otherwise you can rebuild GMenu2X with `-DCHECK_TRANSLATION` CFLAG in Makefile. Then after running binary with selected language you will find `untraslated.txt`file with list of missing translation strings which failed to be generated at the time of using GMenu2X.

### Special strings in translation file
The "\_about\_" assigns corresponding text file for displaying _About_ message, e.g. "\_about\_=\_about\_en" means _about_en.txt is being used for English translation.

The "\_keyboard_<table_number>_<line_number>\_" string will modify alternative overlay keyboard on Input Dialog (accesed with MODIFY button).
The "\_keyboard_en_<table_number>_<line_number>\_" string will modify original English overlay keyboard on Input Dialog (accesed directly or with MANUAL button ).

The "\_TTS_voice\_" string assigns voice type used by TTS engine e.g. "\_TTS_voice\_"=en" for English (check for available voices on your device `/usr/share/espeak-data/voices`)

### Dictionary
To view the full list of string messages available to translate in source see: [dictionary.txt](https://github.com/MiyooCFW/gmenu2x/blob/master/dictionary.txt)

To generate this list of available strings' aliases you can run from src:  
```
(grep -o -rn . -P -e "\ttr\["[^]]*"\]" ; grep -o -rn . -e '>tr\["[^]]*"\]\|(tr\["[^]]*"\]\|\+tr\["[^]]*"\]\|\ tr\["[^]]*"\]\|,tr\["[^]]*"\]') | sed 's/.*\[\(.*\)\].*/\1/' | sed 's/\"\(.*\)\"/\1=/' | sed ':a;N;$!ba;s/\\n/ /g' | tr -d '\\' | sort | uniq
```

## Skins & fonts

Please refer to directory containing appropriate skin for detailed information about authors and licensing of specific theme and its fonts. See below schema for general info:
```
assets/skins
├── Default
│   ├── GNU_Unifont.ttf
│   ├── LICENSE_font-GPLv2.txt
│   └── LICENSE_skin.txt
├── FontiGrid
│   ├── BebasKai.ttf
│   ├── BebasNeue-Bold.ttf
│   ├── font-modify.info
│   ├── LICENSE_font-OFLv1.1.txt
│   └── LICENSE_skin-GPLv2.txt
├── GameShow
│   ├── font-modify.info
│   ├── KdamThmorPro-Regular.ttf
│   ├── LICENSE_font-OFLv1.1.txt
│   └── LICENSE_skin-CC_BY_3.0_US.txt
├── NeonWave
│   ├── Audiowide-Regular.ttf
│   ├── font-modify.info
│   ├── LICENSE_font-OFLv1.1.txt
│   └── LICENSE_skin-CC_BY_3.0_US.txt
└── Nsdark
    ├── BebasKai.ttf
    ├── BebasNeue-Bold.ttf
    ├── font-modify.info
    ├── LICENSE_font-OFLv1.1.txt
    └── LICENSE_skin-GPLv2.txt
```
above list generated with cmd:
`tree assets/skins -L 2 -I 'icons|imgs|logos*|sections|wallpap*|backdrops|free_backdrop|font.ttf|skin.conf|ACKNOW*' -U` 

Fonts under OFL license has been expanded with GNU_Unifont.ttf using `font_merge.sh` tool to include most of the UNICODE glyphs and all CJK characters, thus transforming to Pan-Unicode font type. 

## Controls

(Bittboy's specific mapping in round brackets)

* A: Accept / Launch selected link / Confirm action;
* B: Back / Cancel action;
* X: Goes up one directory in file browser;
* X-hold: Bring up Date&Time quick dialog;
* X + Y: Switch from/to Default font;
* Y: Bring up the manual/readme;
* Y-hold: Restart the GMenu2X;
* L1(TB), R1(TA): Switch between sections left/right or PageUp/PageDown on lists;
* L2, R2: Fast Decrement/Increment of selected value;
* START: GMenu2X settings;
* START-hold:  Toggle Suspend mode;
* SELECT: Bring up the contextual menu;
* SELECT-hold: Disable TV-output;
* SELECT + L1(TB): Volume control;
* SELECT + R1(TA): Change screen brightness;
* SELECT + START: Take a screenshot;
* RESET: Bring up Poweroff dialog;

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
	* press "X" at startup during GMenu2X loading screen
	* discard `dialogAutoStart=1` line in gmenu2x.conf file
	* remove gmenu2x.conf file entirely

## Parameters

You can pass useful variables (which can be treated as arguments for cmd) before launching app in the editLink menu under "Parameters" option: 
- **[selPath]** - this string will be replaced by the selector path.  
- **[selFile]** - this string will be replaced by the selected file's name, in the selector, without its extension.  
- **[selExt]** - this string will be replaced by the selected file's extension.  
- **[selFileFull]** - this is a shortcut value that means [selFile][selExt]  
- **[selFullPath]** - this is a shortcut value that means [selPath][selFile][selExt]  

## How to have previews in Selector Browser

* Select the link you want to edit and press "menu";
* Edit the link;
* Configure the link to match your directory structure. Important fields:
	* Selector Directory: The directory of your roms
	* Selector Browser: Enable selector before launching the app
	* Selector Filter: Filter extensions to be shown in the selector. Separate multiple extensions with commas.
	* Selector Screenshots: The directory of the screenshots/preview of your roms. It can be different than your roms directory.
* The name of the file of rom and preview have to be exactly the same. Supported image types are .png or .jpg;

## Debugging apps

GMenu2X provide option to run debugger (if present in `/usr/bin/gdb`) on non-stripped apps with debug flags, that is if you run it directly from explorer's selector. 