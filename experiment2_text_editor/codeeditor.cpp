#include "codeeditor.h"

#include <QFont>
#include <QPainter>
#include <QPalette>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTextBlock>

#include <algorithm>

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4.0);
    setCursorWidth(1);

    connect(this, &QPlainTextEdit::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (!m_lineNumbersVisible) {
        return 0;
    }

    int digits = 1;
    int maxBlocks = std::max(1, blockCount());
    while (maxBlocks >= 10) {
        maxBlocks /= 10;
        ++digits;
    }

    return 8 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    QColor gutterColor = palette().color(QPalette::AlternateBase);
    gutterColor.setAlpha(255);
    painter.fillRect(event->rect(), gutterColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    painter.setFont(font());
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const int currentBlock = textCursor().blockNumber();
            painter.setPen(blockNumber == currentBlock
                               ? palette().color(QPalette::Text)
                               : palette().color(QPalette::Mid));
            painter.drawText(0, top,
                             m_lineNumberArea->width() - 5,
                             fontMetrics().height(),
                             Qt::AlignRight,
                             QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

bool CodeEditor::lineNumbersVisible() const
{
    return m_lineNumbersVisible;
}

void CodeEditor::setLineNumbersVisible(bool visible)
{
    if (m_lineNumbersVisible == visible) {
        return;
    }

    m_lineNumbersVisible = visible;
    m_lineNumberArea->setVisible(visible);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::setEditorBackgroundColor(const QColor &color)
{
    QPalette editorPalette = palette();
    editorPalette.setColor(QPalette::Base, color);

    QColor gutter = color.lighter(color.lightness() < 128 ? 125 : 96);
    gutter.setAlpha(255);
    editorPalette.setColor(QPalette::AlternateBase, gutter);
    setPalette(editorPalette);

    viewport()->setAutoFillBackground(true);
    viewport()->update();
    m_lineNumberArea->update();
    highlightCurrentLine();
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    const QRect contents = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(contents.left(), contents.top(),
              lineNumberAreaWidth(), contents.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy != 0) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(),
                                 m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = palette().color(QPalette::Base);
        lineColor = lineColor.lightness() < 128
                        ? QColor(QStringLiteral("#3a3d33"))
                        : QColor(QStringLiteral("#fff8c7"));
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);
    m_lineNumberArea->update();
}

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

