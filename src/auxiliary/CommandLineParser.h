#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QVariant>

class CommandLineParser : QObject
{
    Q_OBJECT
public:
    explicit CommandLineParser(QObject* parent = Q_NULLPTR);

    enum CommandLineParseResult {
        CommandLineOk,
        CommandLineError,
        CommandLineMissingArg,
        CommandLineModeError,
        CommandLineInputFileError,
        CommandLineOutputPathError,
        CommandLineTypeError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
    };
    Q_ENUM(CommandLineParseResult)

    QString helpText(){ return m_parser.helpText(); }

    CommandLineParseResult parseCommandLine( const QCoreApplication &, QString *);

    QMap<QString,QVariant> getArgs(){ return m_args; }

private:
    QCommandLineParser m_parser;
    QMap<QString,QVariant> m_args;
};

#endif // COMMANDLINEPARSER_H
