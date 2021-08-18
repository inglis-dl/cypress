#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QDir>

QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

#include "BluetoothLEManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Q_PROPERTY(QString inputFileName READ inputFileName WRITE setInputFileName)
    Q_PROPERTY(QString outputFileName READ outputFileName WRITE setOutputFileName)
    Q_PROPERTY(QString mode READ mode WRITE setMode)
    Q_PROPERTY(bool verbose READ isVerbose WRITE setVerbose)

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Set the expected keys that should be read from an input
    // json file
    //
    void setInputKeys(const QList<QString> &keys);

    // Set the expected output keys that will be written
    // with device data content to json file
    //
    void setOutputKeys(const QList<QString> &keys);

    // Call after setting the input and output keys
    // This method internally calls readInput
    void initialize();

    // Call after initialize, launch the application and run
    // the device
    //
    void run();

    // Call after run to finish any remaining tasks like auto write to output file
    //
    void finish();

    void setInputFileName(const QString& name) { m_inputFileName = name; }
    QString inputFileName() { return m_inputFileName; }
    void setOutputFileName(const QString& name) { m_outputFileName = name; }
    QString outputFileName() { return m_outputFileName; }
    void setMode(const QString& mode) { m_mode = mode.toLower(); }
    QString mode() { return m_mode; }
    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose(){ return m_verbose; }

public slots:
    void updateDeviceList(const QString &label);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:

private:
    void readInput();
    void writeOutput();

    Ui::MainWindow *ui;

    QString m_inputFileName;
    QString m_outputFileName;
    QString m_mode;
    bool m_verbose;
    QMap<QString,QVariant> m_inputData;
    QMap<QString,QVariant> m_outputData;

    BluetoothLEManager m_manager;
};
#endif // MAINWINDOW_H
