#ifndef CYPRESSAPPLICATION_H
#define CYPRESSAPPLICATION_H

#include <QObject>
#include <QCoreApplication>
#include <QCommandLineParser>

QT_FORWARD_DECLARE_CLASS(ManagerBase)
QT_FORWARD_DECLARE_CLASS(CypressDialog)

class CypressApplication : public QObject
{
    Q_OBJECT
public:
    explicit CypressApplication(QObject *parent = Q_NULLPTR);
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
        ChoiceReaction,
        Tonometry
    };
    Q_ENUM(TestType)

    static QMap<QString,TestType> initTestTypeLUT();

    CommandLineParseResult parse(const QCoreApplication &, QString *);
    QString helpText(){ return m_parser.helpText(); }

    void initialize();

    TestType getTestType() const { return m_testType; }

private:
    static QMap<QString,TestType> testTypeLUT;
    QCommandLineParser m_parser;
    QString m_testTypeName;
    QString m_inputFileName;
    QString m_outputFileName;

    QString m_mode;
    bool m_verbose;
    TestType m_testType;


    ManagerBase *m_manager;
    CypressDialog *m_dialog;
};

#endif // CYPRESSAPPLICATION_H
