// ExcelLoader.cpp
#include "ExcelLoader.h"
#include <QAxObject>

#include <objbase.h>

ExcelLoader::ExcelLoader(QObject *parent)
    : QObject(parent)
{
}

void ExcelLoader::loadFile(const QString &path)
{
    // ---- COM initialization for this thread ----
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        emit loadFailed("COM initialization failed");
        return;
    }

    // Use a scope so we can safely call CoUninitialize() at the end
    {
        QAxObject excel("Excel.Application");
        excel.setProperty("Visible", false);

        QAxObject *workbooks = excel.querySubObject("Workbooks");
        QAxObject *workbook  = workbooks->querySubObject("Open(const QString&)", path);
        if (!workbook) {
            excel.dynamicCall("Quit()");
            emit loadFailed("Cannot open workbook");
            CoUninitialize();
            return;
        }

        QAxObject *sheet = workbook->querySubObject("Worksheets(int)", 1);
        if (!sheet) {
            workbook->dynamicCall("Close()");
            excel.dynamicCall("Quit()");
            emit loadFailed("Cannot open first sheet");
            CoUninitialize();
            return;
        }

        QAxObject *usedRange = sheet->querySubObject("UsedRange");
        QVariant var         = usedRange->dynamicCall("Value()");
        if (!var.isValid()) {
            workbook->dynamicCall("Close()");
            excel.dynamicCall("Quit()");
            emit loadFailed("Empty or invalid Excel data");
            CoUninitialize();
            return;
        }

        QVariantList rows = var.toList();

        workbook->dynamicCall("Close()");
        excel.dynamicCall("Quit()");

        emit loaded(rows);
    }

    // ---- COM cleanup for this thread ----
    CoUninitialize();
}

