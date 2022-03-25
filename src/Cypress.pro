QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport bluetooth sql usb xml

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    auxiliary/CommandLineParser.cpp \
    auxiliary/Constants.cpp \
    auxiliary/JsonSettings.cpp \
    auxiliary/Utilities.cpp \
    data/AccessQueryHelper.cpp \
    data/BloodPressureMeasurement.cpp \
    data/BloodPressureTest.cpp \
    data/BodyCompositionMeasurement.cpp \
    data/BodyCompositionTest.cpp \
    data/CDTTMeasurement.cpp \
    data/CDTTTest.cpp \
    data/ChoiceReactionMeasurement.cpp \
    data/ChoiceReactionTest.cpp \
    data/ECGMeasurement.cpp \
    data/ECGTest.cpp \
    data/ExcelQueryHelper.cpp \
    data/FraxMeasurement.cpp \
    data/FraxTest.cpp \
    data/HearingMeasurement.cpp \
    data/HearingTest.cpp \
    data/Measurement.cpp \
#    data/TemperatureMeasurement.cpp \
#    data/TemperatureTest.cpp \
    data/TonometerMeasurement.cpp \
    data/TonometerTest.cpp \
    data/WeightMeasurement.cpp \
    data/WeighScaleTest.cpp \
    managers/AudiometerManager.cpp \
    managers/CDTTManager.cpp \
    managers/ChoiceReactionManager.cpp \
    managers/BodyCompositionManager.cpp \
#    managers/BluetoothLEManager.cpp \
    managers/BloodPressureManager.cpp \
    managers/BPMCommunication.cpp \
    managers/BPMMessage.cpp \
    managers/ECGManager.cpp \
    managers/FraxManager.cpp \
    managers/ManagerBase.cpp \
    managers/SerialPortManager.cpp \
    managers/TonometerManager.cpp \
    managers/WeighScaleManager.cpp \
    widgets/BarcodeWidget.cpp \
    dialogs/AudiometerDialog.cpp \
    dialogs/BloodPressureDialog.cpp \
    dialogs/BodyCompositionDialog.cpp \
    dialogs/CDTTDialog.cpp \
    dialogs/ChoiceReactionDialog.cpp \
    dialogs/DialogBase.cpp \
    dialogs/DialogFactory.cpp \
    dialogs/FraxDialog.cpp \
    dialogs/ECGDialog.cpp \
#    dialogs/ThermometerDialog.cpp \
    dialogs/TonometerDialog.cpp \
    dialogs/WeighScaleDialog.cpp \
    CypressApplication.cpp \
    main.cpp 

HEADERS += \
    auxiliary/CommandLineParser.h \
    auxiliary/Constants.h \
    auxiliary/JsonSettings.h \
    auxiliary/Utilities.h \
    data/AccessQueryHelper.h \
    data/BloodPressureMeasurement.h \
    data/BloodPressureTest.h \
    data/BodyCompositionMeasurement.h \
    data/BodyCompositionTest.h \
    data/CDTTMeasurement.h \
    data/CDTTTest.h \
    data/ChoiceReactionMeasurement.h \
    data/ChoiceReactionTest.h \
    data/ECGMeasurement.h \
    data/ECGTest.h \
    data/ExcelQueryHelper.h \
    data/FraxMeasurement.h \
    data/FraxTest.h \
    data/HearingMeasurement.h \
    data/HearingTest.h \
    data/Measurement.h \
    data/TestBase.h \
    data/TonometerMeasurement.h \
    data/TonometerTest.h \
#    data/TemperatureMeasurement.h \
#    data/TemperatureTest.h \
    data/WeightMeasurement.h \
    data/WeighScaleTest.h \
    managers/AudiometerManager.h \
    managers/BloodPressureManager.h \
#    managers/BluetoothLEManager.h \
    managers/BodyCompositionManager.h \
    managers/BPMCommunication.h \
    managers/BPMMessage.h \
    managers/CDTTManager.h \
    managers/ChoiceReactionManager.h \
    managers/ECGManager.h \
    managers/FraxManager.h \
    managers/ManagerBase.h \
    managers/SerialPortManager.h \
    managers/TonometerManager.h \
    managers/WeighScaleManager.h \
    widgets/BarcodeWidget.h \
    dialogs/AudiometerDialog.h \
    dialogs/BloodPressureDialog.h \
    dialogs/BodyCompositionDialog.h \
    dialogs/CDTTDialog.h \
    dialogs/ChoiceReactionDialog.h \
    dialogs/DialogBase.h \
    dialogs/DialogFactory.h \
    dialogs/ECGDialog.h \
    dialogs/FraxDialog.h \
#    dialogs/ThermometerDialog.h \
    dialogs/TonometerDialog.h \
    dialogs/WeighScaleDialog.h \
    CypressApplication.h

TRANSLATIONS += \
    Cypress_en_CA.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
  widgets/barcodewidget.ui \
  dialogs/audiometerdialog.ui \
  dialogs/bloodpressuredialog.ui \
  dialogs/bodycompositiondialog.ui \
#  dialogs/thermometerdialog.ui \
  dialogs/runnabledialog.ui \
  dialogs/weighscaledialog.ui

RESOURCES += \
