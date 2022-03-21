#ifndef MANAGERBASE_H
#define MANAGERBASE_H

#include "../auxiliary/Constants.h"

#include <QJsonObject>
#include <QObject>
#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class ManagerBase : public QObject
{
    Q_OBJECT

public:
    explicit ManagerBase(QObject *parent = Q_NULLPTR);
    ~ManagerBase() = default;

    // load and save device, paths and other constant settings to .ini
    //
    virtual void loadSettings(const QSettings &) = 0;
    virtual void saveSettings(QSettings*) const = 0;

    // the settings file group heading for derived manager classes
    //
    void setGroup(const QString& group) { m_group = group.toLower(); }
    QString getGroup() const { return m_group; }

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose() const { return m_verbose; }

    void setRunMode(const Constants::RunMode& mode) { m_mode = mode; }
    Constants::RunMode getRunMode() const { return m_mode; }

    // collate test results and device and other meta data
    // for the main application to write to .json
    //
    virtual QJsonObject toJsonObject() const = 0;

    // build a model for UI display of the test results etc.
    //
    virtual void buildModel(QStandardItemModel *) const = 0;

    // get the required model columns and rows depending
    // on test output requirements
    //
    int getNumberOfModelColumns() const { return m_col; }
    int getNumberOfModelRows() const { return m_row; }

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    virtual void setInputData(const QJsonObject &) = 0;

    QVariant getInputDataValue(const QString &);

public slots:

    // subclasses call methods after main initialization just prior
    // to running (eg., emit dataChanged signal)
    //
    virtual void start() = 0;

    // actual measure will only execute if the barcode has been
    // verified.  Subclasses must reimplement accordingly.
    //
    virtual void measure() = 0;

    // subclasses call methods just prior to main close event
    //
    virtual void finish() = 0;

signals:

    // the underlying test data has changed
    //
    void dataChanged();

    // ready to measure and receive data
    // (update GUI enable measure button)
    //
    void canMeasure();

    // valid test completed and ready to write to output
    // (update GUI enable write button and update the results display)
    //
    void canWrite();

    // send a message to the UI status bar
    //
    void message(const QString &, const int &timeOut=0);

protected:

    bool m_verbose { true };

    // mode of operation
    // - "simulate" - no devices are connected and the manager
    // responds to the UI signals and slots as though in live mode with valid
    // device and test data
    // - "live" - production mode
    //
    Constants::RunMode m_mode { Constants::RunMode::modeUnknown };

    // Context dependent clear test data and possibly device data (eg., serial port info)
    // SerialPortManager class clears device data during setDevice() while
    // test data is cleared depending on derived class implementation requirements.
    // Derived classes may also clear test data depending on the nature of the test,
    // such as when multiple measurements are separately acquired.
    //
    virtual void clearData() = 0;

    // key value pairs sorted by key
    //
    QJsonObject m_inputData;

    // an ordered set of input keys
    //
    QList<QString> m_inputKeyList;

    int m_row { 0 };
    int m_col { 0 };

private:

    // the group name for a manager to write settings into
    // TODO: use MeasureType enum converted to string for group names
    //
    QString m_group;
};

#endif // MANAGERBASE_H
