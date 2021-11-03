#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QString>

class CommandLineParser : QObject
{
    Q_OBJECT
public:
    explicit CommandLineParser(QObject* parent = nullptr);

    enum CommandLineParseResult {
        CommandLineOk,
        CommandLineError,
        CommandLineMissingArg,
        CommandLineModeError,
        CommandLineInputFileError,
        CommandLineOutputPathError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
    };
    Q_ENUM(CommandLineParseResult)

    QString helpText(){ return m_parser.helpText(); }

    CommandLineParseResult parseCommandLine( const QCoreApplication &, QString *);

    QString getInputFilename() const {return m_inputFilename;}
    QString getOutputFilename() const {return m_outputFilename;}
    QString getMode() const {return m_mode;}
    bool getVerbose() const {return m_verbose;}

private:
    QCommandLineParser m_parser;
    QString m_inputFilename;
    QString m_outputFilename;
    QString m_mode;
    bool m_verbose;
};

#endif // COMMANDLINEPARSER_H
