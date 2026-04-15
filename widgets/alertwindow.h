#ifndef ALERTWINDOW_H
#define ALERTWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QPalette>
#include <QApplication>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDebug>
#include <QKeyEvent>

class AlertWindow : public QWidget
{
    Q_OBJECT
public:
    AlertWindow(QString msg, QWidget *parent = nullptr);
private:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // ALERTWINDOW_H
