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

	// Command line flags
	static const QString cmdFlag_test;
	static const QString cmdFlag_fullTest;

	// Json arguments
	static const QString jsonArg_ccbFolderPath;
};