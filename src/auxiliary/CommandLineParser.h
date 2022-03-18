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
        parseOk,
        parseError,
        parseMissingArg,
        parseRunModeError,
        parseInputFileError,
        parseOutputPathError,
        parseMeasureTypeError,
        parseVersionRequested,
        parseHelpRequested
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
