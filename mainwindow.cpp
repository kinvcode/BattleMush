#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringListModel>
#include <QDateTime>
#include <vmp/VMProtectSDK.h>
#include "driver/msdriverpro.h"
#include "driver/driverload.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    loadDriver();
    // ui->pushButton_2->setProperty("class","danger");

    // 表格
    ui->tableWidget->setColumnCount(11);
    ui->tableWidget->setHorizontalHeaderLabels({"怪物指针","怪物阵营", "怪物类型", "怪物代码","怪物名称","怪物血量","怪物坐标","宽度","高度","碰撞检测","建筑属性"});

    // 菜单
    ui->listWidget->setProperty("class","leftMenu");
    QListWidgetItem *item1 = new QListWidgetItem(QIcon(":/assets/images/icons/home.svg"), "启动");
    ui->listWidget->addItem(item1);
    QListWidgetItem *item2 = new QListWidgetItem(QIcon(":/assets/images/icons/vip.svg"), "定制");
    ui->listWidget->addItem(item2);
    QListWidgetItem *item3 = new QListWidgetItem(QIcon(":/assets/images/icons/safe.svg"), "配置");
    ui->listWidget->addItem(item3);
    QListWidgetItem *item4 = new QListWidgetItem(QIcon(":/assets/images/icons/sata.svg"), "功能");
    ui->listWidget->addItem(item4);
    ui->listWidget->setCurrentRow(0);

    // tab组件
    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->tabBar()->hide();

    // 日志
    ui->textEdit_log->setProperty("class","commonlog");
    console("系统初始化完毕！");

    ui->comboBox->addItem("组包秒杀");
    ui->comboBox->addItem("搬砖");
    ui->comboBox->addItem("升级");

    qApp->setStyleSheet(qApp->styleSheet()); // 重新设置样式表
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_listWidget_clicked(const QModelIndex &index)
{
    int cur = index.row();
    if(cur > -1 && cur < 4)
    {
        ui->tabWidget->setCurrentIndex(cur);
    }
}

void MainWindow::showFloatingNotice(const QString &message, NoticeWidget::NoticeType type)
{
    auto* notice = new NoticeWidget(message, type, this);
    notice->setAutoClose(3000);  // 自动关闭
    notice->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    notice->setAttribute(Qt::WA_TranslucentBackground);
    notice->setAttribute(Qt::WA_ShowWithoutActivating);

    // 设置大小
    QSize noticeSize = QSize(400, 50);
    notice->resize(noticeSize);

    // 确定主窗口右上角的位置（偏移量可调整）
    const int margin = 20;
    QPoint topRight = mapToGlobal(rect().topRight());
    QPoint pos = QPoint(topRight.x() - noticeSize.width() - margin,
                        topRight.y() + margin);

    notice->move(pos);
    notice->show();
}

void MainWindow::loadDriver()
{
    if (!MSDriverPRO::isLoadDriver())
    {
        DriverLoad driver_loader;
        bool driver_success = driver_loader.loadCommonDriver();
        if(!driver_success)
        {
            qApp->quit();
            return;
        }
    }
    if (MSDriverPRO::isLoadDriver()){
        console("变色龙安全驱动加载完毕~");
    }
}

void MainWindow::console(QString msg)
{
    int cursor = ui->textEdit_log->textCursor().blockNumber();
    if(cursor > 50000){
        ui->textEdit_log->clear();
    }
    // 获取当前日期时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 格式化为字符串
    QString formattedDateTime = currentDateTime.toString("HH:mm:ss ");
    msg = formattedDateTime + msg;
    ui->textEdit_log->append(msg);
    ui->textEdit_log->moveCursor(QTextCursor::End);
}


void MainWindow::on_pushButton_clicked()
{
    VMProtectBegin("__FUNCTION__");
    console("111111111111");
    VMProtectEnd();
}

