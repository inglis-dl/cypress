#pragma once
#include <QString>

class Strings {
public:
	// Languages accepted by CCB.exe
    static const QString English;
	static const QString French;

	// Command line values accepted as a Language
	static const QString cmdInput_English;
	static const QString cmdInput_French;

	// Json arguments
	static const QString jsonArg_userID;
	static const QString jsonArg_siteName;
	static const QString jsonArg_interviewerID;
	static const QString jsonArg_language;
};