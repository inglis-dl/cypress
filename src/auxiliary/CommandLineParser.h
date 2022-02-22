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

    enum ParseResult {
        Ok,
        Error,
        MissingArg,
        RunModeError,
        InputFileError,
        OutputPathError,
        MeasureTypeError,
        VersionRequested,
        HelpRequested
    };
    Q_ENUM(ParseResult)

    QString helpText(){ return m_parser.helpText(); }

    ParseResult parseCommandLine( const QCoreApplication&, QString*);

    QMap<QString,QVariant> getArgs(){ return m_args; }

private:
    QCommandLineParser m_parser;
    QMap<QString,QVariant> m_args;
};

#endif // COMMANDLINEPARSER_H
