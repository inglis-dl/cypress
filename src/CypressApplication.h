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
        CommandLineTestTypeError,
        CommandLineInputFileError,
        CommandLineOutputPathError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
    };
    Q_ENUM(CommandLineParseResult)

    enum TestType {
        None,
        Spirometry,
        Weight,
        BodyComposition,
        Frax,
        CDTT,
        BloodPressure,
        Temperature,
        Hearing,
        ChoiceReaction
    };
    Q_ENUM(TestType)

    static QMap<QString,TestType> initTestTypeLUT();

    CommandLineParseResult parse(const QCoreApplication &, QString *);
    QString helpText(){ return m_parser.helpText(); }

    // This method internally calls readInput
    //
    void initialize();

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
    void finish();

private:
    void readInput();

    static QMap<QString,TestType> testTypeLUT;

    QCommandLineParser m_parser;

    QString m_inputFileName;
    QString m_outputFileName;
    QString m_mode;
    bool m_verbose;
    TestType m_testType;
    QString m_testTypeName;

    QMap<QString,QVariant> m_inputData;
    QMap<QString,QVariant> m_outputData;

    ManagerBase *m_manager;

    CypressDialog *m_dialog;

   //QStandardItemModel m_model;
};

#endif // CYPRESSAPPLICATION_H
