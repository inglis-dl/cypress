#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QDir>
#include <QFile>

class CognitiveTestManager: public QObject
{
public:
	
	//void Run(InputsModel* inputs);

	/// <summary>
	/// Launch the cognitive test
	/// </summary>
	bool LaunchTest(QString CCBFolderPath, QString userId, QString dcsSiteName, QString interviewerId, QString language);
	
	/// <summary>
	/// Move the results file to the proper output path
	/// </summary>
	/// <param name="path"></param>
	bool MoveResultsFile(QString ccbFolderPath, QString outputPath);


	void setVerbose(const bool& verbose) { m_verbose = verbose; }
	bool isVerbose() const { return m_verbose; }

	void setMode(const QString& mode) { m_mode = mode; }

private:
	bool m_verbose;

	// mode of operation
	// - "simulate" - no devices are connected and the manager
	// responds to the UI signals and slots as though in live mode with valid
	// device and test data
	// - "live" - production mode
	//
	QString m_mode;
};

