#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../../managers/AudiometerManager.h"
#include <QDialog>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    bool isVerbose(){ return m_verbose; }

public slots:
    void updateDeviceList(const QString &);
    void writeOutput();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void readInput();

    Ui::MainWindow *ui;
    QString m_inputFileName;
    QString m_outputFileName;
    QString m_mode;
    bool m_verbose;

    QMap<QString,QVariant> m_inputData;
    QMap<QString,QVariant> m_outputData;

    AudiometerManager m_manager;

    QStandardItemModel m_model;
};

#endif // MAINWINDOW_H
