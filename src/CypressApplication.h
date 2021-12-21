#ifndef CYPRESSAPPLICATION_H
#define CYPRESSAPPLICATION_H

#include <QObject>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QMap>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(ManagerBase);
QT_FORWARD_DECLARE_CLASS(CypressDialog);

class CypressApplication : public QObject
{
    Q_OBJECT
public:
    explicit CypressApplication(QObject *parent = nullptr);
    ~CypressApplication();

    enum CommandLineParseResult {
        CommandLineOk,
        CommandLineError,
        CommandLineMissingArg,
        CommandLineModeError,
        CommandLineTestError,
        CommandLineInputFileError,
        CommandLineOutputPathError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
    };
    Q_ENUM(CommandLineParseResult)

    CommandLineParseResult parse(const QCoreApplication &, QString *);
    QString helpText(){ return m_parser.helpText(); }

    // This method internally calls readInput
    //
    void initialize(ManagerBase *ptr){};

    // Call after initialize, launch the application and run
    // the device
    //
    void run(){};

    void show();

    void setInputFileName(const QString& name) { m_inputFileName = name; }
    QString inputFileName() { return m_inputFileName; }

    void setOutputFileName(const QString& name) { m_outputFileName = name; }
    QString outputFileName() { return m_outputFileName; }

    void setMode(const QString& mode) { m_mode = mode.toLower(); }
    QString mode() { return m_mode; }

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose(){ return m_verbose; }

signals:

public slots:
    void writeOutput();

private:
    void readInput();

   QCommandLineParser m_parser;

   QString m_inputFileName;
   QString m_outputFileName;
   QString m_mode;
   bool m_verbose;
   QString m_testName;

   QMap<QString,QVariant> m_inputData;
   QMap<QString,QVariant> m_outputData;

   ManagerBase *m_manager;

   CypressDialog *m_dialog;

   //QStandardItemModel m_model;
};

#endif // CYPRESSAPPLICATION_H
