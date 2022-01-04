#ifndef MANAGERBASE_H
#define MANAGERBASE_H

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class ManagerBase : public QObject
{
    Q_OBJECT

public:
    explicit ManagerBase(QObject *parent = nullptr);

    // load and save device, paths and other constant settings to .ini
    //
    virtual void loadSettings(const QSettings &) = 0;
    virtual void saveSettings(QSettings*) const = 0;

    // the ini file group heading for derived manager classes
    //
    void setGroup(const QString& group) { m_group = group.toLower(); }
    QString getGroup() const { return m_group; }

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose() const { return m_verbose; }

    void setMode(const QString& mode) { m_mode = mode; }
    QString mode() const { return m_mode; }

    // collate test results and device and other meta data
    // for the main application to write to .json
    //
    virtual QJsonObject toJsonObject() const = 0;

    // build a model for UI display of the test results etc.
    //
    virtual void buildModel(QStandardItemModel *) const = 0;

public slots:

    virtual void measure() = 0;
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

protected:

    bool m_verbose;

    // mode of operation
    // - "simulate" - no devices are connected and the manager
    // responds to the UI signals and slots as though in live mode with valid
    // device and test data
    // - "live" - production mode
    //
    QString m_mode;

    // Context dependent clear test data and possibly device data (eg., serial port info)
    // SerialPortManager class clears device data during setDevice() while
    // test data is cleared depending on derived class implementation requirements.
    // Derived classes may also clear test data depending on the nature of the test,
    // such as when multiple measurements are separately acquired.
    //
    virtual void clearData() = 0;

    // the group name for a manager to write settings into
    //
    QString m_group;
};

#endif // MANAGERBASE_H
