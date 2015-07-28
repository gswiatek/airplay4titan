#ifndef _gs_airplay_PropList_h_
#define _gs_airplay_PropList_h_

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <cstddef>
#include <stdint.h>


// See: http://www.opensource.apple.com/source/CF/CF-744/CFBinaryPList.c
/*
HEADER
	magic number ("bplist")
	file format version
	byte length of plist incl. header, an encoded int number object (as below) [v.2+ only]
	32-bit CRC (ISO/IEC 8802-3:1989) of plist bytes w/o CRC, encoded always as
            "0x12 0x__ 0x__ 0x__ 0x__", big-endian, may be 0 to indicate no CRC [v.2+ only]

OBJECT TABLE
	variable-sized objects

	Object Formats (marker byte followed by additional info in some cases)
	null	0000 0000			// null object [v1+ only]
	bool	0000 1000			// false
	bool	0000 1001			// true
	url	0000 1100	string		// URL with no base URL, recursive encoding of URL string [v1+ only]
	url	0000 1101	base string	// URL with base URL, recursive encoding of base URL, then recursive encoding of URL string [v1+ only]
	uuid	0000 1110			// 16-byte UUID [v1+ only]
	fill	0000 1111			// fill byte
	int	0001 0nnn	...		// # of bytes is 2^nnn, big-endian bytes
	real	0010 0nnn	...		// # of bytes is 2^nnn, big-endian bytes
	date	0011 0011	...		// 8 byte float follows, big-endian bytes
	data	0100 nnnn	[int]	...	// nnnn is number of bytes unless 1111 then int count follows, followed by bytes
	string	0101 nnnn	[int]	...	// ASCII string, nnnn is # of chars, else 1111 then int count, then bytes
	string	0110 nnnn	[int]	...	// Unicode string, nnnn is # of chars, else 1111 then int count, then big-endian 2-byte uint16_t
		0111 xxxx			// unused
	uid	1000 nnnn	...		// nnnn+1 is # of bytes
		1001 xxxx			// unused
	array	1010 nnnn	[int]	objref*	// nnnn is count, unless '1111', then int count follows
	ordset	1011 nnnn	[int]	objref* // nnnn is count, unless '1111', then int count follows [v1+ only]
	set	1100 nnnn	[int]	objref* // nnnn is count, unless '1111', then int count follows [v1+ only]
	dict	1101 nnnn	[int]	keyref* objref*	// nnnn is count, unless '1111', then int count follows
		1110 xxxx			// unused
		1111 xxxx			// unused

OFFSET TABLE
	list of ints, byte size of which is given in trailer
	-- these are the byte offsets into the file
	-- number of these is in the trailer

TRAILER
	byte size of offset ints in offset table
	byte size of object refs in arrays and dicts
	number of offsets in offset table (also is number of objects)
	element # in offset table which is top level object
	offset table offset

Version 1.5 binary plists do not use object references (uid),
but instead inline the object serialization itself at that point.
It also doesn't use an offset table or a trailer.  It does have
an extended header, and the top-level object follows the header.

*/
namespace gs {
	namespace airplay {

		class Property {
		public:
			typedef enum {
				T_SIMPLE = 0x00,
				T_INT= 0x01,
				T_REAL = 0x02,
				T_DATE = 0x03,
				T_DATA = 0x04,
				T_ASCII = 0x05,
				T_UNICODE = 0x06,
				T_UID = 0x08,
				T_ARRAY = 0x0a,
				T_SET = 0x0c,
				T_DICT = 0x0d
			} Type;

			typedef enum {
				V_NULL = 0x00,
				V_FALSE = 0x08,
				V_TRUE = 0x09,
				V_FILL = 0x0f
			} Simple;


			explicit Property(bool value);
			explicit Property(const std::string& value);
			explicit Property(Type type, const std::string& value);
			explicit Property(double value);
			explicit Property(Type type, double value);
			explicit Property(int64_t value);
			explicit Property(Simple simple);
			explicit Property(const std::map<std::string, Property>& dict);
			explicit Property(const std::vector<Property>& array);
			Property();

			Type getType() const { return m_type; }
			bool getBool() const { return m_simple == V_TRUE ? true : false; }
			int64_t getInt() const { return m_int; }
			double getReal() const {return m_real; }
			double getDate() const { return m_real; }
			Simple getSimple() const { return m_simple; }
			const std::string& getAscii() const { return m_str; }
			const std::string& getData() const { return m_str; }
			const std::string& getUnicode() const { return m_str; }
			const std::string& getUid() const { return m_str; }

			void add(const std::string& name, const Property& prop);
			void add(const Property& prop);
			Property get(const std::string& name);

			static bool read(const uint8_t* data, size_t len, Property& root);
		private:
			inline static uint64_t read(const uint8_t* is, int bytes = 8) {
				uint64_t v = 0;

				for (int i = 0; i < bytes; i++) {
					v <<= 8;
					v +=  is[i];
				}

				return v;
			}


			inline static uint64_t getLen(const uint8_t* is, uint64_t& offset, uint8_t info) {
				uint64_t res = 0;
	
				++offset;
				if (info == 0x0f) {
					uint8_t next = is[offset++];
					uint8_t bytes = 1 << (next & 0x0f); 
					res = read(is + offset, bytes);
					offset += bytes;
				} else {
					res = info;
				}

				return res;
			}

			static void readObject(const uint8_t* is, size_t max, size_t objectBytes, const std::vector<uint64_t>& offsetTable, uint64_t index, Property& parent);

			Type m_type;

			std::map<std::string, Property> m_dict;
			std::vector<Property> m_array;
			int64_t m_int;
			double m_real;
			Simple m_simple;
			std::string m_str; 

		};
	}
}

#endif
