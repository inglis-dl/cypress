#include "CypressDialog.h"
#include <QVBoxLayout>
#include <QFormBuilder>
#include <QFile>
#include <QPushButton>
#include <QDebug>
#include <QHeaderView>
#include <QStandardItemModel>
#include "managers/ManagerBase.h"

CypressDialog::CypressDialog(QWidget *parent) : QDialog(parent)
{
}

CypressDialog::~CypressDialog()
{
    m_tableView.clear();
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
      case CypressApplication::TestType::Tonometry:
        ui_title = "Tonometry Measurement";
        break;
      case CypressApplication::TestType::BloodPressure:
        ui_title = "Blood Pressure Measurement";
        break;
      case CypressApplication::TestType::None:
        throw std::runtime_error("FATAL ERROR: failed to initialize UI");
        break;
    }

    qDebug() << "building form from " << ui_name;
    QFormBuilder builder;
    QFile file(":/dialogs/"+ui_name);
    file.open(QFile::ReadOnly);
    QWidget *widget = builder.load(&file,this);
    file.close();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(widget);
    this->setLayout(layout);

    this->setWindowTitle(ui_title);

    m_tableView.reset(this->findChild<QTableView*>("testdataTableView"));

    // all ui frames have a close button and a save button
    //
    connect(this->findChild<QPushButton*>("closeButton"),
            &QPushButton::clicked,
            this,[this](){
        qDebug() << "closing the dialog";
        this->accept();
    });

    connect(this->findChild<QPushButton*>("saveButton"),
            &QPushButton::clicked,
            app, &CypressApplication::writeOutput);

    m_tableView->setModel(app->getModel());

    m_tableView->horizontalHeader()->setSectionResizeMode(
      (1 == app->getModel()->columnCount() ?  QHeaderView::Stretch : QHeaderView::Fixed));
    m_tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tableView->verticalHeader()->hide();

    connect(app->getManager(),&ManagerBase::dataChanged,
            this,&CypressDialog::updateTableView);

    // construct the logic between the manager and the ui
    app->getManager()->connectUI(this);
}

void CypressDialog::setStatusMessage(const QString &msg)
{
    this->findChild<QPushButton*>("statusBar")->setText(msg);
}

QString CypressDialog::getBarcode() const
{
   return m_barcodeEdit->text();
}

void CypressDialog::updateTableView()
{
    auto h = m_tableView->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Fixed);    
    QSize ts_pre = m_tableView->size();
    h->resizeSections(QHeaderView::ResizeToContents);

    int total_width = m_tableView->autoScrollMargin() + 1;
    for(int col=0;col<m_tableView->model()->columnCount();col++)
    {
      m_tableView->setColumnWidth(col,h->sectionSize(col));
      total_width += h->sectionSize(col);
    }
    int total_height = h->height() + 1;
    for(int row=0;row<m_tableView->model()->rowCount();row++)
        total_height += m_tableView->rowHeight(row);

    m_tableView->resize(total_width, total_height);
    QSize ts_post = m_tableView->size();
    int dx = ts_post.width()-ts_pre.width();
    int dy = ts_post.height()-ts_pre.height();
    this->resize(this->width()+dx,this->height()+dy);
}
