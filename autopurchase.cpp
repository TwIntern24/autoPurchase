#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QFile>
#include <QTextStream>
#include <QStandardItemModel>
#include <QTableView>
#include <QAxObject>
#include <QStandardItemModel>
#include <QDebug>

#include <QFileDialog>
#include <QListWidgetItem>
#include <QApplication>   // for qApp

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

#include <QSignalBlocker>

#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

#include <QTimer>

#include <QStyleFactory>

AutoPurchase::AutoPurchase(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoPurchase)
{
    ui->setupUi(this);

    qDebug() << "Available styles:" << QStyleFactory::keys();
    //qApp->setStyle("Fusion");

    m_jobReady       = false;
    m_checklistReady = false;
    m_jobAppliedOnce = false;

    ui->btnSubmit->setEnabled(false);

    ui->tableWidgetParts->setColumnCount(4);
    QStringList headers;
    headers << "Material Number" << "Item Name" << "Storage Location" << "Quantity";
    ui->tableWidgetParts->setHorizontalHeaderLabels(headers);
    ui->tableWidgetParts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetParts->verticalHeader()->setVisible(false);
    ui->tableWidgetParts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetParts->setEditTriggers(QAbstractItemView::NoEditTriggers); // spinboxes for quantity
    ui->tableWidgetParts->hide();  // show only when we have data

    ui->listWidgetChecklist->hide();
    //ui->listWidgetParts->hide();

    m_loadingMovie = new QMovie(":/gif/gif/g0R5.gif", QByteArray(), this);
       ui->labelLoading->setMovie(m_loadingMovie);
       ui->labelLoading->setScaledContents(true);
       //  Give the label a reasonable size so the gif is visible
       ui->labelLoading->setMinimumSize(50, 50);   // or 80x80, 100x100 etc.
       ui->labelLoading->setMaximumSize(60, 60); // optional
       //ui->labelLoading->setMinimumSize(0, 0);
       //ui->labelLoading->setMaximumSize(QSizePolicy::Ignored, QSizePolicy::Ignored);
       //ui->labelLoading->setSizePolicy(QSizePolicy::Preferred,
         //                              QSizePolicy::Preferred);

       //m_loadingMovie->start();
      // ui->labelLoading->adjustSize();
       ui->labelLoading->hide();  // hidden until loading starts

    ui->comboExcel->addItem("Select Robot Matrix", "");  // index 0, no file

        //  Example: add some Excel files with display name + full path
        // TODO: change these paths to your real file locations
    QString base = QCoreApplication::applicationDirPath();
        ui->comboExcel->addItem("Double Fold",   base + "/../data/Repair_matrix DF.xlsx");
        ui->comboExcel->addItem("MK5",  base + "/../data/Repair_matrix_MK5 1.xlsx");

        ui->tableWidgetParts->horizontalHeader()
            ->setSectionResizeMode(QHeaderView::ResizeToContents);

        // 2) When user changes the selection, load that Excel

        // worker thread
           m_excelThread = new QThread(this);
           m_loader = new ExcelLoader();
           m_loader->moveToThread(m_excelThread);

           connect(m_excelThread, &QThread::finished,
                   m_loader, &QObject::deleteLater);

           connect(this, &AutoPurchase::startExcelLoad,
                   m_loader, &ExcelLoader::loadFile);

           connect(m_loader, &ExcelLoader::loaded,
                   this, &AutoPurchase::onExcelLoaded);

           connect(m_loader, &ExcelLoader::loadFailed,
                   this, &AutoPurchase::onExcelLoadFailed);

           m_excelThread->start();

        connect(ui->comboExcel,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                this,
                &AutoPurchase::onExcelSelectionChanged);

        m_materialThread = new QThread(this);
            m_materialLoader = new ExcelLoader();
            m_materialLoader->moveToThread(m_materialThread);

            connect(m_materialThread, &QThread::finished,
                    m_materialLoader, &QObject::deleteLater);

            connect(this, &AutoPurchase::startMaterialsLoad,
                    m_materialLoader, &ExcelLoader::loadFile);

            connect(m_materialLoader, &ExcelLoader::loaded,
                    this, &AutoPurchase::onMaterialsLoaded);

            connect(m_materialLoader, &ExcelLoader::loadFailed,
                    this, &AutoPurchase::onMaterialsLoadFailed);

            m_materialThread->start();

            QString materialsPath = base + "/../data/03_Inventory_v0.1.xlsx";
            qDebug() << "Materials Excel path:" << materialsPath
                     << "exists?" << QFileInfo(materialsPath).exists();
            emit startMaterialsLoad(materialsPath);

        connect(ui->listWidgetChecklist,
                    &QListWidget::itemChanged,
                    this,
                    &AutoPurchase::onChecklistItemChanged);


        m_armThread = new QThread(this);
        m_armLoader = new ExcelLoader();
        m_armLoader->moveToThread(m_armThread);

        connect(m_armThread, &QThread::finished,
                m_armLoader, &QObject::deleteLater);

        connect(this, &AutoPurchase::startArmUpgradeLoad,  // or add a new signal if you prefer
                m_armLoader, &ExcelLoader::loadFile);

        connect(m_armLoader, &ExcelLoader::loaded,
                this, &AutoPurchase::onArmUpgradeLoaded);

        connect(m_armLoader, &ExcelLoader::loadFailed,
                this, &AutoPurchase::onArmUpgradeLoadFailed);

        m_armThread->start();

        QString armPath = base + "/../data/DF ARM SET A, B.xlsx";   // <-- adjust filename
        qDebug() << "ARM upgrade Excel:" << armPath << QFileInfo(armPath).exists();
        emit startArmUpgradeLoad(armPath);   // or a separate signal if you made one


        // ---------- DM LIKA UPGRADE EXCEL ----------
        m_dmThread = new QThread(this);
        m_dmLoader = new ExcelLoader();
        m_dmLoader->moveToThread(m_dmThread);

        connect(m_dmThread, &QThread::finished,
                m_dmLoader, &QObject::deleteLater);

        connect(this, &AutoPurchase::startDmUpgradeLoad,   // or a dedicated signal
                m_dmLoader, &ExcelLoader::loadFile);

        connect(m_dmLoader, &ExcelLoader::loaded,
                this, &AutoPurchase::onDmUpgradeLoaded);

        connect(m_dmLoader, &ExcelLoader::loadFailed,
                this, &AutoPurchase::onDmUpgradeLoadFailed);

        m_dmThread->start();

        QString dmPath = base + "/../data/DF DM LIKA UPGRADE.xlsx";  // <-- adjust filename
        qDebug() << "DM Lika Excel:" << dmPath << QFileInfo(dmPath).exists();
        emit startDmUpgradeLoad(dmPath);

        //ui->tableWidgetParts->horizontalHeader()
          //  ->setSectionResizeMode(QHeaderView::ResizeToContents);

        // requester combo
           connect(ui->comboRequester,
                   QOverload<int>::of(&QComboBox::currentIndexChanged),
                   this,
                   &AutoPurchase::onRequesterChanged);

           loadRequesters();

           // default date for request
           ui->dateEditRequest->setDate(QDate::currentDate());

           connect(ui->btnLoadJob, &QPushButton::clicked,
                   this, &AutoPurchase::onLoadJobFile);



           m_jobThread = new QThread(this);
           m_jobLoader = new ExcelLoader();
           m_jobLoader->moveToThread(m_jobThread);

           connect(m_jobThread, &QThread::finished,
                   m_jobLoader, &QObject::deleteLater);

           connect(this, &AutoPurchase::startJobFileLoad,
                   m_jobLoader, &ExcelLoader::loadFile);

           connect(m_jobLoader, &ExcelLoader::loaded,
                   this, &AutoPurchase::onJobFileLoaded);

           connect(m_jobLoader, &ExcelLoader::loadFailed,
                   this, &AutoPurchase::onJobFileLoadFailed);

           m_jobThread->start();

           connect(ui->lineSearchMaterial, &QLineEdit::returnPressed,
                   this, &AutoPurchase::on_btnSearchMaterial_clicked);

           connect(ui->btnClearManual,
                   &QPushButton::clicked,
                   this,
                   &AutoPurchase::on_btnClearManual_clicked);
}


AutoPurchase::~AutoPurchase()
{
    if (m_excelThread) {
            m_excelThread->quit();
            m_excelThread->wait();
        }
    if (m_materialThread) {
            m_materialThread->quit();
            m_materialThread->wait();
        }
    if (m_armThread) {
        m_armThread->quit();
        m_armThread->wait();
    }
    if (m_dmThread) {
        m_dmThread->quit();
        m_dmThread->wait();
    }
    if (m_jobThread) {
           m_jobThread->quit();
           m_jobThread->wait();
       }
    delete ui;
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


QList<AutoPurchase::PartInfo>
AutoPurchase::findPartsForMaterial(const QString &expr) const
{
    QList<PartInfo> result;

    if (m_rowsMaterials.isEmpty())
        return result;

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsMaterials.size()) return "";
        QVariantList cols = m_rowsMaterials[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    // expr like "610846+E-20953*2"
    QStringList tokens = expr.split('+', QString::SkipEmptyParts);

    for (const QString &rawToken : tokens) {
        QString token = rawToken.trimmed();

        // extract quantity
        int qty = 1;
        int starPos = token.indexOf('*');
        if (starPos >= 0) {
            bool ok = false;
            int q = token.mid(starPos + 1).toInt(&ok);
            if (ok && q > 0)
                qty = q;
            token = token.left(starPos);   // strip "*2"
        }

        PartInfo info;
        info.materialId = token;
        info.qty        = qty;
        info.found      = false;

        // search materials table
        for (int r = 0; r < m_rowsMaterials.size(); ++r) {
            QString id   = getCol(r, 1); // material ID
            QString part = getCol(r, 5); // item name
            QString desc = getCol(r, 3); // storage / bracket text

            if (id.compare(info.materialId, Qt::CaseInsensitive) == 0) {
                info.part    = part;
                info.storage = desc;      // e.g. "B2E", "C1D"
                info.found   = true;
                break;
            }
        }

        result.append(info);
    }

    return result;
}

void AutoPurchase::rebuildPartsTable(const QList<PartInfo> &allParts)
{
    ui->tableWidgetParts->clearContents();
    ui->tableWidgetParts->setRowCount(0);

    if (allParts.isEmpty()) {
        ui->tableWidgetParts->hide();
        updateSubmitEnabled();
        return;
    }

    // 1) Group by checklistName (with “Others” fallback)
    QMap<QString, QList<PartInfo>> grouped;
    QStringList groupOrder;

    for (const PartInfo &p : allParts) {
        QString key = p.checklistName.isEmpty() ? QStringLiteral("Others")
                                                : p.checklistName;
        if (!grouped.contains(key))
            groupOrder << key;
        grouped[key].append(p);
    }

    // 2) Sort parts: subOrder → “has ID” → storage → materialId
    auto partLess = [](const PartInfo &a, const PartInfo &b) {
        if (a.subOrder != b.subOrder)
            return a.subOrder < b.subOrder;

        auto hasId = [](const PartInfo &p) {
            QString id = p.materialId.trimmed();
            return !id.isEmpty() && id != "-";
        };

        bool aHasId = hasId(a);
        bool bHasId = hasId(b);
        if (aHasId != bHasId)
            return aHasId && !bHasId;    // with ID first, no-ID at bottom

        if (a.storage != b.storage)
            return a.storage < b.storage;

        return a.materialId < b.materialId;
    };

    // 3) Reorder groups: groups with at least one real ID first
    QStringList normalGroups;
    QStringList noIdGroups;

    for (const QString &name : groupOrder) {
        const QList<PartInfo> &parts = grouped[name];

        bool hasRealId = false;
        for (const PartInfo &p : parts) {
            QString id = p.materialId.trimmed();
            if (!id.isEmpty() && id != "-") {
                hasRealId = true;
                break;
            }
        }

        if (hasRealId)
            normalGroups << name;
        else
            noIdGroups << name;
    }

    QStringList finalGroups = normalGroups + noIdGroups;

    // 4) Build the table with group headers + optional sub-headers
    auto subHeaderText = [](int subOrder) -> QString {
        switch (subOrder) {
        case 1:  return QStringLiteral("ARM A");
        case 2:  return QStringLiteral("ARM B");
        case 3:  return QStringLiteral("DM Lika upgrade");
        default: return QString();
        }
    };

    for (const QString &groupName : finalGroups) {
        QList<PartInfo> parts = grouped[groupName];
        std::sort(parts.begin(), parts.end(), partLess);

        // Group header row (bold, spans all columns)
        int headerRow = ui->tableWidgetParts->rowCount();
        ui->tableWidgetParts->insertRow(headerRow);

        auto *hdr = new QTableWidgetItem(groupName);
        QFont hf = hdr->font();
        hf.setBold(true);
        hdr->setFont(hf);
        hdr->setFlags(Qt::ItemIsEnabled);
        hdr->setBackground(QBrush(Qt::lightGray));
        ui->tableWidgetParts->setItem(headerRow, 0, hdr);

        // Optional: span header across all 4 columns
        ui->tableWidgetParts->setSpan(headerRow, 0, 1, 4);

        int lastSubOrder = -1;

        for (const PartInfo &p : parts) {

            // Sub-header for ARM A / ARM B / DM groups
            if (p.subOrder != 0 && p.subOrder != lastSubOrder) {
                QString shText = subHeaderText(p.subOrder);
                if (!shText.isEmpty()) {
                    int srow = ui->tableWidgetParts->rowCount();
                    ui->tableWidgetParts->insertRow(srow);

                    auto *subHdr = new QTableWidgetItem("  " + shText);
                    QFont sf = subHdr->font();
                    sf.setBold(true);
                    subHdr->setFont(sf);
                    subHdr->setFlags(Qt::ItemIsEnabled);
                    subHdr->setBackground(QBrush(Qt::lightGray));
                    ui->tableWidgetParts->setItem(srow, 0, subHdr);
                    ui->tableWidgetParts->setSpan(srow, 0, 1, 4);
                }
                lastSubOrder = p.subOrder;
            }

            // Real part row
            int row = ui->tableWidgetParts->rowCount();
            ui->tableWidgetParts->insertRow(row);

            auto *matItem = new QTableWidgetItem(p.materialId);
            ui->tableWidgetParts->setItem(row, 0, matItem);

            auto *nameItem = new QTableWidgetItem(
                        p.part.isEmpty() ? "(no details)" : p.part);
            ui->tableWidgetParts->setItem(row, 1, nameItem);

            auto *storItem = new QTableWidgetItem(p.storage);
            ui->tableWidgetParts->setItem(row, 2, storItem);

            auto *spin = new QSpinBox(ui->tableWidgetParts);
            spin->setMinimum(0);
            spin->setMaximum(9999);
            spin->setValue(p.qty);
            ui->tableWidgetParts->setCellWidget(row, 3, spin);
        }
    }

    ui->tableWidgetParts->resizeColumnToContents(0);
    ui->tableWidgetParts->resizeColumnToContents(1);
    ui->tableWidgetParts->show();

    updateSubmitEnabled();
}



AutoPurchase::PartInfo lookupInventory(const QVariantList &rowsMat,
                                       const QString &id,
                                       int defaultQty,
                                       const QString &storageFromUpgrade)
{
    AutoPurchase::PartInfo info;
    info.materialId = id;
    info.qty        = defaultQty;
    info.storage    = storageFromUpgrade;
    info.found      = false;

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= rowsMat.size()) return "";
        QVariantList cols = rowsMat[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    for (int r = 0; r < rowsMat.size(); ++r) {
        QString matId = getCol(r, 1);   // same index you already use
        if (matId.compare(id, Qt::CaseInsensitive) == 0) {
            info.part    = getCol(r, 5);  // part name
            // if you want to override storage from inventory, do it here:
            // QString invStorage = getCol(r, 7);
            // if (!invStorage.isEmpty()) info.storage = invStorage;
            info.found   = true;
            break;
        }
    }

    return info;
}

struct InvLookupResult {
    QString name;
    QString storage;
    bool    found = false;
};

static InvLookupResult findInInventory(const QVariantList &rowsMat,
                                       const QString &id)
{
    InvLookupResult res;
    if (rowsMat.isEmpty() || id.isEmpty())
        return res;

    // Use the SAME column indices that already work
    // for your normal inventory-based parts.
    const int MATERIAL_ID_COL = 1;  // e.g. A
    const int NAME_COL        = 5;  // e.g. C
    const int STORAGE_COL     = 3;  // e.g. G

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= rowsMat.size()) return "";
        QVariantList cols = rowsMat[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    for (int r = 0; r < rowsMat.size(); ++r) {
        QString matId = getCol(r, MATERIAL_ID_COL);
        if (matId.compare(id, Qt::CaseInsensitive) == 0) {
            res.name    = getCol(r, NAME_COL);
            res.storage = getCol(r, STORAGE_COL);
            res.found   = true;
            break;
        }
    }

    return res;
}



QList<AutoPurchase::PartInfo>
AutoPurchase::findArmUpgradeParts(bool sideA) const
{
    QList<PartInfo> result;
    if (m_rowsArmUpgrade.isEmpty() || m_rowsMaterials.isEmpty())
        return result;

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsArmUpgrade.size()) return "";
        QVariantList cols = m_rowsArmUpgrade[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    int colId = sideA ? 4 : 0;      // A=0, B=1, C=2, D=3, E=4 → IDs in A and D
    int colQL = sideA ? 5 : 1;      // B or E = "<qty> <location>"
    int storageLoc = sideA ? 6 : 2;

    for (int r = 1; r < m_rowsArmUpgrade.size(); ++r) {
        QString id = getCol(r, colId);
        if (id.isEmpty())
            continue;

        QString qloc = getCol(r, colQL);   // e.g. "2 02-06B"
        QString qstorageloc = getCol(r, storageLoc);

        int qty = 1;
        bool ok = false;
        int q = qloc.toInt(&ok);
        if (ok && q > 0){ qty = q; }

        //PartInfo info = lookupInventory(m_rowsMaterials, id, qty, loc);
        //PartInfo info = lookupInventory(m_rowsMaterials, id, qty, qstorageloc);

        PartInfo info;
        info.materialId = id;
        info.qty        = qty;
        info.storage    = qstorageloc;   // may be empty
        info.found      = false; // will mark true if inv or ARM has something
        // name from ARM sheet? (if you ever add it)
        // info.part = ...;

        // Fallback to inventory if name or location is missing
        InvLookupResult inv = findInInventory(m_rowsMaterials, id);
        if (info.part.isEmpty() && inv.found)
            info.part = inv.name;
        if (info.storage.isEmpty() && inv.found)
            info.storage = inv.storage;

        info.found = inv.found || !info.part.isEmpty() || !info.storage.isEmpty();

        result.append(info);
    }

    return result;
}


QList<AutoPurchase::PartInfo>
AutoPurchase::findDmUpgradeParts() const
{
    QList<PartInfo> result;
    if (m_rowsDmUpgrade.isEmpty())
        return result;

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsDmUpgrade.size()) return "";
        QVariantList cols = m_rowsDmUpgrade[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    for (int r = 2; r < m_rowsDmUpgrade.size(); ++r) {
        QString id    = getCol(r, 0);   // product number
        if (id.isEmpty())
            continue;

        QString name  = getCol(r, 1);   // item name
        QString qtyStr= getCol(r, 2);   // quantity
        QString loc   = getCol(r, 4);   // storage location

        int qty = 1;
        bool ok = false;
        int q = qtyStr.toInt(&ok);
        if (ok && q > 0) qty = q;

        PartInfo info;
        info.materialId = id;
        info.part       = name;
        info.storage    = loc;
        info.qty        = qty;
        info.found      = true;

        // Fallback to inventory if DM row misses name or location
        InvLookupResult inv = findInInventory(m_rowsMaterials, id);
        if (info.part.isEmpty() && inv.found)
            info.part = inv.name;
        if (info.storage.isEmpty() && inv.found)
            info.storage = inv.storage;

        result.append(info);
    }

    return result;
}


void AutoPurchase::loadRequesters()
{
    QString base = QCoreApplication::applicationDirPath();
    QString path = base + "/../data/requesters.txt";

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open requester list:" << path << file.errorString();
        return;
    }

    QTextStream in(&file);
    // in.setCodec("UTF-8"); // if needed

    ui->comboRequester->clear();
    ui->comboRequester->addItem("Select requester"); // index 0 = placeholder

    m_requesters.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        QStringList parts = line.split(';');
        if (parts.size() < 2)
            continue;   // need at least name + email

        RequesterInfo info;
        info.name  = parts.value(0).trimmed();
        info.email = parts.value(1).trimmed();
        info.id    = parts.value(2).trimmed();  // may be empty

        m_requesters.push_back(info);

        int idx = ui->comboRequester->count();
        ui->comboRequester->addItem(info.name);
        ui->comboRequester->setItemData(idx, info.email, Qt::UserRole);
        ui->comboRequester->setItemData(idx, info.id,    Qt::UserRole + 1);
    }

    file.close();
}


void AutoPurchase::onRequesterChanged(int index)
{
    if (index <= 0){
        updateSubmitEnabled();
        return;
    }

    QString email = ui->comboRequester->itemData(index, Qt::UserRole).toString();
    QString id    = ui->comboRequester->itemData(index, Qt::UserRole + 1).toString();

    qDebug() << "Requester email:" << email;
    qDebug() << "Requester ID:" << id;

    // If you have fields:
    // ui->lineRequesterEmail->setText(email);
    // ui->lineRequesterId->setText(id);
    updateSubmitEnabled();
}


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
    ui->labelStatus->setText("Job file loaded");

    // Detect header row + columns once
    detectJobColumns();

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

void AutoPurchase::applyJobToChecklist()
{
    if (m_rowsJob.isEmpty())
        return;

    if (m_jobHeaderRow < 0 || m_jobColModule < 0 || m_jobColNeed < 0) {
        qDebug() << "Job file: checklist or need column not found!";
        return;
    }

    // 0) Clear all checks and multipliers first
    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QListWidgetItem *it = ui->listWidgetChecklist->item(i);
        it->setCheckState(Qt::Unchecked);
        it->setData(Qt::UserRole + 1, QVariant());
    }

    // helper to read cell
    auto getJobCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsJob.size()) return QString();
        QVariantList cols = m_rowsJob[r].toList();
        if (c < 0 || c >= cols.size()) return QString();
        return cols[c].toString().trimmed();
    };

    // 1) Loop over job rows (skip header)
    for (int r = m_jobHeaderRow + 1; r < m_rowsJob.size(); ++r) {
        QString jobModule = getJobCol(r, m_jobColModule);
        QString needStr      = getJobCol(r, m_jobColNeed);

        if (jobModule.isEmpty())
            continue;

        bool ok = false;
        int needQty = needStr.toInt(&ok);
        if (!ok || needQty <= 0)
            continue;   // if no valid need value, skip

        // 2) Find the matching checklist item and mark it
        for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
            QListWidgetItem *it = ui->listWidgetChecklist->item(i);
            QString label = it->text();

            // simple contains-match; change to == if you want exact match
            if (label.contains(jobModule, Qt::CaseInsensitive)) {
                it->setCheckState(Qt::Checked);
                it->setData(Qt::UserRole + 1, needQty);  // multiplier
            }
        }
    }

    // 3) Now rebuild the parts table with these auto-checked items
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

void AutoPurchase::on_btnSearchMaterial_clicked()
{
    QString id = ui->lineSearchMaterial->text().trimmed();

    if (id.isEmpty()) {
        QMessageBox::information(this,
                                 tr("Material search"),
                                 tr("Please enter a material ID."));
        return;
    }

    if (m_rowsMaterials.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Material search"),
                             tr("Inventory is not loaded yet."));
        return;
    }

    InvLookupResult inv = findInInventory(m_rowsMaterials, id);

    if (!inv.found) {
        QString msg = tr("Material ID \"%1\" not found in inventory.").arg(id);
        QMessageBox::information(this, tr("Material search"), msg);
        if (ui->labelSearchResult)
            ui->labelSearchResult->setText(msg);
        return;
    }

    // Show info in label (optional)
    QString resultText =
            //tr("ID: %1\nName: %2\nStorage: %3")
            tr("ID: %1")
            //.arg(id, inv.name, inv.storage);
            .arg(id);
    if (ui->labelSearchResult)
        ui->labelSearchResult->setText(resultText);

    // Add to manual parts list instead of directly to the table
    PartInfo p;
    p.materialId    = id;
    p.part          = inv.name;
    p.storage       = inv.storage;
    p.qty           = 1;
    p.found         = true;
    p.checklistName = "Manual search";  // group header title
    p.subOrder      = 0;

    m_manualParts.append(p);

    //updateSubmitEnabled();

    // Rebuild full table (checklist + manual)
    rebuildPartsFromChecklist();
}

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

    // 4) Otherwise rebuild table from checklist only
    rebuildPartsFromChecklist();
    updateSubmitEnabled();
}

