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

AutoPurchase::AutoPurchase(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoPurchase)
{
    ui->setupUi(this);
    ui->listWidgetChecklist->hide();

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
        ui->comboExcel->addItem("Double Fold",   base + "/../data/Repair_matrix_MK5 1.xlsx");
        ui->comboExcel->addItem("MK5",  base + "/../data/Repair_matrix DF.xlsx");

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
}

AutoPurchase::~AutoPurchase()
{
    if (m_excelThread) {
            m_excelThread->quit();
            m_excelThread->wait();
        }
    delete ui;
}

/*
void AutoPurchase::loadXlsx(const QString &path)
{
    QAxObject excel("Excel.Application");
        excel.setProperty("Visible", false);

        QAxObject *workbooks = excel.querySubObject("Workbooks");
        QAxObject *workbook = workbooks->querySubObject("Open(const QString&)", path);
        QAxObject *sheet = workbook->querySubObject("Worksheets(int)", 1); // first sheet

        auto *model = new QStandardItemModel(this);

        // find used range
        QAxObject *usedRange = sheet->querySubObject("UsedRange");
        QAxObject *rows = usedRange->querySubObject("Rows");
        QAxObject *cols = usedRange->querySubObject("Columns");

        int rowCount = rows->property("Count").toInt();
        int colCount = cols->property("Count").toInt();

        // read cells
        for (int r = 1; r <= rowCount; ++r) {
            QList<QStandardItem*> rowItems;
            for (int c = 1; c <= colCount; ++c) {
                QAxObject *cell = sheet->querySubObject("Cells(int,int)", r, c);
                rowItems << new QStandardItem(cell->property("Value").toString());
            }
            model->appendRow(rowItems);
        }

        ui->tableView->setModel(model);

        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");
}
*/

/*
void AutoPurchase::loadXlsx(const QString &path)
{
    QAxObject excel("Excel.Application");
    excel.setProperty("Visible", false);

    QAxObject *workbooks = excel.querySubObject("Workbooks");
    QAxObject *workbook  = workbooks->querySubObject("Open(const QString&)", path);
    QAxObject *sheet     = workbook->querySubObject("Worksheets(int)", 1); // first sheet

    QAxObject *usedRange = sheet->querySubObject("UsedRange");
    QVariant var         = usedRange->dynamicCall("Value()");

    if (!var.isValid()) {
        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");
        return;
    }

    QVariantList rows = var.toList();
    if (rows.isEmpty()) {
        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");
        return;
    }

    //  Only build checklist – no table model
    buildChecklistFromExcelRows(rows);

    workbook->dynamicCall("Close()");
    excel.dynamicCall("Quit()");


    int rowCount = rows.size();
       int colCount = rows.first().toList().size();

       m_model = new QStandardItemModel(this);
       m_model->setColumnCount(colCount);
       m_model->setRowCount(rowCount - 1);

       QVariantList headerRow = rows[0].toList();
       for (int c = 0; c < colCount; ++c) {
           QString headerText = (c < headerRow.size() ? headerRow[c].toString() : QString());
           m_model->setHeaderData(c, Qt::Horizontal, headerText);
       }

       for (int r = 1; r < rowCount; ++r) {
           QVariantList rowData = rows[r].toList();
           for (int c = 0; c < colCount; ++c) {
               QString text = (c < rowData.size() ? rowData[c].toString() : QString());
               m_model->setItem(r - 1, c, new QStandardItem(text));
           }

           //  Let UI update every 20 rows
           if (r % 20 == 0) {
               qApp->processEvents();
           }
       }

       //ui->tableView->setModel(m_model);
       //ui->tableView->resizeColumnsToContents();

       // build checklist from rows
       buildChecklistFromExcelRows(rows);

       workbook->dynamicCall("Close()");
       excel.dynamicCall("Quit()");
}
*/

/*
void AutoPurchase::loadXlsx(const QString &path)
{
    // 1) Start Excel (hidden)
    QAxObject excel("Excel.Application");
    excel.setProperty("Visible", false);

    QAxObject *workbooks = excel.querySubObject("Workbooks");
    QAxObject *workbook  = workbooks->querySubObject("Open(const QString&)", path);
    if (!workbook) {
        excel.dynamicCall("Quit()");
        return;
    }

    // first worksheet
    QAxObject *sheet = workbook->querySubObject("Worksheets(int)", 1);
    if (!sheet) {
        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");
        return;
    }

    // 2) Get all used cells as a QVariant
    QAxObject *usedRange = sheet->querySubObject("UsedRange");
    QVariant var         = usedRange->dynamicCall("Value()");

    if (!var.isValid()) {
        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");
        return;
    }

    QVariantList rows = var.toList();
    if (rows.isEmpty()) {
        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");
        return;
    }

    // give UI a chance to update spinner
    qApp->processEvents();

    // 3) Build checklist from those rows
    buildChecklistFromExcelRows(rows);

    // 4) Clean up COM objects
    workbook->dynamicCall("Close()");
    excel.dynamicCall("Quit()");
}
*/


/*
void AutoPurchase::onExcelSelectionChanged(int index)
{
    // Get file path stored in the selected combo item
    QString path = ui->comboExcel->itemData(index).toString();

    if (path.isEmpty()) {
        // "Select Excel..." or invalid item – do nothing
        ui->listWidgetChecklist->hide();
        ui->listWidgetChecklist->clear();
        return;
    }

    // Hide old checklist while new file loads
        ui->listWidgetChecklist->hide();

        // Show loading animation
        ui->labelLoading->show();
        if (m_loadingMovie) {
            m_loadingMovie->start();
        }
        qApp->processEvents();  // let the UI update before heavy work

    // Call your existing loader – it will build the checklist
    loadXlsx(path);

    if (m_loadingMovie) {
           m_loadingMovie->stop();
       }
       ui->labelLoading->hide();

       // Show checklist now that it has data
       ui->listWidgetChecklist->show();
}
*/

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
}


void AutoPurchase::buildChecklistFromExcelRows(const QVariantList &rows)
{
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

    enum Sec { None, Arm, DM, ZM };
    Sec current = None;

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
        if (cleanA.contains("lastdigit") && cleanA.contains("redmark"))
        {
            inUpgradeSection = true;
            inUpgradeTable = false;
            inVisualSection = false;
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
                inUpgradeSection = false;
                inUpgradeTable = false;
                continue;
            }

            if (inUpgradeTable && !colA.isEmpty())
            {
                QString label = QString("Upgrade: %1 (%2)").arg(colA, colB);
                auto *item = new QListWidgetItem(label, ui->listWidgetChecklist);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);
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
            }

            continue;
        }
    }
}

