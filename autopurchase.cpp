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

AutoPurchase::AutoPurchase(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoPurchase)
{
    ui->setupUi(this);

    ui->tableWidgetParts->setColumnCount(4);
    QStringList headers;
    headers << "Material Number" << "Item Name" << "Storage Location" << "Quantity";
    ui->tableWidgetParts->setHorizontalHeaderLabels(headers);
    ui->tableWidgetParts->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidgetParts->verticalHeader()->setVisible(false);
    ui->tableWidgetParts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetParts->setEditTriggers(QAbstractItemView::NoEditTriggers); // we’ll use spinboxes for quantity
    ui->tableWidgetParts->hide();  // show only when we have data

    ui->listWidgetChecklist->hide();
    //ui->listWidgetParts->hide();

    m_loadingMovie = new QMovie(":/gif/gif/g0R5.gif", QByteArray(), this);
       ui->labelLoading->setMovie(m_loadingMovie);
       ui->labelLoading->setScaledContents(true);
       //  Give the label a reasonable size so the gif is visible
       ui->labelLoading->setMinimumSize(64, 64);   // or 80x80, 100x100 etc.
       ui->labelLoading->setMaximumSize(128, 128); // optional
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




void AutoPurchase::onExcelSelectionChanged(int index)
{
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

    /*
    for (int r = 0; r < rows.size() && r < 80; ++r) {
        QVariantList cols = rows[r].toList();
        qDebug() << "Row" << (r + 1);
        for (int c = 0; c < cols.size(); ++c) {
            QString v = cols[c].toString().trimmed();
            if (!v.isEmpty()) {
                qDebug() << "  Col" << (c + 1) << ":" << v;
            }
        }
    }
    */

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

//void AutoPurchase::onChecklistItemChanged(QListWidgetItem * /*item*/)
/*
void AutoPurchase::onChecklistItemChanged(QListWidgetItem *)
{
    */
/*
    // just schedule rebuild once; UI can update immediately
    if (m_rebuildPending)
        return;

    m_rebuildPending = true;
    QTimer::singleShot(50, this, &AutoPurchase::rebuildPartsFromChecklist);
    */
/*
    rebuildPartsFromChecklist();
}
*/

void AutoPurchase::onChecklistItemChanged(QListWidgetItem *item)
{
    if (!item)
        return;

    // Debug: see what state the changed item has
    // qDebug() << "itemChanged:" << item->text() << "state:" << item->checkState();

    // If this item has just become unchecked, check if any other items are still checked
    if (item->checkState() == Qt::Unchecked) {
        bool anyChecked = false;
        for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
            QListWidgetItem *it = ui->listWidgetChecklist->item(i);
            if (it->checkState() == Qt::Checked) {
                anyChecked = true;
                break;
            }
        }

        // If nothing is checked anymore → hard clear + hide table and stop
        if (!anyChecked) {
            // qDebug() << "No checklist items checked → hiding table";
            ui->tableWidgetParts->setRowCount(0);
            ui->tableWidgetParts->hide();
            return;
        }
    }

    // Otherwise, rebuild normally
    rebuildPartsFromChecklist();
}


/*
void AutoPurchase::rebuildPartsFromChecklist()
{
    ui->tableWidgetParts->setUpdatesEnabled(false);
    ui->tableWidgetParts->setRowCount(0);

    if (m_rowsMaterials.isEmpty()) {
        ui->tableWidgetParts->hide();
        ui->tableWidgetParts->setUpdatesEnabled(true);
        return;
    }

    QList<PartInfo> allParts;

    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QListWidgetItem *it = ui->listWidgetChecklist->item(i);
        if (it->checkState() != Qt::Checked)
            continue;

        QString expr           = it->data(Qt::UserRole).toString().trimmed();
        QString checklistLabel = it->text();

        // ---- case: no material ID for this checklist item ----
        if (expr.isEmpty()) {
            PartInfo p;
            p.materialId.clear();   // no ID → goes to bottom
            p.part          = QString("No material ID for: %1").arg(checklistLabel);
            p.storage       = "-";
            p.qty           = 0;
            p.found         = false;
            p.checklistName = checklistLabel;
            p.subOrder      = 99;   // normal group, sorted after real parts
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
                p.subOrder      = 0;   // normal parts
            }
        }

        allParts.append(parts);
    }

    //  Decide show/hide ONLY from allParts
    if (!allParts.isEmpty()) {
        rebuildPartsTable(allParts);
        ui->tableWidgetParts->show();
    } else {
        ui->tableWidgetParts->hide();
    }

    ui->tableWidgetParts->resizeColumnToContents(0);
    ui->tableWidgetParts->resizeColumnToContents(1);
    ui->tableWidgetParts->setUpdatesEnabled(true);
}
*/

void AutoPurchase::rebuildPartsFromChecklist()
{
    ui->tableWidgetParts->setUpdatesEnabled(false);
    ui->tableWidgetParts->setRowCount(0);

    if (m_rowsMaterials.isEmpty()) {
        ui->tableWidgetParts->hide();
        ui->tableWidgetParts->setUpdatesEnabled(true);
        return;
    }

    // 1) Collect all checked checklist items first
    QVector<QListWidgetItem*> checkedItems;
    checkedItems.reserve(ui->listWidgetChecklist->count());

    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QListWidgetItem *it = ui->listWidgetChecklist->item(i);
        if (it->checkState() == Qt::Checked)
            checkedItems.push_back(it);
    }

    // 2) If nothing is checked → clear + hide table and return
    if (checkedItems.isEmpty()) {
        ui->tableWidgetParts->setRowCount(0);
        ui->tableWidgetParts->hide();
        ui->tableWidgetParts->setUpdatesEnabled(true);
        return;
    }

    // 3) Build parts only from the checked items
    QList<PartInfo> allParts;

    for (QListWidgetItem *it : checkedItems) {
        QString expr           = it->data(Qt::UserRole).toString().trimmed();
        QString checklistLabel = it->text();

        // ---- case: no material ID for this checklist item ----
        if (expr.isEmpty()) {
            PartInfo p;
            p.materialId.clear();   // no ID → sorted to bottom in comparator
            p.part          = QString("No material ID for: %1").arg(checklistLabel);
            p.storage       = "-";
            p.qty           = 0;
            p.found         = false;
            p.checklistName = checklistLabel;
            p.subOrder      = 99;   // “after normal” in your ordering
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
                   for (PartInfo &p : parts) {
                       p.qty *= multiplier;
                   }
               }

        allParts.append(parts);
    }

    // 4) Show/hide based on allParts
    if (!allParts.isEmpty()) {
        rebuildPartsTable(allParts);
        ui->tableWidgetParts->show();
    } else {
        // (This would only happen if checked items all somehow produced no parts)
        ui->tableWidgetParts->hide();
    }

    ui->tableWidgetParts->resizeColumnToContents(0);
    ui->tableWidgetParts->resizeColumnToContents(1);
    ui->tableWidgetParts->setUpdatesEnabled(true);
}



void AutoPurchase::rebuildPartsTable(const QList<PartInfo> &allParts)
{
    ui->tableWidgetParts->setRowCount(0);

    if (allParts.isEmpty())
        return;

    // 1) Group by checklistName, but keep insertion order of groups
    QMap<QString, QList<PartInfo>> grouped;
    QStringList groupOrder;

    for (const PartInfo &p : allParts) {
        QString key = p.checklistName;
        if (key.isEmpty())
            key = "Others";

        if (!grouped.contains(key))
            groupOrder.append(key);

        grouped[key].append(p);
    }

    /*
    // 2) For each group, sort by storage (and then by materialId)
    auto partLess = [](const PartInfo &a, const PartInfo &b) {
        if (a.storage != b.storage)
            return a.storage < b.storage;
        return a.materialId < b.materialId;
    };
    */

/*
    auto partLess = [](const PartInfo &a, const PartInfo &b) {
        if (a.subOrder != b.subOrder)
            return a.subOrder < b.subOrder;      // normal → ARM A → ARM B → DM

        if (a.storage != b.storage)
            return a.storage < b.storage;

        return a.materialId < b.materialId;
    };
    */

    auto partLess = [](const PartInfo &a, const PartInfo &b) {
        if (a.subOrder != b.subOrder)
            return a.subOrder < b.subOrder;      // normal → ARM A → ARM B → DM

       // bool aHasId = !a.materialId.trimmed().isEmpty() && a.materialId != "-";;
       // bool bHasId = !b.materialId.trimmed().isEmpty() && b.materialId != "-";;
        bool aHasId = !a.materialId.trimmed().isEmpty();
        bool bHasId = !b.materialId.trimmed().isEmpty();
        if (aHasId != bHasId)
            return aHasId && !bHasId;            // rows with ID first, no-ID rows last

        if (a.storage != b.storage)
            return a.storage < b.storage;

        return a.materialId < b.materialId;
    };

    QStringList normalGroups;
    QStringList noIdGroups;


    for (const QString &name : groupOrder) {
        const QList<PartInfo> &parts = grouped[name];

        bool hasRealId = false;
        for (const PartInfo &p : parts) {
            if (!p.materialId.trimmed().isEmpty() && p.materialId != "-") {
                hasRealId = true;
                break;
            }
        }

        if (hasRealId)
            normalGroups.append(name);
        else
            noIdGroups.append(name);
    }
    QStringList finalOrder = normalGroups + noIdGroups;


    for (const QString &groupName : finalOrder) {
        QList<PartInfo> parts = grouped[groupName];
        std::sort(parts.begin(), parts.end(), partLess);

        // 3) Insert header row for this checklist
        int headerRow = ui->tableWidgetParts->rowCount();
        ui->tableWidgetParts->insertRow(headerRow);

        QTableWidgetItem *hdr = new QTableWidgetItem(groupName);
        QFont f = hdr->font();
        f.setBold(true);
        hdr->setFont(f);
        hdr->setFlags(Qt::ItemIsEnabled);  // not editable/selectable
        ui->tableWidgetParts->setItem(headerRow, 0, hdr);

        // Optional: make header row visually different
        hdr->setBackground(QBrush(Qt::lightGray));

        // If you want to span columns (Qt Designer must allow it):
        // ui->tableWidgetParts->setSpan(headerRow, 0, 1, ui->tableWidgetParts->columnCount());

        /*
        // 4) Insert each part row under this header
        for (const PartInfo &p : parts) {
            int row = ui->tableWidgetParts->rowCount();
            ui->tableWidgetParts->insertRow(row);

            // Material Number
            auto *matItem = new QTableWidgetItem(p.materialId);
            ui->tableWidgetParts->setItem(row, 0, matItem);

            // Item Name
            auto *nameItem = new QTableWidgetItem(
                p.part.isEmpty() ? "(no details)" : p.part);
            ui->tableWidgetParts->setItem(row, 1, nameItem);

            // Storage Location
            auto *storItem = new QTableWidgetItem(p.storage);
            ui->tableWidgetParts->setItem(row, 2, storItem);

            // Quantity spinbox
            auto *spin = new QSpinBox(ui->tableWidgetParts);
            spin->setMinimum(0);
            spin->setMaximum(9999);
            spin->setValue(p.qty);
            ui->tableWidgetParts->setCellWidget(row, 3, spin);

        }
        */

        auto subHeaderText = [](int subOrder) -> QString {
            switch (subOrder) {
            case 1:  return "ARM A";
            case 2:  return "ARM B";
            case 3:  return "DM Lika upgrade";
            default: return QString();
            }
        };

        int lastSubOrder = -1;

        // 4) Insert each part row under this header
        for (const PartInfo &p : parts) {

            // insert sub-header when subOrder changes (and is not 0)
            if (p.subOrder != 0 && p.subOrder != lastSubOrder) {
                QString subText = subHeaderText(p.subOrder);
                if (!subText.isEmpty()) {
                    int srow = ui->tableWidgetParts->rowCount();
                    ui->tableWidgetParts->insertRow(srow);

                    QTableWidgetItem *subHdr = new QTableWidgetItem("  " + subText);
                    QFont fs = subHdr->font();
                    fs.setBold(true);
                    subHdr->setFont(fs);
                    subHdr->setFlags(Qt::ItemIsEnabled);
                    subHdr->setBackground(QBrush(Qt::lightGray));
                    ui->tableWidgetParts->setItem(srow, 0, subHdr);
                }
                lastSubOrder = p.subOrder;
            }

            int row = ui->tableWidgetParts->rowCount();
            ui->tableWidgetParts->insertRow(row);

            // Material Number
            auto *matItem = new QTableWidgetItem(p.materialId);
            ui->tableWidgetParts->setItem(row, 0, matItem);

            // Item Name
            auto *nameItem = new QTableWidgetItem(
                p.part.isEmpty() ? "(no details)" : p.part);
            ui->tableWidgetParts->setItem(row, 1, nameItem);

            // Storage Location
            auto *storItem = new QTableWidgetItem(p.storage);
            ui->tableWidgetParts->setItem(row, 2, storItem);

            // Quantity spinbox
            auto *spin = new QSpinBox(ui->tableWidgetParts);
            spin->setMinimum(0);
            spin->setMaximum(9999);
            spin->setValue(p.qty);
            ui->tableWidgetParts->setCellWidget(row, 3, spin);
        }

    }


    ui->tableWidgetParts->resizeColumnToContents(0); // Material Number
    ui->tableWidgetParts->resizeColumnToContents(1); // Item Name
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

    // ⚠️ Use the SAME column indices that already work
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
        /*
        int qty = 1;
        QString loc;

        if (!qloc.isEmpty()) {
            QStringList parts = qloc.split(' ', QString::SkipEmptyParts);
            if (!parts.isEmpty()) {
                bool ok = false;
                int q = parts[0].toInt(&ok);
                if (ok && q > 0) qty = q;
                if (parts.size() > 1) loc = parts[1];
            }
        }
        */

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
    if (index <= 0)
        return;

    QString email = ui->comboRequester->itemData(index, Qt::UserRole).toString();
    QString id    = ui->comboRequester->itemData(index, Qt::UserRole + 1).toString();

    qDebug() << "Requester email:" << email;
    qDebug() << "Requester ID:" << id;

    // If you have fields:
    // ui->lineRequesterEmail->setText(email);
    // ui->lineRequesterId->setText(id);
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

    if (lower.contains("mk5")) {
        m_jobRobotFromFile = "MK5";
    } else if (lower.contains("doublefold") || lower.contains("df")) {
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
    applyJobToChecklist();
}


void AutoPurchase::onJobFileLoadFailed(const QString &error)
{
    ui->labelStatus->setText("Job file load error: " + error);
}

void AutoPurchase::applyJobToChecklist()
{
    if (m_rowsJob.isEmpty())
        return;

    if (m_jobColChecklist < 0 || m_jobColNeed < 0) {
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
        QString jobChecklist = getJobCol(r, m_jobColChecklist);
        QString needStr      = getJobCol(r, m_jobColNeed);

        if (jobChecklist.isEmpty())
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
            if (label.contains(jobChecklist, Qt::CaseInsensitive)) {
                it->setCheckState(Qt::Checked);
                it->setData(Qt::UserRole + 1, needQty);  // multiplier
            }
        }
    }

    // 3) Now rebuild the parts table with these auto-checked items
    rebuildPartsFromChecklist();
}


void AutoPurchase::detectJobColumns()
{
    m_jobHeaderRow    = 0;
    m_jobColRobot     = -1;
    m_jobColChecklist = -1;
    m_jobColNeed      = -1;

    if (m_rowsJob.isEmpty())
        return;

    QVariantList headerRow = m_rowsJob[m_jobHeaderRow].toList();

    for (int c = 0; c < headerRow.size(); ++c) {
        QString h = headerRow[c].toString().trimmed().toLower();

        if (h.contains("robot") || h.contains("tool")) {
            m_jobColRobot = c;
        } else if (h.contains("checklist") || h.contains("item")) {
            m_jobColChecklist = c;
        } else if (h.contains("need") || h.contains("qty") || h.contains("quantity")) {
            m_jobColNeed = c;
        }
    }

    qDebug() << "Job header detected:"
             << "robot col =" << m_jobColRobot
             << "checklist col =" << m_jobColChecklist
             << "need col =" << m_jobColNeed;
}
