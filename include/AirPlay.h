#ifndef _gs_airplay_AirPlay_h_
#define _gs_airplay_AirPlay_h_

#include "Control.h"
#include "mongoose.h"

#include <map>

namespace gs {
	namespace airplay {

		class Reverse {
		public:
			Reverse(const std::string& purpose, const std::string& sessionId) : purpose(purpose), sessionId(sessionId) { }
			~Reverse() { }

			const std::string& getPurpose() const {
				return purpose;
			}

			const std::string& getSessionId() const {
				return sessionId;
			}

		private:
			std::string purpose;
			std::string sessionId;
		};

		class AirPlay {
		public:
			AirPlay(Control& control, uint32_t playTimeout = 30);
			~AirPlay();

			void start(const std::string& port, const std::string& mac);
			

		private:
			static int eventHandler(struct mg_connection* conn, enum mg_event event);
			static int requestHandler(struct mg_connection* conn);
			static int handle(struct mg_connection* conn, const std::string& method, std::string& uri, const std::string& query, const std::map<std::string, std::string>& param);

			static bool getLocationAndPostionFromText(const std::string& data, std::string& location, float& pos);
			static bool getLocationAndPostionFromPList(const std::string& data, std::string& location, float& pos);
			static bool getDataForPlayback(struct mg_connection* conn, std::string& location, float& startPosition);

			static std::string getData(struct mg_connection* conn);
			void getPlaybackInfo(std::ostringstream& os, Status& status);

			Status playMedia(const std::string& url, uint32_t startPosition = 0, int timeoutInSeconds = 30);
			Status pauseMedia();
			Status resumeMedia();
			Status stopMedia();
			Status seekMedia(uint32_t position);
			Status getStatus();
			void notifyReverseConnection(Status& status);

			Control& control;
			uint32_t playTimeout;
			mg_server* server;
			uint16_t reverseConnections;
			std::string sessionId;
			std::string mediaSessionId;
			std::string mac;
			Status status; // last status
			Status stopped;
		};
	}
}

#endif
