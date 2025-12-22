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
#include <QTimer>

#include "excelloader.h"

namespace Ui {
class ExcelLoader;
class AutoPurchase;
}

class QListWidgetItem;

struct RequesterInfo {
    QString name;
    QString email;
    QString id;
};

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

    void startJobFileLoad(const QString &path);

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

    void onRequesterChanged(int index);

    void onLoadJobFile();                               // browse job xlsx
    void onJobFileLoaded(const QVariantList &rows);     // job rows ready
    void onJobFileLoadFailed(const QString &error);     // job load failed
    //void onRobotSelected(int index);                    // robot combo changed

    void on_btnSearchMaterial_clicked();

    void on_btnSubmit_clicked();
    //void exportPdfFromCurrentTable();

    void on_btnClear_clicked();


private:
    Ui::AutoPurchase *ui;
    //QStandardItemModel *m_model = nullptr;

    bool m_rebuildPending = false;
    //int  m_rebuildGen = 0;
    QTimer m_rebuildTimer;
    int m_checkedCount = 0;

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

    QVector<RequesterInfo> m_requesters;
    void loadRequesters();

    QThread      *m_jobThread = nullptr;   // thread for job Excel
    ExcelLoader  *m_jobLoader = nullptr;   // loader for job Excel
    QVariantList m_rowsJob;      // rows from the automatic job Excel
    //void applyJobFromRows();     // apply Need column → checklist

    //void populateRobotDropdown();   // fill c from m_rowsJob
    void applyJobToChecklist();     // auto-check checklist + multipliers

    int m_jobHeaderRow      = 0;   // which row has headers (usually 0)
    //int m_jobColRobot       = -1;  // if you later want a robot column
    //int m_jobColChecklist   = -1;  // column for checklist label
    int m_jobColNeed        = -1;  // column for "Need"
    int m_jobColModule = -1;
    QString m_jobRobotFromFile;    // robot name inferred from file name

    bool m_jobReady       = false;   // job Excel loaded and columns detected
    bool m_checklistReady = false;   // robot matrix checklist built
    bool m_jobAppliedOnce = false;   // job auto-applied once

    void tryApplyJobToChecklist();

    void detectJobColumns();

    void updateSubmitEnabled();

    QList<PartInfo> m_manualParts;

    void on_btnClearManual_clicked();
};

#endif // AUTOPURCHASE_H
