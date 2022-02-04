#include "ExcelQueryHelper.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDriver>
#include <QJsonArray>

#include <stdexcept>

ExcelQueryHelper::ExcelQueryHelper(
        const QString& cellStart,
        const QString& cellEnd,
        const QString& sheet = "Sheet1") :
  m_cellStart(cellStart),
  m_cellEnd(cellEnd),
  m_sheet(sheet),
  m_order(Order::Column),
  m_mode(HeaderMode::Integrated)
{
  initialize();
}

ExcelQueryHelper::ExcelQueryHelper(const ExcelQueryHelper &other)
{
  m_cellStart = other.m_cellStart;
  m_cellEnd = other.m_cellEnd;
  m_sheet = other.m_sheet;
  m_order = other.m_order;
  m_mode = other.m_mode;
  m_header = other.m_header;
  initialize();
}

ExcelQueryHelper& ExcelQueryHelper::operator=(const ExcelQueryHelper &other)
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

void ExcelQueryHelper::initialize()
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

void ExcelQueryHelper::setSheet(const QString &sheet)
{
    m_sheet = sheet.isEmpty() ? "Sheet1" : sheet;
}

inline int ExcelQueryHelper::columnToIndex(const QString &s)
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

void ExcelQueryHelper::setOrder(const ExcelQueryHelper::Order &order)
{
  m_order = order;
}

void ExcelQueryHelper::setHeaderMode(const ExcelQueryHelper::HeaderMode &mode)
{
  m_mode = mode;
}

bool ExcelQueryHelper::buildQuery(const QSqlDatabase &db)
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
    qDebug() << "driver"<<db.driverName()<<"supports unicode strings" <<
      (db.driver()->hasFeature(QSqlDriver::Unicode)?"YES":"NO");

    return ok;
}

// the order must be specified correctly
// header runs horizontally, values run vertically
// order = Column
// case 1: n_row = 1, n_col = 1, n > 1   ERROR
// case 2: n_row = 1, n_col = 1, n = 1   header confirmation {Integrated} OR label value with header {Detached}, mode can be None
// case 3: n_row = 1, n_col = m, n != m  ERROR
// case 4: n_row = 1, n_col = m, n = m   header confirmation {Integrated} OR label value with header {Detached}
// case 5: n_row > 1, n_col = 1, n > 1   ERROR
// case 6: n_row > 1, n_col = 1, n = 1   n_row - 1 values {Integrated} OR assign n_row values {Detached}
// case 7: n_row > 1, n_col = m, n != m  ERROR
// case 8: n_row > 1, n_col = m, n = m   n_row - 1 values {Integrated} OR assign n_row values {Detached}
//
// simplifies to
// case 1': n_row = k, n_col = m, n != m  ERROR
// case 2': n_row = 1, n_col = m, n = m   header confirmation {Integrated} OR label value with header {Detached}
// record processing strategy ?
// case 3': n_row > 1, n_col = m, n = m   n_row - 1 values {Integrated} OR assign n_row values {Detached}
// record processing strategy ?
//
// header runs vertically, values run horizontally
// order = Row
// case 1: n_col = 1, n_row = 1, n > 1   ERROR
// case 2: n_col = 1, n_row = 1, n = 1   header confirmation {Integrated} OR label value with header {Detached}, mode can be None
// case 3: n_col = 1, n_row = m, n != m  ERROR
// case 4: n_col = 1, n_row = m, n = m   header confirmation {Integrated} OR label value with header {Detached}
// case 5: n_col > 1, n_row = 1, n > 1   ERROR
// case 6: n_col > 1, n_row = 1, n = 1   n_col - 1 values {Integrated} OR assign n_col values {Detached}
// case 7: n_col > 1, n_row = m, n != m  ERROR
// case 8: n_col > 1, n_row = m, n = m   n_col - 1 values {Integrated} OR assign n_col values {Detached}
//
// simplifies to
// case 1': n_col = k, n_row = m, n != m  ERROR
// case 2': n_col = 1, n_row = m, n = m   header confirmation {Integrated} OR label value with header {Detached}
// record processing strategy ?
// case 3': n_col > 1, n_row = m, n = m   n_row - 1 values {Integrated} OR assign n_row values {Detached}
// record processing strategy ?
//
void ExcelQueryHelper::setHeader(const QStringList &header)
{
    m_header = header;
    if(!m_header.isEmpty())
    {
        int n = m_header.size();
        if(Order::Column == m_order)
        {
            if(n != n_col)
            {
              qDebug() << "ERROR: invalid row header for data in columns";
              throw std::runtime_error("invalid row header for data in columns");
              return;
            }
        }
        else // row order
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

// For Excel files and QSqlQuery SELECT, data is returned
// in a block sized n_col x n_row, row by row.
// ODBC does not support QSqlQuery size() to determine number of rows.
// For multiple rows, the first row is interpreted as a list of field names.
// If a requested cell range contains fewer rows of populated cells
// the query ceases to provide records past the last filled row.
//
void ExcelQueryHelper::processQuery()
{
  QSqlRecord r = m_query.record();
  if(n_col != r.count())
  {
      qDebug() << "ERROR: data recovery failed for query" << m_query.lastQuery();
  }
  QVector<QVector<QVariant>> data;
  QVector<QVariant> head;
  QVector<QVariant::Type> field_type;
  for(int i=0;i<r.count();i++)
  {
    QSqlField f = r.field(i);
    qDebug() << QMetaType::typeName(f.type());
    field_type.push_back(f.type());
    head << f.name();
  }
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

  m_output = QJsonObject();
  if(!m_header.isEmpty())
  {
    // header is not within the query data
    if(HeaderMode::Detached == m_mode)
    {
        if(Order::Column == m_order)
        {
          if(1 == n_row) // store header key QJsonValue pairs
          {
            int i = 0;
            for(auto&& col : head)
              m_output.insert(m_header.at(i++),col.toJsonValue());
          }
          else // store header key QJsonArray pairs
          {
            QVector<QJsonArray> arr(n_col);
            for(auto&& row : data)
            {
              QJsonArray *p = arr.data();
              for(auto&& col : row )
              {
                p->push_back(col.toJsonValue());
                p++;
              }
            }
            for(int i=0;i<n_col;i++)
               m_output.insert(m_header.at(i),arr.value(i));
          }
        }
        else // row order
        {
          int i = 0;
          for(auto&& row : data)
          {
            QJsonArray arr;
            for(auto&& col : row)
               arr.push_back(QJsonValue::fromVariant(col));

             m_output.insert(m_header.at(i++),(1 == arr.size() ? arr.first() : arr));
           }
        }
    }
    else
    {
      // header is contained within the query data
      // add an additional key value pair indicating
      // header validity
      //
      bool valid = true;
      if(Order::Column == m_order) // first row of data is the header ?
      {
        for(int i=0;i<n_col;i++)
        {
          valid = m_header.at(i) == head.at(i).toString();
          if(!valid)
            break;
        }
        if(valid && 1 < n_row)
        {
          QVector<QJsonArray> arr(n_col);
          for(auto&& row : data)
          {
            if(head == row) continue;
            QJsonArray *p = arr.data();
            for(auto&& col : row )
            {
              p->push_back(QJsonValue::fromVariant(col));
              p++;
            }
          }
          for(int i=0;i<n_col;i++)
          {
            QJsonArray json = arr.value(i);
            m_output.insert(m_header.at(i),(1 == json.size() ?  json.first() : json));
          }
        }
      }
      else // row order
      {
        if(1 == n_row)
        {
          valid = m_header.first() == head.first().toString();
          if(valid && 1 < n_col)
          {
            QJsonArray arr;
            for(int i=1;i<n_col;i++)
              arr.push_back(head.at(i).toJsonValue());

            m_output.insert(m_header.first(),(1 == arr.size() ? arr.first() : arr));
          }
        }
        else
        {
          int i = 0;
          for(auto&& row : data)
          {
            valid = m_header.at(i++) == row.first().toString();
            if(!valid)
              break;
          }
          if(valid && 1 < n_col)
          {
            i = 0;
            for(auto&& row : data)
            {
              QJsonArray arr;
              for(auto&& col : row)
              {
                if(col == row.first()) continue;
                arr.push_back(col.toJsonValue());
              }
              m_output.insert(m_header.at(i++),(1 == arr.size() ? arr.first() : arr));
            }
          }
        }
      }
      m_output.insert("header_valid",QJsonValue(valid));
    }
  }
  else // empty header, just fill in the data
  {
      // string keys are column or row numbers depending on order mode
      // data returned as single values or QJsonArrays as required
      //
      int sz = Order::Column == m_order ? n_col : n_row;
      QVector<QJsonArray> arr(sz);
      if(Order::Column == m_order)
      {
        for(auto&& row : data)
        {
          QJsonArray *p = arr.data();
          int i = 0;
          for(auto&& col : row)
          {
            col.convert(field_type.at(i++));
            p->push_back(col.toJsonValue());
            p++;
          }
        }
      }
      else
      {
        QJsonArray *p = arr.data();
        for(auto&& row : data)
        {
           int i = 0;
           for(auto&& col : row)
           {
             col.convert(field_type.at(i++));
             p->push_back(col.toJsonValue());
           }
           p++;
        }
      }
      int i = 0;
      if(m_prefix.isEmpty())
        m_prefix = Order::Column == m_order ? "column_" : "row_";
      for(auto&& json : arr)
        m_output.insert(m_prefix + QString::number(i++),(1 == json.size() ? json.first() : json));
  }
}
