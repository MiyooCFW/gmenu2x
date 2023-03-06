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


#ifndef UTILITIES_H
#define UTILITIES_H

#ifndef PATH_MAX
#define PATH_MAX 2048
#endif

#include <string>
#include <sstream>
#include <vector>
#include <tr1/unordered_map>
#include <unistd.h>
#include <sys/statvfs.h>

using std::tr1::unordered_map;
using std::tr1::hash;
using std::string;
using std::vector;
using std::stringstream;

class case_less {
public:
	bool operator()(const string &left, const string &right) const;
};

string trim(const string& s);
string strreplace (string orig, const string &search, const string &replace);
string cmdclean (string cmdline);

char *string_copy(const string &);
void string_copy(const string &, char **);

bool file_exists(const string &path);
bool dir_exists(const string &path);
bool rmtree(string path);

int max(int a, int b);
int min(int a, int b);
int constrain(int x, int imin, int imax);

int evalIntConf(int val, int def, int imin, int imax);
int evalIntConf(int *val, int def, int imin, int imax);
const string &evalStrConf(const string &val, const string &def);
const string &evalStrConf(string *val, const string &def);

float max(float a, float b);
float min(float a, float b);
float constrain(float x, float imin, float imax);

bool split(vector<string> &vec, const string &str, const string &delim, bool destructive=true);

int intTransition(int from, int to, int32_t tickStart, int32_t duration = 500, int32_t tickNow = -1);

string exec(const char* cmd);
string real_path(const string &path);
string dir_name(const string &path);
string base_name(string path, bool strip_extension = false);
string file_ext(const string &path, bool tolower = false);
string lowercase(string s);
string unique_filename(string path, string ext);
string exe_path();
string disk_free(const char *path);
const string get_date_time();
void sync_date_time(time_t t);
void init_date_time();
void build_date_time();
void set_date_time(const char* timestamp);

bool file_copy(const string &src, const string &dst);

#endif
