#pragma once

#include <QWidget>
#include <QTimer>

class QLabel;
class QHBoxLayout;
class QPushButton;
class QPropertyAnimation;

class NoticeWidget : public QWidget {
    Q_OBJECT

public:
    enum NoticeType {
        Success,
        Info,
        Warning,
        Error
    };

    explicit NoticeWidget(const QString& text, NoticeType type, QWidget* parent = nullptr);
    void setAutoClose(int msec); // 设置自动关闭时间，单位毫秒

private slots:
    void handleClose();
    void handleFadeOutFinished();

private:
    void setupUI(const QString& text, NoticeType type);
    void startFadeOutAnimation();

    QTimer* closeTimer = nullptr;
    QPropertyAnimation* fadeAnimation = nullptr;
};
