#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

#include "CognitiveTestManager.h"

#include <QString>
#include <QStandardPaths>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class CognitiveTest; }
QT_END_NAMESPACE

class CognitiveTest : public QMainWindow
{
    Q_OBJECT

public:
    CognitiveTest(QWidget* parent = nullptr);
    ~CognitiveTest();

    // This method internally calls readInput
    //
    void initialize();

    // Call after initialize, launch the application and run
    // the device
    //
    void run();

    void setInputFileName(const QString& name) { m_inputFileName = name; }
    QString inputFileName() { return m_inputFileName; }

    void setOutputFileName(const QString& name) { m_outputFileName = name; }
    QString outputFileName() { return m_outputFileName; }

    void setMode(const QString& mode) { m_mode = mode.toLower(); }
    QString mode() { return m_mode; }

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose() { return m_verbose; }

private:
    void readInput();

    Ui::CognitiveTestClass *ui;
    QString m_inputFileName;
    QString m_outputFileName;
    QString m_mode;
    bool m_verbose;

    QMap<QString, QVariant> m_inputData;

    CognitiveTestManager m_manager;
};

#endif // MAINWINDOW_H
