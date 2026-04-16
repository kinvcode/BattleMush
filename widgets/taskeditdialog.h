#ifndef TASKEDITDIALOG_H
#define TASKEDITDIALOG_H

#include <QDialog>
#include "global/struct.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class TaskEditDialog;
}
QT_END_NAMESPACE

class TaskEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TaskEditDialog(QWidget *parent = nullptr, bool is_add = true);
    ~TaskEditDialog();

private slots:
    void on_comboType_currentIndexChanged(int index);

    void on_pushButton_save_clicked();

    void on_pushButton_cancel_clicked();

private:
    Ui::TaskEditDialog *ui;
    bool m_is_add;
    void initCompomentUI();

public:
    Task getTask() const;

    void setTask(const Task& t);
};

#endif // TASKEDITDIALOG_H
