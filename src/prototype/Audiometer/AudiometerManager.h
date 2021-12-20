#ifndef AUDIOMETERMANAGER_H
#define AUDIOMETERMANAGER_H

#include "../../managers/SerialPortManager.h"
#include "../../data/AudiometerTest.h"

class AudiometerManager : public SerialPortManager
{
    Q_OBJECT

public:
    explicit AudiometerManager(QObject *parent = nullptr);

    QJsonObject toJsonObject() const override;

    void buildModel(QStandardItemModel *) const override;

public slots:

    void measure() override;

private slots:

    // retrieve data from the audiometer over RS232
    // emits canWrite signal if the test data is valid
    //
    void readDevice() override;

    void writeDevice() override;

private:

    bool hasEndCode(const QByteArray &);

    AudiometerTest m_test;

    void clearData() override;
};

#endif // AUDIOMETERMANAGER_H
