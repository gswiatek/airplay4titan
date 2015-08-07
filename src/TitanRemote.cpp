#include "TitanRemote.h"
#include "Util.h"

#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using namespace gs::airplay;
using namespace gs::util;

TitanRemote::TitanRemote(const string& host, int timeout) : host(host), timeout(timeout), lastRequestTime(0) { 

}

TitanRemote::~TitanRemote() { }

int TitanRemote::eventHandler(struct mg_connection* conn, mg_event ev) {
	int result = MG_FALSE;
	Request* request = (Request*)conn->connection_param;

	switch (ev) {
	case MG_AUTH:
		result = MG_TRUE;
		break;

	case MG_CONNECT:
		result = connectHandler(conn);
		break;

	case MG_REPLY:
		if (request) {
			request->setDone(true);
			request->setStatusFromData(conn->content, conn->content_len);
		}

		break;

	default:
		break;
	}

	return result;
}

Status TitanRemote::makeRequest(Action action, const string& url, int timeout) {
	time_t currentTime = time(0);

	if (currentTime == lastRequestTime) {
#ifndef _WIN32
		usleep(200 * 1000);
#else
		Sleep(200);
#endif
		lastRequestTime = time(0);
	} else {
		lastRequestTime = currentTime;
	}

	Request request(action, url, host);

	mg_server* server = mg_create_server(this, &eventHandler);
	struct mg_connection *client = mg_connect(server, host.c_str());

	if (!client) {
		mg_destroy_server(&server);
		return request.getStatus();
	} else {
		client->connection_param = (void*) &request;
	}

	int seconds = 0;

	while (!request.isDone() && (seconds < timeout)) {
		++seconds;
		mg_poll_server(server, 1000);
	}

	mg_destroy_server(&server);
	return request.getStatus();
}

Status TitanRemote::play(const string& url, uint32_t startPosition, int timeout) {
	Status status = getStatus();

	int count = 0;

	while (count < 10 && status.playback != STOPPED) {
		count++;
		makeRequest(STOP, "", timeout);
		status = getStatus();
	}

	if (status.playback != STOPPED) {
		// TODO: error
	}

	size_t pos = url.find_last_of(' ');

	if (pos != string::npos && pos != (url.size() - 1)) {
		return makeRequest(PLAY, url.substr(pos + 1, url.size() - pos - 1), timeout);
	} else {
		return makeRequest(PLAY, url, timeout);
	}
}


Status TitanRemote::pause() {
	Status status = getStatus();

	if (status.playback == PLAYING) {
		return makeRequest(PAUSE, "", timeout);
	}
		
	return status;
}

Status TitanRemote::resume() {
	Status status = getStatus();

	if (status.playback == PAUSED) {
		return makeRequest(RESUME, "", timeout);
	}

	return status;
}

Status TitanRemote::stop() {
	Status status = getStatus();

	if (status.playback != STOPPED) {
		return makeRequest(STOP, "", timeout);
	}

	return status;
}

Status TitanRemote::seek(uint32_t position) {
	Status status = getStatus();

	if (status.playback != STOPPED) {
		return makeRequest(SEEK, "", timeout);
	}

	return status;
}

Status TitanRemote::getStatus() {
	return makeRequest(STATUS, "", timeout);
}

int TitanRemote::connectHandler(struct mg_connection* conn) {

	if (conn->status_code) {
		return MG_FALSE;
	}

	ostringstream os;

	TitanRemote* titanRemote = (TitanRemote *)conn->server_param;
	Request* request = (Request *)conn->connection_param;
	Action action = request->getAction();

	if (action == PLAY) {
		//titanRemote->lastMediaUrl = Util::urlEncode(request->getUrl());
		os << "GET /queryraw?videoplay&play&url=" << Util::urlEncode(request->getUrl());
	} else if (action == PAUSE) {
		//os << "GET /queryraw?videoplay&pause&url=" << titanRemote->lastMediaUrl;
		os << "GET /queryraw?sendrc&rcpause";
	} else if (action == RESUME) {
		//os << "GET /queryraw?videoplay&play&url=" << titanRemote->lastMediaUrl;
		os << "GET /queryraw?sendrc&rcplay";
	} else if (action == STATUS) {
		os << "GET /queryraw?videoplay&status";
	} else if (action == STOP) {
		//os << "GET /queryraw?videoplay&stop&url="; // +titanRemote->lastMediaUrl;
		os << "GET /queryraw?sendrc&rcstop";
	}

	string host = request->getHost();
	string::size_type pos = host.find_last_of(':');

	if (pos != string::npos) {
		host.substr(0, pos);
	}

	os << " HTTP/1.0\r\n";
	os << "Host: " << host << "\r\n";
	os << "\r\n";

	string data = os.str();
	mg_write(conn, data.c_str(), data.size());

	return MG_TRUE;
}

void TitanRemote::Request::setStatusFromData(char* data, std::size_t len) {
	string content;

	if (len > 0 && data) {

		content.reserve(len);

		for (unsigned int i = 0; i < len; i++) {
			content.push_back(*(data + i));
		}
	}

	if (!content.empty()) {
		if (action == STATUS) {
			int active = 0;
			int playing = 0;
			char skip;

			istringstream is(content);
			is >> active;
			is >> skip;
			is >> playing;
			is >> skip;
			is >> status.position;
			is >> skip;
			is >> status.length;

			if (!active) {
				status.playback = STOPPED;
			} else {
				status.playback = (playing ? PLAYING : PAUSED);

				if ((status.length == 0 && status.position == 0) && (status.playback == PLAYING || status.playback == PAUSED)) {
					status.playback = LOADING;
				}
			}
		} else if (action == PLAY) {
			if (content.empty() || content == "can not start playback") {
				status.playback = STOPPED;
			} else {
				status.playback = LOADING;
			}
		} else if (action == PAUSE) {
			status.playback = PAUSED;
		} else if (action == RESUME) {
			status.playback = PLAYING;
		}
		
	}
}
