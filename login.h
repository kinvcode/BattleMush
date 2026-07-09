#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>

namespace Ui {
class Login;
}

class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    void loadLoginParam();
private slots:

    void on_pushButton_querycard_btn_clicked(); // 查卡完成

    void on_pushButton_login_btn_clicked();

    void on_pushButton_register_btn_clicked(); // 注册完成

    void on_pushButton_unbind_btn_clicked(); // 解绑完成

    void on_pushButton_renewal_btn_clicked(); // 续费完成

private:
    Ui::Login *ui;
protected:
    void showEvent(QShowEvent *event) override;
signals:
    void processingFinished();
public slots:
    void checkUpdate();
};

#endif // LOGIN_H
