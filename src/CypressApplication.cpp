#include "CypressApplication.h"

#include <QCommandLineOption>
#include <QDebug>
#include <QDir>
#include <QDate>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QMetaMethod>
#include <QSettings>
#include <QStandardItemModel>
#include <QFormBuilder>
#include <QVBoxLayout>

#include "./dialogs/AudiometerDialog.h"
#include "./dialogs/BodyCompositionDialog.h"
#include "./dialogs/CDTTDialog.h"
#include "./dialogs/ChoiceReactionDialog.h"
#include "./dialogs/FraxDialog.h"
#include "./dialogs/ThermometerDialog.h"
#include "./dialogs/TonometerDialog.h"
#include "./dialogs/WeighScaleDialog.h"

#include "./managers/ManagerBase.h"
#include "./managers/AudiometerManager.h"
#include "./managers/BluetoothLEManager.h"
#include "./managers/CDTTManager.h"
#include "./managers/ChoiceReactionManager.h"
#include "./managers/FraxManager.h"
#include "./managers/BodyCompositionAnalyzerManager.h"
#include "./managers/TonometerManager.h"
#include "./managers/WeighScaleManager.h"

QMap<QString,CypressApplication::TestType> CypressApplication::testTypeLUT =
        CypressApplication::initTestTypeLUT();

CypressApplication::CypressApplication(QObject *parent) : QObject(parent),
    m_mode("default"),
    m_verbose(true),
    m_testType(None),
    m_manager(Q_NULLPTR)
{
    // TESTING ONLY
    m_testType = TestType::Hearing;
    m_testTypeName = "hearing";
}

CypressApplication::~CypressApplication()
{
    delete m_dialog;
    delete m_manager;
}

QMap<QString,CypressApplication::TestType> CypressApplication::initTestTypeLUT()
{
    QMap<QString,CypressApplication::TestType> lut;
    lut["weight"] = TestType::Weight;
    lut["hearing"] = TestType::Hearing;
    lut["spirometry"] = TestType::Spirometry;
    lut["temperature"] = TestType::Temperature;
    lut["frax"] = TestType::Frax;
    lut["body_composition"] = TestType::BodyComposition;
    lut["cdtt"] = TestType::CDTT;
    lut["choice_reaction"] = TestType::ChoiceReaction;
    lut["blood_pressure"] = TestType::BloodPressure;
    lut["tonometry"] = TestType::Tonometry;
    return lut;
}

void CypressApplication::initialize()
{
    // select the appropriate manager based on test type
    // if the test type was specified as a cli then it will not be None
    // if it is, then it must be specified in the input data read from the input file
    //
    switch(m_testType)
    {
      case TestType::Weight:
        m_manager = new WeighScaleManager(this);
        m_dialog = new WeighScaleDialog();
        break;
      case TestType::BodyComposition:
        m_manager = new BodyCompositionAnalyzerManager(this);
        m_dialog = new BodyCompositionDialog();
        break;
      case TestType::Hearing:
        m_manager = new AudiometerManager(this);
        m_dialog = new AudiometerDialog();
        break;
      case TestType::ChoiceReaction:
        m_manager = new ChoiceReactionManager(this);
        m_dialog = new ChoiceReactionDialog();
        break;
      case TestType::Temperature:
        m_manager = new BluetoothLEManager(this);
        m_dialog = new ThermometerDialog();
        break;
      case TestType::Frax:
        m_manager = new FraxManager(this);
        m_dialog = new FraxDialog();
        break;
      case TestType::CDTT:
        m_manager = new CDTTManager(this);
        m_dialog = new CDTTDialog();
        break;
      case TestType::Tonometry:
        m_manager = new TonometerManager(this);
        m_dialog = new TonometerDialog();
        break;
      case TestType::Spirometry:
        m_manager = Q_NULLPTR;
        break;
      case TestType::BloodPressure:
        m_manager = Q_NULLPTR;
        break;
      case TestType::None:
        m_manager = Q_NULLPTR;
        break;
    }
    if(Q_NULLPTR == m_manager)
        throw std::runtime_error("FATAL ERROR: failed to initialize a manager");
    if(Q_NULLPTR == m_dialog)
        throw std::runtime_error("FATAL ERROR: failed to initialize a dialog");

    qDebug() << "created manager of class type " << m_manager->metaObject()->className();

    m_dialog->setInputFileName(m_inputFileName);
    m_dialog->setOutputFileName(m_outputFileName);
    m_dialog->setMode(m_mode);
    m_dialog->setVerbose(m_verbose);

    m_dialog->show();
    m_dialog->initialize();
}

CypressApplication::CommandLineParseResult CypressApplication::parse(const QCoreApplication &app,
                                                                     QString *errMessage)
{
    // process command line args
    // if the run mode arg is 'test' and no UI is required
    // do not open the window, just write dummy output data to json
    //

    const QCommandLineOption helpOption = m_parser.addHelpOption();
    const QCommandLineOption versionOption = m_parser.addVersionOption();

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

    QCommandLineOption testOption(
      QStringList() << "t" << "test",
      QCoreApplication::translate(
        "main", "Test <type>"),"test","type");
    m_parser.addOption(testOption);

    QCommandLineOption modeOption(
      QStringList() << "m" << "mode",
      QCoreApplication::translate(
        "main", "Run mode <default,live,simulate>"),"mode","default");
    m_parser.addOption(modeOption);

    QCommandLineOption verboseOption(
       QStringList() << "d" << "verbose",
       QCoreApplication::translate(
         "main","Verbose mode. Prints out verbose debug information."));
    m_parser.addOption(verboseOption);

    m_parser.process(app);

    if (m_parser.isSet(versionOption))
        return CommandLineVersionRequested;

    if (m_parser.isSet(helpOption))
        return CommandLineHelpRequested;

    // Default when not run from command line we assume verbose is desired
    //
    m_verbose = true;
    if(1<QCoreApplication::arguments().size())
    {
      qDebug() << " not empty app args: " << QString::number(QCoreApplication::arguments().size());
      m_verbose = m_parser.isSet(verboseOption);
      qDebug() << (m_verbose ? "verbose option set" : "verbose option not set");
    }

    // Catch the first parser error
    //
    CommandLineParseResult result = CommandLineOk;

    // Default mode is "default"
    // - simulate runs the app with GUI interaction but without
    //   connecting to physical hardware.  A default instrument response
    //   (eg., valid hearing test results etc.) is provided
    // - live runs the app with GUI interaction and expects valid input file
    //   and valid path for an output file to be written.  Connection
    //   to a device (hardware and / or software) is required
    // - default is similar to live but an input file and  output file path are
    //   not required.  A dummy barcode ID is used if no input file is present,
    //   and a default output file is written to the working directory in
    //   response to write / save requests.
    //
    if(m_parser.isSet(modeOption))
    {
        QString s = m_parser.value(modeOption).toLower();
        QStringList l;
        l << "default" << "simulate" << "live";
        if(l.contains(s))
        {
          m_mode = s;
          if(m_verbose)
            qDebug() << "mode option set with " << m_mode;
        }
        else
        {
          result = CommandLineModeError;
          *errMessage = "Invalid mode: " + s;
        }
    }

    bool hasValidInput = false;
    bool hasValidOutput = false;

    if(m_parser.isSet(inputOption) && CommandLineOk==result)
    {
        QString s = m_parser.value(inputOption);
        QFileInfo info(s);
        if(info.exists(s))
        {
            hasValidInput = true;
            m_inputFileName = s;
            if(m_verbose)
              qDebug() << "in file option set with " << m_inputFileName;
        }
        else
        {
            qDebug() << "ERROR: input file does not exist: " <<  s;
            result = CommandLineInputFileError;
            *errMessage = "Invalid input file " + s;
        }
    }

    // if command line parsing determined an error do not continue
    // and report on the next potential error
    //
    if(m_parser.isSet(outputOption) && CommandLineOk==result)
    {
        QString s = m_parser.value(outputOption);
        QFileInfo info(s);
        if(info.dir().exists())
        {
            hasValidOutput = true;
            m_outputFileName = s;
            if(m_verbose)
              qDebug() << "out file option set with " << m_outputFileName;
        }
        else
        {
            qDebug() << "ERROR: output file path does not exist: " <<  m_outputFileName;
            result = CommandLineOutputPathError;
            *errMessage = "Invalid output file path " + s;
        }
    }

    if(m_parser.isSet(testOption) && CommandLineOk==result)
    {
        QString s = m_parser.value(testOption).toLower();
        qDebug() << "test option parsing " << s;
        // determine which manager and dialog is needed based on test type
        if(CypressApplication::testTypeLUT.contains(s))
        {
            m_testType = CypressApplication::testTypeLUT[s];
            m_testTypeName = s;
            if(m_verbose)
              qDebug() << "in test option set with " << s << m_testType;
        }
        else
        {
            qDebug() << "ERROR: input test does not exist: " <<  s;
            result = CommandLineTestTypeError;
            *errMessage = "Invalid input test " + s;
        }
    }
    else {qDebug() << "test option not set";}

    if(CommandLineOk == result)
    {
        if("live" == m_mode)
        {
            if(!(hasValidInput && hasValidOutput))
            {
               result = CommandLineError;
               *errMessage = "One or more expected arguments are missng";
            }
        }
    }

    qDebug() << "parser result: " << result;
    return result;
}
