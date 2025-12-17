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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AutoPurchase
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QWidget *mainContainer;
    QVBoxLayout *verticalLayout_8;
    QWidget *topBar;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QComboBox *comboExcel;
    QComboBox *comboRequester;
    QPushButton *btnLoadJob;
    QGridLayout *gridLayout;
    QDateEdit *dateEditRequest;
    QLabel *labelSearchResult;
    QLineEdit *lineSearchMaterial;
    QPushButton *btnSearchMaterial;
    QWidget *loadingBar;
    QHBoxLayout *horizontalLayout_3;
    QLabel *labelLoading;
    QLabel *labelStatus;
    QWidget *contentArea;
    QHBoxLayout *horizontalLayout_5;
    QWidget *leftpanel;
    QVBoxLayout *verticalLayout_5;
    QListWidget *listWidgetChecklist;
    QWidget *rightpanel;
    QVBoxLayout *verticalLayout_6;
    QTableWidget *tableWidgetParts;
    QWidget *rightSideButtons;
    QVBoxLayout *verticalLayout_7;
    QVBoxLayout *verticalLayout;
    QPushButton *btnClearManual;
    QPushButton *btnClear;
    QWidget *bottomBar;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *btnSubmit;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *AutoPurchase)
    {
        if (AutoPurchase->objectName().isEmpty())
            AutoPurchase->setObjectName(QStringLiteral("AutoPurchase"));
        AutoPurchase->resize(1153, 805);
        AutoPurchase->setStyleSheet(QStringLiteral(""));
        centralWidget = new QWidget(AutoPurchase);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setStyleSheet(QStringLiteral(""));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        mainContainer = new QWidget(centralWidget);
        mainContainer->setObjectName(QStringLiteral("mainContainer"));
        mainContainer->setStyleSheet(QLatin1String("QWidget {\n"
"    background-color: #7cae7a;\n"
"    color: #000000;\n"
"    font-size: 15pt;\n"
"}\n"
"\n"
"QLineEdit, QComboBox, QDateEdit {\n"
"    background-color: #F8F8FF;\n"
"    border: 1px solid #3a3a3a;\n"
"    padding: 6px 8px;\n"
"}\n"
"\n"
"QLineEdit:focus, QComboBox:focus, QDateEdit:focus {\n"
"    border: 1px solid #4da3ff;\n"
"}\n"
"\n"
"QPushButton {\n"
"    background-color: #FFEBCD;\n"
"    border: 1px solid #3a3a3a;\n"
"    border-radius: 10px;\n"
"    padding: 7px 14px;\n"
"}\n"
"\n"
"QPushButton:hover {\n"
"    background-color: #D2691E;\n"
"}\n"
"\n"
"QPushButton:pressed {\n"
"    background-color: #3a3a3a;\n"
"}\n"
"\n"
"QPushButton:disabled {\n"
"    background-color: #888888;\n"
"    color: #000000;\n"
"}\n"
"\n"
"QListWidget {\n"
"    background-color: #91a55f;\n"
"    border: 1px solid #3a3a3a;\n"
"}\n"
"\n"
" QTableWidget {\n"
"    background-color: #5fa588;\n"
"    border: 1px solid #3a3a3a;\n"
"}\n"
"\n"
"QScrollBar {\n"
"    background: #6fa66e;\n"
"}\n"
"\n"
"QSpinBox{\n"
"    "
                        "background-color: #669984;\n"
"    border: 1px solid #3a3a3a;\n"
"}\n"
"\n"
"QComboBox QAbstractItemView {\n"
"    background-color: #495549;\n"
"    color: #ffffff;\n"
"    selection-background-color: #323a58;\n"
"    selection-color: white;\n"
"    border: 1px solid #444;\n"
"    outline: 0;\n"
"}"));
        verticalLayout_8 = new QVBoxLayout(mainContainer);
        verticalLayout_8->setSpacing(6);
        verticalLayout_8->setContentsMargins(11, 11, 11, 11);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        topBar = new QWidget(mainContainer);
        topBar->setObjectName(QStringLiteral("topBar"));
        topBar->setStyleSheet(QStringLiteral(""));
        horizontalLayout = new QHBoxLayout(topBar);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        comboExcel = new QComboBox(topBar);
        comboExcel->setObjectName(QStringLiteral("comboExcel"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboExcel->sizePolicy().hasHeightForWidth());
        comboExcel->setSizePolicy(sizePolicy);

        verticalLayout_3->addWidget(comboExcel);

        comboRequester = new QComboBox(topBar);
        comboRequester->setObjectName(QStringLiteral("comboRequester"));

        verticalLayout_3->addWidget(comboRequester);

        btnLoadJob = new QPushButton(topBar);
        btnLoadJob->setObjectName(QStringLiteral("btnLoadJob"));

        verticalLayout_3->addWidget(btnLoadJob);


        horizontalLayout->addLayout(verticalLayout_3);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        dateEditRequest = new QDateEdit(topBar);
        dateEditRequest->setObjectName(QStringLiteral("dateEditRequest"));
        dateEditRequest->setMinimumSize(QSize(0, 0));
        dateEditRequest->setMaximumSize(QSize(150, 16777215));
        dateEditRequest->setCalendarPopup(true);

        gridLayout->addWidget(dateEditRequest, 0, 0, 1, 1);

        labelSearchResult = new QLabel(topBar);
        labelSearchResult->setObjectName(QStringLiteral("labelSearchResult"));

        gridLayout->addWidget(labelSearchResult, 2, 0, 1, 1);

        lineSearchMaterial = new QLineEdit(topBar);
        lineSearchMaterial->setObjectName(QStringLiteral("lineSearchMaterial"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lineSearchMaterial->sizePolicy().hasHeightForWidth());
        lineSearchMaterial->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(lineSearchMaterial, 1, 0, 1, 1);

        btnSearchMaterial = new QPushButton(topBar);
        btnSearchMaterial->setObjectName(QStringLiteral("btnSearchMaterial"));
        sizePolicy1.setHeightForWidth(btnSearchMaterial->sizePolicy().hasHeightForWidth());
        btnSearchMaterial->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(btnSearchMaterial, 1, 1, 1, 1);


        horizontalLayout->addLayout(gridLayout);


        verticalLayout_8->addWidget(topBar);

        loadingBar = new QWidget(mainContainer);
        loadingBar->setObjectName(QStringLiteral("loadingBar"));
        loadingBar->setLayoutDirection(Qt::LeftToRight);
        loadingBar->setStyleSheet(QStringLiteral(""));
        horizontalLayout_3 = new QHBoxLayout(loadingBar);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        labelLoading = new QLabel(loadingBar);
        labelLoading->setObjectName(QStringLiteral("labelLoading"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(labelLoading->sizePolicy().hasHeightForWidth());
        labelLoading->setSizePolicy(sizePolicy2);
        labelLoading->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(labelLoading);

        labelStatus = new QLabel(loadingBar);
        labelStatus->setObjectName(QStringLiteral("labelStatus"));
        sizePolicy2.setHeightForWidth(labelStatus->sizePolicy().hasHeightForWidth());
        labelStatus->setSizePolicy(sizePolicy2);
        labelStatus->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(labelStatus);

        horizontalLayout_3->setStretch(0, 2);
        horizontalLayout_3->setStretch(1, 1);

        verticalLayout_8->addWidget(loadingBar);

        contentArea = new QWidget(mainContainer);
        contentArea->setObjectName(QStringLiteral("contentArea"));
        contentArea->setStyleSheet(QStringLiteral(""));
        horizontalLayout_5 = new QHBoxLayout(contentArea);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        leftpanel = new QWidget(contentArea);
        leftpanel->setObjectName(QStringLiteral("leftpanel"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(leftpanel->sizePolicy().hasHeightForWidth());
        leftpanel->setSizePolicy(sizePolicy3);
        leftpanel->setStyleSheet(QStringLiteral(""));
        verticalLayout_5 = new QVBoxLayout(leftpanel);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        listWidgetChecklist = new QListWidget(leftpanel);
        listWidgetChecklist->setObjectName(QStringLiteral("listWidgetChecklist"));
        listWidgetChecklist->setMinimumSize(QSize(0, 0));

        verticalLayout_5->addWidget(listWidgetChecklist);


        horizontalLayout_5->addWidget(leftpanel);

        rightpanel = new QWidget(contentArea);
        rightpanel->setObjectName(QStringLiteral("rightpanel"));
        sizePolicy3.setHeightForWidth(rightpanel->sizePolicy().hasHeightForWidth());
        rightpanel->setSizePolicy(sizePolicy3);
        rightpanel->setStyleSheet(QStringLiteral(""));
        verticalLayout_6 = new QVBoxLayout(rightpanel);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        tableWidgetParts = new QTableWidget(rightpanel);
        tableWidgetParts->setObjectName(QStringLiteral("tableWidgetParts"));
        tableWidgetParts->setMinimumSize(QSize(0, 0));
        tableWidgetParts->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableWidgetParts->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        tableWidgetParts->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        tableWidgetParts->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        tableWidgetParts->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_6->addWidget(tableWidgetParts);


        horizontalLayout_5->addWidget(rightpanel);

        rightSideButtons = new QWidget(contentArea);
        rightSideButtons->setObjectName(QStringLiteral("rightSideButtons"));
        rightSideButtons->setStyleSheet(QStringLiteral(""));
        verticalLayout_7 = new QVBoxLayout(rightSideButtons);
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setContentsMargins(11, 11, 11, 11);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        btnClearManual = new QPushButton(rightSideButtons);
        btnClearManual->setObjectName(QStringLiteral("btnClearManual"));

        verticalLayout->addWidget(btnClearManual);

        btnClear = new QPushButton(rightSideButtons);
        btnClear->setObjectName(QStringLiteral("btnClear"));

        verticalLayout->addWidget(btnClear);


        verticalLayout_7->addLayout(verticalLayout);


        horizontalLayout_5->addWidget(rightSideButtons);

        horizontalLayout_5->setStretch(0, 5);
        horizontalLayout_5->setStretch(1, 10);
        horizontalLayout_5->setStretch(2, 1);

        verticalLayout_8->addWidget(contentArea);

        bottomBar = new QWidget(mainContainer);
        bottomBar->setObjectName(QStringLiteral("bottomBar"));
        bottomBar->setStyleSheet(QStringLiteral(""));
        horizontalLayout_4 = new QHBoxLayout(bottomBar);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        btnSubmit = new QPushButton(bottomBar);
        btnSubmit->setObjectName(QStringLiteral("btnSubmit"));
        sizePolicy1.setHeightForWidth(btnSubmit->sizePolicy().hasHeightForWidth());
        btnSubmit->setSizePolicy(sizePolicy1);

        horizontalLayout_4->addWidget(btnSubmit);


        verticalLayout_8->addWidget(bottomBar);


        gridLayout_2->addWidget(mainContainer, 0, 0, 1, 1);

        AutoPurchase->setCentralWidget(centralWidget);
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
        btnLoadJob->setText(QApplication::translate("AutoPurchase", "Automatic Robot Parts Select", Q_NULLPTR));
        labelSearchResult->setText(QString());
        lineSearchMaterial->setPlaceholderText(QApplication::translate("AutoPurchase", "Enter Material Id", Q_NULLPTR));
        btnSearchMaterial->setText(QApplication::translate("AutoPurchase", "Search", Q_NULLPTR));
        labelLoading->setText(QApplication::translate("AutoPurchase", "l", Q_NULLPTR));
        labelStatus->setText(QString());
        btnClearManual->setText(QApplication::translate("AutoPurchase", "Clear Manual search items", Q_NULLPTR));
        btnClear->setText(QApplication::translate("AutoPurchase", "All Clear", Q_NULLPTR));
        btnSubmit->setText(QApplication::translate("AutoPurchase", "Submit", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AutoPurchase: public Ui_AutoPurchase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTOPURCHASE_H
