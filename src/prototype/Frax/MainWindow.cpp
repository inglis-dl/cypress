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

    // Read inputs from json and display
    inputs = FraxIO::ReadInputs(jsonInputsPath);
    //FraxUI::SetInputs(ui, &inputs);
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
    FraxIO::CreateInputTxt(&inputs);

    // Run blackbox.exe
    QProcess process;
    process.start(m_manager.getExecutableFullPath());
    process.waitForFinished(2000);
    process.close();

    // read output.txt into outputs and display
    outputs = FraxIO::ReadOutputs(m_manager.getOutputFullPath());
    SetOutputs();

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

void MainWindow::SetUiInputs()
{
    ui->val1->setText(m_inputData["val1"].toString());
    ui->val2->setText(m_inputData["val2"].toString());
    ui->val3->setText(m_inputData["val3"].toString());
    ui->val4->setText(m_inputData["val4"].toString());
    ui->val5->setText(m_inputData["val5"].toString());
    ui->val6->setText(m_inputData["val6"].toString());
    ui->val7->setText(m_inputData["val7"].toString());
    ui->val8->setText(m_inputData["val8"].toString());
    ui->val9->setText(m_inputData["val9"].toString());
    ui->val10->setText(m_inputData["val10"].toString());
    ui->val11->setText(m_inputData["val11"].toString());
    ui->val12->setText(m_inputData["val12"].toString());
    ui->dxaHipTScore->setText(m_inputData["dxaHipTScore"].toString());
}

void MainWindow::SetOutputs()
{
    // TODO: Probably should double check none have changed
    SetUiInputs();

    // Load outputs on UI
    ui->fracRisk1->setText(QString::number(outputs.fracRisk1));
    ui->fracRisk2->setText(QString::number(outputs.fracRisk2));
    ui->fracRisk3->setText(QString::number(outputs.fracRisk3));
    ui->fracRisk4->setText(QString::number(outputs.fracRisk4));
}
