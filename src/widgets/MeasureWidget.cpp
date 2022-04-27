#include "MeasureWidget.h"

#include <QDebug>
#include <QStandardItemModel>

MeasureWidget::MeasureWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MeasureWidget)
{
    ui->setupUi(this);

    auto h = ui->testdataTableView->horizontalHeader();
    h->setSectionResizeMode(QHeaderView::Fixed);
    ui->testdataTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->testdataTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->testdataTableView->verticalHeader()->hide();

    connect(ui->measureButton, &QPushButton::clicked,
            this, [this](){emit measure();});

    connect(ui->saveButton, &QPushButton::clicked,
            this, [this](){emit writeToFile();});

    connect(ui->closeButton, &QPushButton::clicked,
            this, [this](){emit closeApplication();});

    ui->measureButton->setEnabled(false);
    ui->saveButton->setEnabled(false);
}

MeasureWidget::~MeasureWidget()
{
}

void MeasureWidget::initialize(QStandardItemModel* model)
{
  ui->testdataTableView->setModel(model);
  // Disable all buttons by default
  //
  foreach(auto button, this->findChildren<QPushButton *>())
  {
    if("Close" != button->text())
      button->setEnabled(false);

    // disable enter key press event passing onto auto focus buttons
    //
    button->setDefault(false);
    button->setAutoDefault(false);
  }
}

void MeasureWidget::disableMeasure()
{
  foreach(auto button, this->findChildren<QPushButton *>())
  {
    if("Close" != button->text())
      button->setEnabled(false);
  }
}

void MeasureWidget::enableMeasure()
{
    foreach(auto button, this->findChildren<QPushButton *>())
    {
      if("Close" != button->text())
        button->setEnabled(true);
    }
}

void MeasureWidget::enableWriteToFile()
{
    ui->saveButton->setEnabled(true);
}

void MeasureWidget::updateModelView()
{
    auto h = ui->testdataTableView->horizontalHeader();
    int nrow = qMin(m_displayRowCount, ui->testdataTableView->model()->rowCount());
    int ncol = ui->testdataTableView->model()->columnCount();

    QSize ts_pre = ui->testdataTableView->size();
    h->resizeSections(QHeaderView::ResizeToContents);
    int colWidth = 0;
    for(int i = 0; i < ncol; i++)
    {
      ui->testdataTableView->setColumnWidth(i, h->sectionSize(0));
      colWidth += h->sectionSize(i) + 1;
    }

    int rowHeight = nrow * (ui->testdataTableView->rowHeight(0) + 1);

    ui->testdataTableView->resize(
      colWidth + ui->testdataTableView->autoScrollMargin(),
      rowHeight + h->height()
    );
    QSize ts_post = ui->testdataTableView->size();
    int dx = ts_post.width() - ts_pre.width();
    int dy = ts_post.height() - ts_pre.height();

    const QWidgetList list = QApplication::topLevelWidgets();
    if(!list.isEmpty())
      list.first()->resize(this->width() + dx, this->height() + dy);
}
