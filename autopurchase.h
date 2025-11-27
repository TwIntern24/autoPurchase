#ifndef AUTOPURCHASE_H
#define AUTOPURCHASE_H

#include <QMainWindow>
//#include "xlsxdocument.h"

namespace Ui {
class AutoPurchase;
}

class AutoPurchase : public QMainWindow
{
    Q_OBJECT

public:
    explicit AutoPurchase(QWidget *parent = 0);
    ~AutoPurchase();

private:
    Ui::AutoPurchase *ui;
    void loadXlsx(const QString &path);
};

#endif // AUTOPURCHASE_H
