#ifndef JSONSETTINGS_H
#define JSONSETTINGS_H

#include <QtCore>

class JsonSettings {

public:
    static bool readSettingsJson(QIODevice &device, QVariantMap &map);
    static bool writeSettingsJson(QIODevice &device, const QVariantMap &map);

    static QSettings::Format JsonFormat;

private:
    static void paraseJsonObject(QJsonObject &json, QString prefix, QVariantMap &map);
    static QJsonObject restoreJsonObject(QVariantMap &map);
};

#endif // JSONSETTINGS_H
