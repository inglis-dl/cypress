#ifndef BPMCOMMUNICATION_H
#define BPMCOMMUNICATION_H

#include <QObject>
#include <QDateTime>
#include <QQueue>
#include <QTime>
#include <QUsb>
#include <functional>

QT_FORWARD_DECLARE_CLASS(BPMMessage)
QT_FORWARD_DECLARE_CLASS(QHidDevice)

class BPMCommunication: public QObject
{
	Q_OBJECT

public:
	explicit BPMCommunication(QObject* parent = Q_NULLPTR);
    ~BPMCommunication();

    QString serialNumber() const {return m_serialNumber;};
    QString manufacturer() const {return m_manufacturer;};
    QString product() const {return m_product;};
    QString version() const {return m_version;};

public slots:
    void connect(const QUsb::Id&);
    void disconnect();
	void measure();
    void abort(QThread*);

signals:
    void connectionStatus(const bool&);
    void deviceInfoReady();
	void measurementReady(const int& sbp, const int& dbp, const int& pulse, const QDateTime& start, const QDateTime& end, const int& readingNum);
	void averageReady(const int& sbp, const int& dbp, const int& pulse);
	void finalReviewReady(const int& sbp, const int& dbp, const int& pulse);
    void measurementError(const QString& error, const int& timeout=0);
    void abortFinished(const bool& successful);

private:
	bool attemptConnectionToBpm();
	bool startBpm();
	bool stopBpm();
	bool cycleBpm();
	bool clearBpm();
	bool reviewBpm();
	bool handshakeBpm();
    bool timedWriteBpm(const quint8& msgId, const quint8& data0,
                       const quint8& data1 = 0x00,
                       const quint8& data2 = 0x00,
                       const quint8& data3 = 0x00);
	void readFromBpm();

    bool timedLoop(const int& timeout, const std::function<bool()>& func, const QString& debugName = "");
    bool timedReadLoop(const int& timeout, const std::function<bool()>& func, const QString& debugName = "");
    bool ackCheck(const int& expectedData0, const QString& logName);
	void resetValues();

    QHidDevice* m_bpm200  { Q_NULLPTR };
    QQueue<BPMMessage>* m_msgQueue { Q_NULLPTR };

	// The vid and pid for connecting to the bpm
    QUsb::Id m_info;

	void startReading();
	void endReading(const int& sbp, const int& dbp, const int& pulse);
    QDateTime m_readingStartTime { QDateTime::fromMSecsSinceEpoch(0) };

	// The last reported cycle time
    int m_cycleTime { -1 };

	// The last reported cuff pressure
    int m_cuffPressure { -1 };

	// The last reported reading number
	//int m_readingNumber = -1;
    int m_readingNumberCalc { 0 };

	// The last result code
	//int m_resultCode = -1;

	// True if the bpm is currently taking measurements
    bool m_measuring { false };

    bool m_aborted { false };

    bool m_stopUnexpectedReceived { false };

	QTime m_lastBpmMessageTime = QTime::currentTime();

    QString m_serialNumber;
    QString m_version;
    QString m_manufacturer;
    QString m_product;
};

#endif //BPMCOMMUNICATION_H
