#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QDebug>


void AutoPurchase::onExcelSelectionChanged(int index)
{
    m_checklistReady = false;

    QString path = ui->comboExcel->itemData(index).toString();

    if (path.isEmpty()) {
        ui->listWidgetChecklist->hide();
        ui->listWidgetChecklist->clear();
        return;
    }


    ui->listWidgetChecklist->hide();

    ui->labelLoading->show();
    m_loadingMovie->start();

    ui->labelStatus->setText("Loading Excel…");

    emit startExcelLoad(path);   // runs in worker thread
}


void AutoPurchase::onExcelLoaded(const QVariantList &rows)
{
    m_loadingMovie->stop();
    ui->labelLoading->hide();

    ui->labelStatus->setText("Building checklist…");
    qApp->processEvents();

    buildChecklistFromExcelRows(rows);

    ui->labelStatus->setText("Done");
    ui->listWidgetChecklist->show();

    m_checklistReady = true;
    tryApplyJobToChecklist();

}

void AutoPurchase::onExcelLoadFailed(const QString &error)
{
    m_loadingMovie->stop();
    ui->labelLoading->hide();

    ui->labelStatus->setText("Error: " + error);
    //qDebug() << "Excel load failed:" << error
               // << "target:" << (m_currentLoadTarget == LoadMaterials ? "Materials" : "Matrix");
}


void AutoPurchase::onMaterialsLoaded(const QVariantList &rows)
{
    m_rowsMaterials = rows;
    qDebug() << "Materials rows loaded:" << m_rowsMaterials.size();
}

void AutoPurchase::onMaterialsLoadFailed(const QString &error)
{
    qDebug() << "Materials Excel load failed:" << error;
}

void AutoPurchase::onArmUpgradeLoaded(const QVariantList &rows)
{
    m_rowsArmUpgrade = rows;
    qDebug() << "ARM upgrade rows loaded:" << m_rowsArmUpgrade.size();
}

void AutoPurchase::onArmUpgradeLoadFailed(const QString &error)
{
    qDebug() << "ARM upgrade Excel load failed:" << error;
}

void AutoPurchase::onDmUpgradeLoaded(const QVariantList &rows)
{
    m_rowsDmUpgrade = rows;
    qDebug() << "DM Lika upgrade rows loaded:" << m_rowsDmUpgrade.size();
}

void AutoPurchase::onDmUpgradeLoadFailed(const QString &error)
{
    qDebug() << "DM Lika Excel load failed:" << error;
}
