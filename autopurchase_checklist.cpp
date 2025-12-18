#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QTimer>
#include <QSignalBlocker>


void AutoPurchase::buildChecklistFromExcelRows(const QVariantList &rows)
{
    QSignalBlocker blocker(ui->listWidgetChecklist);
    ui->listWidgetChecklist->clear();
    if (rows.isEmpty())
        return;

    // get text from Excel cell
    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= rows.size()) return "";
        QVariantList cols = rows[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    bool inUpgradeSection = false;
    bool inUpgradeTable   = false;
    bool inVisualSection  = false;
    bool inAgeSection  = false;

    enum Sec { None, Arm, DM, ZM, ArmAfter3or5year, DMAfter3or5year, ZMAfter5year };
    Sec current = None;

    for (int r = 0; r < rows.size(); ++r)
    {
        // let UI update every 20 rows so the GIF animates
                if (r % 20 == 0) {
                    qApp->processEvents();
                }
        QString colA = getCol(r, 1);
        QString colB = getCol(r, 2);

        QString cleanA = colA.toLower().replace(" ", "");

        //-----------------------------
        // 1) ENTER UPGRADE SECTION
        //-----------------------------
        if (cleanA.startsWith("age"))
        {
            inAgeSection  = true;
            inVisualSection = false;
            inUpgradeSection = false;
            inUpgradeTable = false;
            current = None;
            continue;
        }

        if (cleanA.contains("lastdigit") && cleanA.contains("redmark"))
        {
            inUpgradeSection = true;
            inUpgradeTable = false;
            inVisualSection = false;
            inAgeSection  = false;
            current = None;
            continue;
        }

        //-----------------------------
        // 2) ENTER VISUAL SECTION
        //-----------------------------
        if (cleanA.startsWith("visualcheck"))
        {
            inVisualSection = true;
            inUpgradeSection = false;
            inUpgradeTable = false;
            inAgeSection  = false;
            current = None;
            continue;
        }

        //-----------------------------
        //  UPGRADE SUB-TABLE
        //-----------------------------
        if (inUpgradeSection)
        {
            // header row
            if (cleanA.startsWith("upgradepart"))
            {
                inUpgradeTable = true;
                continue;
            }

            // blank line closes upgrade section
            if (colA.isEmpty() && colB.isEmpty())
            {
                //inUpgradeSection = false;
                //inUpgradeTable = false;
                continue;
            }

            if (inUpgradeTable && !colA.isEmpty())
            {
                //QString label = QString("Upgrade: %1 (%2)").arg(colA, colB);
                QString label = QString("Upgrade: %1").arg(colA);
                auto *item = new QListWidgetItem(label, ui->listWidgetChecklist);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);

                item->setData(Qt::UserRole, colB);
            }

            continue;
        }

        //-----------------------------
        //  VISUAL CHECK SECTION
        //-----------------------------
        if (inVisualSection)
        {
            // detect subgroup headings using safe match
            if (cleanA == "arm")
            {
                current = Arm;
                continue;
            }

            if (cleanA.startsWith("drivemodule"))
            {
                current = DM;
                continue;
            }

            if (cleanA.startsWith("z-module") || cleanA.startsWith("zmodule"))
            {
                current = ZM;
                continue;
            }

            // blank line resets subgroup
            if (colA.isEmpty() && colB.isEmpty())
            {
                current = None;
                continue;
            }

            //---------------------------
            // add ARM/DM/ZM part item
            //---------------------------
            if (current != None && !colA.isEmpty())
            {
                QString prefix =
                    (current == Arm ? "Arm: " :
                     current == DM  ? "DM: "  :
                                      "ZM: ");

                QString label = prefix + colA;

                auto *item = new QListWidgetItem(label, ui->listWidgetChecklist);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);

                QString matExpr = getCol(r, 2);  // set correct column index
                item->setData(Qt::UserRole, matExpr);
            }

            continue;
        }

        if (inAgeSection)
        {
            //qDebug()<<cleanA;
            // detect subgroup headings using safe match
            if (cleanA == "armparts(after3,5years)")
            {
                current = ArmAfter3or5year;
                continue;
            }

            if (cleanA.startsWith("drivemodule(dm)parts(after3,5years)"))
            {
                current = DMAfter3or5year;
                continue;
            }

            if (cleanA.startsWith("z-strokemoduleparts(after5years)"))
            {
                current = ZMAfter5year;
                continue;
            }

            // blank line resets subgroup
            if (colA.isEmpty() && colB.isEmpty())
            {
                current = None;
                continue;
            }

            //---------------------------
            // add ARM/DM/ZM part item
            //---------------------------
            if (current != None && !colA.isEmpty())
            {
                QString prefix =
                    (current == ArmAfter3or5year ? "Arm After 3,5 year: " :
                     current == DMAfter3or5year  ? "DM After 3,5 year: "  :
                                      "ZM After 5 year: ");

                QString label = prefix + colA;

                auto *item = new QListWidgetItem(label, ui->listWidgetChecklist);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);

                QString matExpr = getCol(r, 2);  // set correct column index
                item->setData(Qt::UserRole, matExpr);
            }

            continue;
        }
    }
}

void AutoPurchase::onChecklistItemChanged(QListWidgetItem *item)
{
    if (!item) return;

    // update count incrementally (O(1))
    if (item->checkState() == Qt::Checked)
        ++m_checkedCount;
    else
        --m_checkedCount;

    //m_rebuildTimer.stop();

    const bool anyChecked = (m_checkedCount > 0);
    const bool hasManual  = !m_manualParts.isEmpty();

    if (!anyChecked) {
        if (hasManual) {
            rebuildPartsFromChecklist();
        } else {
            ui->tableWidgetParts->clearContents();
            ui->tableWidgetParts->setRowCount(0);
            ui->tableWidgetParts->hide();
            updateSubmitEnabled();
        }
        return;
    }

    // debounce rebuild
    m_rebuildTimer.stop();
    m_rebuildTimer.start();
}


void AutoPurchase::rebuildPartsFromChecklist()
{

    const bool anyChecked = (m_checkedCount > 0);
    const bool hasManual  = !m_manualParts.isEmpty();

    if (!anyChecked && !hasManual) {
        ui->tableWidgetParts->clearContents();
        ui->tableWidgetParts->setRowCount(0);
        ui->tableWidgetParts->hide();
        updateSubmitEnabled();
        return;
    }

    ui->tableWidgetParts->setUpdatesEnabled(false);
    ui->tableWidgetParts->setSortingEnabled(false);
    ui->tableWidgetParts->clearContents();
    ui->tableWidgetParts->setRowCount(0);

    QList<PartInfo> allParts;

    // -----------------------------
    // 1) Build from checked items
    //    (only if materials are loaded)
    // -----------------------------
    if (!m_rowsMaterials.isEmpty()) {
        QVector<QListWidgetItem*> checkedItems;
        checkedItems.reserve(ui->listWidgetChecklist->count());

        for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
            QListWidgetItem *it = ui->listWidgetChecklist->item(i);
            if (it->checkState() == Qt::Checked)
                checkedItems.push_back(it);
        }

        if (checkedItems.isEmpty() && m_manualParts.isEmpty()) {
            ui->tableWidgetParts->setRowCount(0);
            ui->tableWidgetParts->hide();
            ui->tableWidgetParts->setUpdatesEnabled(true);
            updateSubmitEnabled();          // <--- add this
            return;
        }

        for (QListWidgetItem *it : checkedItems) {
            QString expr           = it->data(Qt::UserRole).toString().trimmed();
            QString checklistLabel = it->text();

            // ---- case: no material ID for this checklist item ----
            if (expr.isEmpty()) {
                PartInfo p;
                p.materialId.clear();
                p.part          = QString("No material ID for: %1").arg(checklistLabel);
                p.storage       = "-";
                p.qty           = 0;
                p.found         = false;
                p.checklistName = checklistLabel;
                p.subOrder      = 99;
                allParts.append(p);
                continue;
            }

            // ---- normal / special expressions ----
            QString e = expr.toLower().remove(' ').remove('-');
            QList<PartInfo> parts;

            if (e.contains("arma")) {
                parts = findArmUpgradeParts(true);
                for (PartInfo &p : parts) {
                    p.checklistName = checklistLabel;
                    p.subOrder      = 1;   // ARM A
                }
            } else if (e.contains("armb")) {
                parts = findArmUpgradeParts(false);
                for (PartInfo &p : parts) {
                    p.checklistName = checklistLabel;
                    p.subOrder      = 2;   // ARM B
                }
            } else if (e.contains("dmlika")) {
                parts = findDmUpgradeParts();
                for (PartInfo &p : parts) {
                    p.checklistName = checklistLabel;
                    p.subOrder      = 3;   // DM Lika
                }
            } else {
                parts = findPartsForMaterial(expr);
                for (PartInfo &p : parts) {
                    p.checklistName = checklistLabel;
                    p.subOrder      = 0;   // normal checklist parts
                }
            }

            int multiplier = it->data(Qt::UserRole + 1).toInt();
            if (multiplier > 1) {
                for (PartInfo &p : parts)
                    p.qty *= multiplier;
            }

            allParts.append(parts);
        }
    }

    // -----------------------------
    // 2) Always add manual search parts
    // -----------------------------
    if (!m_manualParts.isEmpty()) {
        allParts.append(m_manualParts);
    }

    //qDebug() << "rebuildPartsFromChecklist: allParts count =" << allParts.size();

    // -----------------------------
    // 3) Show / hide table
    // -----------------------------
    if (allParts.isEmpty()) {
        ui->tableWidgetParts->setRowCount(0);
        ui->tableWidgetParts->hide();
        ui->tableWidgetParts->setUpdatesEnabled(true);
        updateSubmitEnabled();
        return;
    }

    rebuildPartsTable(allParts);
    ui->tableWidgetParts->show();

    //ui->tableWidgetParts->resizeColumnToContents(0);
    //ui->tableWidgetParts->resizeColumnToContents(1);
    ui->tableWidgetParts->setSortingEnabled(true);
    ui->tableWidgetParts->setUpdatesEnabled(true);

    updateSubmitEnabled();   // important: enable submit when we have rows
}
