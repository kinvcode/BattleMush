#include "alertwindow.h"
#include "global/globalplayer.h"

AlertWindow::AlertWindow(QString msg, QWidget *parent) : QWidget(parent)
{
    // 暂停所有线程

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowTransparentForInput);

    // 创建中央控件
    QWidget *centerWidget = new QWidget(this);
    centerWidget->setObjectName("centerWidget");

    // 先设置一个初始样式
    centerWidget->setStyleSheet(
        "#centerWidget {"
        "   background-color: rgba(255, 255, 0, 128);"
        "}"
        );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(centerWidget);

    // 添加文字
    QLabel *label = new QLabel(msg, centerWidget);
    label->setAlignment(Qt::AlignCenter);
    QFont font = label->font();
    font.setPointSize(50);
    font.setBold(true);
    label->setFont(font);
    label->setStyleSheet("color: red; background: transparent;font-size: 50px;font-weight: 800;");

    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->addWidget(label);

    // setFocusPolicy(Qt::StrongFocus);

    // 使用定时器动画（不依赖 Q_PROPERTY）
    QTimer *timer = new QTimer(this);
    int *opacity = new int(128);
    bool *increasing = new bool(true);

    connect(timer, &QTimer::timeout, this, [this, centerWidget, opacity, increasing]() {
        if (*increasing) {
            *opacity += 5;
            if (*opacity >= 204) {
                *opacity = 204;
                *increasing = false;
            }
        } else {
            *opacity -= 5;
            if (*opacity <= 51) {
                *opacity = 51;
                *increasing = true;
            }
        }

        // 动态更新样式表
        QString styleSheet = QString(
                                 "#centerWidget {"
                                 "   background-color: rgba(255, 255, 0, %1);"
                                 "}"
                                 ).arg(*opacity);

        centerWidget->setStyleSheet(styleSheet);
    });

    timer->start(15);

    showFullScreen();
}



void AlertWindow::keyPressEvent(QKeyEvent *event)
{
    // Gplayer.show_warning = false;
    // close();

    int sc = event->nativeScanCode();
    qDebug() << "当前sc按键：" << sc;
}

void AlertWindow::mousePressEvent(QMouseEvent *event)
{
    // Gplayer.show_warning = false;
    // close();
}

