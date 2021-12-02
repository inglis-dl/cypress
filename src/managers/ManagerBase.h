#ifndef MANAGERBASE_H
#define MANAGERBASE_H

#include <QObject>
#include <QStandardItemModel>

QT_FORWARD_DECLARE_CLASS(QSettings)

class ManagerBase : public QObject
{
    Q_OBJECT

public:
    explicit ManagerBase(QObject *parent = nullptr);

    virtual void loadSettings(const QSettings &) = 0;
    virtual void saveSettings(QSettings*) const = 0;

    void setVerbose(const bool& verbose) { m_verbose = verbose; }
    bool isVerbose() const { return m_verbose; }

    void setMode(const QString& mode) { m_mode = mode; }
    QString mode() { return m_mode; }

    virtual QJsonObject toJsonObject() const = 0;

    virtual void buildModel(QStandardItemModel *) const = 0;

public slots:

    virtual void measure() = 0;

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
    // such as when multiple measurements are seaprately acquired.
    //
    virtual void clearData() = 0;

};

#endif // MANAGERBASE_H
