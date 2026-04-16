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

    void on_comboOption1_currentIndexChanged(int index);

// ====== 成员变量 ======
private:
    // std::vector<MapRoomPath> m_clean_room_list;
    // std::vector<MapRoomPath> m_rest_point_list;
private:
    Ui::TaskEditDialog *ui;
    bool m_is_add;
    void initCompomentUI();
    void initData();

    void flushCleanRoomOption(); // 刷图列表
    void flushRestOption(); // 休息列表
    void flushQuestOption(); // 任务列表

    void flushCleanRoomPath(int room_id);
    void flushRestPoint(int rest_id);
public:
    Task getTask() const;

    void setTask(const Task& t);
};

#endif // TASKEDITDIALOG_H
