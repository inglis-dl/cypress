#ifndef QUERYHELPER_H
#define QUERYHELPER_H

#include <QObject>
#include <QSqlQuery>

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

class QueryHelper
{
    Q_OBJECT

    enum Order {
        None,
        Row,
        Column
    };
    Q_ENUM(Order)

public:
    QueryHelper(const QString&,
                const QString&,
                const QString&,
                const QStringList&,
                const QueryHelper::Order&
                 );

private:

    int columnToIndex(const QString&);
    void buildQuery(const QSqlDatabase&);
    void processQuery();

    int n_row;
    int n_col;

    QString m_cellStart;
    QString m_cellEnd;
    QString m_sheet;
    QStringList m_header;
    Order m_order;

    QSqlQuery m_query;
};

#endif // QUERYHELPTER_H
