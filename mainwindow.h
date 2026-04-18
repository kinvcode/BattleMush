#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logviewer.h"
#include "widgets/noticewidget.h"
#include "threads/threadmanager.h"
#include "widgets/alertwindow.h"
#include <QPointer>
#include "global/struct.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // 线程管理器
    ThreadManager* m_threadManager;
    QPointer<AlertWindow> m_alert;  // 警报组件指针
    static MainWindow* getInstance();  // 单例模式
private:
    Ui::MainWindow *ui;
    LogViewer* logger = nullptr;


    // 数据更新线程ID
    const int m_UpdateGameData_tid = 1;
    // 人物健康状态监控线程ID
    const int m_HealthMonitor_tid = 2;
    // 测试工具
    const int m_MonitorToolTest_tid = 3;
    // 击杀怪物线程
    const int m_KillMonster_tid = 4;

    // 提示框
    void showNotice(const QString& message);
    // 加载驱动
    void loadDriver();
    void initCompomentUI();
    void initConfig();
    void initDir();
    void initSinalBind();
    void initThreads();
    void initTask();

private slots:

    void on_listWidget_clicked(const QModelIndex &index);

    // 测试按钮
    void on_pushButton_get_user_pos_clicked();

    void on_lineEdit_auto_add_mp_rate_editingFinished();

    void on_lineEdit_auto_add_hp_rate_editingFinished();

    void on_checkBox_auto_add_mp_stateChanged(int arg1);

    void on_checkBox_auto_add_hp_stateChanged(int arg1);

private slots:
    void onHotkeyChanged(int key);

// 线程信号
private slots:
    void onStartAll();
    void onStopAll();
    void onAddThread();
    void onRemoveThread();
    void onStartThread();
    void onStopThread();
    void onPauseThread();
    void onResumeThread();
    void onThreadProgress(int threadId, int value);
    void onThreadFinished(int threadId);
    void onThreadStarted(int threadId, const QString& workerType);
    void onAllThreadsFinished();

    void on_pushButton_move_pos_list_clean_clicked();

    void on_pushButton_print_move_pos_list_clicked();

    void on_lineEdit_atk_range_general_x_editingFinished();

    void on_lineEdit_atk_range_general_y_editingFinished();

    void on_lineEdit_atk_range_skill1_x_editingFinished();

    void on_lineEdit_atk_range_skill1_y_editingFinished();

    void on_pushButton_waring_alert_clicked();

    void on_pushButton_print_ui_child_clicked();

    void on_pushButton_show_room_map_clicked();

    void on_pushButton_show_room_map_name_clicked();

    void on_pushButton_add_plan_clicked();

    void on_tableWidget_cellDoubleClicked(int row, int column);

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_pushButton_print_map_child_clicked();

    void on_checkBox_potion_num_stateChanged(int arg1);

    void on_lineEdit_potion_num_editingFinished();

    void on_checkBox_atk_switch_general_stateChanged(int arg1);

    void on_checkBox_atk_switch_skill1_stateChanged(int arg1);

    void on_checkBox_atk_switch_buff1_stateChanged(int arg1);

    void on_lineEdit_recast_buff1_editingFinished();

    void on_pushButton_show_room_map_items_clicked();

    void on_pushButton_print_map_collision_clicked();

public slots:
    void console(QString msg);
    void warning(QString msg);
    void onApplicationQuit();
    void updateCompoment(bool status);
signals:
    void saveConfig();

    // 普通函数
public:
    void updateTaskMonitor(const TaskInfo &task);
private:
    void refreshTable();
    void deletePlanRow(int row);
    void editPlanRow(int row);
};
#endif // MAINWINDOW_H
