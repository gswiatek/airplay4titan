#ifndef _gs_airplay_TitanRemote_h_
#define _gs_airplay_TitanRemote_h_

#include "Control.h"
#include "mongoose.h"
#include <ctime>

namespace gs {
	namespace airplay {
		class TitanRemote : public Control {
		public:
			TitanRemote(const std::string& host = "127.0.0.1:80", int timeout = 15);
			virtual ~TitanRemote();

			virtual Status play(const std::string& url, uint32_t startPosition = 0, int timeoutInSeconds = 30);
			virtual Status pause();
			virtual Status resume();
			virtual Status stop();
			virtual Status getStatus();
			virtual Status seek(uint32_t position);

		private:
			typedef enum action {
				UNKNOWN = -1,
				PLAY,
				PAUSE,
				RESUME,
				STOP,
				SEEK,
				STATUS
			} Action;

			class Request {
			public:
				Request(Action action, const std::string& url, const std::string& host) : action(action), url(url), host(host), done(false) { initStatus();  }
				const std::string& getHost() const { return host; }
				const std::string& getUrl() const { return url;  }
				Action getAction() const { return action;  }
				bool isDone() { return done; }
				void setDone(bool val) { done = val; status.timeout = !done; }
				Status getStatus() { return status; }
				void setStatusFromData(char* data, std::size_t len);

			private:
				void initStatus() { airplay::initStatus(status); }

				Action action;
				const std::string& url;
				const std::string& host;
				bool done;
				Status status;
			};

			static int eventHandler(struct mg_connection* conn, enum mg_event event);
			static int connectHandler(struct mg_connection* conn);

			Status makeRequest(Action action, const std::string& url, int timeout);

			std::string host;
			uint32_t timeout;
			time_t lastRequestTime;
		};
	}
}

#endif
