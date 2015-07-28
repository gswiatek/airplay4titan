#ifndef _gs_airplay_Control_h_
#define _gs_airplay_Control_h_

#include <cstdint>
#include <string>

namespace gs {
	namespace airplay {
		typedef enum {
			STOPPED = 0,
			PLAYING,
			PAUSED,
			LOADING
		} Playback;


		typedef struct {
			Playback playback;
			uint32_t length;
			uint32_t position;
			bool timeout;
		} Status;

		void initStatus(Status& status);

		class Control {
		protected:
			virtual ~Control() {}
		public:
			
			virtual Status play(const std::string& url, uint32_t startPosition = 0, int timeoutInSeconds = 30) = 0;
			virtual Status pause() = 0;
			virtual Status resume() = 0;
			virtual Status stop() = 0;
			virtual Status seek(uint32_t position) = 0;
			virtual Status getStatus() = 0;

			static const std::string& lookupPlayback(Playback playback);
		};
	}
}

#endif
