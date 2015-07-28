#include "AirPlay.h"
#include "Util.h"
#include "Log.h"
#include "PropList.h"

#include <sstream>

using namespace std;
using namespace gs::airplay;
using namespace gs::util;

AirPlay::AirPlay(Control& control, uint32_t playTimeout) : control(control), playTimeout(playTimeout), server(0), reverseConnections(0) {
	initStatus(status);
	initStatus(stopped);
	stopped.timeout = false;
}

AirPlay::~AirPlay() {

}


int AirPlay::eventHandler(struct mg_connection* conn, enum mg_event ev) {
	AirPlay* airPlay = (AirPlay*) (conn->server_param);

	if (ev != MG_POLL && ev != MG_AUTH) {
		static const string socketEvent[] = { "POLL", "CONNECT", "AUTH", "REQUEST", "REPLY", "RECV", "CLOSE", "WS_HANDSHAKE", "WS_CONNECT", "ERROR" };
	}

	switch (ev) {
	case MG_AUTH:
		return MG_TRUE;
		break;

	case MG_REQUEST:
		if (conn->connection_param) { // reverse connection
			return MG_TRUE;
		}
		return requestHandler(conn);
		break;

	case MG_RECV:
		if (conn->connection_param) { // discard client data on reverse connection
			return conn->content_len;
		}

		return 0; // don't discard input data
		break;
	case MG_CLOSE:
		if (conn->connection_param) {
			Reverse* reverse = (Reverse*)(conn->connection_param);
			string session = reverse->getSessionId();
			delete reverse;
			--airPlay->reverseConnections;

			Log::getLogger()->log(Log::DEBUG, "SRV", "reverse conn closed: sessionId=" + session + ", revCon=" + Util::valueOf(airPlay->reverseConnections));
		}
		return MG_TRUE;
		break;

	case MG_POLL:
		return MG_FALSE;

		break;

	default:
		return MG_FALSE;

	}

	return MG_FALSE;
}

int AirPlay::requestHandler(struct mg_connection* conn) {
	string uri = conn->uri;
	string method = conn->request_method;

	string query;
	map<string, string> queryParam;

	if (conn->query_string) {
		query = conn->query_string;

		queryParam = Util::parseQuery(query);

		char buf[512];
		buf[0] = 0;
		Util::urlDecode(query.c_str(), query.size(), buf, 512);
		query = buf;
	}

	return handle(conn, method, uri, query, queryParam);
}

int AirPlay::handle(struct mg_connection* conn, const string& method, string& uri, const string& query, const map<string, string>& param) {
	static const string nl = "\r\n";
	static const string defaultContentType = "text/x-apple-plist+xml";
	string contentType = defaultContentType;


	AirPlay* airPlay = (AirPlay*)(conn->server_param);
	Status status;

	bool notFound = false;
	bool badRequest = false;
	bool logStatus = false;
	bool timeout = false;

	const char* sessionId = mg_get_header(conn, "X-Apple-Session-ID");

	if (sessionId) {
		airPlay->sessionId = string(sessionId);
	}
	else {
		airPlay->sessionId = "00000000-0000-0000-0000-000000000000";
	}

	ostringstream response;
	ostringstream os;

	Log::getLogger()->log(Log::DEBUG, "SRV", string(conn->remote_ip) + " --> " + method + " " + uri + (query.empty() ? "" : "?" + query) + ", sessionId=" + airPlay->sessionId);

	if (method == "POST" && uri == "/reverse") {

		if (airPlay->reverseConnections == 10) { // to many reverse connections
			response << "HTTP/1.1 503 Too many reverse connections" << nl;
			response << "Date: " << Util::getHttpDate() << nl;
			response << "Server: AppleTVEmu/1.0" << nl;
			response << "Content-Length: " << 0 << nl << nl;

			string resp = response.str();
			mg_write(conn, resp.c_str(), resp.size());

			return MG_TRUE;
		}

		response << "HTTP/1.1 101 Switching Protocols" << nl;
		response << "Date: " << Util::getHttpDate() << nl;
		response << "Upgrade: PTTH/1.0" << nl;
		response << "Connection: Upgrade" << nl;
		response << "Content-Length: 0" << nl;
		response << nl;

		++airPlay->reverseConnections;
		
		string xApplePurpose;
		const char* purpose = mg_get_header(conn, "X-Apple-Purpose");
		
		if (purpose) {
			xApplePurpose = string(purpose);
		}

		Reverse* rev = new Reverse(xApplePurpose, airPlay->sessionId);
		conn->connection_param = (void*) rev;

		Log::getLogger()->log(Log::DEBUG, "SRV", string(conn->remote_ip) + " <-- 101 Switching Protocols, revCon=" + Util::valueOf(airPlay->reverseConnections));

		string resp = response.str();
		mg_write(conn, resp.c_str(), resp.size());


		return MG_MORE; // don't close the connection
	} else if (method == "POST" && uri == "/play") {

		string location;
		float startPosition = 0.0; // TODO: start position

		if (getDataForPlayback(conn, location, startPosition)) {
			status = airPlay->playMedia(location, 0);
			timeout = status.timeout;
		} else {
			Log::getLogger()->log(Log::ERROR, "SRV", "location not set");
			badRequest = true;
		}
	} else if (method == "GET" && uri == "/playback-info") {
		status = airPlay->getStatus();
		airPlay->getPlaybackInfo(os, status);
		timeout = status.timeout;
		logStatus = true;
	} else if (method == "POST" && uri == "/stop") {
		status = airPlay->stopMedia();
		timeout = status.timeout;

	} else if (method == "GET" && uri == "/server-info") {
		os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
		os << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\"" << endl;
		os << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << endl;
		os << "<plist version=\"1.0\">" << endl;
		os << "<dict>" << endl;
		os << "<key>deviceid</key><string>" << airPlay->mac << "</string>" << endl;
		os << "<key>model</key><string>" << "AppleTV2,1" << "</string>" << endl;
		//os << "<key>features</key><integer>" << "119" << "</integer>" << endl;
		os << "<key>features</key><integer>" << "9" << "</integer>" << endl;
		os << "<key>srcvers</key><string>" << "101.28" << "</string>" << endl;

		os << "</dict>" << endl;
		os << "</plist>";

	} else if (method == "POST" && uri == "/rate") {
		int rate = Util::getInt(param, "value", 0);
		
		if (rate == 0) {
			status = airPlay->pauseMedia();
		} else {
			status = airPlay->resumeMedia();
		}

		timeout = status.timeout;
	} else if (method == "PUT" && uri == "/setProperty") {
		/*os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
		os << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\"" << endl;
		os << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << endl;
		os << "<plist version=\"1.0\">" << endl;
		os << "<dict>" << endl;
		os << "<key>errorCode</key><integer>" << "0" << "</integer>" << endl;
		os << "</dict>" << endl;
		os << "</plist>";*/
		notFound = true;
	} else if (method == "POST" && uri == "/scrub") {
		// TODO: seek
		notFound = true;
	} else if (method == "GET" && uri == "/scrub") {
		contentType = "text/parameters";

		status = airPlay->getStatus();
		timeout = status.timeout;
		os << "duration: " << status.length << endl;
		os << "position: " << status.position << endl;
		logStatus = true;
	} else {
		notFound = true;
	}

	response << "HTTP/1.1";

	if (badRequest) {
		Log::getLogger()->log(Log::DEBUG, "SRV", string(conn->remote_ip)  + " <-- 400 Bad Request");
		response << " 400 Bad Request" << nl;
	}
	else if (notFound) {
		Log::getLogger()->log(Log::DEBUG, "SRV", string(conn->remote_ip) + " <-- 404 Not Found");
		response << " 404 Not Found" << nl;
	} else if (timeout) {
		Log::getLogger()->log(Log::DEBUG, "SRV", string(conn->remote_ip) + " <-- 503 Time-Out");
		response << " 503 Time-out" << nl;
	} else {
		ostringstream logEntry;

		logEntry << string(conn->remote_ip) << " <-- 200 OK";

		if (logStatus) {
			Status status = airPlay->getStatus();
			logEntry << " (" << status.position << "/" << status.length << ": " << Control::lookupPlayback(status.playback) << ")";
		}

		Log::getLogger()->log(Log::DEBUG, "SRV", logEntry.str());
		response << " 200 OK" << nl;
	}

	string data = os.str();

	response << "Date: " << Util::getHttpDate() << nl;
	response << "Server: AppleTVEmu/1.0" << nl;
	response << "Content-Length: " << data.size() << nl;

	if (!data.empty()) {
		response << "Content-Type: " << contentType << endl;
	}

	response << nl;

	string headers = response.str();

	mg_write(conn, headers.c_str(), headers.size());

	if (!data.empty()) {
		mg_write(conn, data.c_str(), data.size());
	}

	return MG_TRUE;
}

Status AirPlay::playMedia(const string& url, uint32_t startPosition, int timeoutInSeconds) {
	Log::getLogger()->log(Log::DEBUG, "SRV", "playMedia: " + url);
	mediaSessionId = sessionId;
	Status st = control.play(url, startPosition, timeoutInSeconds);
	notifyReverseConnection(st);
	return st;
}

Status AirPlay::pauseMedia() {
	Status st = stopped;

	if (sessionId == mediaSessionId) {
		st = control.pause();
	}

	notifyReverseConnection(st);

	return st;
}

Status AirPlay::resumeMedia() {
	Status st = stopped;

	if (sessionId == mediaSessionId) {
		st = control.resume();
	}

	notifyReverseConnection(st);

	return st;
}

Status AirPlay::stopMedia() {
	Status st = stopped;

	if (sessionId == mediaSessionId) {
		st = control.stop();
	}

	notifyReverseConnection(st);
	
	return st;
}

Status AirPlay::seekMedia(uint32_t position) {
	Status st = stopped;

	if (sessionId == mediaSessionId) {
		st = control.seek(position);
	}

	notifyReverseConnection(st);
	
	return st;
}


Status AirPlay::getStatus() {
	Status st = stopped;

	if (sessionId == mediaSessionId) {
		st = control.getStatus();
	}

	notifyReverseConnection(st);
	
	return st;
}

void AirPlay::start(const string& port, const string& mac) {
	Log::getLogger()->log(Log::DEBUG, "SRV", "start");
	this->mac = mac;

	server = mg_create_server((void *) this, &eventHandler);

	if (!server) {
		Log::getLogger()->log(Log::ERROR, "SRV", "mg_create_server failed");
		return;
	}

	mg_set_option(server, "listening_port", port.c_str());

	for (;;) {
		mg_poll_server(server, 1000);
	}

	mg_destroy_server(&server);
	server = 0;
}

void AirPlay::getPlaybackInfo(ostringstream& os, Status& status) {

	if (status.playback == STOPPED) { // no playing file set
		os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
		os << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\"" << endl;
		os << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << endl;
		os << "<plist version=\"1.0\">" << endl;
		os << "<dict/>" << endl;
		os << "</plist>";

		return;
	}

	if (status.playback == LOADING) {
		os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
		os << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\"" << endl;
		os << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << endl;
		os << "<plist version=\"1.0\">" << endl;
		os << "<dict>" << endl;
		os << "<key>readyToPlay</key>" << endl;
		os << "<false/>" << endl;
		os << "</dict>" << endl;
		os << "</plist>";

		return;
	}

	int rate = 0;

	if (status.playback == PLAYING) {
		rate = 1;
	}

	os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	os << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\"" << endl;
	os << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << endl;
	os << "<plist version=\"1.0\">" << endl;
	os << "<dict>" << endl;
	os << "<key>duration</key><real>" << status.length << "</real>" << endl;
	os << "<key>loadedTimeRanges</key>";
	os << "<array><dict><key>duration</key><real>" << status.position << "</real><key>start</key><real>0.0</real></dict></array>" << endl;
	os << "<key>playbackBufferEmpty</key><true/>" << endl;
	os << "<key>playbackBufferFull</key><false/>" << endl;
	os << "<key>playbackLikelyToKeepUp</key><true/>" << endl;
	os << "<key>position</key><real>" << status.position << "</real>" << endl;
	os << "<key>rate</key><real>" << rate << "</real>" << endl;
	os << "<key>readyToPlay</key><true/>" << endl;
	os << "<key>seekableTimeRanges</key>" << endl;
	os << "<array><dict><key>duration</key><real>" << status.length << "</real><key>start</key><real>0.0</real></dict></array>" << endl;

	os << "</dict>" << endl;
	os << "</plist>";
}

bool AirPlay::getDataForPlayback(struct mg_connection* conn, string& location, float& startPosition) {
	const char* ct = mg_get_header(conn, "Content-Type");

	if (!conn->content) {
		return false;
	}

	string content = getData(conn);

	if (ct != 0) {
		string contentType(ct);

		if (contentType == "text/parameters") {
			if (getLocationAndPostionFromText(content, location, startPosition)) {
				return true;
			}
		}
		else if (contentType == "application/x-apple-binary-plist") {
			Property dict;

			if (Property::read((const uint8_t *)content.c_str(), content.length(), dict)) {
				if (dict.getType() == Property::T_DICT) {
					Property p = dict.get("Content-Location");
					location = p.getAscii();

					return !location.empty();
				}
			}
		}
	}

	if (getLocationAndPostionFromText(content, location, startPosition)) {
		return true;
	}

	if (getLocationAndPostionFromPList(content, location, startPosition)) {
		return true;
	}

	return false;
}


bool AirPlay::getLocationAndPostionFromText(const string& data, string& location, float& pos) {

	static const string cl("Content-Location: ");
	static const string sp("Start-Position: ");

	pos = 0.0f;

	string::size_type clPos = data.find(cl);

	if (clPos != string::npos) {
		char c;

		for (string::size_type i = clPos + cl.length(); i < data.length(); i++) {
			c = data[i];

			if (c != '\r' && c != '\n') {
				location.push_back(c);
			}
			else {
				break;
			}
		}


		Util::trim(location);

		return true;
	}
	else {
		return false;
	}
}


bool AirPlay::getLocationAndPostionFromPList(const string& data, string& location, float& pos) {

	static const string cl("<key>Content-Location</key>");
	static const string sp("<key>Start-Position</key>");

	bool res = false;

	string::size_type clPos = data.find(cl);

	if (clPos != string::npos) {
		string::size_type pos1 = data.find("<string>", clPos);

		if (pos1 != string::npos) {
			string::size_type pos2 = data.find("</string>", pos1);

			if (pos2 != string::npos) {
				location = data.substr(pos1 + 8, pos2 - pos1 - 7);
				res = true;
			}
		}

	}

	if (res) {
		Util::trim(location);
	}

	return res;
}

string AirPlay::getData(struct mg_connection* conn) {
	string content;

	if (conn->content_len > 0 && conn->content) {

		content.reserve(conn->content_len);

		for (unsigned int i = 0; i < conn->content_len; i++) {
			content.push_back(*(conn->content + i));
		}
	}

	return content;
}


void AirPlay::notifyReverseConnection(Status& status) {
	if (reverseConnections > 0 && this->status.playback != status.playback && !status.timeout) {
		struct mg_connection* c = mg_next(server, 0);

		while (c) {
			if (c->connection_param) {
				Reverse* reverse = (Reverse*)c->connection_param;

				if (reverse->getSessionId() != sessionId) {
					c = mg_next(server, c);
					continue;
				}

				const string& playbackEvent = Control::lookupPlayback(status.playback);

				Log::getLogger()->log(Log::DEBUG, "SRV", string(c->remote_ip) +  " <-- POST /event: " + playbackEvent);

				ostringstream os;
				os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
				os << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\"" << endl;
				os << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << endl;
				os << "<plist version=\"1.0\">" << endl;
				os << "<dict>" << endl;
				os << "<key>category</key>" << endl;
				os << "<string>video</string>" << endl;
				os << "<key>sessionID</key>" << endl;
				os << "<integer>15</integer>" << endl;
				os << "<key>state</key>" << endl;
				os << "<string>" << playbackEvent << "</string>" << endl;
				os << "</dict>" << endl;
				os << "</plist>";

				string data = os.str();

				ostringstream hdr;
				hdr << "POST /event HTTP/1.1\r\n";
				hdr << "X-Apple-Session-ID: " << reverse->getSessionId() << "\r\n";
				hdr << "Content-Type: text/x-apple-plist+xml" << "\r\n";
				hdr << "Content-Length: " << data.size() << "\r\n\r\n";
				hdr << data;

				string resp = hdr.str();

				mg_write(c, resp.c_str(), resp.size());
			}

			c = mg_next(server, c);
		}
	}

	if (sessionId == mediaSessionId) {
		this->status = status;
	}
}
