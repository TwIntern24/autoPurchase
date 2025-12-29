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

    ui->tableWidgetParts->setColumnCount(5);
    QStringList headers;
    headers << "Material ID" << "Item Name" << "Storage Location" << "Robot No." << "Qty";
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

           m_rebuildTimer.setSingleShot(true);
           m_rebuildTimer.setInterval(150);
           connect(&m_rebuildTimer, &QTimer::timeout,
                   this, &AutoPurchase::rebuildPartsFromChecklist);


           ui->tableWidgetParts->horizontalHeader()->setSectionsClickable(false);

           auto *hh = ui->tableWidgetParts->horizontalHeader();
           hh->setStretchLastSection(false);

           // Good layout: name stretches, others fit/fixed
           hh->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Material
           hh->setSectionResizeMode(1, QHeaderView::Stretch); // Item Name
           hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);          // Storage
           //hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);          // Item Name
           hh->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Robot number
           //hh->setSectionResizeMode(2, QHeaderView::Fixed); // Storage
           hh->setSectionResizeMode(4, QHeaderView::Fixed);            // Quantity

           ui->tableWidgetParts->setColumnWidth(4, 100);  // adjust 70–110 as you like
           //ui->tableWidgetParts->setColumnWidth(0,500);
           //ui->tableWidgetParts->setColumnWidth(2,250);

           //ui->tableWidgetParts->setColumnWidth(0, 500); // or 160
           //connect(ui->btnSubmit, &QPushButton::clicked,
                  // this, &AutoPurchase::exportPdfFromCurrentTable);
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

static QString lastNDigits(QString s, int n)
{
    QString digits;
    for (const QChar &ch : s) {
        if (ch.isDigit()) digits.append(ch);
    }
    if (digits.size() < n) return QString();
    return digits.right(n);
}

// TODO: replace with your real 5 mappings
static QString robotNameFrom12NcLast5(const QString &last5)
{
    if (last5 == "17075") return "DoubleFold";
    if (last5 == "49401") return "DoubleFold";
    if (last5 == "94871") return "Scara";
    if (last5 == "94861") return "NT";
    if (last5 == "22862") return "NXT";
    return QString(); // unknown -> default behavior
}

void AutoPurchase::resetAutoRobotInfo()
{
    m_autoInfoAvailable = false;
    m_autoRobotNumber.clear();
    m_autoRobotName.clear();
}

/*
void AutoPurchase::updateRobotColumnsVisibility()
{
    const bool show = m_autoInfoAvailable;
    ui->tableWidgetParts->setColumnHidden(3, !show); // Robot Number col
    //ui->tableWidgetParts->setColumnHidden(1, !show); // Robot Name col
}
*/

void AutoPurchase::extractAutoRobotInfoFromJob()
{
    resetAutoRobotInfo();

    if (m_rowsJob.isEmpty())
        return;

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsJob.size()) return QString();
        QVariantList cols = m_rowsJob[r].toList();
        if (c < 0 || c >= cols.size()) return QString();
        return cols[c].toString().trimmed();
    };

    QString serialText;
    QString ncText;

    for (int r = 0; r < m_rowsJob.size(); ++r) {
        QVariantList row = m_rowsJob[r].toList();
        for (int c = 0; c < row.size(); ++c) {
            QString cell = row[c].toString().trimmed();
            QString norm = cell.toLower();
            norm.remove(' ');

            if (norm.contains("unitserialnumber"))
                serialText = getCol(r, c + 1);

            if (norm.contains("unit12ncreceived"))
                ncText = getCol(r, c + 1);
        }
    }

    // mark: we tried extraction from an auto file
    m_autoInfoAvailable = true;

    // fill what we can
    m_autoRobotNumber = lastNDigits(serialText, 4);            // may be empty
    QString last5     = lastNDigits(ncText, 5);                // may be empty
    if (!last5.isEmpty())
        m_autoRobotName = robotNameFrom12NcLast5(last5);       // may be empty if not mapped
}

/*
void AutoPurchase::extractAutoRobotInfoFromJob()
{
    resetAutoRobotInfo();

    if (m_rowsJob.isEmpty())
        return;

    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsJob.size()) return QString();
        QVariantList cols = m_rowsJob[r].toList();
        if (c < 0 || c >= cols.size()) return QString();
        return cols[c].toString().trimmed();
    };

    QString serialText;
    QString ncText;

    for (int r = 0; r < m_rowsJob.size(); ++r) {
        QVariantList row = m_rowsJob[r].toList();
        for (int c = 0; c < row.size(); ++c) {
            QString cell = row[c].toString().trimmed();
            QString norm = cell.toLower();
            norm.remove(' ');

            if (norm.contains("unitserialnumber")) {
                serialText = getCol(r, c + 1);
            } else if (norm.contains("unit12ncreceived")) {
                ncText = getCol(r, c + 1);
            }
        }
    }

    QString last4 = lastNDigits(serialText, 4);
    QString last5 = lastNDigits(ncText, 5);
    QString name  = robotNameFrom12NcLast5(last5);

    if (!last4.isEmpty() && !name.isEmpty()) {
        m_autoRobotNumber = last4;
        m_autoRobotName   = name;
        m_hasAutoJobInfo  = true;
    }
}
*/

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
        hdr->setBackground(QBrush(Qt::darkGray));
        ui->tableWidgetParts->setItem(headerRow, 0, hdr);

        // Optional: span header across all 4 columns
        ui->tableWidgetParts->setSpan(headerRow, 0, 1, 5);

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
                    //subHdr->setBackground(QBrush(Qt::darkGreen));
                    subHdr->setBackground(QColor("#4E8F6B"));   // muted green
                    subHdr->setForeground(QColor("#FFFFFF"));
                    ui->tableWidgetParts->setItem(srow, 0, subHdr);
                    ui->tableWidgetParts->setSpan(srow, 0, 1, 5);
                }
                lastSubOrder = p.subOrder;
            }

            // Real part row
            int row = ui->tableWidgetParts->rowCount();
            ui->tableWidgetParts->insertRow(row);

            auto *robotNoItem = new QTableWidgetItem(m_autoInfoAvailable ? m_autoRobotNumber : "");
            robotNoItem->setFlags(Qt::ItemIsEnabled);
            ui->tableWidgetParts->setItem(row, 3, robotNoItem);

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
            spin->setFixedWidth(100);                 // prevents column “ballooning”
            spin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            ui->tableWidgetParts->setCellWidget(row, 4, spin);
        }
    }

    //ui->tableWidgetParts->resizeColumnToContents(0);
    //ui->tableWidgetParts->resizeColumnToContents(1);

    int maxW = 0;
    QFontMetrics fm(ui->tableWidgetParts->font());

    for (int r = 0; r < ui->tableWidgetParts->rowCount(); ++r) {
        if (ui->tableWidgetParts->columnSpan(r, 0) > 1)
            continue; // skip spanned headers/subheaders

        QTableWidgetItem *it = ui->tableWidgetParts->item(r, 0);
        if (!it) continue;

        QString id = it->text().trimmed();
        if (id.isEmpty() || id == "-")
            continue; // ignore no-ID rows

        maxW = qMax(maxW, fm.width(id));
    }

    maxW = qMax(maxW, fm.width("E-00000")); // minimum sensible width

    auto *hh = ui->tableWidgetParts->horizontalHeader();
    hh->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidgetParts->setColumnWidth(0, maxW + 50);

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
        QString msg = tr("ID \"%1\" not found").arg(id);
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
