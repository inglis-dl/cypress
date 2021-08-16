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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setApplicationDir(QString path) {
        this->m_appDir = QDir(path);
    }

public slots:
    void updateDeviceList(const QString &label);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:

    // Write the measurement data (temperature, datetime, barcode, device) in json formal to file
    //
    void writeMeasurement();

private:
    Ui::MainWindow *ui;

    QDir m_appDir;
    BluetoothLEManager m_manager;
};
#endif // MAINWINDOW_H
