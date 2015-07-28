#include "PropList.h"
#include <fstream>

using namespace std;
using namespace gs::airplay;


Property::Property(): m_type(T_SIMPLE), m_simple(V_NULL) {

}

Property::Property(int64_t val): m_type(T_INT), m_int(val) {

}

Property::Property(double val): m_type(T_REAL), m_real(val) {

}

Property::Property(Simple simple): m_type(T_SIMPLE), m_simple(simple) {

}

Property::Property(Type type, double val): m_type(type), m_real(val) {

}

Property::Property(bool val): m_type(T_SIMPLE), m_simple(val ? V_TRUE : V_FALSE) {

}

Property::Property(const string& val): m_type(T_ASCII), m_str(val) {

}

Property::Property(Type type, const string& val): m_type(type), m_str(val) {

}

Property::Property(const map<string, Property>& val): m_type(T_DICT), m_dict(val) {

}

Property::Property(const vector<Property>& val): m_type(T_ARRAY), m_array(val) {

}

void Property::add(const Property& prop) {
	m_type = T_ARRAY;
	m_array.push_back(prop);
}

void Property::add(const string& name, const Property& prop) {
	m_type = T_DICT;
	m_dict[name] = prop;
}

bool Property::read(const uint8_t* data, size_t len, Property& root) {
	if (len < 40) {
		cerr << "Data too short: len=" << len << endl;
		return false;
	}

	static const string magic("bplist");
	static const uint8_t trailerSize = 32;

	for (int i = 0; i < 6; i++) {
		if (data[i] != magic[i]) {
			cerr << "Invalid magic: pos=" << i << ", char=" << data[i] << endl;
			return false;
		}
	}

	uint8_t major = data[6];
	uint8_t minor = data[7];

	if (major != '0' && minor != '0') {
		cerr << "Unsported version: " << major << minor << endl;
		return false;
	}


	const uint8_t* trailer = data + len - 26;

	uint64_t pos = 0;
	size_t offsetBytes = trailer[pos++];
	size_t objectBytes = trailer[pos++];

	uint64_t numOfObjects = read(trailer +  pos);
	pos += 8;

	uint64_t rootId = read(trailer + pos);
	pos += 8;
	uint64_t offsetTablePos = read(trailer + pos);
	
	if (offsetBytes < 1 || offsetBytes > 8 ||
		objectBytes < 1 || objectBytes > 8 ||
		offsetTablePos < 8) {
		cerr << "Invalid trailer" << endl;
		return false;
	}

	uint64_t offsetTableSize = offsetBytes * numOfObjects;

	if (offsetTablePos + offsetTableSize + trailerSize > len) {
		cerr << "Invalid binary plist: offset out of range" << endl;
		return false;
	}

	vector<uint64_t> offsetTable;
	pos = offsetTablePos;

	// read offset table
	for (uint64_t i = 0; i < numOfObjects; i++) {
		offsetTable.push_back(read(data + pos, offsetBytes));
		pos += offsetBytes;
	}
	
	readObject(data, len, objectBytes, offsetTable, rootId, root);

	return true;
}


void Property::readObject(const uint8_t* is, size_t max, size_t objectBytes, const std::vector<uint64_t>& offsetTable, uint64_t index, Property& parent) {
	if (index >= 0 && index < offsetTable.size()) {
		uint64_t objectOffset = offsetTable[index];

		const uint8_t * data = is + objectOffset;
		uint64_t t = read(data, 1);

		Type type = (Type) (t >> 4);

		switch(type) {
		case T_SIMPLE:
			parent = Property((Simple) t);
			break;

		case T_ASCII:
		case T_DATA:
			{
				uint64_t offset = 0;
				uint64_t len = getLen(data, offset, (t & 0x0f));
				string str;


				for (size_t i = 0; i < len; i++) {
					str.push_back(data[offset + i]);
				}

				parent = Property(type, str);
			}

			break;

		case T_UNICODE:
			{
				uint64_t offset = 0;
				uint64_t len = getLen(data, offset, (t & 0x0f));
				string str;

				for (size_t i = 0; i < len * 2; i++) {
					str.push_back(data[offset + i]);
				}

				parent = Property(type, str);
			}

			break;

		case T_INT:
			{
				uint64_t len = 1 << (t & 0x0f);
				int64_t val = read(data + 1, len);
				parent = Property(val);
			}

			break;

		case T_REAL:
			{
				
				/*uint64_t len = 1 << (t & 0x0f);
				double d = 0.0;
				is.read(reinterpret_cast<char*>(&d), len);
				parent = Property(d);*/
			}

			break;
		case T_DATE:
			{
				/*double d = 0.0;
				is.read(reinterpret_cast<char*>(&d), 8);
				parent = Property(type, d);*/
			}

			break;

		case T_ARRAY:
		case T_SET:
			{
				uint64_t offset = 0;
				uint64_t len = getLen(data, offset, (t & 0x0f));
				vector<uint64_t> arrayValueIds;

				for (size_t i = 0; i < len; i++) {
					uint64_t v = read(data + offset + i, objectBytes);
					arrayValueIds.push_back(v);
				}

				for (size_t i = 0; i < len; i++) {
					Property child;

					readObject(is, max, objectBytes, offsetTable, arrayValueIds[i], child);
					parent.add(child);
				}
			}
			break;

		case T_UID:
			{
				/*uint64_t len = (t & 0x0f) + 1;
				string str;

				for (size_t i = 0; i < len; i++) {
					str.push_back(is.get());
				}

				parent = Property(type, str);*/
			}

			break;
		case T_DICT:
			{
				uint64_t offset = 0;
				uint64_t len = getLen(data, offset, (t & 0x0f));
				vector<uint64_t> keyIds;
				vector<uint64_t> valueIds;
				uint64_t id;

				for (size_t i = 0; i < len; i++) {
					data = is + objectOffset + offset + (i*objectBytes);
					id = read(data, objectBytes);
					keyIds.push_back(id);
					data = is + objectOffset + offset + (len * objectBytes) + (i*objectBytes);
					id = read(data, objectBytes);
					valueIds.push_back(id);
				}

				for (size_t i = 0; i < len; i++) {
					Property key;
					Property value;

					readObject(is, max, objectBytes, offsetTable, keyIds[i], key);
					readObject(is, max, objectBytes, offsetTable, valueIds[i], value);

					parent.add(key.getAscii(), value);
				}
			}

			break;
		default:
			break;
		}
	} else {
		// TODO: exception
	}
}

Property Property::get(const string& name) {
	std::map<string, Property>::const_iterator it = m_dict.find(name);

	if (it != m_dict.end()) {
		return it->second;
	}
	
	return Property(); // NULL
}

/*int main(int argc, char** argv) {

	ifstream plist("c:/Users/swiatek/projects/plist/ipad.plist", ios::in|ios::binary|ios::ate);

	if (plist.is_open()) {
		streampos size = plist.tellg();
		char* data;
		if (size > 0) {
			data = new char[size];

			plist.seekg(0, ios::beg);
			plist.read(data, size);
		}
		
		plist.close();

		if (size > 0) {
			Property root;
			bool res = Property::read((uint8_t*) data, size, root);
			cout << "result: " << res;

			delete[] data;
		}
	}

	return 0;
}*/
