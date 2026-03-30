#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logviewer.h"
#include "noticewidget.h"

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

private:
    Ui::MainWindow *ui;
    LogViewer* logger = nullptr;
    void showFloatingNotice(const QString& message, NoticeWidget::NoticeType type);
    void loadDriver();

public slots:
    void console(QString msg);
};
#endif // MAINWINDOW_H
