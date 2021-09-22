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
        CommandLineInputFileError,
        CommandLineOutputPathError
    };
    Q_ENUM(CommandLineParseResult)

    CommandLineParseResult parseCommandLine( const QCoreApplication& app);

    QString getInputFilename() const {return m_inputFilename;}
    QString getOutputFilename() const {return m_outputFilename;}
    QString getRunMode() const {return m_runMode;}
    bool getVerbose() const {return m_verbose;}

private:
    QCommandLineParser m_parser;
    QString m_inputFilename;
    QString m_outputFilename;
    QString m_runMode;
    bool m_verbose;
};

#endif // COMMANDLINEPARSER_H
