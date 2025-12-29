#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

#include <QSignalBlocker>

// ============================
//  JOB EXCEL: load + robots
// ============================

void AutoPurchase::onLoadJobFile()
{
    QString path = QFileDialog::getOpenFileName(
                this,
                tr("Open job Excel"),
                QString(),
                tr("Excel files (*.xlsx *.xls)"));

    if (path.isEmpty())
        return;

    ui->labelStatus->setText("Loading job Excel...");
    resetAutoRobotInfo();
    emit startJobFileLoad(path);

    // Infer robot name from file name
    QString lower = QFileInfo(path).fileName().toLower();

    if (lower.contains("mk5") || lower.contains("mk6")) {
        m_jobRobotFromFile = "MK5";
    } else if (lower.contains("doublefold") || lower.contains("df") || lower.contains("avr")) {
        m_jobRobotFromFile = "Double Fold";
    } else {
        m_jobRobotFromFile.clear(); // unknown / not used
    }

    qDebug() << "Robot from job file name:" << m_jobRobotFromFile;
}

void AutoPurchase::onJobFileLoaded(const QVariantList &rows)
{
    m_rowsJob = rows;
    ui->labelStatus->setText("Done - modified same matrix");

    // Detect header row + columns once
    detectJobColumns();

    extractAutoRobotInfoFromJob();

    // Apply file to checklist automatically
    //applyJobToChecklist();

    // mark whether job file is usable
        m_jobReady = (m_jobHeaderRow >= 0 &&
                      m_jobColModule >= 0 &&
                      m_jobColNeed   >= 0);

        m_jobAppliedOnce = false;    // NEW: allow auto-apply once for this job

        // Optional: auto-select matrix robot in combo based on file name
        if (!m_jobRobotFromFile.isEmpty()) {
            int idx = ui->comboExcel->findText(m_jobRobotFromFile,
                                               Qt::MatchContains);
            if (idx > 0) {
                ui->comboExcel->setCurrentIndex(idx);
                // this will trigger onExcelSelectionChanged → onExcelLoaded later
            }
        }

        // If checklist is already ready, this will apply now.
        // If checklist is not ready yet, tryApplyJobToChecklist()
        // will be called again from onExcelLoaded().
        tryApplyJobToChecklist();
}


void AutoPurchase::onJobFileLoadFailed(const QString &error)
{
    ui->labelStatus->setText("Job file load error: " + error);
}

static QString normKey(QString s)
{
    s = s.toLower().trimmed();

    // normalize common separators
    s.replace(QChar(0x00A0), ' '); // non-breaking space
    s.remove(' ');
    s.remove('-');
    s.remove('_');
    s.remove('\t');
    s.remove('\r');
    s.remove('\n');

    // Optional: if you have brackets/commas in labels
    s.remove('(');
    s.remove(')');
    s.remove(',');
    s.remove('.');

    return s;
}


void AutoPurchase::applyJobToChecklist()
{
    if (m_rowsJob.isEmpty())
        return;

    if (m_jobHeaderRow < 0 || m_jobColModule < 0 || m_jobColNeed < 0) {
        qDebug() << "Job file: module/need column not found!";
        return;
    }

    // Prevent any pending rebuild from firing after we rebuild manually
    m_rebuildTimer.stop();

    // Block itemChanged storms while we set many check states
    QSignalBlocker blocker(ui->listWidgetChecklist);

    // 0) Clear all checks and multipliers first
    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QListWidgetItem *it = ui->listWidgetChecklist->item(i);
        it->setCheckState(Qt::Unchecked);
        it->setData(Qt::UserRole + 1, QVariant());
    }

    // Helper to read job Excel cell
    auto getJobCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsJob.size()) return QString();
        QVariantList cols = m_rowsJob[r].toList();
        if (c < 0 || c >= cols.size()) return QString();
        return cols[c].toString().trimmed();
    };

    // 1) Build lookup: normalized checklist label -> indices
    QHash<QString, QList<int>> exactMap;
    exactMap.reserve(ui->listWidgetChecklist->count());

    QVector<QString> labelNorm;
    labelNorm.reserve(ui->listWidgetChecklist->count());

    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QString lbl = ui->listWidgetChecklist->item(i)->text();
        QString k   = normKey(lbl);
        labelNorm.push_back(k);
        exactMap[k].append(i);
    }

    // 2) Loop over job rows (skip header)
    for (int r = m_jobHeaderRow + 1; r < m_rowsJob.size(); ++r) {
        QString jobModuleRaw = getJobCol(r, m_jobColModule);
        QString needStr      = getJobCol(r, m_jobColNeed);

        if (jobModuleRaw.isEmpty())
            continue;

        bool ok = false;
        int needQty = needStr.toInt(&ok);
        if (!ok || needQty <= 0)
            continue;

        const QString jmKey = normKey(jobModuleRaw);

        // A) Exact normalized match first
        const QList<int> exactHits = exactMap.value(jmKey);
        if (!exactHits.isEmpty()) {
            for (int idx : exactHits) {
                QListWidgetItem *it = ui->listWidgetChecklist->item(idx);
                it->setCheckState(Qt::Checked);
                it->setData(Qt::UserRole + 1, needQty);
            }
            continue;
        }

        // B) Best-match fallback: choose ONE shortest label that contains jmKey
        int bestIdx = -1;
        int bestLen = INT_MAX;

        for (int i = 0; i < labelNorm.size(); ++i) {
            const QString &ln = labelNorm[i];
            if (ln.contains(jmKey)) {
                // pick the shortest containing label (more specific to base item)
                if (ln.size() < bestLen) {
                    bestLen = ln.size();
                    bestIdx = i;
                }
            }
        }

        if (bestIdx >= 0) {
            QListWidgetItem *it = ui->listWidgetChecklist->item(bestIdx);
            it->setCheckState(Qt::Checked);
            it->setData(Qt::UserRole + 1, needQty);
        }
    }

    // 3) Update checked count once (signals were blocked)
    m_checkedCount = 0;
    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        if (ui->listWidgetChecklist->item(i)->checkState() == Qt::Checked)
            ++m_checkedCount;
    }

    // 4) Single rebuild
    rebuildPartsFromChecklist();
}


void AutoPurchase::tryApplyJobToChecklist()
{
    if (!m_jobReady || !m_checklistReady)
        return;

    // Don’t overwrite user changes repeatedly
       if (m_jobAppliedOnce)
           return;

       // Only apply if the current matrix looks like the robot for this job
       if (!m_jobRobotFromFile.isEmpty()) {
           QString selected = ui->comboExcel->currentText().trimmed();
           if (!selected.isEmpty() &&
               !selected.contains(m_jobRobotFromFile, Qt::CaseInsensitive)) {
               // different robot selected → do NOT auto-apply
               return;
           }
       }

    applyJobToChecklist();
    m_jobAppliedOnce = true;
}


void AutoPurchase::detectJobColumns()
{
    //m_jobHeaderRow    = 10;
    m_jobHeaderRow    = -1;
    //m_jobColRobot     = -1;
    //m_jobColChecklist = -1;
    m_jobColNeed      = -1;
    m_jobColModule = -1;

    if (m_rowsJob.isEmpty())
        return;

    // Scan every row until we find one that has both "module part" and "need/qty"
        for (int r = 0; r < m_rowsJob.size(); ++r) {
            QVariantList row = m_rowsJob[r].toList();
            int colModule = -1;
            int colNeed   = -1;

            for (int c = 0; c < row.size(); ++c) {
                QString h = row[c].toString().trimmed().toLower();

                if (h.contains("module") && h.contains("part")) {
                    colModule = c;
                } else if (h.contains("need") ||
                           h.contains("qty")  ||
                           h.contains("quantity")) {
                    colNeed = c;
                }
            }

            if (colModule >= 0 && colNeed >= 0) {
                m_jobHeaderRow = r;
                m_jobColModule = colModule;
                m_jobColNeed   = colNeed;
                break;
            }
        }

    qDebug() << "Job header detected:"
             << "robot module =" << m_jobColModule
             << "need col =" << m_jobColNeed;
}
