#include "Control.h"

using namespace gs::airplay;
using namespace std;

void gs::airplay::initStatus(Status& status)  {
	status.playback = STOPPED;
	status.length = 0;
	status.position = 0;
	status.timeout = true;
}

const string& Control::lookupPlayback(Playback playback) {
	static const string playbackEvent[] = { "stopped", "playing", "paused", "loading"};
	return playbackEvent[playback];
}
