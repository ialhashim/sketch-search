#ifndef REMOVEDUP_H
#define REMOVEDUP_H

#include <QWidget>

namespace Ui {
class RemoveDup;
}

class RemoveDup : public QWidget
{
    Q_OBJECT

public:
    explicit RemoveDup(QWidget *parent = 0);
    ~RemoveDup();

public slots:
    void load();
    void removeDuplicates();

private:
    Ui::RemoveDup *ui;
};

#endif // REMOVEDUP_H
