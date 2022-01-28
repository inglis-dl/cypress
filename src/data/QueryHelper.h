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

    enum HeaderMode {
        Detach,
        Integrate
    };

    QueryHelper(const QString&,
                const QString&,
                const QString&);

    bool buildQuery(const QSqlDatabase&);
    void processQuery();

    void setHeader(const QStringList&);
    void setOrder(const QueryHelper::Order&);
    void setHeaderMode(const QueryHelper::HeaderMode&);

private:

    int columnToIndex(const QString&);

    int n_row;
    int n_col;

    QString m_cellStart;
    QString m_cellEnd;
    QString m_sheet;
    QStringList m_header;
    Order m_order;
    HeaderMode m_mode;

    QSqlQuery m_query;    
    QJsonObject m_output;
};

#endif // QUERYHELPTER_H
