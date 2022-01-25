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
#include <QTimeLine>

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
    , m_verbose(false)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize()
{
  initializeModel();
  initializeConnections();
}

void MainWindow::initializeModel()
{
    // allocate 2 columns x 8 rows of hearing measurement items
    //
    for(int col=0;col<2;col++)
    {
      for(int row=0;row<8;row++)
      {
        QStandardItem* item = new QStandardItem();
        m_model.setItem(row,col,item);
      }
    }
    m_model.setHeaderData(0,Qt::Horizontal,"Left Test Results",Qt::DisplayRole);
    m_model.setHeaderData(1,Qt::Horizontal,"Right Test Results",Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
}

// set up signal slot connections between GUI front end
// and device management back end
//
void MainWindow::initializeConnections()
{
  // Disable all buttons by default
  //
  for(auto&& x : this->findChildren<QPushButton *>())
  {
      x->setEnabled(false);

      // disable enter key press event passing onto auto focus buttons
      //
      x->setDefault(false);
      x->setAutoDefault(false);
  }

  // Close the application
  //
  ui->closeButton->setEnabled(true);

  // Relay messages from the manager to the status bar
  //
  connect(&m_manager,&ManagerBase::message,
          ui->statusBar, &QStatusBar::showMessage);

  // Every instrument stage launched by an interviewer requires input
  // of the interview barcode that accompanies a participant.
  // The expected barcode is passed from upstream via .json file.
  // In simulate mode this value is ignored and a default barcode "00000000" is
  // assigned instead.
  // In production mode the input to the barcodeLineEdit is verified against
  // the content held by the manager and a message or exception is thrown accordingly
  //
  // TODO: for DCS interviews, the first digit corresponds the the wave rank
  // for inhome interviews there is a host dependent prefix before the barcode
  //
  if("simulate"==m_mode)
  {
    ui->barcodeLineEdit->setText("00000000");
  }

  //TODO: handle the case for in home DCS visits where
  // the barcode is prefixed with a host name code
  //
  QRegExp rx("\\d{8}");
  QRegExpValidator *v_barcode = new QRegExpValidator(rx);
  ui->barcodeLineEdit->setValidator(v_barcode);

  connect(ui->barcodeLineEdit, &QLineEdit::editingFinished,
          this,[this](){
      bool valid = false;
      if(m_inputData.contains("barcode"))
      {
          QString str = ui->barcodeLineEdit->text().simplified();
          str.replace(" ","");
          valid = str == m_inputData["barcode"].toString();
      }
      auto p = this->findChild<QTimeLine *>("timer");
      if(valid)
      {
          p->stop();
          p->setCurrentTime(0);
          auto p = ui->barcodeLineEdit->palette();
          p.setBrush(QPalette::Base,QBrush(QColor(0,255,0,128)));
          ui->barcodeLineEdit->setPalette(p);

          // launch the manager
          //
          this->run();
      }
      else
      {
          QMessageBox::critical(
            this, QApplication::applicationName(),
            tr("The input does not match the expected barcode for this participant."));

          p->resume();
      }
  });

  auto timeLine = new QTimeLine(2000,this);
  timeLine->setFrameRange(0,255);
  timeLine->setLoopCount(0);
  timeLine->setObjectName("timer");
  connect(timeLine, &QTimeLine::frameChanged,
          this,[this](int frame){
      auto p = ui->barcodeLineEdit->palette();
      p.setBrush(QPalette::Base,QBrush(QColor(255,255,0,frame)));
      ui->barcodeLineEdit->setPalette(p);
  });
  connect(timeLine, &QTimeLine::finished, timeLine, &QTimeLine::deleteLater);
  timeLine->start();

    connect(&m_manager,&ChoiceReactionManager::canSelectRunnable,
            this,[this](){
        for(auto&& x : this->findChildren<QPushButton *>())
            x->setEnabled(false);
        ui->closeButton->setEnabled(true);
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
            QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the exe by clicking Open and browsing to the "
            "required executable (CCB.exe) and selecting the file.  If the executable "
            "is valid click the Run button to start the test otherwise check the installation."));
            warn = false;
        }
    });

    connect(ui->openButton, &QPushButton::clicked,
        this, [this]() {
            QString fileName =
                QFileDialog::getOpenFileName(
                    this, tr("Open File"),
                    QCoreApplication::applicationDirPath(),
                    tr("Applications (*.exe, *)"));

            m_manager.selectRunnable(fileName);
        });

    // CCB.exe was found or set up successfully
    //
    connect(&m_manager, &ChoiceReactionManager::canMeasure,
            this,[this](){
        ui->measureButton->setEnabled(true);
        ui->saveButton->setEnabled(false);
    });

    // Request a measurement from the device (run CCB.exe)
    //
    connect(ui->measureButton, &QPushButton::clicked,
          &m_manager, &ChoiceReactionManager::measure);

    // Update the UI with any data
    //
    connect(&m_manager, &ChoiceReactionManager::dataChanged,
            this,[this](){
        auto h = ui->testdataTableView->horizontalHeader();
        h->setSectionResizeMode(QHeaderView::Fixed);

        m_manager.buildModel(&m_model);

        QSize ts_pre = ui->testdataTableView->size();
        h->resizeSections(QHeaderView::ResizeToContents);
        ui->testdataTableView->setColumnWidth(0,h->sectionSize(0));
        ui->testdataTableView->setColumnWidth(1,h->sectionSize(1));
        ui->testdataTableView->resize(
                    h->sectionSize(0)+h->sectionSize(1)+
                    ui->testdataTableView->autoScrollMargin(),
                    8*ui->testdataTableView->rowHeight(0)+1+
                    h->height());
        QSize ts_post = ui->testdataTableView->size();
        int dx = ts_post.width()-ts_pre.width();
        int dy = ts_post.height()-ts_pre.height();
        this->resize(this->width()+dx,this->height()+dy);
    });

    // All measurements received: enable write test results
    //
    connect(&m_manager, &ChoiceReactionManager::canWrite,
            this,[this](){
        ui->saveButton->setEnabled(true);
    });

    // Write test data to output
    //
    connect(ui->saveButton, &QPushButton::clicked,
      this, &MainWindow::writeOutput);

    // Close the application
    //
    connect(ui->closeButton, &QPushButton::clicked,
            this, &MainWindow::close);

    // Read inputs required to launch cognitive test
    // In simulate mode the barcode is always populated with a default of "00000000"
    //
    readInput();
}

void MainWindow::run()
{
    m_manager.setVerbose(m_verbose);
    m_manager.setMode(m_mode);

    // Read the path to C:\Program Files (x86)\Cardiff_University\CCB\CCB.exe
    //
    QDir dir = QCoreApplication::applicationDirPath();
    qDebug() << "Dir: " << dir;
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.loadSettings(settings);

    // Pass the input to the manager for verification
    //
    m_manager.setInputData(m_inputData);

    m_manager.start();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(m_verbose)
        qDebug() << "close event called";

    QDir dir = QCoreApplication::applicationDirPath();
    QSettings settings(dir.filePath(m_manager.getGroup() + ".ini"), QSettings::IniFormat);
    m_manager.saveSettings(&settings);
    m_manager.finish();
    event->accept();
}

void MainWindow::readInput()
{
    // TODO: if the run mode is not debug, an input file name is mandatory, throw an error
    //
    if(m_inputFileName.isEmpty())
    {
        if("simulate" == m_mode)
        {
            m_inputData["barcode"]="00000000";
        }
        else
        {
            qDebug() << "ERROR: no input json file";
        }
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

   QString barcode = ui->barcodeLineEdit->text().simplified().remove(" ");
   jsonObj.insert("verification_barcode",QJsonValue(barcode));

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
         list
           << m_manager.getInputDataValue("barcode").toString()
           << QDate().currentDate().toString("yyyyMMdd")
           << m_manager.getGroup()
           << "test.json";
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

   ui->statusBar->showMessage("Cognitive Test data recorded.  Close when ready.");
}
