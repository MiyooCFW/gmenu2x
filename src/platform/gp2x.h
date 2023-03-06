#ifndef HW_GP2X_H
#define HW_GP2X_H

const int MAX_VOLUME_SCALE_FACTOR = 200;
// Default values - going to add settings adjustment, saving, loading and such
const int VOLUME_SCALER_MUTE = 0;
const int VOLUME_SCALER_PHONES = 65;
const int VOLUME_SCALER_NORMAL = 100;
const int BATTERY_READS = 10;

volatile uint16_t *memregs;
int SOUND_MIXER = SOUND_MIXER_READ_PCM;
int memdev = 0;

uint32_t hwCheck(unsigned int interval = 0, void *param = NULL) {
	printf("%s:%d: %s\n", __FILE__, __LINE__, __func__);
	return interval;
}

uint8_t getMMCStatus() {
	return MMC_REMOVE;
}

uint8_t getUDCStatus() {
	return UDC_REMOVE;
}

uint8_t getTVOutStatus() {
	return TV_REMOVE;
}

uint8_t getVolumeMode(uint8_t vol) {
	return VOLUME_MODE_NORMAL;
}

int32_t getBatteryStatus() {
	return -1;
}

class GMenuNX : public GMenu2X {
private:
	typedef struct {
		uint16_t batt;
		uint16_t remocon;
	} MMSP2ADC;

	int batteryHandle;
	string ip, defaultgw;

	bool inet, //!< Represents the configuration of the basic network services. @see readCommonIni @see usbnet @see samba @see web
		usbnet,
		samba,
		web;
	volatile uint16_t *MEM_REG;
	int cx25874; //tv-out
	void gp2x_tvout_on(bool pal);
	void gp2x_tvout_off();
	void readCommonIni();
	void initServices();

	void hwInit() {
		setenv("SDL_NOMOUSE", "1", 1);

#if defined(TARGET_GP2X) || defined(TARGET_WIZ) || defined(TARGET_CAANOO) || defined(TARGET_RETROFW)
		memdev = open("/dev/mem", O_RDWR);
		if (memdev < 0) WARNING("Could not open /dev/mem");
#endif

		if (memdev > 0) {
#if defined(TARGET_GP2X)
			memregs = (uint16_t*)mmap(0, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, 0xc0000000);
			MEM_REG = &memregs[0];

			//Fix tv-out
			if (memregs[0x2800 >> 1] & 0x100) {
				memregs[0x2906 >> 1] = 512;
				//memregs[0x290C >> 1]=640;
				memregs[0x28E4 >> 1] = memregs[0x290C >> 1];
			}
			memregs[0x28E8 >> 1] = 239;

#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
			memregs = (uint16_t*)mmap(0, 0x20000, PROT_READ|PROT_WRITE, MAP_SHARED, memdev, 0xc0000000);
#endif
			if (memregs == MAP_FAILED) {
				ERROR("Could not mmap hardware registers!");
				close(memdev);
			}
		}

#if defined(TARGET_GP2X)
		if (file_exists("/etc/open2x")) fwType = "open2x";
		else fwType = "gph";

		f200 = file_exists("/dev/touchscreen/wm97xx");

		//open2x
		savedVolumeMode = 0;
		volumeScalerNormal = VOLUME_SCALER_NORMAL;
		volumeScalerPhones = VOLUME_SCALER_PHONES;
		o2x_usb_net_on_boot = false;
		o2x_usb_net_ip = "";
		o2x_ftp_on_boot = false;
		o2x_telnet_on_boot = false;
		o2x_gp2xjoy_on_boot = false;
		o2x_usb_host_on_boot = false;
		o2x_usb_hid_on_boot = false;
		o2x_usb_storage_on_boot = false;
		usbnet = samba = inet = web = false;
		if (fwType=="open2x") {
			readConfigOpen2x();
			//	VOLUME MODIFIER
			switch(volumeMode) {
				case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
				case VOLUME_MODE_PHONES: setVolumeScaler(volumeScalerPhones); break;
				case VOLUME_MODE_NORMAL: setVolumeScaler(volumeScalerNormal); break;
			}
		}
		readCommonIni();
		cx25874 = 0;
		batteryHandle = 0;
		// useSelectionPng = false;

		batteryHandle = open(f200 ? "/dev/mmsp2adc" : "/dev/batt", O_RDONLY);
		//if wm97xx fails to open, set f200 to false to prevent any further access to the touchscreen
		if (f200) f200 = ts.init();
#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
		/* get access to battery device */
		batteryHandle = open("/dev/pollux_batt", O_RDONLY);
#endif

		w = 320;
		h = 240;

		initServices();
		setGamma(confInt["gamma"]);
		applyDefaultTimings();

		INFO("GP2X Init Done!");
	}

	void hwDeinit() {
#if defined(TARGET_GP2X)
		if (memdev > 0) {
			//Fix tv-out
			if (memregs[0x2800 >> 1] & 0x100) {
				memregs[0x2906 >> 1] = 512;
				memregs[0x28E4 >> 1] = memregs[0x290C >> 1];
			}

			memregs[0x28DA >> 1] = 0x4AB;
			memregs[0x290C >> 1] = 640;
		}
		if (f200) ts.deinit();
		if (batteryHandle != 0) close(batteryHandle);

		if (memdev > 0) {
			memregs = NULL;
			close(memdev);
		}
#endif
	}

public:
	// Open2x settings ---------------------------------------------------------
	bool o2x_usb_net_on_boot, o2x_ftp_on_boot, o2x_telnet_on_boot, o2x_gp2xjoy_on_boot, o2x_usb_host_on_boot, o2x_usb_hid_on_boot, o2x_usb_storage_on_boot;
	string o2x_usb_net_ip;
	int savedVolumeMode;		//	just use the const int scale values at top of source

	//  Volume scaling values to store from config files
	int volumeScalerPhones;
	int volumeScalerNormal;
	//--------------------------------------------------------------------------

	void ledOn() {
#if defined(TARGET_GP2X)
		if (memdev != 0 && !f200) memregs[0x106E >> 1] ^= 16;
		//SDL_SYS_JoystickGp2xSys(joy.joystick, BATT_LED_ON);
#endif
	}

	void ledOff() {
#if defined(TARGET_GP2X)
		if (memdev != 0 && !f200) memregs[0x106E >> 1] ^= 16;
		//SDL_SYS_JoystickGp2xSys(joy.joystick, BATT_LED_OFF);
#endif
	}

	uint16_t getBatteryLevel() {
		int32_t val = getBatteryStatus();

#if defined(TARGET_GP2X)
		//if (batteryHandle<=0) return 6; //AC Power
		if (f200) {
			MMSP2ADC val;
			read(batteryHandle, &val, sizeof(MMSP2ADC));

			if (val.batt==0) return 5;
			if (val.batt==1) return 3;
			if (val.batt==2) return 1;
			if (val.batt==3) return 0;
			return 6;
		} else {
			int battval = 0;
			uint16_t cbv, min=900, max=0;

			for (int i = 0; i < BATTERY_READS; i ++) {
				if ( read(batteryHandle, &cbv, 2) == 2) {
					battval += cbv;
					if (cbv>max) max = cbv;
					if (cbv<min) min = cbv;
				}
			}

			battval -= min+max;
			battval /= BATTERY_READS-2;

			if (battval>=850) return 6;
			if (battval>780) return 5;
			if (battval>740) return 4;
			if (battval>700) return 3;
			if (battval>690) return 2;
			if (battval>680) return 1;
		}

#elif defined(TARGET_WIZ) || defined(TARGET_CAANOO)
		uint16_t cbv;
		if ( read(batteryHandle, &cbv, 2) == 2) {
			// 0=fail, 1=100%, 2=66%, 3=33%, 4=0%
			switch (cbv) {
				case 4: return 1;
				case 3: return 2;
				case 2: return 4;
				case 1: return 5;
				default: return 6;
			}
		}
#endif
	}

	void setCPU(uint32_t mhz) {
		// mhz = constrain(mhz, CPU_CLK_MIN, CPU_CLK_MAX);
		if (memdev > 0) {
			DEBUG("Setting clock to %d", mhz);

#if defined(TARGET_GP2X)
			uint32_t v, mdiv, pdiv=3, scale=0;

			#define SYS_CLK_FREQ 7372800
			mhz *= 1000000;
			mdiv = (mhz * pdiv) / SYS_CLK_FREQ;
			mdiv = ((mdiv-8)<<8) & 0xff00;
			pdiv = ((pdiv-2)<<2) & 0xfc;
			scale &= 3;
			v = mdiv | pdiv | scale;
			MEM_REG[0x910>>1] = v;

#elif defined(TARGET_CAANOO) || defined(TARGET_WIZ)
			volatile uint32_t *memregl = static_cast<volatile uint32_t*>((volatile void*)memregs);
			int mdiv, pdiv = 9, sdiv = 0;
			uint32_t v;

			#define SYS_CLK_FREQ 27
			#define PLLSETREG0   (memregl[0xF004>>2])
			#define PWRMODE      (memregl[0xF07C>>2])
			mdiv = (mhz * pdiv) / SYS_CLK_FREQ;
			if (mdiv & ~0x3ff) return;
			v = pdiv<<18 | mdiv<<8 | sdiv;

			PLLSETREG0 = v;
			PWRMODE |= 0x8000;
			for (int i = 0; (PWRMODE & 0x8000) && i < 0x100000; i++);
#endif
			setTVOut(TVOut);
		}
	}
	void gp2x_tvout_on(bool pal) {
		if (memdev != 0) {
			/*Ioctl_Dummy_t *msg;
			#define FBMMSP2CTRL 0x4619
			int TVHandle = ioctl(SDL_videofd, FBMMSP2CTRL, msg);*/
			if (cx25874!=0) gp2x_tvout_off();
			//if tv-out is enabled without cx25874 open, stop
			//if (memregs[0x2800 >> 1]&0x100) return;
			cx25874 = open("/dev/cx25874",O_RDWR);
			ioctl(cx25874, _IOW('v', 0x02, uint8_t), pal ? 4 : 3);
			memregs[0x2906 >> 1] = 512;
			memregs[0x28E4 >> 1] = memregs[0x290C >> 1];
			memregs[0x28E8 >> 1] = 239;
		}
	}

	void gp2x_tvout_off() {
		if (memdev != 0) {
			close(cx25874);
			cx25874 = 0;
			memregs[0x2906 >> 1] = 1024;
		}
	}

	void settingsOpen2x() {
		SettingsDialog sd(this, ts, tr["Open2x Settings"]);
		sd.addSetting(new MenuSettingBool(this, tr["USB net on boot"], tr["Allow USB networking to be started at boot time"],&o2x_usb_net_on_boot));
		sd.addSetting(new MenuSettingString(this, tr["USB net IP"], tr["IP address to be used for USB networking"],&o2x_usb_net_ip));
		sd.addSetting(new MenuSettingBool(this, tr["Telnet on boot"], tr["Allow telnet to be started at boot time"],&o2x_telnet_on_boot));
		sd.addSetting(new MenuSettingBool(this, tr["FTP on boot"], tr["Allow FTP to be started at boot time"],&o2x_ftp_on_boot));
		sd.addSetting(new MenuSettingBool(this, tr["GP2XJOY on boot"], tr["Create a js0 device for GP2X controls"],&o2x_gp2xjoy_on_boot));
		sd.addSetting(new MenuSettingBool(this, tr["USB host on boot"], tr["Allow USB host to be started at boot time"],&o2x_usb_host_on_boot));
		sd.addSetting(new MenuSettingBool(this, tr["USB HID on boot"], tr["Allow USB HID to be started at boot time"],&o2x_usb_hid_on_boot));
		sd.addSetting(new MenuSettingBool(this, tr["USB storage on boot"], tr["Allow USB storage to be started at boot time"],&o2x_usb_storage_on_boot));
	//sd.addSetting(new MenuSettingInt(this, tr["Speaker Mode on boot"], tr["Set Speaker mode. 0 = Mute, 1 = Phones, 2 = Speaker"],&volumeMode,0,2,1));
		sd.addSetting(new MenuSettingInt(this, tr["Speaker Scaler"], tr["Set the Speaker Mode scaling 0-150\% (default is 100\%)"],&volumeScalerNormal,100, 0,150));
		sd.addSetting(new MenuSettingInt(this, tr["Headphones Scaler"], tr["Set the Headphones Mode scaling 0-100\% (default is 65\%)"],&volumeScalerPhones,65, 0,100));

		if (sd.exec() && sd.edited()) {
			writeConfigOpen2x();
			switch(volumeMode) {
				case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
				case VOLUME_MODE_PHONES: setVolumeScaler(volumeScalerPhones); break;
				case VOLUME_MODE_NORMAL: setVolumeScaler(volumeScalerNormal); break;
			}
			setVolume(confInt["globalVolume"]);
		}
	}

	void readConfigOpen2x() {
		string conffile = "/etc/config/open2x.conf";
		if (!file_exists(conffile)) return;
		ifstream inf(conffile.c_str(), ios_base::in);
		if (!inf.is_open()) return;
		string line;
		while (getline(inf, line, '\n')) {
			string::size_type pos = line.find("=");
			string name = trim(line.substr(0,pos));
			string value = trim(line.substr(pos+1,line.length()));

			if (name=="USB_NET_ON_BOOT") o2x_usb_net_on_boot = value == "y" ? true : false;
			else if (name=="USB_NET_IP") o2x_usb_net_ip = value;
			else if (name=="TELNET_ON_BOOT") o2x_telnet_on_boot = value == "y" ? true : false;
			else if (name=="FTP_ON_BOOT") o2x_ftp_on_boot = value == "y" ? true : false;
			else if (name=="GP2XJOY_ON_BOOT") o2x_gp2xjoy_on_boot = value == "y" ? true : false;
			else if (name=="USB_HOST_ON_BOOT") o2x_usb_host_on_boot = value == "y" ? true : false;
			else if (name=="USB_HID_ON_BOOT") o2x_usb_hid_on_boot = value == "y" ? true : false;
			else if (name=="USB_STORAGE_ON_BOOT") o2x_usb_storage_on_boot = value == "y" ? true : false;
			else if (name=="VOLUME_MODE") volumeMode = savedVolumeMode = constrain( atoi(value.c_str()), 0, 2);
			else if (name=="PHONES_VALUE") volumeScalerPhones = constrain( atoi(value.c_str()), 0, 100);
			else if (name=="NORMAL_VALUE") volumeScalerNormal = constrain( atoi(value.c_str()), 0, 150);
		}
		inf.close();
	}

	void writeConfigOpen2x() {
		ledOn();
		string conffile = "/etc/config/open2x.conf";
		ofstream inf(conffile.c_str());
		if (inf.is_open()) {
			inf << "USB_NET_ON_BOOT=" << ( o2x_usb_net_on_boot ? "y" : "n" ) << endl;
			inf << "USB_NET_IP=" << o2x_usb_net_ip << endl;
			inf << "TELNET_ON_BOOT=" << ( o2x_telnet_on_boot ? "y" : "n" ) << endl;
			inf << "FTP_ON_BOOT=" << ( o2x_ftp_on_boot ? "y" : "n" ) << endl;
			inf << "GP2XJOY_ON_BOOT=" << ( o2x_gp2xjoy_on_boot ? "y" : "n" ) << endl;
			inf << "USB_HOST_ON_BOOT=" << ( (o2x_usb_host_on_boot || o2x_usb_hid_on_boot || o2x_usb_storage_on_boot) ? "y" : "n" ) << endl;
			inf << "USB_HID_ON_BOOT=" << ( o2x_usb_hid_on_boot ? "y" : "n" ) << endl;
			inf << "USB_STORAGE_ON_BOOT=" << ( o2x_usb_storage_on_boot ? "y" : "n" ) << endl;
			inf << "VOLUME_MODE=" << volumeMode << endl;
			if (volumeScalerPhones != VOLUME_SCALER_PHONES) inf << "PHONES_VALUE=" << volumeScalerPhones << endl;
			if (volumeScalerNormal != VOLUME_SCALER_NORMAL) inf << "NORMAL_VALUE=" << volumeScalerNormal << endl;
			inf.close();
			sync();
		}
		ledOff();
	}

	void activateSdUsb() {
		if (usbnet) {
			MessageBox mb(this, tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
			mb.exec();
		} else {
			MessageBox mb(this, tr["USB Enabled (SD)"],"skin:icons/usb.png");
			mb.setButton(CONFIRM, tr["Turn off"]);
			mb.exec();
			system("scripts/usbon.sh nand");
		}
	}

	void activateNandUsb() {
		if (usbnet) {
			MessageBox mb(this, tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
			mb.exec();
		} else {
			system("scripts/usbon.sh nand");
			MessageBox mb(this, tr["USB Enabled (Nand)"],"skin:icons/usb.png");
			mb.setButton(CONFIRM, tr["Turn off"]);
			mb.exec();
			system("scripts/usboff.sh nand");
		}
	}

	void activateRootUsb() {
		if (usbnet) {
			MessageBox mb(this,tr["Operation not permitted."]+"\n"+tr["You should disable Usb Networking to do this."]);
			mb.exec();
		} else {
			system("scripts/usbon.sh root");
			MessageBox mb(this,tr["USB Enabled (Root)"],"skin:icons/usb.png");
			mb.setButton(CONFIRM, tr["Turn off"]);
			mb.exec();
			system("scripts/usboff.sh root");
		}
	}

	void applyRamTimings() {
		// 6 4 1 1 1 2 2
		if (memdev!=0) {
			int tRC = 5, tRAS = 3, tWR = 0, tMRD = 0, tRFC = 0, tRP = 1, tRCD = 1;
			memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
			memregs[0x3804>>1] = ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
		}
	}

	void applyDefaultTimings() {
		// 8 16 3 8 8 8 8
		if (memdev!=0) {
			int tRC = 7, tRAS = 15, tWR = 2, tMRD = 7, tRFC = 7, tRP = 7, tRCD = 7;
			memregs[0x3802>>1] = ((tMRD & 0xF) << 12) | ((tRFC & 0xF) << 8) | ((tRP & 0xF) << 4) | (tRCD & 0xF);
			memregs[0x3804>>1] = ((tRC & 0xF) << 8) | ((tRAS & 0xF) << 4) | (tWR & 0xF);
		}
	}

	void setGamma(int gamma) {
		float fgamma = (float)constrain(gamma,1,100)/10;
		fgamma = 1 / fgamma;
		MEM_REG[0x2880>>1] &= ~(1<<12);
		MEM_REG[0x295C>>1] = 0;

		for (int i = 0; i < 256; i++) {
			uint8_t g = (uint8_t)(255.0*pow(i/255.0,fgamma));
			uint16_t s = (g << 8) | g;
			MEM_REG[0x295E >> 1] = s;
			MEM_REG[0x295E >> 1] = g;
		}
	}

	void setVolumeScaler(int scale) {
		scale = constrain(scale,0,MAX_VOLUME_SCALE_FACTOR);
		uint32_t soundDev = open("/dev/mixer", O_WRONLY);
		if (soundDev) {
			ioctl(soundDev, SOUND_MIXER_PRIVATE2, &scale);
			close(soundDev);
		}
	}

	int getVolumeScaler() {
		int currentscalefactor = -1;
		uint32_t soundDev = open("/dev/mixer", O_RDONLY);
		if (soundDev) {
			ioctl(soundDev, SOUND_MIXER_PRIVATE1, &currentscalefactor);
			close(soundDev);
		}
		return currentscalefactor;
	}

	void readCommonIni() {
		if (!file_exists("/usr/gp2x/common.ini")) return;
		ifstream inf("/usr/gp2x/common.ini", ios_base::in);
		if (!inf.is_open()) return;
		string line;
		string section = "";
		while (getline(inf, line, '\n')) {
			line = trim(line);
			if (line[0]=='[' && line[line.length()-1]==']') {
				section = line.substr(1,line.length()-2);
			} else {
				string::size_type pos = line.find("=");
				string name = trim(line.substr(0,pos));
				string value = trim(line.substr(pos+1,line.length()));

				if (section=="usbnet") {
					if (name=="enable")
						usbnet = value=="true" ? true : false;
					else if (name=="ip")
						ip = value;

				} else if (section=="server") {
					if (name=="inet")
						inet = value=="true" ? true : false;
					else if (name=="samba")
						samba = value=="true" ? true : false;
					else if (name=="web")
						web = value=="true" ? true : false;
				}
			}
		}
		inf.close();
	}

	void initServices() {
		if (usbnet) {
			string services = "scripts/services.sh "+ip+" "+(inet?"on":"off")+" "+(samba?"on":"off")+" "+(web?"on":"off")+" &";
			system(services.c_str());
		}
	}


// void GMenu2X::about() {
// 	vector<string> text;
// 	string temp, buf;

// 	temp = tr["Build date: "] + __DATE__ + "\n";

// 	// { stringstream ss; ss << w << "x" << h << "px"; ss >> buf; }
// 	// temp += tr["Resolution: "] + buf + "\n";

// #ifdef TARGET_RETROFW
// 	// temp += tr["CPU: "] + entryPoint() + "\n";
// 	float battlevel = getBatteryStatus();
// 	{ stringstream ss; ss.precision(2); ss << battlevel/1000; ss >> buf; }
// 	temp += tr["Battery: "] + ((battlevel < 0 || battlevel > 10000) ? tr["Charging"] : buf + " V") + "\n";
// #endif

// 	// char *hms = ;
// 	temp += tr["Uptime: "] + ms2hms(SDL_GetTicks()) + "\n";

// 	// temp += "----\n";
// 	// temp += tr["Storage:"];
// 	// temp += "\n    " + tr["Root: "] + getDiskFree("/");
// 	// temp += "\n    " + tr["Internal: "] + getDiskFree("/mnt/int_sd");
// 	// temp += "\n    " + tr["External: "] + getDiskFree("/mnt/ext_sd");
// 	temp += "----\n";

// 	TextDialog td(this, "GMenuNX", tr["Info about system"], "skin:icons/about.png");

// // #if defined(TARGET_CAANOO)
// // 	string versionFile = "";
// // // 	if (file_exists("/usr/gp2x/version"))
// // // 		versionFile = "/usr/gp2x/version";
// // // 	else if (file_exists("/tmp/gp2x/version"))
// // // 		versionFile = "/tmp/gp2x/version";
// // // 	if (!versionFile.empty()) {
// // // 		ifstream f(versionFile.c_str(), ios_base::in);
// // // 		if (f.is_open()) {
// // // 			string line;
// // // 			if (getline(f, line, '\n'))
// // // 				temp += "\nFirmware version: " + line + "\n" + exec("uname -srm");
// // // 			f.close();
// // // 		}
// // // 	}
// // 	td.appendText("\nFirmware version: ");
// // 	td.appendFile(versionFile);
// // 	td.appendText(exec("uname -srm"));
// // #endif

// 	td.appendText(temp);
// 	td.appendFile("about.txt");
// 	td.exec();
// }

}

// 		// VOLUME SCALE MODIFIER
// #if defined(TARGET_GP2X)
// 		else if ( fwType=="open2x" && input[CANCEL] ) {
// 			volumeMode = constrain(volumeMode - 1, -VOLUME_MODE_MUTE - 1, VOLUME_MODE_NORMAL);
// 			if (volumeMode < VOLUME_MODE_MUTE)
// 				volumeMode = VOLUME_MODE_NORMAL;
// 			switch(volumeMode) {
// 				case VOLUME_MODE_MUTE:   setVolumeScaler(VOLUME_SCALER_MUTE); break;
// 				case VOLUME_MODE_PHONES: setVolumeScaler(volumeScalerPhones); break;
// 				case VOLUME_MODE_NORMAL: setVolumeScaler(volumeScalerNormal); break;
// 			}
// 			setVolume(confInt["globalVolume"]);
// 		}
// #endif

#endif
