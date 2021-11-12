#include "MainWindow.h"

CognitiveTest::CognitiveTest(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CognitiveTestClass)
    , m_verbose(false)
{
    ui->setupUi(this);
}

CognitiveTest::~CognitiveTest()
{
    delete ui;
}

void CognitiveTest::initialize()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setMode(m_mode);

    // Read inputs required to launch cognitive test
    //
    readInput();

    // TODO: Add necessary UI elements
}

void CognitiveTest::run()
{
    // TODO: Check that these actually exist inside the json file
    QString CCBFolderPath = (m_inputData[QString("CCBFolderPath")]).toString();
    QString userId = (m_inputData[QString("userID")]).toString();
    QString dcsSiteName = (m_inputData[QString("dcsSiteName")]).toString();;
    QString interviewerId = (m_inputData[QString("interviewerID")]).toString();;
    QString language = (m_inputData[QString("language")]).toString();;
    m_manager.LaunchTest(CCBFolderPath,userId, dcsSiteName, interviewerId, language);
    m_manager.MoveResultsFile(CCBFolderPath, m_outputFileName);
}

void CognitiveTest::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if (m_inputFileName.isEmpty())
    {
        qDebug() << "no input file";
        return;
    }
    QFileInfo info(m_inputFileName);
    if (info.exists())
    {
        QFile file;
        file.setFileName(m_inputFileName);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString val = file.readAll();
        file.close();
        qDebug() << val;

        QJsonDocument jsonDoc = QJsonDocument::fromJson(val.toUtf8());
        QJsonObject jsonObj = jsonDoc.object();
        QMapIterator<QString, QVariant> it(m_inputData);
        QList<QString> keys = jsonObj.keys();
        for (int i = 0; i < keys.size(); i++)
        {
            QJsonValue v = jsonObj.value(keys[i]);
            // TODO: error report all missing expected key values
            //
            if (!v.isUndefined())
            {
                m_inputData[keys[i]] = v.toVariant();
                qDebug() << keys[i] << v.toVariant();
            }
        }
    }
    else
        qDebug() << m_inputFileName << " file does not exist";
}
