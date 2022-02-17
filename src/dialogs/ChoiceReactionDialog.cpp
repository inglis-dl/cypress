#include "ChoiceReactionDialog.h"
#include "../managers/ChoiceReactionManager.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimeLine>

ChoiceReactionDialog::ChoiceReactionDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::RunnableDialog)
{
    ui->setupUi(this);
    m_manager.reset(new ChoiceReactionManager(this));
    this->setWindowTitle("Choice Reaction Test");
}

ChoiceReactionDialog::~ChoiceReactionDialog()
{
    delete ui;
}

void ChoiceReactionDialog::initializeModel()
{
    for(int col=0;col<m_manager->getNumberOfModelColumns();col++)
    {
      for(int row=0;row<m_manager->getNumberOfModelRows();row++)
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
void ChoiceReactionDialog::initializeConnections()
{
  QSharedPointer<ChoiceReactionManager> derived =
    m_manager.staticCast<ChoiceReactionManager>();

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
  connect(m_manager.get(),&ManagerBase::message,
          ui->statusBar, &QStatusBar::showMessage, Qt::DirectConnection);

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
  if(CypressConstants::Mode::Simulate == m_mode)
  {
    ui->barcodeLineEdit->setText("00000000");
  }

  //TODO: handle the case for in home DCS visits where
  // the barcode is prefixed with a host name code
  //
  QRegExp rx("\\d{8}");
  QRegExpValidator *v_barcode = new QRegExpValidator(rx);
  ui->barcodeLineEdit->setValidator(v_barcode);

  connect(ui->barcodeLineEdit, &QLineEdit::returnPressed,
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
          ui->barcodeLineEdit->repaint();

          // launch the manager
          //
          this->run();
      }
      else
      {
          QMessageBox::critical(
            this, QApplication::applicationName(),
            tr("The input does not match the expected barcode for this participant."));
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

  connect(derived.get(),&ChoiceReactionManager::canSelectRunnable,
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
        this, [this,derived]() {
            QString fileName =
                QFileDialog::getOpenFileName(
                    this, tr("Open File"),
                    QCoreApplication::applicationDirPath(),
                    tr("Applications (*.exe, *)"));

            derived->selectRunnable(fileName);
        });

  // CCB.exe was found or set up successfully
  //
  connect(m_manager.get(), &ChoiceReactionManager::canMeasure,
            this,[this](){
        ui->measureButton->setEnabled(true);
        ui->saveButton->setEnabled(false);
    });

  // Request a measurement from the device (run CCB.exe)
  //
  connect(ui->measureButton, &QPushButton::clicked,
          derived.get(), &ChoiceReactionManager::measure);

  // Update the UI with any data
  //
  connect(m_manager.get(), &ChoiceReactionManager::dataChanged,
            this,[this](){
        auto h = ui->testdataTableView->horizontalHeader();
        h->setSectionResizeMode(QHeaderView::Fixed);

        m_manager->buildModel(&m_model);

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
  connect(m_manager.get(), &ChoiceReactionManager::canWrite,
            this,[this](){
        ui->saveButton->setEnabled(true);
    });

  // Write test data to output
  //
  connect(ui->saveButton, &QPushButton::clicked,
      this, &ChoiceReactionDialog::writeOutput);

  // Close the application
  //
  connect(ui->closeButton, &QPushButton::clicked,
            this, &ChoiceReactionDialog::close);

  // Read inputs required to launch cognitive test
  // In simulate mode the barcode is always populated with a default of "00000000"
  //
  readInput();
}

QString ChoiceReactionDialog::getVerificationBarcode() const
{
  return ui->barcodeLineEdit->text().simplified().remove(" ");
}
