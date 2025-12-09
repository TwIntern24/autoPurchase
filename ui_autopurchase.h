/********************************************************************************
** Form generated from reading UI file 'autopurchase.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTOPURCHASE_H
#define UI_AUTOPURCHASE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AutoPurchase
{
public:
    QWidget *centralWidget;
    QListWidget *listWidgetChecklist;
    QComboBox *comboExcel;
    QLabel *labelLoading;
    QLabel *labelStatus;
    QTableWidget *tableWidgetParts;
    QComboBox *comboRequester;
    QDateEdit *dateEditRequest;
    QPushButton *btnLoadJob;
    QLineEdit *lineSearchMaterial;
    QPushButton *btnSearchMaterial;
    QLabel *labelSearchResult;
    QPushButton *btnClearManual;
    QPushButton *btnSubmit;
    QPushButton *btnClear;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *AutoPurchase)
    {
        if (AutoPurchase->objectName().isEmpty())
            AutoPurchase->setObjectName(QStringLiteral("AutoPurchase"));
        AutoPurchase->resize(1153, 805);
        centralWidget = new QWidget(AutoPurchase);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        listWidgetChecklist = new QListWidget(centralWidget);
        listWidgetChecklist->setObjectName(QStringLiteral("listWidgetChecklist"));
        listWidgetChecklist->setGeometry(QRect(220, 160, 481, 281));
        comboExcel = new QComboBox(centralWidget);
        comboExcel->setObjectName(QStringLiteral("comboExcel"));
        comboExcel->setGeometry(QRect(370, 10, 121, 22));
        labelLoading = new QLabel(centralWidget);
        labelLoading->setObjectName(QStringLiteral("labelLoading"));
        labelLoading->setGeometry(QRect(350, 110, 171, 41));
        labelLoading->setAlignment(Qt::AlignCenter);
        labelStatus = new QLabel(centralWidget);
        labelStatus->setObjectName(QStringLiteral("labelStatus"));
        labelStatus->setGeometry(QRect(540, 120, 131, 20));
        tableWidgetParts = new QTableWidget(centralWidget);
        tableWidgetParts->setObjectName(QStringLiteral("tableWidgetParts"));
        tableWidgetParts->setGeometry(QRect(180, 480, 591, 231));
        comboRequester = new QComboBox(centralWidget);
        comboRequester->setObjectName(QStringLiteral("comboRequester"));
        comboRequester->setGeometry(QRect(370, 50, 131, 22));
        dateEditRequest = new QDateEdit(centralWidget);
        dateEditRequest->setObjectName(QStringLiteral("dateEditRequest"));
        dateEditRequest->setGeometry(QRect(560, 20, 110, 22));
        btnLoadJob = new QPushButton(centralWidget);
        btnLoadJob->setObjectName(QStringLiteral("btnLoadJob"));
        btnLoadJob->setGeometry(QRect(360, 80, 161, 23));
        lineSearchMaterial = new QLineEdit(centralWidget);
        lineSearchMaterial->setObjectName(QStringLiteral("lineSearchMaterial"));
        lineSearchMaterial->setGeometry(QRect(560, 70, 111, 20));
        btnSearchMaterial = new QPushButton(centralWidget);
        btnSearchMaterial->setObjectName(QStringLiteral("btnSearchMaterial"));
        btnSearchMaterial->setGeometry(QRect(700, 70, 75, 23));
        labelSearchResult = new QLabel(centralWidget);
        labelSearchResult->setObjectName(QStringLiteral("labelSearchResult"));
        labelSearchResult->setGeometry(QRect(580, 91, 81, 21));
        btnClearManual = new QPushButton(centralWidget);
        btnClearManual->setObjectName(QStringLiteral("btnClearManual"));
        btnClearManual->setGeometry(QRect(690, 100, 141, 23));
        btnSubmit = new QPushButton(centralWidget);
        btnSubmit->setObjectName(QStringLiteral("btnSubmit"));
        btnSubmit->setGeometry(QRect(410, 730, 75, 23));
        btnClear = new QPushButton(centralWidget);
        btnClear->setObjectName(QStringLiteral("btnClear"));
        btnClear->setGeometry(QRect(810, 400, 75, 23));
        AutoPurchase->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(AutoPurchase);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        AutoPurchase->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(AutoPurchase);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        AutoPurchase->setStatusBar(statusBar);

        retranslateUi(AutoPurchase);

        QMetaObject::connectSlotsByName(AutoPurchase);
    } // setupUi

    void retranslateUi(QMainWindow *AutoPurchase)
    {
        AutoPurchase->setWindowTitle(QApplication::translate("AutoPurchase", "AutoPurchase", Q_NULLPTR));
        comboExcel->setCurrentText(QString());
        labelLoading->setText(QApplication::translate("AutoPurchase", "l", Q_NULLPTR));
        labelStatus->setText(QString());
        btnLoadJob->setText(QApplication::translate("AutoPurchase", "Automatic Robot Parts Select", Q_NULLPTR));
        lineSearchMaterial->setPlaceholderText(QApplication::translate("AutoPurchase", "Enter Material Id", Q_NULLPTR));
        btnSearchMaterial->setText(QApplication::translate("AutoPurchase", "Search", Q_NULLPTR));
        labelSearchResult->setText(QString());
        btnClearManual->setText(QApplication::translate("AutoPurchase", "Clear Manual search items", Q_NULLPTR));
        btnSubmit->setText(QApplication::translate("AutoPurchase", "Submit", Q_NULLPTR));
        btnClear->setText(QApplication::translate("AutoPurchase", "All Clear", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AutoPurchase: public Ui_AutoPurchase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTOPURCHASE_H
