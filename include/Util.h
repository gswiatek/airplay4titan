#ifndef _gs_util_Util_h_
#define _gs_util_Util_h_

#include <string>
#include <vector>
#include <ctime>
#include <iostream>
#include <map>

namespace gs {
	namespace util {

		class Util {
		public:
			/** Performs XML escaping of the given input string */
			static std::string getXml(const std::string& str);
			/** Extracts unsigned int value from a string */
			static unsigned int getUInt(const std::string& data);
			/** Extracts unsigned int value from a string */
			static unsigned long long getULongLong(const std::string& data);
			/** Extracts int value from a string */
			static int getInt(const std::string& data);
			/** Extracts time_t value from a string */
			static time_t getTime(const std::string& data);
			/** Extracts time from Titan specific file name format (used to get recording timestamp for a movie) */
			static time_t parseTime(const std::string& fileName);
			/** Computes current date/time string to be used in HTTP response */
			static std::string getHttpDate();
			/** Computes date used for record timers */
			static std::string getRecDate(time_t t);
			/** Parses Titan specific lines with values separated by # and delievers the extracted tokens */
			static std::vector<std::string> getTokens(const std::string& line);
			/** Converts int value to string */
			static std::string valueOf(int val);
			/** Converts unsigned int value to string */
			static std::string valueOf(unsigned int val);
			static std::string valueOf(unsigned long long val);

			static int urlDecode(const char* src, int src_len, char* dst, int dst_len);
			/**
			 * URL encoding
			 *
			 * @param data input data to be encoded
			 * @param mongooseClient the client download uses _vsnprintf so we must double escape the perecent char (%)
			 */
			static std::string urlEncode(const std::string& data, bool mongooseClient = false);
			/** Removes all trailing whitespace of the input string */
			static void rtrim(std::string& str);
			/** Removes all leading whitespaces of the input string */
			static void ltrim(std::string& str);
			/** Removes all trailing and leading whitespace of the input string */
			static void trim(std::string& str);

			static std::map<std::string, std::string> parseQuery(const std::string& query);

			/** Gets a string value for a desired key from the parameters map */
			static const std::string& getString(const std::map<std::string, std::string>& params, const std::string& name, const std::string& defaultValue = "");
			/** Gets int value for a desired key from the parameters map */
			static int getInt(const std::map<std::string, std::string>& params, const std::string& name, int defaultValue = 0);
			static time_t getTime(const std::map<std::string, std::string>& params, const std::string& name, time_t defaultValue = 0);
			static std::string getXmlValue(const std::string& str, const std::string& startTag, const std::string& endTag);
		private:
			Util();
			~Util();
		};
	}
}

#endif
