#ifndef CYPRESSAPPLICATION_H
#define CYPRESSAPPLICATION_H

#include <QObject>
#include <QCoreApplication>
#include <QCommandLineParser>

QT_FORWARD_DECLARE_CLASS(DialogBase)

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

    CommandLineParseResult parse(const QCoreApplication &, QString *);
    QString helpText(){ return m_parser.helpText(); }

    void initialize();

private:
    QCommandLineParser m_parser;
    QString m_testTypeName;
    QString m_inputFileName;
    QString m_outputFileName;

    QString m_mode;
    bool m_verbose;

    QScopedPointer<DialogBase> m_dialog;
};

#endif // CYPRESSAPPLICATION_H
