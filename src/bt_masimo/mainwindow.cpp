#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QDebug>
#include <QMetaEnum>
#include <QMessageBox>
#include <QBitArray>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QLowEnergyController>

#include "BluetoothLEManager.h"



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_manager(this)
{
    ui->setupUi(this);

    if(!m_manager.lowEnergyEnabled())
    {
      QMessageBox msgBox;
      msgBox.setText(tr("The host operating system does not support bluetooth "
                        "low energy discovery."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      close();
    }

    // Read the .ini file for cached local and peripheral device addresses
    //
    QSettings settings(m_appDir.filePath("bt_masimo.ini"), QSettings::IniFormat);
    m_manager.loadSettings(settings);

    if(!m_manager.localAdapterEnabled())
    {
      QMessageBox msgBox;
      msgBox.setText(tr("The host operating system has no local bluetooth "
                          "low energy adapter."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();
      qDebug() << "failed to find a local adapter";
      close();
    }


    // Connect button to connect a controller to a verified peripheral device
    //
    ui->connectButton->setEnabled(false);

    // Save button to store measurement and device info to .json
    //
    ui->saveButton->setEnabled(false);

    // as soon as there are LE devices in the ui list, allow click to select a mac address
    //
    connect(ui->deviceListWidget, &QListWidget::itemDoubleClicked,
            this,[this](QListWidgetItem* item)
      {
        qDebug() << "device selected from list " <<  item->text();
        m_manager.selectDevice(item->text());
      }
    );

    connect(&m_manager, &BluetoothLEManager::peripheralMACChanged,
            ui->addressLineEdit, &QLineEdit::setText);

    connect(&m_manager, &BluetoothLEManager::temperatureChanged,
            ui->temperatureLineEdit, &QLineEdit::setText);

    connect(&m_manager, &BluetoothLEManager::datetimeChanged,
            ui->dateTimeLineEdit, &QLineEdit::setText);

    connect(&m_manager, &BluetoothLEManager::scanning,
            this,[this]()
      {
        ui->deviceListWidget->clear();
        ui->statusBar->showMessage("Discovering devices ... please wait");
      }
    );

    connect(&m_manager, &BluetoothLEManager::discovered,
            this, &MainWindow::updateDeviceList);

    connect(&m_manager, &BluetoothLEManager::canConnect,
            this,[this](){
       ui->connectButton->setEnabled(true);
       ui->statusBar->showMessage("Ready to connect to peripheral device.");
    });

    connect(&m_manager, &BluetoothLEManager::connected,
            this,[this](){
       ui->connectButton->setEnabled(false);
    });

    connect(&m_manager, &BluetoothLEManager::canWrite,
            this,[this](){
       ui->saveButton->setEnabled(true);
       ui->statusBar->showMessage("Ready to save temperature data.");
    });

    connect(&m_manager, &BluetoothLEManager::select,
            this,[this](){
        // Prompt the user to select the MAC address
        QMessageBox msgBox;
        msgBox.setText(tr("Double click the bluetooth thermometer from the list.  If the device "
          "is not in the list, quit the application and check that the bluetooth adapeter is "
          "working and pair the thermometer to it before running this application."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
        msgBox.setButtonText(QMessageBox::Abort,tr("Quit"));
        connect(msgBox.button(QMessageBox::Abort),&QPushButton::clicked,this,&MainWindow::close);
        msgBox.exec();
    });

    ui->barcodeLineEdit->setText("40000001"); // dummy ID for now

    connect(ui->connectButton, &QPushButton::clicked,
          &m_manager, &BluetoothLEManager::connectPeripheral);

    connect(ui->saveButton, &QPushButton::clicked,
          this, &MainWindow::writeMeasurement);
 }

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    m_manager.scanDevices();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "close event called";

    QSettings settings(m_appDir.filePath("bt_masimo.ini"), QSettings::IniFormat);
    m_manager.saveSettings(&settings);

    event->accept();
}

void MainWindow::updateDeviceList(const QString &label)
{
    // Add the device to the list
    //
    QList<QListWidgetItem *> items = ui->deviceListWidget->findItems(label, Qt::MatchExactly);
    if(items.empty())
    {
        QListWidgetItem *item = new QListWidgetItem(label);
        if(m_manager.isPairedTo(label))
          item->setForeground(QColor(Qt::green));
        else
          item->setForeground(QColor(Qt::black));

        ui->deviceListWidget->addItem(item);
    }
}

void MainWindow::writeMeasurement()
{
   qDebug() << "begin write process ... ";

   QMap<QString,QVariant> m_measurementData = m_manager.getMeasurementData();

   // Create a json object with measurement key value pairs
   //
   QJsonObject json;
   QMap<QString,QVariant>::const_iterator it = m_measurementData.constBegin();
   while(it != m_measurementData.constEnd())
   {
     json.insert(it.key(),QJsonValue::fromVariant(it.value()));
     ++it;
   }

   QMap<QString,QVariant> m_deviceData = m_manager.getDeviceData();

   it = m_deviceData.constBegin();
   while(it != m_deviceData.constEnd())
   {
     json.insert(it.key(),QJsonValue::fromVariant(it.value()));
     ++it;
   }

   // Insert participant barcode
   //
   QString barcode = ui->barcodeLineEdit->text().simplified().remove(" ");
   json.insert("Barcode",QJsonValue(barcode));

   // Output .json file will be of the form <participant ID>_<now>_<devicename>.json
   //
   QStringList jsonFile;
   jsonFile << barcode;
   jsonFile << QDate().currentDate().toString("yyyyMMdd");
   jsonFile << "bt_masimo.json";
   QFile saveFile( m_appDir.filePath( jsonFile.join("_") ) );
   saveFile.open(QIODevice::WriteOnly);
   saveFile.write(QJsonDocument(json).toJson());

   qDebug() << "wrote to file " << m_appDir.filePath( jsonFile.join("_") );

   ui->statusBar->showMessage("Temperature data recorded.  Close when ready.");
}
