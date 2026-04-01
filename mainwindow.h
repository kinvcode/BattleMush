#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logviewer.h"
#include "widgets/noticewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_listWidget_clicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void on_lineEdit_auto_add_mp_rate_editingFinished();

    void on_lineEdit_auto_add_hp_rate_editingFinished();

    void on_checkBox_auto_add_mp_stateChanged(int arg1);

    void on_checkBox_auto_add_hp_stateChanged(int arg1);

    void on_lineEdit_auto_add_hp_rate_textEdited(const QString &arg1);

    void on_lineEdit_auto_add_mp_rate_textEdited(const QString &arg1);

private slots:
    void onHotkeyChanged(int key);

private:
    Ui::MainWindow *ui;
    LogViewer* logger = nullptr;
    // 提示框
    void showNotice(const QString& message);
    // 加载驱动
    void loadDriver();
    // initCompoment();
    void initCompomentUI();
    void initConfig();
    void initSinalBind();
    void initThreads();

public slots:
    void console(QString msg);

signals:
    void saveConfig();
};
#endif // MAINWINDOW_H
