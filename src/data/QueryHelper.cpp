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
        const QString& _cellStart,
        const QString& _cellEnd,
        const QString& _sheet = "Sheet1",
        const QStringList& _header = QStringList(),
        const QueryHelper::Order& _order = QueryHelper::Order::None) :
  m_cellStart(_cellStart),
  m_cellEnd(_cellEnd),
  m_sheet(_sheet),
  m_header(_header),
  m_order(_order)
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

  // check if the header is provided otherwise we must assume a simple
  // block of cell values of the same type is being requested
  // if we are providing a header, then we expect these to act as
  // verification keys that should be contained in the first row or
  // colmun of the block of cells
  // if we want to verify that the header matches a row or column
  // of cells, then the header size must equal the size of one or the other
  // with the non matching dimension = 1
  //
  if(!m_header.isEmpty())
  {
      int n = m_header.size();

      // column order or row order ?
      // size has to equal either n_row or n_col
      if(n != n_row && n != n_col)
      {
          qDebug() << "ERROR: incorrect header size";
          throw std::runtime_error("incorrect header size");
          return;
      }

      // the order must be specified correctly
      // header runs horizontally values run vertically ||
      // header runs vertically values run horizontally
      //
      if((Order::Column == m_order && n != n_col) ||
         (Order::Row == m_order && n != n_row))
      {
          qDebug() << "ERROR: incorrect header for data order";
          throw std::runtime_error("incorrect header for data order");
          return;
      }
  }
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
};

void QueryHelper::processQuery()
{
   qDebug() << "order requested is" <<
    (Order::None==m_order? "none" : (Order::Row==m_order ? "row":"column"));

   if(1 == n_row)
   {
       QSqlRecord r = m_query.record();
       int n_val = r.count();
       if(n_col != n_val)
       {
           qDebug() << "ERROR: expecting "<< QString::number(n_col) << "received" << QString::number(n_val);
           throw std::runtime_error("ERROR: incorrect record size");
       }
       qDebug() << "processing 1 row containing" << QString::number(n_val) << "values";
       for(int i=0;i<n_val;i++)
           qDebug() << "value" << QString::number(i+1) << r.value(i).toString();

       if(m_header.isEmpty())
       {
           // just return the values
       }
       else
       {
           // if the header size is 1
           //
           // if the header size is n_val
           // create
       }
       // case 1 : 1 x 1
       //      1.1: order = none - return the value
       //      1.2: order = row - return the values
       //      1.3: order = column - return the values
       //
       // case 2 : 1 x m
       //      1.1: order = none
       //      1.2: order = row
       //      1.3: order = column
       //
   }
   else
   {
     int rnum = 1;
     do{
       QSqlRecord r = m_query.record();
       qDebug() << "record number"<< QString::number(rnum)<< "size" << QString::number(r.count());
       for(int i=0;i<r.count();i++)
           qDebug() << "value" << QString::number(i+1) << r.value(i).toString();
       rnum++;
     }while(m_query.next());

     // case 1 : n x 1
     //      1.1: order = none
     //      1.2: order = row
     //      1.3: order = column
     //
     // case 2 : n x m
     //      1.1: order = none
     //      1.2: order = row
     //      1.3: order = column
     //

   }
}

