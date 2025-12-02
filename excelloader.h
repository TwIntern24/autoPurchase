#ifndef EXCELLOADER_H
#define EXCELLOADER_H

// ExcelLoader.h
#pragma once

#include <QObject>
#include <QVariant>

class ExcelLoader : public QObject
{
    Q_OBJECT
public:
    explicit ExcelLoader(QObject *parent = nullptr);

public slots:
    void loadFile(const QString &path);   // runs in worker thread

signals:
    void loaded(const QVariantList &rows);  // success
    void loadFailed(const QString &error);  // error message
};


#endif // EXCELLOADER_H
