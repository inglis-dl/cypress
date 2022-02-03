#ifndef XLSXQUERYHELPER_H
#define XLSXQUERYHELPER_H

#include <QObject>
#include <QSqlQuery>
#include <QJsonObject>

/*!
 * \class QueryHelper
 * \brief Helper class to query MS Excel files
 *
 * MS Excel files can be queried over ODBC driver
 * using the basic SELECT * FROM [SHEETNAME$CELL_j:CELL_k]
 * SQL statement.  The sheet name must be specified
 * and defaults to "Sheet1".  Cell ranges (eg, A1:BA3)
 * can be specified in lower or upper case and in any order.
 * Internally the class capitalizes starting and ending cells
 * such that the order is logical and satisfies the query
 * statement constraints.
 * Depending on the number of rows, columns and the optional
 * specification of a sting list of (row or column) header labels, data can
 * be retrieved and interpreted in json format (QJsonObject).
 *
 * Caveats:
 * The data Order must be set before setting a header.
 * The sheet name must be set before building the query.
 *
 * \sa CDTTTest
 *
 */

QT_FORWARD_DECLARE_CLASS(QSqlDatabase)

class XLSXQueryHelper
{
public:

    enum Order {
        Row,
        Column
    };

    enum HeaderMode {
        Detached,
        Integrated
    };

    XLSXQueryHelper(const QString&,
                const QString&,
                const QString&);
    XLSXQueryHelper(const XLSXQueryHelper &other);
    XLSXQueryHelper& operator=(const XLSXQueryHelper &other);

    // Build a QSqlQuery from the provided cell range and sheet name
    // and query the provided QSqlDatabase.  The database connection must be
    // established and verified before calling.
    //
    bool buildQuery(const QSqlDatabase&);

    // Process the data from the QSqlQuery based on order and optional
    // header and header mode.
    //
    void processQuery();

    // Set the sheet name.  An empty sheet name defaults to "Sheet1"
    // used in the SELECT query.
    //
    void setSheet(const QString&);

    // Set the keys for labelling the data and verifying against record fields
    // returned from the query.
    // CAVEAT: cell strings having "." character are mis-interpreted by the OSBC driver
    // and returned as "#" character (eg., "St. Dev." is returned as "St# Dev#").  Set
    // expected header keys accordingly.
    //
    void setHeader(const QStringList&);

    // Set the order to interpret the returned query data.
    // Column order implies data are stored in columns.  If a header
    // is supplied, then the first row of data is expected to match
    // the header keys with subsequent rows of data partitioned into columns.
    // Row order implies data are stored in rows.  If a header is supplied
    // then the first item of each row is expected to match a
    // header key with remaining row elements interpreted as data.
    //
    void setOrder(const XLSXQueryHelper::Order&);

    // Set the mode of the header keys.
    // In Detached mode the header is not part of the returned query data.
    // In Integrated mode the header is part of the returned query data
    // and a validation check is applied and provided in the json output.
    // HeaderMode has no effect if the supplied header is empty.
    //
    void setHeaderMode(const XLSXQueryHelper::HeaderMode&);

    // Output is stored in a QJsonObject which requires keys for
    // json values. When there is no supplied header, use the
    // prefix for generating n keys (eg., item_ for item_1, item_2 ... item_n).
    // The prefix (empty prefix) defaults to "column_" or "row_"
    // depending on the data order.  Numerical suffixes are zero-based.
    //
    void setPrefix(const QString &prefix) { m_prefix = prefix; }

    // The data from the query in json format.
    //
    const QJsonObject& getOutput() { return m_output; }

private:

    void initialize();
    int columnToIndex(const QString&);

    int n_row;
    int n_col;
    QString m_cellStart;
    QString m_cellEnd;
    QString m_sheet;
    QStringList m_header;
    QString m_prefix;
    Order m_order;
    HeaderMode m_mode;
    QSqlQuery m_query;
    QJsonObject m_output;
};

#endif // XLSXQUERYHELPER_H
