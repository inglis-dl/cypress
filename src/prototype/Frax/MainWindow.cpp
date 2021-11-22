#include "MainWindow.h"

#include <QCloseEvent>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_verbose(false)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::CalculateClicked()
{
    // Save example input.txt to be put back later
    QFile::rename(m_manager.getInputFullPath(), m_manager.getOldInputFullPath());

    // Create new input.txt from our inputs
    m_manager.createInputsTxt();

    // Run blackbox.exe
    QProcess process;
    process.start(m_manager.getExecutableFullPath());
    process.waitForFinished(2000);
    process.close();

    // read output.txt into outputs and display
    m_manager.readOutputs();
    SetUiOutputs();

    // restore files to original state
    QFile::remove(m_manager.getInputFullPath());
    QFile::remove(m_manager.getOutputFullPath());
    QFile::rename(m_manager.getOldInputFullPath(), m_manager.getInputFullPath());
}

void MainWindow::initialize()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setMode(m_mode);

    // Read inputs required to launch frax test
    //
    readInput();
    SetUiInputs();

    QDir dir = QCoreApplication::applicationDirPath();
    qDebug() << "Dir: " << dir;
    QSettings settings(dir.filePath("frax.ini"), QSettings::IniFormat);

    // read the path to C:\Program Files (x86)\Cardiff_University\CCB\CCB.exe
    //
    m_manager.loadSettings(settings);
}

void MainWindow::run()
{
}

void MainWindow::readInput()
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
        QMapIterator<QString, QVariant> it(m_manager.m_inputData);
        QList<QString> keys = jsonObj.keys();
        for (int i = 0; i < keys.size(); i++)
        {
            QJsonValue v = jsonObj.value(keys[i]);
            // TODO: error report all missing expected key values
            //
            if (!v.isUndefined())
            {
                m_manager.m_inputData[keys[i]] = v.toVariant();
                qDebug() << keys[i] << v.toVariant();
            }
        }
    }
    else
        qDebug() << m_inputFileName << " file does not exist";
}

void MainWindow::SetUiInputs()
{
    QMap<QString, QVariant> inMap = m_manager.m_inputData;
    ui->val1->setText(inMap["val1"].toString());
    ui->val2->setText(inMap["val2"].toString());
    ui->val3->setText(inMap["val3"].toString());
    ui->val4->setText(inMap["val4"].toString());
    ui->val5->setText(inMap["val5"].toString());
    ui->val6->setText(inMap["val6"].toString());
    ui->val7->setText(inMap["val7"].toString());
    ui->val8->setText(inMap["val8"].toString());
    ui->val9->setText(inMap["val9"].toString());
    ui->val10->setText(inMap["val10"].toString());
    ui->val11->setText(inMap["val11"].toString());
    ui->val12->setText(inMap["val12"].toString());
    ui->dxaHipTScore->setText(inMap["dxaHipTScore"].toString());
}

void MainWindow::SetUiOutputs()
{
    QMap<QString, QVariant> outMap = m_manager.m_outputData;
    ui->val1->setText(outMap["val1"].toString());
    ui->val2->setText(outMap["val2"].toString());
    ui->val3->setText(outMap["val3"].toString());
    ui->val4->setText(outMap["val4"].toString());
    ui->val5->setText(outMap["val5"].toString());
    ui->val6->setText(outMap["val6"].toString());
    ui->val7->setText(outMap["val7"].toString());
    ui->val8->setText(outMap["val8"].toString());
    ui->val9->setText(outMap["val9"].toString());
    ui->val10->setText(outMap["val10"].toString());
    ui->val11->setText(outMap["val11"].toString());
    ui->val12->setText(outMap["val12"].toString());
    ui->dxaHipTScore->setText(outMap["dxaHipTScore"].toString());
    ui->fracRisk1->setText(outMap["fracRisk1"].toString());
    ui->fracRisk2->setText(outMap["fracRisk2"].toString());
    ui->fracRisk3->setText(outMap["fracRisk3"].toString());
    ui->fracRisk4->setText(outMap["fracRisk4"].toString());
}
