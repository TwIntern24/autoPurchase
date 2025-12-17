#include "autopurchase.h"
#include "ui_autopurchase.h"


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
