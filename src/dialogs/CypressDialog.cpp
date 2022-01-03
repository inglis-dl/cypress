#include "CypressDialog.h"
#include <QVBoxLayout>
#include <QFormBuilder>
#include <QFile>
#include <QPushButton>
#include <QDebug>
#include <QHeaderView>
#include <QStandardItemModel>

CypressDialog::CypressDialog(QWidget *parent) : QDialog(parent)
{
}

CypressDialog::~CypressDialog()
{
}

void CypressDialog::initialize(CypressApplication *app)
{
    QString ui_name;
    QString ui_title;
    switch(app->getTestType())
    {
      case CypressApplication::TestType::Weight:
        ui_name = "weighscale.ui";
        ui_title = "Weight Measurement";
        break;
      case CypressApplication::TestType::BodyComposition:
        ui_name = "bodycomposition.ui";
        ui_title = "Body Composition Measurement";
        break;
      case CypressApplication::TestType::Hearing:
        ui_name = "audiometer.ui";
        ui_title = "Hearing Measurement";
        break;
      case CypressApplication::TestType::ChoiceReaction:
        ui_name = "choicereaction.ui";
        ui_title = "Choice Reaction Test";
        break;
      case CypressApplication::TestType::Temperature:
        ui_name = "thermometer.ui";
        ui_title = "Temperature Measurement";
        break;
      case CypressApplication::TestType::Frax:
        ui_name = "frax.ui";
        ui_title = "FRAX Measurement";
        break;
      case CypressApplication::TestType::CDTT:
        ui_title = "CDTT Test";
        break;
      case CypressApplication::TestType::Spirometry:
        ui_title = "Spirometry Measurement";
        break;
      case CypressApplication::TestType::BloodPressure:
        ui_title = "Blood Pressure Measurement";
        break;
      case CypressApplication::TestType::None:
        throw std::runtime_error("FATAL ERROR: failed to initialize UI");
        break;
    }

    qDebug() << "buidling form from " << ui_name;
    QFormBuilder builder;
    QFile file(":/dialogs/"+ui_name);
    file.open(QFile::ReadOnly);
    QWidget *widget = builder.load(&file,this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(widget);
    this->setLayout(layout);

    this->setWindowTitle(ui_title);

    m_status.reset(this->findChild<QStatusBar*>("statusBar"));
    m_barcodeEdit.reset(this->findChild<QLineEdit*>("barcodeLineEdit"));
    m_tableView.reset(this->findChild<QTableView*>("testdataTableView"));

    // all ui frames have a close button and a save button
    //
    QPushButton* ui_closeButton = this->findChild<QPushButton*>("closeButton");
    if(nullptr!=ui_closeButton)
        qDebug() << "found UI element " << ui_closeButton->objectName();

    QPushButton* ui_saveButton = this->findChild<QPushButton*>("saveButton");
    if(nullptr!=ui_saveButton)
        qDebug() << "found UI element " << ui_saveButton->objectName();

    connect(ui_closeButton,&QPushButton::clicked,
            this,[this](){
        qDebug() << "closing the dialog";
        this->accept();
    });

    connect(ui_saveButton,&QPushButton::clicked,
            app, &CypressApplication::writeOutput);

    m_tableView->setModel(app->getModel());

    m_tableView->horizontalHeader()->setSectionResizeMode(
      (1 == app->getModel()->columnCount() ?  QHeaderView::Stretch : QHeaderView::Fixed));
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableView->verticalHeader()->hide();

    // construct the logic between the manager and the ui
}

void CypressDialog::updateTableView(QStandardItemModel *model)
{
    QHeaderView *h = m_tableView->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Fixed);
    QSize ts_pre = m_tableView->size();
    h->resizeSections(QHeaderView::ResizeToContents);
    int total_width = m_tableView->autoScrollMargin() + 1;
    for(int col=0;col<model->columnCount();col++)
    {
      m_tableView->setColumnWidth(col,h->sectionSize(col));
      total_width += h->sectionSize(col);
    }
    int total_height = h->height() + 1;
    for(int row=0;row<model->rowCount();row++)
        total_height += m_tableView->rowHeight(row);

    m_tableView->resize(total_width, total_height);
    QSize ts_post = m_tableView->size();
    int dx = ts_post.width()-ts_pre.width();
    int dy = ts_post.height()-ts_pre.height();
    this->resize(this->width()+dx,this->height()+dy);
}

QString CypressDialog::getBarcode() const
{
   return m_barcodeEdit->text();
}

void CypressDialog::setStatusMessage(const QString &message)
{
  m_status->showMessage(message);
}

