/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo   *
 *   massimiliano.torromeo@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <fstream>
#include <sstream>
#include <stdarg.h>
#if defined(CHECK_TRANSLATION)
#include <set>
#include <algorithm>
#endif

#include "translator.h"
#include "debug.h"

using namespace std;

Translator::Translator(const string &lang) {
	_lang = "";
	if (!lang.empty())
		setLang(lang);
}

Translator::~Translator() {}

bool Translator::exists(const string &term) {
	return translations.find(term) != translations.end();
}

void Translator::setLang(const string &lang) {
	translations.clear();

	string line;
	ifstream infile (string("translations/"+lang).c_str(), ios_base::in);
	if (infile.is_open()) {
		while (getline(infile, line, '\n')) {
			line = trim(line);
			if (line=="") continue;
			if (line[0]=='#') continue;
			if (line.back() == '=') continue;

			string::size_type position = line.find("=");
			translations[trim(line.substr(0, position))] = trim(line.substr(position + 1));
		}
		infile.close();
		_lang = lang;
	}
}

string Translator::translate(const string &term,const char *replacestr,...) {
	string result = term;

	if (!_lang.empty()) {
		unordered_map<string, string>::iterator i = translations.find(term);
		if (i != translations.end()) {
			result = i->second;
		#if defined(CHECK_TRANSLATION)
		} else {
			WARNING("Untranslated string: '%s'", term.c_str());
			ofstream langLog("untranslated.txt", ios::app);

			if (langLog.is_open()) {
				// Write strings to the file, each on a new line
				langLog << term + "=\n";

				INFO("String written to the file successfully.");
				langLog.close();
				
				//sort and remove double entries in generated list
				ifstream input("untranslated.txt");
				if (input.is_open()) {
					vector<std::string> lines;
					string line;
					while (std::getline(input, line)) {
						lines.push_back(line);
					}
					input.close();
					sort(lines.begin(), lines.end());
					set<std::string> uniqueLines(lines.begin(), lines.end());
					ofstream output("untranslated.txt");
					for (const auto& uniqueLine : uniqueLines) {
						output << uniqueLine << '\n';
					}
					output.close();
				}
			} else {
				WARNING("Unable to open the file.");
			}
		#endif
		}
	}

	va_list arglist;
	va_start(arglist, replacestr);
	const char *param = replacestr;
	int argnum = 1;
	while (param!=NULL) {
		string id = "";
		stringstream ss; ss << argnum; ss >> id;
		result = strreplace(result, "$" + id, param);

		param = va_arg(arglist, const char*);
		argnum++;
	}
	va_end(arglist);

	return result;
}

string Translator::operator[](const string &term) {
	return translate(term);
}

string Translator::lang() {
	return _lang;
}
