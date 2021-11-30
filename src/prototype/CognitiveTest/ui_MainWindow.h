/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *idGroupBox;
    QHBoxLayout *horizontalLayout;
    QLabel *iDlabel;
    QLineEdit *barcodeLineEdit;
    QSpacerItem *horizontalSpacer;
    QGroupBox *pathGgroupBox;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *openButton;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *measureGroupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *measureButton;
    QPushButton *saveButton;
    QPushButton *closeButton;
    QSpacerItem *horizontalSpacer_3;
    QTableView *testdataTableView;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(600, 389);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        idGroupBox = new QGroupBox(centralwidget);
        idGroupBox->setObjectName(QString::fromUtf8("idGroupBox"));
        sizePolicy.setHeightForWidth(idGroupBox->sizePolicy().hasHeightForWidth());
        idGroupBox->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(idGroupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        iDlabel = new QLabel(idGroupBox);
        iDlabel->setObjectName(QString::fromUtf8("iDlabel"));

        horizontalLayout->addWidget(iDlabel);

        barcodeLineEdit = new QLineEdit(idGroupBox);
        barcodeLineEdit->setObjectName(QString::fromUtf8("barcodeLineEdit"));
        barcodeLineEdit->setMaxLength(15);
        barcodeLineEdit->setCursorPosition(15);
        barcodeLineEdit->setClearButtonEnabled(true);

        horizontalLayout->addWidget(barcodeLineEdit);

        horizontalSpacer = new QSpacerItem(80, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_2->addWidget(idGroupBox);

        pathGgroupBox = new QGroupBox(centralwidget);
        pathGgroupBox->setObjectName(QString::fromUtf8("pathGgroupBox"));
        sizePolicy.setHeightForWidth(pathGgroupBox->sizePolicy().hasHeightForWidth());
        pathGgroupBox->setSizePolicy(sizePolicy);
        horizontalLayout_2 = new QHBoxLayout(pathGgroupBox);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        openButton = new QPushButton(pathGgroupBox);
        openButton->setObjectName(QString::fromUtf8("openButton"));

        horizontalLayout_2->addWidget(openButton);

        horizontalSpacer_2 = new QSpacerItem(469, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_2->addWidget(pathGgroupBox);

        measureGroupBox = new QGroupBox(centralwidget);
        measureGroupBox->setObjectName(QString::fromUtf8("measureGroupBox"));
        verticalLayout = new QVBoxLayout(measureGroupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        measureButton = new QPushButton(measureGroupBox);
        measureButton->setObjectName(QString::fromUtf8("measureButton"));

        horizontalLayout_3->addWidget(measureButton);

        saveButton = new QPushButton(measureGroupBox);
        saveButton->setObjectName(QString::fromUtf8("saveButton"));

        horizontalLayout_3->addWidget(saveButton);

        closeButton = new QPushButton(measureGroupBox);
        closeButton->setObjectName(QString::fromUtf8("closeButton"));

        horizontalLayout_3->addWidget(closeButton);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_3);

        testdataTableView = new QTableView(measureGroupBox);
        testdataTableView->setObjectName(QString::fromUtf8("testdataTableView"));

        verticalLayout->addWidget(testdataTableView);


        verticalLayout_2->addWidget(measureGroupBox);

        MainWindow->setCentralWidget(centralwidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
#if QT_CONFIG(shortcut)
        iDlabel->setBuddy(barcodeLineEdit);
#endif // QT_CONFIG(shortcut)

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Cognitive Test", nullptr));
        idGroupBox->setTitle(QCoreApplication::translate("MainWindow", "barcode", nullptr));
        iDlabel->setText(QCoreApplication::translate("MainWindow", "Participant ID:", nullptr));
        barcodeLineEdit->setInputMask(QCoreApplication::translate("MainWindow", "0 0 0 0 0 0 0 0;_", nullptr));
        barcodeLineEdit->setText(QString());
        pathGgroupBox->setTitle(QCoreApplication::translate("MainWindow", "executable", nullptr));
        openButton->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        measureGroupBox->setTitle(QCoreApplication::translate("MainWindow", "measure", nullptr));
        measureButton->setText(QCoreApplication::translate("MainWindow", "Run", nullptr));
        saveButton->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
        closeButton->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
