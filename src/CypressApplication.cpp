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

#include "./dialogs/CypressDialog.h"
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
    m_manager(nullptr)
{
    m_dialog = new CypressDialog();
    m_model = new QStandardItemModel(this);

    // TESTING ONLY
    m_testType = TestType::Hearing;
    m_testTypeName = "hearing";
}

CypressApplication::~CypressApplication()
{
    delete m_dialog;
    delete m_model;
    if(nullptr!=m_manager)
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
    if(TestType::None==m_testType)
    {
        QString s;
        if(m_inputData.contains("test_type"))
        {
            s = m_inputData["test_type"].toString();
        }
        if(CypressApplication::testTypeLUT.contains(s))
        {
            m_testType = CypressApplication::testTypeLUT[s];
            m_testTypeName = s;
            if(m_verbose)
              qDebug() << "test option set from input file with " << s;
        }
        else
        {
            throw std::runtime_error("FATAL ERROR: no test type specified");
        }
    }
    int n_col = 1;
    int n_row = 1;
    QStringList titles;
    switch(m_testType)
    {
      case TestType::Weight:
        qDebug() << "creating weigh scale manager";
        m_manager = new WeighScaleManager(this);
        titles << "Weight Results";
        n_row = 2;
        break;
      case TestType::BodyComposition:
        qDebug() << "creating boyd composition manager";
        m_manager = new BodyCompositionAnalyzerManager(this);
        titles << "Body Composition Results";
        n_row = 8;
        break;
      case TestType::Hearing:
        qDebug() << "creating audiometer manager";
        m_manager = new AudiometerManager(this);
        titles << "Left Test Results" << "Right Test Results";
        n_col = 2;
        n_row = 8;
        break;
      case TestType::ChoiceReaction:
        qDebug() << "creating coginitive test manager";
        m_manager = new ChoiceReactionManager(this);
        titles << "Left Test Results" << "Right Test Results";
        n_col = 2;
        n_row = 8;
        break;
      case TestType::Temperature:
        qDebug() << "creating BLE manager";
        m_manager = new BluetoothLEManager(this);
        titles << "Temperature Results";
        n_row = 2;
        break;
      case TestType::Frax:
        qDebug() << "creating frax manager";
        m_manager = new FraxManager(this);
        titles << "10 Year Fracture Risk Probabilities";
        n_row = 4;
        break;
      case TestType::CDTT:
        qDebug() << "creating CDTT manager";
        m_manager = new CDTTManager(this);
        titles << "Left Test Results" << "Right Test Results";
        n_col = 2;
        n_row = 8;
        break;
      case TestType::Tonometry:
        m_manager = new TonometerManager(this);
        break;
      case TestType::Spirometry:
        m_manager = nullptr;
        break;
      case TestType::BloodPressure:
        m_manager = nullptr;
        break;
      case TestType::None:
        m_manager = nullptr;
        break;
    }
    if(nullptr==m_manager)
        throw std::runtime_error("FATAL ERROR: failed to initialize a manager");

    qDebug() << "created manager of class type " << m_manager->metaObject()->className();
//    for(int i=0;i<m_manager->metaObject()->methodCount();i++)
//        qDebug() << m_manager->metaObject()->method(i).name();

    m_manager->setVerbose(m_verbose);
    m_manager->setMode(m_mode);

    // Read the cypress.ini file
    // Each manager class must create/read/write to its own unique group
    // withing the cypress.ini file stored on the host
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("cypress.ini"), QSettings::IniFormat);
    m_manager->loadSettings(settings);

    qDebug() << "initializing the dialog to test type " << m_testType;
    m_dialog->initialize(this);

    connect(m_dialog, &QDialog::accepted, this, &CypressApplication::finish);

    // init the model
    //
    qDebug() << "initializing the model";
    for(int col=0;col<n_col;col++)
    {
      for(int row=0;row<n_row;row++)
      {
        QStandardItem* item = new QStandardItem();
        m_model->setItem(row,col,item);
      }
      if(!titles.empty())
        m_model->setHeaderData(col,Qt::Horizontal,titles.at(col),Qt::DisplayRole);
    }

    connect(m_manager, &ManagerBase::dataChanged, this, [this](){
        m_manager->buildModel(m_model);
        m_dialog->updateTableView(m_model);
    });


    m_dialog->setStatusMessage("Ready to roll!!!");
}

void CypressApplication::show()
{
    m_dialog->show();
}

void CypressApplication::finish()
{
    qDebug() << "its closing time!!!";
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("cypress.ini"), QSettings::IniFormat);
    m_manager->saveSettings(&settings);
    m_manager->finish();
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

void CypressApplication::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if(m_inputFileName.isEmpty())
    {
        qDebug() << "no input file";
        return;
    }
    QFileInfo info(m_inputFileName);
    if(info.exists())
    {
      QFile file;
      file.setFileName(m_inputFileName);
      file.open(QIODevice::ReadOnly | QIODevice::Text);
      QString val = file.readAll();
      file.close();
      qDebug() << val;

      QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
      QJsonObject jsonObj = jsonDoc.object();
      QMapIterator<QString,QVariant> it(m_inputData);
      QList<QString> keys = jsonObj.keys();
      for(int i=0;i<keys.size();i++)
      {
          QJsonValue v = jsonObj.value(keys[i]);
          // TODO: error report all missing expected key values
          //
          if(!v.isUndefined())
          {
              m_inputData[keys[i]] = v.toVariant();
              qDebug() << keys[i] << v.toVariant();
          }
      }
    }
    else
        qDebug() << m_inputFileName << " file does not exist";
}

void CypressApplication::writeOutput()
{
   if(m_verbose)
       qDebug() << "begin write process ... ";

   QJsonObject jsonObj;

   if(nullptr!=m_manager)
     jsonObj = m_manager->toJsonObject();

   QString barcode = m_dialog->getBarcode().simplified().remove(" ");
   jsonObj.insert("barcode",QJsonValue(barcode));

   if(m_verbose)
       qDebug() << "determine file output name ... ";

   QString fileName;

   // Use the output filename if it has a valid path
   // If the path is invalid, use the directory where the application exe resides
   // If the output filename is empty default output .json file is of the form
   // <participant ID>_<now>_<devicename>.json
   //
   bool constructDefault = false;

   // TODO: if the run mode is not debug, an output file name is mandatory, throw an error
   //
   if(m_outputFileName.isEmpty())
       constructDefault = true;
   else
   {
     QFileInfo info(m_outputFileName);
     QDir dir = info.absoluteDir();
     if(dir.exists())
       fileName = m_outputFileName;
     else
       constructDefault = true;
   }
   if(constructDefault)
   {
       QDir dir = QCoreApplication::applicationDirPath();
       if(m_outputFileName.isEmpty())
       {
         QStringList list;
         list
           << barcode
           << QDate().currentDate().toString("yyyyMMdd")
           << m_manager->getGroup()
           << "test.json";
         fileName = dir.filePath( list.join("_") );
       }
       else
         fileName = dir.filePath( m_outputFileName );
   }

   QFile saveFile( fileName );
   saveFile.open(QIODevice::WriteOnly);
   saveFile.write(QJsonDocument(jsonObj).toJson());

   if(m_verbose)
       qDebug() << "wrote to file " << fileName;

   m_dialog->setStatusMessage("Test data recorded.  Close when ready.");
}
