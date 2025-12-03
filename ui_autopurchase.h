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
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
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
    QListWidget *listWidgetParts;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *AutoPurchase)
    {
        if (AutoPurchase->objectName().isEmpty())
            AutoPurchase->setObjectName(QStringLiteral("AutoPurchase"));
        AutoPurchase->resize(1153, 741);
        centralWidget = new QWidget(AutoPurchase);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        listWidgetChecklist = new QListWidget(centralWidget);
        listWidgetChecklist->setObjectName(QStringLiteral("listWidgetChecklist"));
        listWidgetChecklist->setGeometry(QRect(220, 130, 481, 281));
        comboExcel = new QComboBox(centralWidget);
        comboExcel->setObjectName(QStringLiteral("comboExcel"));
        comboExcel->setGeometry(QRect(370, 30, 121, 22));
        labelLoading = new QLabel(centralWidget);
        labelLoading->setObjectName(QStringLiteral("labelLoading"));
        labelLoading->setGeometry(QRect(350, 70, 171, 41));
        labelLoading->setAlignment(Qt::AlignCenter);
        labelStatus = new QLabel(centralWidget);
        labelStatus->setObjectName(QStringLiteral("labelStatus"));
        labelStatus->setGeometry(QRect(740, 103, 131, 20));
        listWidgetParts = new QListWidget(centralWidget);
        listWidgetParts->setObjectName(QStringLiteral("listWidgetParts"));
        listWidgetParts->setGeometry(QRect(265, 450, 391, 221));
        AutoPurchase->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(AutoPurchase);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1153, 21));
        AutoPurchase->setMenuBar(menuBar);
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
    } // retranslateUi

};

namespace Ui {
    class AutoPurchase: public Ui_AutoPurchase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTOPURCHASE_H
