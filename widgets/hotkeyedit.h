#ifndef HOTKEYEDIT_H
#define HOTKEYEDIT_H

#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

class HotkeyEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit HotkeyEdit(QWidget *parent = nullptr);

    int m_currentKey = 0;

    QString m_config_path = "";

    // 设置当前快捷键
    void setHotkey(int key);

    int getHotkey() const { return m_currentKey; }

    void updateDisplay();
protected:
    void keyPressEvent(QKeyEvent *event) override;
    QString scanCodeToString(int sc);
private:

};

#endif // HOTKEYEDIT_H
