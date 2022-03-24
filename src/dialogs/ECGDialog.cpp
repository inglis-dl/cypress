#include "ECGDialog.h"
#include "../managers/ECGManager.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

ECGDialog::ECGDialog(QWidget *parent)
    : DialogBase(parent)
    , ui(new Ui::RunnableDialog)
{
    ui->setupUi(this);
    m_manager.reset(new ECGManager(this));
    this->setWindowTitle("ECG");
}

ECGDialog::~ECGDialog()
{
    delete ui;
}

void ECGDialog::initializeModel()
{
    // allocate 1 columns x n rows of ECG measurement items
    //
    for(int row = 0; row < m_manager->getNumberOfModelRows(); row++)
    {
      QStandardItem* item = new QStandardItem();
      m_model.setItem(row, 0, item);
    }

    m_model.setHeaderData(0, Qt::Horizontal, "Electrocardiogram Results ", Qt::DisplayRole);
    ui->testdataTableView->setModel(&m_model);

    ui->testdataTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->testdataTableView->verticalHeader()->hide();
}

// set up signal slot connections between GUI front end
// and device management back end
//
void ECGDialog::initializeConnections()
{
  QSharedPointer<ECGManager> derived = m_manager.staticCast<ECGManager>();

  // Disable all buttons by default
  //
  foreach(auto button, this->findChildren<QPushButton *>())
  {
    button->setEnabled(false);

    // disable enter key press event passing onto auto focus buttons
    //
    button->setDefault(false);
    button->setAutoDefault(false);
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
  // In simulate mode this value is ignored and a default barcode Constants::DefaultBarcode is
  // assigned instead.
  // In production mode the input to the barcodeLineEdit is verified against
  // the content held by the manager and a message or exception is thrown accordingly
  //
  // TODO: for DCS interviews, the first digit corresponds the the wave rank
  // for inhome interviews there is a host dependent prefix before the barcode
  //
  if(Constants::RunMode::modeSimulate == m_mode)
  {
    ui->barcodeWidget->setBarcode(Constants::DefaultBarcode);
  }

  connect(ui->barcodeWidget,&BarcodeWidget::validated,
          this,[this](const bool& valid)
    {
      if(valid)
      {
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

    connect(derived.get(),&ECGManager::canSelectRunnable,
            this,[this](){
        foreach(auto button, this->findChildren<QPushButton *>())
            button->setEnabled(false);

        ui->closeButton->setEnabled(true);
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
            QMessageBox::warning(
                        this, QApplication::applicationName(),
                        tr("Select the exe by clicking Open and browsing to the "
                        "required executable (Cardio.exe) and selecting the file.  If the executable "
                        "is valid click the Run button to start the test otherwise check the installation."));
            warn = false;
        }
    });

    connect(ui->openButton, &QPushButton::clicked,
            derived.get(), &ECGManager::select);

    connect(derived.get(),&ECGManager::canSelectWorking,
            this,[this](){
        foreach(auto button, this->findChildren<QPushButton *>())
            button->setEnabled(false);
        ui->closeButton->setEnabled(true);
        ui->openButton->setEnabled(true);
        static bool warn = true;
        if(warn)
        {
            QMessageBox::warning(
            this, QApplication::applicationName(),
            tr("Select the path to the Cardiosoft database directory by clicking Open and browsing to "
            "and selecting the folder (C:/CARDIO).  If the folder "
            "is valid click the Run button to start the test otherwise check the installation."));
            warn = false;
        }
    });

    // ora.exe was found or set up successfully
    //
    connect(m_manager.get(), &ECGManager::canMeasure,
        this, [this]() {
            ui->measureButton->setEnabled(true);
            ui->saveButton->setEnabled(false);
        });

    // Request a measurement from the device (run ora.exe)
    //
    connect(ui->measureButton, &QPushButton::clicked,
        derived.get(), &ECGManager::measure);

    // Update the UI with any data
    //
    connect(m_manager.get(), &ECGManager::dataChanged,
        this, [this]() {
      auto h = ui->testdataTableView->horizontalHeader();
      h->setSectionResizeMode(QHeaderView::Fixed);

      m_manager->buildModel(&m_model);

      QSize ts_pre = ui->testdataTableView->size();
      h->resizeSections(QHeaderView::ResizeToContents);
      ui->testdataTableView->setColumnWidth(0,h->sectionSize(0));
      ui->testdataTableView->setColumnWidth(1,h->sectionSize(1));
      ui->testdataTableView->resize(
                  h->sectionSize(0)+h->sectionSize(1)+1,
                  8*ui->testdataTableView->rowHeight(0)+1+
                  h->height());
      QSize ts_post = ui->testdataTableView->size();
      int dx = ts_post.width() - ts_pre.width();
      int dy = ts_post.height() - ts_pre.height();
      this->resize(this->width() + dx, this->height() + dy);
    });

    // All measurements received: enable write test results
    //
    connect(m_manager.get(), &ECGManager::canWrite,
        this, [this]() {
            ui->saveButton->setEnabled(true);
        });

    // Write test data to output
    //
    connect(ui->saveButton, &QPushButton::clicked,
        this, &ECGDialog::writeOutput);

    // Close the application
    //
    connect(ui->closeButton, &QPushButton::clicked,
        this, &ECGDialog::close);

    // Read inputs, such as interview barcode
    //
    readInput();
}

QString ECGDialog::getVerificationBarcode() const
{
  return ui->barcodeWidget->barcode();
}

void ECGDialog::setVerificationBarcode(const QString &barcode)
{
    ui->barcodeWidget->setBarcode(barcode);
}
