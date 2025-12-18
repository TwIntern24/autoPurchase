#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextStream>
#include <QSignalBlocker>

void AutoPurchase::on_btnSubmit_clicked()
{
    if (!ui->btnSubmit->isEnabled())
        return;

    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save parts list"),
                QDir::homePath() + "/Parts_List.csv",
                tr("CSV files (*.csv);;All files (*.*)"));
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".csv", Qt::CaseInsensitive))
        fileName += ".csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             tr("Error"),
                             tr("Cannot open file:\n%1").arg(fileName));
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    const QString dateStr      = ui->dateEditRequest->date().toString("yyyy-MM-dd");
    const QString requesterStr = ui->comboRequester->currentText().trimmed();

    // helper for CSV escaping
    auto csv = [](const QString &s) {
        QString t = s;
        t.replace('"', "\"\"");     // escape "
        return "\"" + t + "\"";     // wrap in ""
    };

    // -----------------------------
    // 1) Collect all real data rows
    // -----------------------------
    struct ExportRow {
        QString material;
        QString name;
        QString storage;
        int     qty = 0;
    };

    QVector<ExportRow> exportRows;
    exportRows.reserve(ui->tableWidgetParts->rowCount());

    for (int r = 0; r < ui->tableWidgetParts->rowCount(); ++r) {
        QTableWidgetItem *matItem  = ui->tableWidgetParts->item(r, 0);
        QTableWidgetItem *nameItem = ui->tableWidgetParts->item(r, 1);
        QTableWidgetItem *storItem = ui->tableWidgetParts->item(r, 2);

        // skip rows that don't have a normal material cell
        if (!matItem || nameItem == nullptr)
            continue;

        // skip group headers / subheaders (bold font)
        if (matItem->font().bold())
            continue;

        QString mat  = matItem->text().trimmed();
        QString name = nameItem ? nameItem->text().trimmed() : QString();
        QString stor = storItem ? storItem->text().trimmed() : QString();

        // quantity from spinbox
        int qty = 0;
        if (QWidget *w = ui->tableWidgetParts->cellWidget(r, 3)) {
            if (auto *spin = qobject_cast<QSpinBox*>(w))
                qty = spin->value();
        }

        // skip empty / zero-qty rows
        if (mat.isEmpty() || qty <= 0)
            continue;

        ExportRow row;
        row.material = mat;
        row.name     = name;
        row.storage  = stor;
        row.qty      = qty;
        exportRows.push_back(row);
    }

    // -----------------------------
    // 2) Sort by storage location (then material)
    // -----------------------------
    /*
    std::sort(exportRows.begin(), exportRows.end(),
              [](const ExportRow &a, const ExportRow &b) {
        if (a.storage != b.storage)
            return a.storage < b.storage;   // primary: storage
        return a.material < b.material;     // secondary: material ID
    });
    */
    std::sort(exportRows.begin(), exportRows.end(),
              [](const ExportRow &a, const ExportRow &b) {

        bool aHasStorage = !a.storage.trimmed().isEmpty() && a.storage.trimmed() != "-";
        bool bHasStorage = !b.storage.trimmed().isEmpty() && b.storage.trimmed() != "-";

        // Case 1: Only one has storage → that one comes first
        if (aHasStorage != bHasStorage)
            return aHasStorage;   // true first

        // Case 2: Both have storage → sort alphabetically by storage
        if (aHasStorage && bHasStorage) {
            if (a.storage != b.storage)
                return a.storage < b.storage;
        }

        // Case 3: Both have no storage → order by material
        return a.material < b.material;
    });


    // -----------------------------
    // 3) Write header + sorted rows
    // -----------------------------
    out << "Date,Requester,Material Number,Item Name,Storage Location,Quantity\n";

    for (const ExportRow &row : exportRows) {
        out << csv(dateStr)        << ','
            << csv(requesterStr)   << ','
            << csv(row.material)   << ','
            << csv(row.name)       << ','
            << csv(row.storage)    << ','
            << row.qty             << '\n';
    }

    file.close();

    QMessageBox::information(this,
                             tr("Export"),
                             tr("Export finished.\nFile saved as:\n%1").arg(fileName));
}

void AutoPurchase::updateSubmitEnabled()
{
    bool hasRequester = (ui->comboRequester->currentIndex() > 0);

    // Check if there is at least one real data row (not just headers)
    bool hasParts = false;
    for (int r = 0; r < ui->tableWidgetParts->rowCount(); ++r) {
        QTableWidgetItem *matItem = ui->tableWidgetParts->item(r, 0);

        if (!matItem)
            continue;

        // Skip group headers (bold font)
        if (matItem->font().bold())
            continue;

        if (!matItem->text().trimmed().isEmpty()) {
            hasParts = true;
            break;
        }
    }

    // dateEditRequest always has a date, but if you want you can still check
    //bool hasDate = ui->dateEditRequest->date().isValid();

    ui->btnSubmit->setEnabled(hasRequester && hasParts);
}

void AutoPurchase::on_btnClear_clicked()
{
    // 1) Uncheck all checklist items & remove multipliers
    {
        QSignalBlocker block(ui->listWidgetChecklist);
        for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
            QListWidgetItem *it = ui->listWidgetChecklist->item(i);
            it->setCheckState(Qt::Unchecked);
            it->setData(Qt::UserRole + 1, QVariant());
        }
    }

    m_rebuildTimer.stop();
    m_checkedCount = 0;

    // 2) Clear *stored* manual parts
    m_manualParts.clear();            // <--- important

    // 3) Clear and hide table
    ui->tableWidgetParts->setRowCount(0);
    ui->tableWidgetParts->hide();

    // 4) Reset requester (date can stay)
    ui->comboRequester->setCurrentIndex(0);

    // 5) Optional: clear status
    ui->labelStatus->clear();

    // 6) Update submit button
    updateSubmitEnabled();
}

void AutoPurchase::on_btnClearManual_clicked()
{
    // 1) Clear stored manual parts and UI for search
    m_manualParts.clear();
    ui->lineSearchMaterial->clear();
    if (ui->labelSearchResult)
        ui->labelSearchResult->clear();

    // 2) Check if any checklist items are still checked
    bool anyChecked = false;
    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QListWidgetItem *it = ui->listWidgetChecklist->item(i);
        if (it->checkState() == Qt::Checked) {
            anyChecked = true;
            break;
        }
    }

    // 3) If nothing is checked, completely clear + hide the table
    if (!anyChecked) {
        ui->tableWidgetParts->setRowCount(0);
        ui->tableWidgetParts->hide();
        updateSubmitEnabled();
        return;
    }

    m_rebuildTimer.stop();
    // 4) Otherwise rebuild table from checklist only
    rebuildPartsFromChecklist();
    updateSubmitEnabled();
}
