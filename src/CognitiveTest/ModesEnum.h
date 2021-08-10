#pragma once
enum Modes {
	unselected = -1,
	live = 0,
	ghost = 1,
	debug = 2,
	unknown = 3
};

class ModesHelper{
public:
	static int DetermineMode(QString modeStr) {
		QString lowerModeStr = modeStr.toLower();
		if (lowerModeStr == "live") {
			return Modes::live;
		}
		else if (lowerModeStr == "ghost") {
			return Modes::ghost;
		}
		else if (lowerModeStr == "debug") {
			return Modes::debug;
		}
		else {
			return Modes::unknown;
		}
	}
};
