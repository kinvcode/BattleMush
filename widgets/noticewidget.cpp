#include "NoticeWidget.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

NoticeWidget::NoticeWidget(const QString& text, QWidget* parent)
    : QWidget(parent) {
    setupUI(text);
}

void NoticeWidget::setupUI(const QString& text) {

    setFixedHeight(40);
    setFixedWidth(400);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 10, 0);
    layout->setSpacing(10);

    QLabel* textLabel = new QLabel(this);
    textLabel->setStyleSheet("color: #052c65; font-weight: bold; background: #cfe2ff");
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setWordWrap(true);

    // 获取图标路径或base64编码
    QString iconPath = ":/assets/images/icons/3.svg";  // 使用资源文件中的图标

    // 使用 HTML 格式组合图标和文本
    QString htmlText = QString(
                           "<table cellspacing='0' cellpadding='0'>"
                           "  <tr>"
                           "    <td><img src='%1' width='24' height='24'></td>"
                           "    <td width='10'></td>"
                           "    <td>%2</td>"
                           "  </tr>"
                           "</table>"
                           ).arg(iconPath).arg(text);

    textLabel->setText(htmlText);

    layout->addWidget(textLabel, 1);

    layout->addStretch();

    // // 设置透明度效果
    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);

    // 无边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    // 背景透明
    setAttribute(Qt::WA_TranslucentBackground);
    // 禁止焦点
    setAttribute(Qt::WA_ShowWithoutActivating);
}

void NoticeWidget::setAutoClose(int msec) {
    if (!closeTimer) {
        closeTimer = new QTimer(this);
        closeTimer->setSingleShot(true);
        connect(closeTimer, &QTimer::timeout, this, &NoticeWidget::handleClose);
    }
    closeTimer->start(msec);
}

void NoticeWidget::handleClose() {
    startFadeOutAnimation();
}

void NoticeWidget::startFadeOutAnimation() {
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!effect) return;

    fadeAnimation = new QPropertyAnimation(effect, "opacity", this);
    fadeAnimation->setDuration(600);  // 600 ms 渐隐
    fadeAnimation->setStartValue(1.0);
    fadeAnimation->setEndValue(0.0);
    connect(fadeAnimation, &QPropertyAnimation::finished, this, &NoticeWidget::handleFadeOutFinished);
    fadeAnimation->start();
}

void NoticeWidget::handleFadeOutFinished() {
    this->deleteLater();
}
