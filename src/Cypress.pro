QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport bluetooth sql designer

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    data/MeasurementBase.cpp \
    data/HearingTest.cpp \
    data/BodyCompositionTest.cpp \
    data/CDTTTest.cpp \
    data/FraxTest.cpp \
    data/WeighScaleTest.cpp \
    data/ChoiceReactionTest.cpp \
    data/TemperatureTest.cpp \
    data/HearingMeasurement.cpp \
    data/BodyCompositionMeasurement.cpp \
    data/CDTTMeasurement.cpp \
    data/FraxMeasurement.cpp \
    data/WeightMeasurement.cpp \
    data/ChoiceReactionMeasurement.cpp \
    data/TemperatureMeasurement.cpp \
    dialogs/CypressDialog.cpp \
    managers/ManagerBase.cpp \
    managers/AudiometerManager.cpp \
    managers/ChoiceReactionManager.cpp \
    managers/SerialPortManager.cpp \
    managers/BluetoothLEManager.cpp \
    managers/FraxManager.cpp \
    managers/CDTTManager.cpp \
    managers/WeighScaleManager.cpp \
    managers/BodyCompositionAnalyzerManager.cpp \
    CypressApplication.cpp \
    main.cpp 

HEADERS += \
    data/MeasurementBase.h \
    data/TestBase.h \
    data/HearingTest.h \
    data/BodyCompositionTest.h \
    data/CDTTTest.h \
    data/FraxTest.h \
    data/WeighScaleTest.h \
    data/ChoiceReactionTest.h \
    data/TemperatureTest.h \
    data/HearingMeasurement.h \
    data/BodyCompositionMeasurement.h \
    data/CDTTMeasurement.h \
    data/FraxMeasurement.h \
    data/WeightMeasurement.h \
    data/ChoiceReactionMeasurement.h \
    data/TemperatureMeasurement.h \
    dialogs/CypressDialog.h \
    managers/ManagerBase.h \
    managers/AudiometerManager.h \
    managers/ChoiceReactionManager.h \
    managers/SerialPortManager.h \
    managers/BluetoothLEManager.h \
    managers/FraxManager.h \
    managers/BodyCompositionAnalyzerManager.h \
    managers/CDTTManager.h \
    managers/WeighScaleManager.h \
    CypressApplication.h

TRANSLATIONS += \
    Cypress_en_CA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS +=

RESOURCES += \
    dialogs/cypress.qrc
