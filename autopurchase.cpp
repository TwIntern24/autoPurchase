#include "autopurchase.h"
#include "ui_autopurchase.h"

#include <QFile>
#include <QTextStream>
#include <QStandardItemModel>
#include <QTableView>
#include <QAxObject>
#include <QStandardItemModel>

#include <QFileDialog>

AutoPurchase::AutoPurchase(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AutoPurchase)
{
    ui->setupUi(this);
    QString path = QFileDialog::getOpenFileName(
            this,
            "Select Excel File",
            "",
            "Excel Files (*.xlsx *.xls)"
        );

        if (!path.isEmpty()) {
            loadXlsx(path);
        }
}

AutoPurchase::~AutoPurchase()
{
    delete ui;
}


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
