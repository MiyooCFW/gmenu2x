#include "batteryloggerdialog.h"
#include "powermanager.h"
// #include "debug.h"

BatteryLoggerDialog::BatteryLoggerDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon)
	: Dialog(gmenu2x)
{
	this->title = title;
	this->description = description;
	this->icon = icon;
}

void BatteryLoggerDialog::exec() {
	bool close = false;

	drawTopBar(this->bg, title, description, icon);

	this->bg->box(gmenu2x->listRect, gmenu2x->skinConfColors[COLOR_LIST_BG]);

	drawBottomBar(this->bg);
	gmenu2x->drawButton(this->bg, "start", gmenu2x->tr["Exit"],
	gmenu2x->drawButton(this->bg, "select", gmenu2x->tr["Del battery.csv"],
	gmenu2x->drawButton(this->bg, "down", gmenu2x->tr["Scroll"],
	gmenu2x->drawButton(this->bg, "up", "", 5)-10)));

	this->bg->blit(gmenu2x->s,0,0);

	gmenu2x->setBacklight(100);

	gmenu2x->s->flip();

	MessageBox mb(gmenu2x, gmenu2x->tr["Welcome to the Battery Logger.\nMake sure the battery is fully charged.\nAfter pressing OK, leave the device ON until\nthe battery has been fully discharged.\nThe log will be saved in 'battery.csv'."]);
	mb.exec();

	uint32_t rowsPerPage = gmenu2x->listRect.h/gmenu2x->font->getHeight();

	int32_t firstRow = 0, tickNow = 0, tickStart = SDL_GetTicks(), tickBatteryLogger = -1000000;
	string logfile = gmenu2x->getExePath()+"battery.csv";

	char buf[100];
	sprintf(buf, "echo '----' >> %s/battery.csv; sync", cmdclean(gmenu2x->getExePath()).c_str());
	system(buf);

	if (!fileExists(logfile)) return;

	ifstream inf(logfile.c_str(), ios_base::in);
	if (!inf.is_open()) return;
	vector<string> log;

	gmenu2x->powerManager->clearTimer();

	string line;
	while (getline(inf, line, '\n'))
		log.push_back(line);
	inf.close();

	while (!close) {
		tickNow = SDL_GetTicks();
		if ((tickNow - tickBatteryLogger) >= 60000) {
			tickBatteryLogger = tickNow;

			sprintf(buf, "echo '%s,%d,%d' >> %s/battery.csv; sync", ms2hms(tickNow - tickStart, true, false), gmenu2x->getBatteryStatus(), gmenu2x->getBatteryLevel(), cmdclean(gmenu2x->getExePath()).c_str());
			system(buf);

			ifstream inf(logfile.c_str(), ios_base::in);
			log.clear();

			while (getline(inf, line, '\n'))
				log.push_back(line);
			inf.close();
		}

		this->bg->blit(gmenu2x->s,0,0);

		for (uint32_t i = firstRow; i < firstRow + rowsPerPage && i < log.size(); i++) {
			int rowY, j = log.size() - i - 1;
			if (log.at(j) == "----") { // draw a line
				rowY = 42 + (int)((i - firstRow + 0.5) * gmenu2x->font->getHeight());
				gmenu2x->s->box(5, rowY, gmenu2x->resX - 16, 1, 255, 255, 255, 130);
				gmenu2x->s->box(5, rowY + 1, gmenu2x->resX - 16, 1, 0, 0, 0, 130);
			} else {
				rowY = 42 + (i - firstRow) * gmenu2x->font->getHeight();
				gmenu2x->font->write(gmenu2x->s, log.at(j), 5, rowY);
			}
		}

		gmenu2x->drawScrollBar(rowsPerPage, log.size(), firstRow, gmenu2x->listRect);

		gmenu2x->s->flip();

		bool inputAction = gmenu2x->input.update(false);
		// if (gmenu2x->inputCommonActions(inputAction)) continue;
		if ( gmenu2x->input[UP  ] && firstRow > 0 ) firstRow--;
		else if ( gmenu2x->input[DOWN] && firstRow + rowsPerPage < log.size() ) firstRow++;
		else if ( gmenu2x->input[PAGEUP] || gmenu2x->input[LEFT]) {
			firstRow -= rowsPerPage - 1;
			if (firstRow < 0) firstRow = 0;
		}
		else if ( gmenu2x->input[PAGEDOWN] || gmenu2x->input[RIGHT]) {
			firstRow += rowsPerPage - 1;
			if (firstRow + rowsPerPage >= log.size()) firstRow = max(0, log.size() - rowsPerPage);
		}
		else if ( gmenu2x->input[SETTINGS] || gmenu2x->input[CANCEL] ) close = true;
		else if (gmenu2x->input[MENU]) {
			MessageBox mb(gmenu2x, gmenu2x->tr.translate("Deleting $1", "battery.csv", NULL) + "\n" + gmenu2x->tr["Are you sure?"]);
			mb.setButton(CONFIRM, gmenu2x->tr["Yes"]);
			mb.setButton(CANCEL,  gmenu2x->tr["No"]);
			if (mb.exec() == CONFIRM) {
				system("rm battery.csv");
				log.clear();
			}
		}
	}
	gmenu2x->setBacklight(gmenu2x->confInt["backlight"]);
}
