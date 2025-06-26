#pragma once

#include <QTextEdit>
#include <QPixmap>
#include <QMap>
#include <QColor>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

class LogViewer {
public:
    LogViewer(QTextEdit* textEdit);

    void log(LogLevel level, const QString& message);

private:
    QTextEdit* m_textEdit;
    QMap<LogLevel, QColor> m_colorMap;
    QMap<LogLevel, QPixmap> m_iconMap;

    QString levelToString(LogLevel level);
};
