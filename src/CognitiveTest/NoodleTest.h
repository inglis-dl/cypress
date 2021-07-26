#pragma once
#include "InputsModel.h"

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>

static class NoodleTest
{
public:
	static void Run(InputsModel* inputs);
	static void MoveResultsFile(QString path);
};

