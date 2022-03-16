#ifndef JSONSETTINGS_H
#define JSONSETTINGS_H

#include <QtCore>

class JsonSettings {

public:
    static bool readSettingsJson(QIODevice &device, QMap<QString, QVariant> &map);
    static bool writeSettingsJson(QIODevice &device, const QMap<QString, QVariant> &map);

    static QSettings::Format JsonFormat;

private:
    static void paraseJsonObject(QJsonObject &json, QString prefix, QMap<QString, QVariant> &map);
    static QJsonObject restoreJsonObject(QMap<QString, QVariant> &map);
};

#endif // JSONSETTINGS_H
