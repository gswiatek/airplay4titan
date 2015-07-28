#include "TitanRemote.h"
//#include "VlcControl.h"
#include "AirPlay.h"
#include "Log.h"
#include <iostream>
#include <cerrno>
#include <cstdlib>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

using namespace std;
using namespace gs::airplay;
using namespace gs::util;


static string mac("01:02:03:04:05:06"); // MAC used for Bonjour
const int airplayPort = 7000;

void startAsDaemon() {
#ifndef _WIN32
	if (getppid() == 1) { // already daemon
		return;
	}

	pid_t p = fork();

	if (p < 0) {
		cerr << "fork failed: " << errno << endl;
		exit(errno);
	}
	else if (p > 0) { // parent process
		exit(0);
	}

	// child

	pid_t sid = setsid();

	if (sid < 0) {
		cerr << "setsid failed: " << errno << endl;
		exit(errno);
	}

	chdir("/");
	umask(0);

	freopen("/dev/null", "r", stdin);
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);
#endif
}

void readMac() {
#ifndef _WIN32
	ifstream in;

	in.open("/sys/class/net/eth0/address");

	if (in.is_open()) {
		if (in.good()) {
			in >> mac;
		}

		in.close();
	}
#endif
}


int main(int argc, char** argv) {
	startAsDaemon();
	readMac();

	Log log("/tmp/airplay.log", 200 * 1024);
	Log::setLogger(&log);

	TitanRemote control("127.0.0.1:80");
	//VlcControl control;
	AirPlay airPlay(control);
	airPlay.start("7000", mac);

	return 0;
}
