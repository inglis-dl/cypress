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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // after constructor is called, launch the application
    void run();

    void setInputFileName(const QString& name) { m_inputFileName = name; }
    QString inputFileName() { return m_inputFileName; }
    void setOutputFileName(const QString& name) { m_outputFileName = name; }
    QString outputFileName() { return m_outputFileName; }
    void setMode(const QString& mode) { m_mode = mode.toLower(); }
    QString mode() { return m_mode; }

    void readInput();
    void writeOutput();
    void setInputKeys(const QList<QString> &keys);
    void setOutputKeys(const QList<QString> &keys);

public slots:
    void updateDeviceList(const QString &label);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:

private:
    Ui::MainWindow *ui;

    QString m_inputFileName;
    QString m_outputFileName;
    QString m_mode;
    QMap<QString,QVariant> m_inputData;
    QMap<QString,QVariant> m_outputData;

    BluetoothLEManager m_manager;
};
#endif // MAINWINDOW_H
