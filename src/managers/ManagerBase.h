#ifndef MANAGERBASE_H
#define MANAGERBASE_H

#include <QObject>
#include <QWidget>
#include <QMap>
#include <QVariant>

QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

class ManagerBase : public QObject
{
    Q_OBJECT

public:
    explicit ManagerBase(QObject *parent = nullptr);
    ~ManagerBase() { if(!p_widget.isNull()) p_widget.clear(); };

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

    virtual void connectUI(QWidget *) = 0;

    // Set the input data.
    // The input data is read from the input
    // json file to the main application.  This method should be
    // used to filter the minimum inputs needed to run
    // a test.  Filtering keys are stored in member
    // m_inputKeyList.
    //
    virtual void setInputData(const QMap<QString,QVariant> &) = 0;

    QVariant getInputDataValue(const QString &);

public slots:

    // subclasses call methods after main initialization just prior
    // to running (eg., emit dataChanged signal)
    //
    virtual void start() = 0;

    // actual measure will only execute if the barcode has been
    // verified.  Subclasses must reimplement accordingly.
    //
    virtual void measure() { if(!m_validBarcode) return; }

    // subclasses call methods just prior to main close event
    //
    virtual void finish() = 0;

    // verify a barcode against the value held in m_inputData
    //
    bool verifyBarcode(const QString &);

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

    // locking mechanism: barcode must be verified before measuring
    //
    bool m_validBarcode;

    // Context dependent clear test data and possibly device data (eg., serial port info)
    // SerialPortManager class clears device data during setDevice() while
    // test data is cleared depending on derived class implementation requirements.
    // Derived classes may also clear test data depending on the nature of the test,
    // such as when multiple measurements are separately acquired.
    //
    virtual void clearData() = 0;

    QSharedPointer<QWidget> p_widget;

    QMap<QString,QVariant> m_inputData;
    QList<QString> m_inputKeyList;

private:

    // the group name for a manager to write settings into
    //
    QString m_group;

};

#endif // MANAGERBASE_H
