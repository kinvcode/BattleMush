#include "NoticeWidget.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyle>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

NoticeWidget::NoticeWidget(const QString& text, NoticeType type, QWidget* parent)
    : QWidget(parent) {
    setupUI(text, type);
}

void NoticeWidget::setupUI(const QString& text, NoticeType type) {
    setFixedHeight(50);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 10, 0);
    layout->setSpacing(10);

    QLabel* iconLabel = new QLabel(this);
    QLabel* textLabel = new QLabel(text, this);
    textLabel->setStyleSheet("color: white; font-weight: bold;");

    QPushButton* closeButton = new QPushButton("✕", this);
    closeButton->setFlat(true);
    closeButton->setStyleSheet("color: white; font-weight: bold; background: transparent;");
    closeButton->setFixedSize(20, 20);
    connect(closeButton, &QPushButton::clicked, this, &NoticeWidget::handleClose);

    QPixmap iconPixmap;
    switch (type) {
    case Success:
        iconPixmap = style()->standardPixmap(QStyle::SP_DialogApplyButton);
        setStyleSheet("background-color: #28a745; border-radius: 5px;");
        break;
    case Info:
        iconPixmap = style()->standardPixmap(QStyle::SP_MessageBoxInformation);
        setStyleSheet("background-color: #17a2b8; border-radius: 5px;");
        break;
    case Warning:
        iconPixmap = style()->standardPixmap(QStyle::SP_MessageBoxWarning);
        setStyleSheet("background-color: #f0ad4e; border-radius: 5px;");
        break;
    case Error:
        iconPixmap = style()->standardPixmap(QStyle::SP_MessageBoxCritical);
        setStyleSheet("background-color: #e83e8c; border-radius: 5px;");
        break;
    }

    iconLabel->setPixmap(iconPixmap);
    iconLabel->setFixedSize(24, 24);

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->addStretch();
    layout->addWidget(closeButton);

    // 设置透明度效果
    auto* effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
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
