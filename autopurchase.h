#ifndef AUTOPURCHASE_H
#define AUTOPURCHASE_H

#include <QMainWindow>
//#include "xlsxdocument.h"
//#include <QHash>
//#include <QStandardItemModel>
#include <QStringList>
#include <QVariantList>
#include <QMovie>
#include <QThread>

#include "excelloader.h"

namespace Ui {
class AutoPurchase;
}

class QListWidgetItem;

class AutoPurchase : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutoPurchase(QWidget *parent = nullptr);
    ~AutoPurchase();

signals:
    void startExcelLoad(const QString &path);   // to worker
    void startMaterialsLoad(const QString &path);

private slots:
    // Slot for checklist item changed (when user ticks/unticks)
    //void onChecklistItemChanged(QListWidgetItem *item);
    void onExcelSelectionChanged(int index);
    void onExcelLoaded(const QVariantList &rows);
    void onExcelLoadFailed(const QString &error);

    void onMaterialsLoaded(const QVariantList &rows);
    void onMaterialsLoadFailed(const QString &error);


    void onChecklistItemChanged(QListWidgetItem *item);


private:
    Ui::AutoPurchase *ui;
    //QStandardItemModel *m_model = nullptr;

    QMovie *m_loadingMovie = nullptr;
    //QMovie *m_model = nullptr;
    QThread *m_excelThread = nullptr;
    ExcelLoader *m_loader = nullptr;

    QThread     *m_materialThread = nullptr;
    ExcelLoader *m_materialLoader = nullptr;


    //QVariantList m_rows;
    QVariantList m_rowsMaterials;
    QStringList findPartsForMaterial(const QString &expr) const;

    //void loadXlsx(const QString &path);
    void buildChecklistFromExcelRows(const QVariantList &rows);
};

#endif // AUTOPURCHASE_H
