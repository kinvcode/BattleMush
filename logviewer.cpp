#include "LogViewer.h"
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTextImageFormat>

LogViewer::LogViewer(QTextEdit* textEdit) : m_textEdit(textEdit) {
    m_colorMap[LogLevel::Info] = Qt::black;
    m_colorMap[LogLevel::Warning] = QColor("#FFA500"); // orange
    m_colorMap[LogLevel::Error] = Qt::red;
    m_colorMap[LogLevel::Debug] = Qt::blue;

    m_iconMap[LogLevel::Info] = QPixmap(":/assets/images/icons/a.svg");
    m_iconMap[LogLevel::Warning] = QPixmap(":/assets/images/icons/b.svg");
    m_iconMap[LogLevel::Error] = QPixmap(":/assets/images/icons/c.svg");
    m_iconMap[LogLevel::Debug] = QPixmap(":/assets/images/icons/a.svg");
}

void LogViewer::log(LogLevel level, const QString& message) {
    if (!m_textEdit) return;

    m_textEdit->moveCursor(QTextCursor::End);

    QTextCursor cursor = m_textEdit->textCursor();

    // 插入图标
    QPixmap pix = m_iconMap[level].scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QTextImageFormat imgFormat;
    QString imgName = QString("log_icon_%1").arg(static_cast<int>(level));
    m_textEdit->document()->addResource(QTextDocument::ImageResource, QUrl(imgName), QVariant(pix));
    imgFormat.setName(imgName);
    cursor.insertImage(imgFormat);
    cursor.insertText(" ");

    // 插入彩色文本
    QTextCharFormat fmt;
    fmt.setForeground(m_colorMap[level]);
    cursor.setCharFormat(fmt);
    cursor.insertText(message + "\n");
}
