#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "BluetoothLEManager.h"

#include <QBitArray>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMetaEnum>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_verbose(false)
    , m_manager(this)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
    m_manager.setVerbose(m_verbose);

    if(!m_manager.lowEnergyEnabled())
    {
      QMessageBox msgBox;
      msgBox.setText(tr("The host operating system does not support bluetooth "
                        "low energy discovery."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();

      // TODO: return error code
      //
      close();
    }

    readInput();

    // Populate barcode display
    //
    if(m_inputData.contains("barcode") && m_inputData["barcode"].isValid())
       ui->barcodeLineEdit->setText(m_inputData["barcode"].toString());
    else
       ui->barcodeLineEdit->setText("00000000"); // dummy

    // Read the .ini file for cached local and peripheral device addresses
    //
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("lowenergythermometer.ini"), QSettings::IniFormat);
    m_manager.loadSettings(settings);

    if(!m_manager.localAdapterEnabled())
    {
      QMessageBox msgBox;
      msgBox.setText(tr("The host operating system has no local bluetooth "
                          "low energy adapter."));
      msgBox.setIcon(QMessageBox::Critical);
      msgBox.exec();

      if(m_verbose)
          qDebug() << "failed to find a local adapter";

      // TODO: return error code
      //
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
        if(m_verbose)
            qDebug() << "device selected from list " <<  item->text();
        m_manager.selectDevice(item->text());
      }
    );

    connect(&m_manager, &BluetoothLEManager::measured,
            ui->temperatureLineEdit, &QLineEdit::setText);

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
       // do not update status bar message if waiting for data to be saved
       //
       if(!ui->saveButton->isEnabled())
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

    connect(&m_manager, &BluetoothLEManager::canSelect,
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



    connect(ui->connectButton, &QPushButton::clicked,
          &m_manager, &BluetoothLEManager::connectPeripheral);

    connect(ui->saveButton, &QPushButton::clicked,
      this, [this](){
        writeOutput();
      }
    );
}

void MainWindow::run()
{
    m_manager.scanDevices();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_verbose)
        qDebug() << "close event called";
    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath("lowenergythermometer.ini"), QSettings::IniFormat);
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

void MainWindow::readInput()
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

void MainWindow::writeOutput()
{
    if(m_verbose)
        qDebug() << "begin write process ... ";

    QJsonObject jsonObj = m_manager.toJsonObject();

    qDebug() << "received json measurement data";

    QString barcode = ui->barcodeLineEdit->text().simplified().remove(" ");
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
          list << barcode;
          list << QDate().currentDate().toString("yyyyMMdd");
          list << "bluetooth_LE_thermometer.json";
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

   ui->statusBar->showMessage("Temperature data recorded.  Close when ready.");
}
