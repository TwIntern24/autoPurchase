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

AutoPurchase::AutoPurchase(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoPurchase)
{
    ui->setupUi(this);
    ui->listWidgetChecklist->hide();
    ui->listWidgetParts->hide();

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
    delete ui;
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


QStringList AutoPurchase::findPartsForMaterial(const QString &expr) const
{
    QStringList result;

    if (m_rowsMaterials.isEmpty()) {
        result << QString("Materials Excel not loaded (expression: %1)").arg(expr);
        return result;
    }

    // helper to read cell from materials Excel
    auto getCol = [&](int r, int c) -> QString {
        if (r < 0 || r >= m_rowsMaterials.size()) return "";
        QVariantList cols = m_rowsMaterials[r].toList();
        if (c < 0 || c >= cols.size()) return "";
        return cols[c].toString().trimmed();
    };

    // 1) split by '+'  -> one token per material
    QStringList tokens = expr.split('+', QString::SkipEmptyParts);

    for (const QString &rawToken : tokens) {
        QString token = rawToken.trimmed();   // e.g. "610846", "E-20953*2"

        // 2) extract quantity (default 1)
        int qty = 1;
        int starPos = token.indexOf('*');
        if (starPos >= 0) {
            bool ok = false;
            int q = token.mid(starPos + 1).toInt(&ok);   // text after '*'
            if (ok && q > 0)
                qty = q;
            token = token.left(starPos);  // base id without "*2"
        }

        QString matId = token;           // e.g. "610846" or "E-20953"
        //QString matIdLower = matId.toLower();
        bool found = false;

        // 3) search this single id in the materials Excel
        //qDebug()<< m_rowsMaterials.size();
        //for(int i=0; i<10; i++){
            //qDebug()<<"i"+getCol(i,i);
       // }
        for (int r = 0; r < m_rowsMaterials.size(); ++r) {
            QString id   = getCol(r, 1);   // column 0 = material ID
            QString part = getCol(r, 5);   // column 1 = part name/number
            QString desc = getCol(r, 3);   // column 2 = description (optional)
            //qDebug()<< id+part+desc;

            if (id.compare(matId, Qt::CaseInsensitive) == 0) {
                QString line = QString("%1  x%2  -  %3")
                                   .arg(id)
                                   .arg(qty)
                                   .arg(part);
                if (!desc.isEmpty())
                    line += " (" + desc + ")";
                result << line;
                found = true;
                break;   // stop on first match
            }
        }

        // 4) if not found, still show id + qty
        if (!found) {
            result << QString("%1  x%2  (no details found)")
                         .arg(matId)
                         .arg(qty);
        }
    }

    if (result.isEmpty())
        result << QString("No parts for expression: %1").arg(expr);

    return result;
}

/*
void AutoPurchase::onChecklistItemChanged(QListWidgetItem *item)
{
    if (!item)
        return;

    if (item->checkState() != Qt::Checked)
        return;    // only when checked; you can add logic for unchecked if needed

    // get the full expression from the hidden data
    QString expr = item->data(Qt::UserRole).toString().trimmed();
    qDebug() << "Expression from item:" << expr;

    ui->listWidgetParts->clear();

    if (expr.isEmpty()) {
        ui->listWidgetParts->addItem("No material expression for this item.");
        ui->listWidgetParts->show();
        return;
    }

    QStringList lines = findPartsForMaterial(expr);
    qDebug() << "Parts lines count:" << lines.size();

    foreach (const QString &line, lines) {
        ui->listWidgetParts->addItem(line);
    }
    ui->listWidgetParts->show();
}
*/


void AutoPurchase::onChecklistItemChanged(QListWidgetItem * /*item*/)
{
    ui->listWidgetParts->clear();

    if (m_rowsMaterials.isEmpty()) {
        ui->listWidgetParts->addItem("Materials Excel not loaded yet.");
        ui->listWidgetParts->show();
        return;
    }

    bool anyChecked = false;

    for (int i = 0; i < ui->listWidgetChecklist->count(); ++i) {
        QListWidgetItem *it = ui->listWidgetChecklist->item(i);
        if (it->checkState() != Qt::Checked)
            continue;

        anyChecked = true;

        QString expr = it->data(Qt::UserRole).toString().trimmed();
        if (expr.isEmpty())
            continue;

        ui->listWidgetParts->addItem("=== " + it->text() + " ===");

        QStringList lines = findPartsForMaterial(expr);
        foreach (const QString &line, lines)
            ui->listWidgetParts->addItem("  " + line);
    }

    if (anyChecked)
        ui->listWidgetParts->show();
    else
        ui->listWidgetParts->hide();
}
