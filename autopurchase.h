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
    struct PartInfo {
            QString materialId;
            QString part;
            QString storage;
            int     qty = 1;
            bool    found = false;

            QString checklistName;
            int     subOrder = 0;

            bool hasId() const       { return !materialId.isEmpty(); }
            bool hasStorage() const  { return !storage.isEmpty(); }
        };

signals:
    void startExcelLoad(const QString &path);   // to worker
    void startMaterialsLoad(const QString &path);
    void startArmUpgradeLoad(const QString &path);   // ARM A/B upgrade file
        void startDmUpgradeLoad(const QString &path);

private slots:
    // Slot for checklist item changed (when user ticks/unticks)
    //void onChecklistItemChanged(QListWidgetItem *item);
    void onExcelSelectionChanged(int index);
    void onExcelLoaded(const QVariantList &rows);
    void onExcelLoadFailed(const QString &error);

    void onMaterialsLoaded(const QVariantList &rows);
    void onMaterialsLoadFailed(const QString &error);


    void onChecklistItemChanged(QListWidgetItem *item);

    void rebuildPartsFromChecklist();   // new helper

    void onArmUpgradeLoaded(const QVariantList &rows);
    void onArmUpgradeLoadFailed(const QString &error);
    void onDmUpgradeLoaded(const QVariantList &rows);
    void onDmUpgradeLoadFailed(const QString &error);


private:
    Ui::AutoPurchase *ui;
    //QStandardItemModel *m_model = nullptr;

    bool m_rebuildPending = false;

    QMovie *m_loadingMovie = nullptr;
    //QMovie *m_model = nullptr;
    QThread *m_excelThread = nullptr;
    ExcelLoader *m_loader = nullptr;

    QThread     *m_materialThread = nullptr;
    ExcelLoader *m_materialLoader = nullptr;


    //QVariantList m_rows;
    QVariantList m_rowsMaterials;
    //QStringList findPartsForMaterial(const QString &expr) const;

    QVariantList m_rowsArmUpgrade;
    QVariantList m_rowsDmUpgrade;

    QThread     *m_armThread   = nullptr;
    ExcelLoader *m_armLoader   = nullptr;
    QThread     *m_dmThread    = nullptr;
    ExcelLoader *m_dmLoader    = nullptr;

    //void loadXlsx(const QString &path);
    void buildChecklistFromExcelRows(const QVariantList &rows);

    QList<PartInfo> findPartsForMaterial(const QString &expr) const;
    //void addPartRow(const PartInfo &p);

    QList<PartInfo> findArmUpgradeParts(bool sideA) const;   // true = ARM A, false = ARM B
    QList<PartInfo> findDmUpgradeParts() const;

    void rebuildPartsTable(const QList<PartInfo> &allParts);
};

#endif // AUTOPURCHASE_H
