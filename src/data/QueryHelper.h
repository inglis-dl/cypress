#ifndef QUERYHELPER_H
#define QUERYHELPER_H

#include <QObject>
#include <QSqlQuery>
#include <QJsonObject>

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

class QueryHelper
{
public:

    enum Order {
        None,
        Row,
        Column
    };

    QueryHelper(const QString&,
                const QString&,
                const QString&,
                const QStringList&,
                const QueryHelper::Order&
                 );

    bool buildQuery(const QSqlDatabase&);
    void processQuery();

private:

    int columnToIndex(const QString&);

    int n_row;
    int n_col;

    QString m_cellStart;
    QString m_cellEnd;
    QString m_sheet;
    QStringList m_header;
    Order m_order;

    QSqlQuery m_query;    
    QJsonObject m_output;
};

#endif // QUERYHELPTER_H
