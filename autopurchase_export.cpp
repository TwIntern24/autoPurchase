#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSpinBox>
#include <QTextStream>
#include <QSignalBlocker>

#include <QPrinter>
#include <QPainter>
#include <QFileDialog>
#include <QFontMetrics>

/*
#include <algorithm>

// QXlsx
#include "xlsxdocument.h"
#include "xlsxformat.h"


void AutoPurchase::on_btnSubmit_clicked()
{
    if (!ui->btnSubmit->isEnabled())
        return;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save parts list"),
        QDir::homePath() + "/Parts_List.xlsx",
        tr("Excel files (*.xlsx);;All files (*.*)")
    );
    if (fileName.isEmpty())
        return;

    if (!fileName.endsWith(".xlsx", Qt::CaseInsensitive))
        fileName += ".xlsx";

    const QString dateStr      = ui->dateEditRequest->date().toString("yyyy-MM-dd");
    const QString requesterStr = ui->comboRequester->currentText().trimmed();

    struct ExportRow {
        QString material;
        QString name;
        QString storage;
        int qty = 0;
    };

    QVector<ExportRow> exportRows;
    exportRows.reserve(ui->tableWidgetParts->rowCount());

    for (int r = 0; r < ui->tableWidgetParts->rowCount(); ++r) {
        QTableWidgetItem *matItem  = ui->tableWidgetParts->item(r, 0);
        QTableWidgetItem *nameItem = ui->tableWidgetParts->item(r, 1);
        QTableWidgetItem *storItem = ui->tableWidgetParts->item(r, 2);

        if (!matItem || !nameItem)
            continue;

        if (matItem->font().bold()) // skip group headers / subheaders
            continue;

        QString mat  = matItem->text().trimmed();
        QString name = nameItem->text().trimmed();
        QString stor = storItem ? storItem->text().trimmed() : QString();

        int qty = 0;
        if (QWidget *w = ui->tableWidgetParts->cellWidget(r, 3)) {
            if (auto *spin = qobject_cast<QSpinBox*>(w))
                qty = spin->value();
        }

        if (mat.isEmpty() || qty <= 0)
            continue;

        exportRows.push_back({mat, name, stor, qty});
    }

    std::sort(exportRows.begin(), exportRows.end(),
              [](const ExportRow &a, const ExportRow &b) {
        bool aHasStorage = !a.storage.trimmed().isEmpty() && a.storage.trimmed() != "-";
        bool bHasStorage = !b.storage.trimmed().isEmpty() && b.storage.trimmed() != "-";

        if (aHasStorage != bHasStorage)
            return aHasStorage;

        if (aHasStorage && bHasStorage) {
            if (a.storage != b.storage)
                return a.storage < b.storage;
        }
        return a.material < b.material;
    });

    QXlsx::Document xlsx;

    // Formats
    QXlsx::Format headerFmt;
    headerFmt.setFontBold(true);
    headerFmt.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    headerFmt.setVerticalAlignment(QXlsx::Format::AlignVCenter);

    QXlsx::Format textLeft;   // Material, Name, Storage
    textLeft.setHorizontalAlignment(QXlsx::Format::AlignHLeft);
    textLeft.setVerticalAlignment(QXlsx::Format::AlignVCenter);

    QXlsx::Format intCenter;  // Quantity
    intCenter.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    intCenter.setVerticalAlignment(QXlsx::Format::AlignVCenter);

    // Row 1 headers
    xlsx.write(1, 1, "Date", headerFmt);
    xlsx.write(1, 2, "Requester", headerFmt);
    xlsx.write(1, 3, "Material ID", headerFmt);
    xlsx.write(1, 4, "Item Name", headerFmt);
    xlsx.write(1, 5, "Storage Location", headerFmt);
    xlsx.write(1, 6, "Quantity", headerFmt);

    // Column widths (minimum-ish)
    xlsx.setColumnWidth(1, 1, 12);  // Date
    xlsx.setColumnWidth(2, 2, 18);  // Requester
    xlsx.setColumnWidth(3, 3, 16);  // Material ID
    xlsx.setColumnWidth(4, 4, 40);  // Item Name
    xlsx.setColumnWidth(5, 5, 18);  // Storage
    xlsx.setColumnWidth(6, 6, 10);  // Qty

    // Write data starting from row 2
    int row = 2;
    for (const auto &r : exportRows) {
        xlsx.write(row, 1, dateStr, textLeft);
        xlsx.write(row, 2, requesterStr, textLeft);

        // IMPORTANT: force Material ID as TEXT so Excel doesn't auto-format
        xlsx.write(row, 3, r.material, textLeft);

        xlsx.write(row, 4, r.name, textLeft);
        xlsx.write(row, 5, r.storage, textLeft);
        xlsx.write(row, 6, r.qty, intCenter);

        ++row;
    }

    if (!xlsx.saveAs(fileName)) {
        QMessageBox::warning(this, tr("Export"), tr("Failed to save:\n%1").arg(fileName));
        return;
    }

    QMessageBox::information(this,
        tr("Export"),
        tr("Export finished.\nFile saved as:\n%1").arg(fileName));
}
*/

void AutoPurchase::on_btnSubmit_clicked()
{
    if (!ui->btnSubmit->isEnabled())
        return;

    QString robotNumber, robotName;
    if (m_autoInfoAvailable) {
        robotNumber = m_autoRobotNumber;
        robotName   = m_autoRobotName;
    }

    QString baseName = "Parts_List";
    if (!robotName.isEmpty())
        baseName = robotName + robotNumber;

    //QString baseName = "Parts_List";
    //if (m_autoInfoAvailable)
        //baseName = m_autoRobotName;

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save parts list"),
        QDir::homePath() + "/" + baseName + ".csv",
        tr("CSV files (*.csv);;All files (*.*)")
    );


    /*
    QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Save parts list"),
                QDir::homePath() + "/Parts_List.csv",
                tr("CSV files (*.csv);;All files (*.*)"));
    */

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
        QString robot;
        QString material;
        QString name;
        QString storage;
        int     qty = 0;
    };

    QVector<ExportRow> exportRows;
    exportRows.reserve(ui->tableWidgetParts->rowCount());

    for (int r = 0; r < ui->tableWidgetParts->rowCount(); ++r) {
        QTableWidgetItem *robotNameItem = ui->tableWidgetParts->item(r, 3);
        QTableWidgetItem *matItem  = ui->tableWidgetParts->item(r, 0);
        QTableWidgetItem *nameItem = ui->tableWidgetParts->item(r, 1);
        QTableWidgetItem *storItem = ui->tableWidgetParts->item(r, 2);

        // skip rows that don't have a normal material cell
        if (!matItem || nameItem == nullptr)
            continue;

        // skip group headers / subheaders (bold font)
        if (matItem->font().bold())
            continue;

        QString robotNaming  = robotNameItem->text().trimmed();
        QString mat  = matItem->text().trimmed();
        QString name = nameItem ? nameItem->text().trimmed() : QString();
        QString stor = storItem ? storItem->text().trimmed() : QString();

        // quantity from spinbox
        int qty = 0;
        if (QWidget *w = ui->tableWidgetParts->cellWidget(r, 4)) {
            if (auto *spin = qobject_cast<QSpinBox*>(w))
                qty = spin->value();
        }

        // skip empty / zero-qty rows
        if (mat.isEmpty() || qty <= 0)
            continue;

        ExportRow row;
        row.robot = robotNaming;
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
    //out << "Date,Requester,Material ID,Item Name,Storage Location,Quantity\n";
    out << "Robot Number,Material ID,Item Name,Storage Location,Quantity,Requester,Date\n";


    for (const ExportRow &row : exportRows) {
        out << csv(row.robot)      << ','
            << csv(row.material)   << ','
            << csv(row.name)       << ','
            << csv(row.storage)    << ','
            << row.qty             << ','
            << csv(requesterStr)   << ','
            << csv(dateStr)        << '\n';
    }
    /*
    for (const ExportRow &row : exportRows) {
        out << csv(dateStr)        << ','
            << csv(requesterStr)   << ','
            << csv(row.material)   << ','
            << csv(row.name)       << ','
            << csv(row.storage)    << ','
            << row.qty             << '\n';
    }
    */

    file.close();

    QMessageBox::information(this,
                             tr("Export"),
                             tr("Export finished.\nFile saved as:\n%1").arg(fileName));
}


/*
void AutoPurchase::exportPdfFromCurrentTable()
{
    // 1) Collect rows exactly like your CSV export
    struct ExportRow {
        QString material;
        QString name;
        QString storage;
        int qty = 0;
    };

    QVector<ExportRow> exportRows;
    exportRows.reserve(ui->tableWidgetParts->rowCount());

    for (int r = 0; r < ui->tableWidgetParts->rowCount(); ++r) {
        QTableWidgetItem *matItem  = ui->tableWidgetParts->item(r, 0);
        QTableWidgetItem *nameItem = ui->tableWidgetParts->item(r, 1);
        QTableWidgetItem *storItem = ui->tableWidgetParts->item(r, 2);

        if (!matItem || !nameItem) continue;
        if (matItem->font().bold()) continue; // skip group/sub headers

        QString mat  = matItem->text().trimmed();
        QString name = nameItem->text().trimmed();
        QString stor = storItem ? storItem->text().trimmed() : QString();

        int qty = 0;
        if (QWidget *w = ui->tableWidgetParts->cellWidget(r, 3)) {
            if (auto *spin = qobject_cast<QSpinBox*>(w))
                qty = spin->value();
        }

        if (mat.isEmpty() || qty <= 0) continue;

        exportRows.push_back({mat, name, stor, qty});
    }

    if (exportRows.isEmpty()) {
        QMessageBox::information(this, tr("Export PDF"), tr("No rows to export."));
        return;
    }

    // 2) Choose file
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save parts list (PDF)"),
        QDir::homePath() + "/Parts_List.pdf",
        tr("PDF files (*.pdf);;All files (*.*)")
    );
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pdf", Qt::CaseInsensitive)) fileName += ".pdf";

    // 3) Printer setup
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOrientation(QPageLayout::Landscape);
    printer.setPageMargins(QMarginsF(12, 12, 12, 12));

    QPainter p;
    if (!p.begin(&printer)) {
        QMessageBox::warning(this, tr("Export PDF"), tr("Cannot write PDF."));
        return;
    }

    // 4) Header info
    const QString dateStr      = ui->dateEditRequest->date().toString("yyyy-MM-dd");
    const QString requesterStr = ui->comboRequester->currentText().trimmed();

    QRect page = printer.pageRect(QPrinter::DevicePixel);
    int left = page.left(), top = page.top(), right = page.right(), bottom = page.bottom();

    QFont headerFont = p.font();
    headerFont.setBold(true);

    QFont normalFont = p.font();
    normalFont.setBold(false);

    QFontMetrics fm(normalFont);
    const int rowH    = fm.height() + 10;
    const int headerH = rowH + 4;

    // Column widths (tweak if needed)
    const int wMat  = 170;
    const int wStor = 160;
    const int wQty  = 80;
    const int wName = (right - left + 1) - (wMat + wStor + wQty);

    const int xMat  = left;
    const int xName = xMat + wMat;
    const int xStor = xName + wName;
    const int xQty  = xStor + wStor;

    auto drawPageHeader = [&]() -> int {
        int y = top;

        // Title + meta
        p.setFont(headerFont);
        p.drawText(QRect(left, y, right-left+1, headerH),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Parts List"));

        y += headerH;

        p.setFont(normalFont);
        p.drawText(QRect(left, y, right-left+1, headerH),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   tr("Date: %1    Requester: %2").arg(dateStr, requesterStr));

        y += headerH;

        // Table header
        p.setFont(headerFont);

        // light gray header background
        p.fillRect(QRect(left, y, right-left+1, headerH), QColor(230,230,230));
        p.setPen(Qt::black);

        p.drawRect(QRect(xMat,  y, wMat,  headerH));
        p.drawRect(QRect(xName, y, wName, headerH));
        p.drawRect(QRect(xStor, y, wStor, headerH));
        p.drawRect(QRect(xQty,  y, wQty,  headerH));

        p.drawText(QRect(xMat+6,  y, wMat-12,  headerH), Qt::AlignLeft  | Qt::AlignVCenter, tr("Material ID"));
        p.drawText(QRect(xName+6, y, wName-12, headerH), Qt::AlignLeft  | Qt::AlignVCenter, tr("Item Name"));
        p.drawText(QRect(xStor+6, y, wStor-12, headerH), Qt::AlignLeft  | Qt::AlignVCenter, tr("Storage Location"));
        p.drawText(QRect(xQty,    y, wQty,     headerH), Qt::AlignCenter,                  tr("Qty"));

        p.setFont(normalFont);
        return y + headerH;
    };

    int y = drawPageHeader();

    auto newPage = [&]() {
        printer.newPage();
        y = drawPageHeader();
    };

    // 5) Draw rows (multi-page)
    p.setFont(normalFont);

    for (const ExportRow &r : exportRows) {
        if (y + rowH > bottom) {
            newPage();
        }

        // borders
        p.drawRect(QRect(xMat,  y, wMat,  rowH));
        p.drawRect(QRect(xName, y, wName, rowH));
        p.drawRect(QRect(xStor, y, wStor, rowH));
        p.drawRect(QRect(xQty,  y, wQty,  rowH));

        // text (elide long names)
        QString name = fm.elidedText(r.name, Qt::ElideRight, wName - 12);

        p.drawText(QRect(xMat+6,  y, wMat-12,  rowH), Qt::AlignLeft  | Qt::AlignVCenter, r.material);
        p.drawText(QRect(xName+6, y, wName-12, rowH), Qt::AlignLeft  | Qt::AlignVCenter, name);
        p.drawText(QRect(xStor+6, y, wStor-12, rowH), Qt::AlignLeft  | Qt::AlignVCenter, r.storage);
        p.drawText(QRect(xQty,    y, wQty,     rowH), Qt::AlignCenter | Qt::AlignVCenter, QString::number(r.qty));

        y += rowH;
    }

    p.end();

    QMessageBox::information(this, tr("Export PDF"),
                             tr("PDF exported.\nFile saved as:\n%1").arg(fileName));
}
*/

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
    resetAutoRobotInfo();
    //updateRobotColumnsVisibility();
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
