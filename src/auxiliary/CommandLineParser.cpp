#include "CommandLineParser.h"

#include <QCommandLineOption>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

CommandLineParser::CommandLineParser(QObject* parent) : QObject(parent),
    m_runMode("test")
{
}

CommandLineParser::CommandLineParseResult CommandLineParser::parseCommandLine( const QCoreApplication& app)
{
    // process command line args
    // if the run mode arg is 'test' and no UI is required
    // do not open the window, just write dummy output data to json
    //
    m_parser.addHelpOption();

    // expect a full path to an input.json file
    // which contains the minimum necessary input data to
    // run the device (eg., barcode, gender etc.)

    QCommandLineOption inputOption(
      QStringList() << "i" << "input",
      QCoreApplication::translate(
        "main", "Read json input from <file>"),"file");
    m_parser.addOption(inputOption);
    QCommandLineOption outputOption(
      QStringList() << "o" << "output",
      QCoreApplication::translate(
        "main", "Write json output to <file>"),"file");
    m_parser.addOption(outputOption);
    QCommandLineOption modeOption(
      QStringList() << "m" << "mode",
      QCoreApplication::translate(
        "main", "Run mode <production,test>"),"runMode","test");
    m_parser.addOption(modeOption);
    QCommandLineOption verboseOption(
       QStringList() << "v" << "verbose",
       QCoreApplication::translate(
         "main","Verbose mode. Prints out verbose debug information."));
    m_parser.addOption(verboseOption);
    m_parser.process(app);

    // Default when not run from command line we assume verbose is desired
    //
    m_verbose = true;
    if(1<QCoreApplication::arguments().size())
    {
      qDebug() << " not empty app args: " << QString::number(QCoreApplication::arguments().size());
      m_verbose = m_parser.isSet(verboseOption);
    }

    // Catch the first parser error
    //
    CommandLineParseResult result = CommandLineOk;

    if(m_parser.isSet(inputOption))
    {
        m_inputFilename = m_parser.value(inputOption);
        if(m_verbose)
          qDebug() << "in file option set with " << m_inputFilename;
        QFileInfo info(m_inputFilename);
        if(!info.exists())
        {
          result = CommandLineInputFileError;
        }
    }
    if(m_parser.isSet(outputOption) && CommandLineOk==result)
    {
        m_outputFilename = m_parser.value(outputOption);
        if(m_verbose)
          qDebug() << "out file option set with " << m_outputFilename;
        QFileInfo info(m_outputFilename);
        if(!info.dir().exists())
        {
          result = CommandLineOutputPathError;
        }
    }
    if(m_parser.isSet(modeOption) && CommandLineOk==result)
    {
        m_runMode = m_parser.value(modeOption).toLower();
        if(m_verbose)
          qDebug() << "mode option set with " << m_runMode;
        if( !(m_parser.isSet(inputOption) &&
              m_parser.isSet(outputOption)))
        {
          result = CommandLineMissingArg;
        }
    }

    return result;
}
