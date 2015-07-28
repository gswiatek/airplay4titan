// Copyright (c) 2013, Grzegorz Swiatek. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Grzegorz Swiatek nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Util.h"

#include <iostream>
#include <sstream>
#include <cstring>

using namespace gs::util;
using namespace std;

string Util::getXml(const string& data) {
	string::size_type len = data.length();

	string res;
	res.reserve(len * 2);
	char c;

	for (string::size_type i = 0; i <len; i++) {
		c = data[i];

		if (c == '&') {
			res.append("&amp;");
		} else if (c == '<') {
			res.append("&lt;");
		} else if (c == '>') {
			res.append("&gt;");
		} else if (c == '"') {
			res.append("&quot;");
		} else if (c == '\'') {
			res.append("&apos;");
		} else {
			res.append(1, c);
		}
	}

	return res;
}



int Util::getInt(const string& data) {
	int res;

	istringstream is(data);
	is >> res;

	return res;
}

unsigned int Util::getUInt(const string& data) {
	unsigned int res;

	istringstream is(data);
	is >> res;

	return res;
}

unsigned long long Util::getULongLong(const string& data) {
	unsigned long long res;

	istringstream is(data);
	is >> res;

	return res;
}

time_t Util::getTime(const string& data) {
	time_t res;

	istringstream is(data);
	is >> res;

	return res;
}


string Util::getHttpDate() {
	time_t t = time(0);
	struct tm tm;

#ifdef _WIN32
	gmtime_s(&tm, &t);
#else
	gmtime_r(&t, &tm);
#endif
	char buf[80];
	strftime(buf, 80, "%a, %d %b %Y %H:%M:%S GMT", &tm);

	return string(buf);
}

string Util::getRecDate(time_t t) {
	struct tm tm;

#ifdef _WIN32
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif
	char buf[80];
	strftime(buf, 80, "%H:%M %d-%m-%Y", &tm);

	return string(buf);
}

vector<string> Util::getTokens(const string& line) {
	vector<string> res;
	
	string::size_type last = 0;
	string::size_type pos = line.find('#');

	while (pos != string::npos) {
		res.push_back(line.substr(last, pos - last));
		last = pos + 1;

		pos = line.find('#', last);
	}

	res.push_back(line.substr(last));

	return res;
}

time_t Util::parseTime(const string& name) {
	if (name.length() < 14) {
		return 0;
	}

	struct tm tm;

	memset(&tm, 0, sizeof(tm));

	tm.tm_year = getInt(name.substr(0, 4)) - 1900;
	tm.tm_mon = getInt(name.substr(4, 2)) - 1;
	tm.tm_mday = getInt(name.substr(6, 2));
	tm.tm_hour = getInt(name.substr(8, 2));
	tm.tm_min = getInt(name.substr(10, 2));
	tm.tm_sec = 0; // we ignore seconds
	

	time_t res = mktime(&tm);

	if (res == -1) {
		res = 0;
	}

	return res;
}

std::string Util::urlEncode(const std::string& data, bool mongooseClient) {
	static const char *hex = "0123456789abcdef";

	string::size_type len = data.size();

	string res;
	res.reserve(len * 2);

	char c;

	for (string::size_type i = 0; i < len; i++) {
		c = data[i];

		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
			res.append(1, c);
		} else {
			res.append(mongooseClient ? 2: 1, '%');
			res.append(1, hex[(((const unsigned char) c) >> 4)]);
			res.append(1, hex[((const unsigned char) c) & 0x0f]);
		}
	}

	return res;
}

// TODO: copied from mongoose
int Util::urlDecode(const char *src, int src_len, char *dst,
                      int dst_len) {
  int i, j, a, b;
#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')

  for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++) {
    if (src[i] == '%' && i < src_len - 2 &&
        isxdigit(* (const unsigned char *) (src + i + 1)) &&
        isxdigit(* (const unsigned char *) (src + i + 2))) {
      a = tolower(* (const unsigned char *) (src + i + 1));
      b = tolower(* (const unsigned char *) (src + i + 2));
      dst[j] = (char) ((HEXTOI(a) << 4) | HEXTOI(b));
      i += 2;
	} else if (src[i] == '+') {
		dst[j] = ' ';
    } else {
      dst[j] = src[i];
    }
  }

  dst[j] = '\0'; // Null-terminate the destination

  return i >= src_len ? j : -1;
}


string Util::valueOf(int val) {
	ostringstream os;

	os << val;

	return os.str();
}

string Util::valueOf(unsigned int val) {
	ostringstream os;

	os << val;

	return os.str();
}


string Util::valueOf(unsigned long long val) {
	ostringstream os;

	os << val;

	return os.str();
}

void Util::ltrim(string& str) {
	str.erase(0, str.find_first_not_of(" \n\r\t"));
}

void Util::rtrim(string& str) {
	str.erase(str.find_last_not_of(" \n\r\t") + 1);
}

void Util::trim(string& str) {
	str.erase(0, str.find_first_not_of(" \n\r\t"));
	str.erase(str.find_last_not_of(" \n\r\t") + 1);
}


map<string, string> Util::parseQuery(const string& query) {
	std::map<string, string> res;

	string::size_type start = 0;
	string::size_type pos = query.find('&');
	string::size_type pos2;

	string param;
	string name;
	string val;
	char buf[512];

	while (pos != string::npos) {
		param = query.substr(start, pos - start);

		pos2 = param.find('=');

		if (pos2 != string::npos) {
			name = param.substr(0, pos2);
			val = param.substr(pos2 + 1);

			buf[0] = 0;
			urlDecode(val.c_str(), val.size(), buf, 512);
			res[name] = buf;
		} else {
			res[param] = "";	
		}

		start = pos + 1;

		pos = query.find('&', start);
	}

	param = query.substr(start, pos - start);

	pos2 = param.find('=');

	if (pos2 != string::npos) {
		name = param.substr(0, pos2);
		val = param.substr(pos2 + 1);

		urlDecode(val.c_str(), val.size(), buf, 512);
		res[name] = buf;
	} else {
		res[param] = "";	
	}
	
	return res;
}

const string&  Util::getString(const map<string, string>& params, const string& name, const string& def) {


	map<string, string>::const_iterator it = params.find(name);

	if (it != params.end()) {
		return it->second;
	} else {
		return def;
	}
}

int Util::getInt(const map<string, string>& params, const string& name, int def) {
	int res;


	map<string, string>::const_iterator it = params.find(name);

	if (it != params.end()) {
		res = getInt(it->second);
	} else {
		res = def;
	}

	return res;
}

time_t Util::getTime(const map<string, string>& params, const string& name, time_t def) {
	time_t res;


	map<string, string>::const_iterator it = params.find(name);

	if (it != params.end()) {
		res = getTime(it->second);
	} else {
		res = def;
	}

	return res;
}

string Util::getXmlValue(const string& str, const string& startTag, const std::string& endTag) {
	string res;

	string::size_type startPos = str.find(startTag);

	if (startPos != string::npos) {
		string::size_type endPos = str.find(endTag, startPos + startTag.size());

		if (endPos != string::npos) {
			res = str.substr(startPos + startTag.size(), endPos - (startPos + startTag.size()));
		}
	}

	return res;
}