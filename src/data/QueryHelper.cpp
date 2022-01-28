#include "QueryHelper.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDriver>

#include <stdexcept>

QueryHelper::QueryHelper(
        const QString& cellStart,
        const QString& cellEnd,
        const QString& sheet = "Sheet1") :
  m_cellStart(cellStart),
  m_cellEnd(cellEnd),
  m_sheet(sheet),
  m_order(Order::None),
  m_mode(HeaderMode::Integrate)
{
  initialize();
}

QueryHelper::QueryHelper(const QueryHelper &other)
{
  m_cellStart = other.m_cellStart;
  m_cellEnd = other.m_cellEnd;
  m_sheet = other.m_sheet;
  m_order = other.m_order;
  m_mode = other.m_mode;
  m_header = other.m_header;
  initialize();
}

QueryHelper& QueryHelper::operator=(const QueryHelper &other)
{
    if(this != &other)
    {
      m_cellStart = other.m_cellStart;
      m_cellEnd = other.m_cellEnd;
      m_sheet = other.m_sheet;
      m_order = other.m_order;
      m_mode = other.m_mode;
      m_header = other.m_header;
      initialize();
    }
    return *this;
}

void QueryHelper::initialize()
{
    m_cellStart = m_cellStart.toUpper();
    m_cellEnd = m_cellEnd.toUpper();

    QRegExp rx("^[A-Z]{1,2}[1-9]{1,1}[0-9]*$");
    if(!(rx.exactMatch(m_cellStart) &&
         rx.exactMatch(m_cellEnd)))
    {
        qDebug() << "ERROR: incorrect cell definitions" << m_cellStart << m_cellEnd;
        throw std::runtime_error("incorrect cell definition");
        return;
    }

    QString l1 = QString(m_cellStart).remove(QRegExp("[0-9]"));
    QString l2 = QString(m_cellEnd).remove(QRegExp("[0-9]"));
    int c1 = columnToIndex(l1);
    int c2 = columnToIndex(l2);
    qDebug() << "l1" <<l1 << "index"<< QString::number(c1);
    qDebug() << "l2" <<l2 << "index"<< QString::number(c2);
    if(c1 > c2)
    {
        QString t = m_cellEnd;
        m_cellEnd = m_cellStart;
        m_cellStart = t;
    }
    n_col = abs(c2-c1)+1;
    qDebug() << "n columns" << QString::number(n_col);

    l1 = QString(m_cellStart).remove(QRegExp("[A-Z]"));
    l2 = QString(m_cellEnd).remove(QRegExp("[A-Z]"));
    int r1 = l1.toInt();
    int r2 = l2.toInt();
    n_row = abs(r2-r1)+1;
    qDebug() << "n rows" << QString::number(n_row);

   // case where the cell definitions read incorrectly eg., B2 -> C1
   // the definition should alway lead to a reading from top left to bottom right
   // up to here we are sorted by column label according to index value
   // if the row value of start cell is > row value of end cell
   // we swap the row values eg., B2 -> C1 becomes B1 -> C2
   //
   if(r1 > r2)
   {
     m_cellStart.replace(l1,l2);
     m_cellEnd.replace(l2,l1);
   }
   qDebug() << "final cell labels" << m_cellStart<<m_cellEnd;
}

void QueryHelper::setSheet(const QString &sheet)
{
    m_sheet = sheet.isEmpty() ? "Sheet1" : sheet;
}

inline int QueryHelper::columnToIndex(const QString &s)
{
    int result = 0;
    auto c = s.constBegin();
    while( c != s.constEnd() )
    {
        result *= 26;
        result += (*c).toLatin1() - 'A' + 1;
        c++;
    }
    return result;
}

void QueryHelper::setOrder(const QueryHelper::Order &order)
{
  m_order = order;
}

void QueryHelper::setHeaderMode(const QueryHelper::HeaderMode &mode)
{
  m_mode = mode;
}

bool QueryHelper::buildQuery(const QSqlDatabase &db)
{
    bool ok = true;

    QString q_str =
      QString("select * from [%1$%2:%3]").arg(m_sheet,m_cellStart,m_cellEnd);

    m_query = QSqlQuery(q_str,db);

    if(!(m_query.isActive() && m_query.isSelect()))
    {
        qDebug() << "ERROR: cannot process query" << q_str;
        ok = false;
    }
    qDebug() << "driver"<<db.driverName()<<"supports query size()" <<
      (db.driver()->hasFeature(QSqlDriver::QuerySize)?"YES":"NO");

    return ok;
}

// the order must be specified correctly
// header runs horizontally, values run vertically
// order = Column
// case 1: n_row = 1, n_col = 1, n > 1   ERROR
// case 2: n_row = 1, n_col = 1, n = 1   header confirmation {Integrate} OR label value with header {Detach}, mode can be None
// case 3: n_row = 1, n_col = m, n != m  ERROR
// case 4: n_row = 1, n_col = m, n = m   header confirmation {Integrate} OR label value with header {Detach}
// case 5: n_row > 1, n_col = 1, n > 1   ERROR
// case 6: n_row > 1, n_col = 1, n = 1   n_row - 1 values {Integrate} OR assign n_row values {Detach}
// case 7: n_row > 1, n_col = m, n != m  ERROR
// case 8: n_row > 1, n_col = m, n = m   n_row - 1 values {Integrate} OR assign n_row values {Detach}
//
// simplifies to
// case 1': n_row = k, n_col = m, n != m  ERROR
// case 2': n_row = 1, n_col = m, n = m   header confirmation {Integrate} OR label value with header {Detach}
// record processing strategy ?
// case 3': n_row > 1, n_col = m, n = m   n_row - 1 values {Integrate} OR assign n_row values {Detach}
// record processing strategy ?

// header runs vertically, values run horizontally
// order = Row
// case 1: n_col = 1, n_row = 1, n > 1   ERROR
// case 2: n_col = 1, n_row = 1, n = 1   header confirmation {Integrate} OR label value with header {Detach}, mode can be None
// case 3: n_col = 1, n_row = m, n != m  ERROR
// case 4: n_col = 1, n_row = m, n = m   header confirmation {Integrate} OR label value with header {Detach}
// case 5: n_col > 1, n_row = 1, n > 1   ERROR
// case 6: n_col > 1, n_row = 1, n = 1   n_col - 1 values {Integrate} OR assign n_col values {Detach}
// case 7: n_col > 1, n_row = m, n != m  ERROR
// case 8: n_col > 1, n_row = m, n = m   n_col - 1 values {Integrate} OR assign n_col values {Detach}
//
// simplifies to
// case 1': n_col = k, n_row = m, n != m  ERROR
// case 2': n_col = 1, n_row = m, n = m   header confirmation {Integrate} OR label value with header {Detach}
// record processing strategy ?
// case 3': n_col > 1, n_row = m, n = m   n_row - 1 values {Integrate} OR assign n_row values {Detach}
// record processing strategy ?
//
void QueryHelper::setHeader(const QStringList &header)
{
    m_header = header;
    if(!m_header.isEmpty())
    {
        int n = m_header.size();
        if(Order::None == m_order)
        {
            // a single value or row of values is being retrieved in one read
            //
            if(1 < n_row || n != n_col)
            {
                qDebug() << "ERROR: invalid header for single row of data in columns";
                throw std::runtime_error("invalid header for single row of data in columns");
                return;
            }
        }
        else if(Order::Column == m_order)
        {
            if(n != n_col)
            {
              qDebug() << "ERROR: invalid row header for data in columns";
              throw std::runtime_error("invalid row header for data in columns");
              return;
            }
        }
        else if(Order::Row == m_order)
        {
            if(n != n_row)
            {
              qDebug() << "ERROR: invalid column header for data in rows";
              throw std::runtime_error("invalid column header for data in rows");
              return;
            }
        }
    }
    else
        qDebug() << "WARNING: possible header empty or invalid read order";
}

// for Excel files and QSqlQuery SELECT, data is returned
// in a block n_col x n_row, row by row
// ODBC does not support QSqlQuery size() to determine number of rows
// the first row is always interpreted as a list of field names
//
void QueryHelper::processQuery()
{
  QSqlRecord r = m_query.record();
  if(n_col != r.count())
  {
      qDebug() << "ERROR: data recovery failed for query" << m_query.lastQuery();
  }
  QVector<QVector<QVariant>> data;
  QVector<QVariant> head;
  for(int i=0;i<r.count();i++)
    head << r.fieldName(i);
  data << head;

  if(n_col != head.size())
  {
    qDebug() << "ERROR: data recovery failed for query" << m_query.lastQuery();
  }

  if(1 < n_row)
  {
    while(m_query.next())
    {
      r = m_query.record();
      QVector<QVariant> row;
      for(int i=0;i<r.count();i++)
        row << r.field(i).value();
      data << row;

      if(n_col != row.size())
      {
        qDebug() << "ERROR: data recovery failed for query" << m_query.lastQuery();
      }
    }
  }

  // dump the data
  int j = 1;
  for(auto&& row : data)
  {
      int k = 1;
      for(auto&& col : row )
      {
          qDebug() << "["<<QString::number(j)<<","<<QString::number(k++)<<"]:"<<col.toString();
      }
      j++;
  }

  //
}
