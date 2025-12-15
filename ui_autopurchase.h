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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AutoPurchase
{
public:
    QWidget *centralWidget;
    QLabel *labelLoading;
    QLabel *labelStatus;
    QPushButton *btnSubmit;
    QWidget *widget;
    QVBoxLayout *verticalLayout_3;
    QComboBox *comboExcel;
    QComboBox *comboRequester;
    QPushButton *btnLoadJob;
    QWidget *widget1;
    QVBoxLayout *verticalLayout_4;
    QDateEdit *dateEditRequest;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_2;
    QLineEdit *lineSearchMaterial;
    QLabel *labelSearchResult;
    QPushButton *btnSearchMaterial;
    QWidget *widget2;
    QVBoxLayout *verticalLayout;
    QPushButton *btnClearManual;
    QPushButton *btnClear;
    QSplitter *splitter;
    QListWidget *listWidgetChecklist;
    QTableWidget *tableWidgetParts;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *AutoPurchase)
    {
        if (AutoPurchase->objectName().isEmpty())
            AutoPurchase->setObjectName(QStringLiteral("AutoPurchase"));
        AutoPurchase->resize(1153, 805);
        centralWidget = new QWidget(AutoPurchase);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        labelLoading = new QLabel(centralWidget);
        labelLoading->setObjectName(QStringLiteral("labelLoading"));
        labelLoading->setGeometry(QRect(980, 195, 91, 61));
        labelLoading->setAlignment(Qt::AlignCenter);
        labelStatus = new QLabel(centralWidget);
        labelStatus->setObjectName(QStringLiteral("labelStatus"));
        labelStatus->setGeometry(QRect(669, 110, 16, 16));
        btnSubmit = new QPushButton(centralWidget);
        btnSubmit->setObjectName(QStringLiteral("btnSubmit"));
        btnSubmit->setGeometry(QRect(980, 130, 75, 23));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(330, 10, 150, 77));
        verticalLayout_3 = new QVBoxLayout(widget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        comboExcel = new QComboBox(widget);
        comboExcel->setObjectName(QStringLiteral("comboExcel"));

        verticalLayout_3->addWidget(comboExcel);

        comboRequester = new QComboBox(widget);
        comboRequester->setObjectName(QStringLiteral("comboRequester"));

        verticalLayout_3->addWidget(comboRequester);

        btnLoadJob = new QPushButton(widget);
        btnLoadJob->setObjectName(QStringLiteral("btnLoadJob"));

        verticalLayout_3->addWidget(btnLoadJob);

        widget1 = new QWidget(centralWidget);
        widget1->setObjectName(QStringLiteral("widget1"));
        widget1->setGeometry(QRect(560, 20, 220, 71));
        verticalLayout_4 = new QVBoxLayout(widget1);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(0, 0, 0, 0);
        dateEditRequest = new QDateEdit(widget1);
        dateEditRequest->setObjectName(QStringLiteral("dateEditRequest"));

        verticalLayout_4->addWidget(dateEditRequest);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        lineSearchMaterial = new QLineEdit(widget1);
        lineSearchMaterial->setObjectName(QStringLiteral("lineSearchMaterial"));

        verticalLayout_2->addWidget(lineSearchMaterial);

        labelSearchResult = new QLabel(widget1);
        labelSearchResult->setObjectName(QStringLiteral("labelSearchResult"));

        verticalLayout_2->addWidget(labelSearchResult);


        horizontalLayout->addLayout(verticalLayout_2);

        btnSearchMaterial = new QPushButton(widget1);
        btnSearchMaterial->setObjectName(QStringLiteral("btnSearchMaterial"));

        horizontalLayout->addWidget(btnSearchMaterial);


        verticalLayout_4->addLayout(horizontalLayout);

        widget2 = new QWidget(centralWidget);
        widget2->setObjectName(QStringLiteral("widget2"));
        widget2->setGeometry(QRect(960, 270, 135, 54));
        verticalLayout = new QVBoxLayout(widget2);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        btnClearManual = new QPushButton(widget2);
        btnClearManual->setObjectName(QStringLiteral("btnClearManual"));

        verticalLayout->addWidget(btnClearManual);

        btnClear = new QPushButton(widget2);
        btnClear->setObjectName(QStringLiteral("btnClear"));

        verticalLayout->addWidget(btnClear);

        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setGeometry(QRect(100, 110, 741, 331));
        splitter->setOrientation(Qt::Horizontal);
        listWidgetChecklist = new QListWidget(splitter);
        listWidgetChecklist->setObjectName(QStringLiteral("listWidgetChecklist"));
        splitter->addWidget(listWidgetChecklist);
        tableWidgetParts = new QTableWidget(splitter);
        tableWidgetParts->setObjectName(QStringLiteral("tableWidgetParts"));
        splitter->addWidget(tableWidgetParts);
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
        labelLoading->setText(QApplication::translate("AutoPurchase", "l", Q_NULLPTR));
        labelStatus->setText(QString());
        btnSubmit->setText(QApplication::translate("AutoPurchase", "Submit", Q_NULLPTR));
        comboExcel->setCurrentText(QString());
        btnLoadJob->setText(QApplication::translate("AutoPurchase", "Automatic Robot Parts Select", Q_NULLPTR));
        lineSearchMaterial->setPlaceholderText(QApplication::translate("AutoPurchase", "Enter Material Id", Q_NULLPTR));
        labelSearchResult->setText(QString());
        btnSearchMaterial->setText(QApplication::translate("AutoPurchase", "Search", Q_NULLPTR));
        btnClearManual->setText(QApplication::translate("AutoPurchase", "Clear Manual search items", Q_NULLPTR));
        btnClear->setText(QApplication::translate("AutoPurchase", "All Clear", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AutoPurchase: public Ui_AutoPurchase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTOPURCHASE_H
