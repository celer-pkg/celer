// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "codehelper.h"
#include "effectmanager.h"
#include "syntaxhighlighterdata.h"
#include <QString>

CodeHelper::CodeHelper(QObject *parent)
    : QObject{parent}
{
    m_effectManager = static_cast<EffectManager *>(parent);
    m_codeCompletionModel = new CodeCompletionModel(this);
    m_codeCompletionTimer.setInterval(500);
    m_codeCompletionTimer.setSingleShot(true);
    connect(&m_codeCompletionTimer, &QTimer::timeout, this, &CodeHelper::showCodeCompletion);

    // Cache syntax highlight data
    auto args = SyntaxHighlighterData::reservedArgumentNames();
    for (const auto &arg : args)
        m_reservedArgumentNames << QString::fromUtf8(arg);
    auto funcs = SyntaxHighlighterData::reservedFunctionNames();
    for (const auto &func : funcs)
        m_reservedFunctionNames << QString::fromUtf8(func);
    auto tags = SyntaxHighlighterData::reservedTagNames();
    for (const auto &tag : tags)
        m_reservedTagNames << QString::fromUtf8(tag);

    std::sort(m_reservedArgumentNames.begin(), m_reservedArgumentNames.end());
    std::sort(m_reservedFunctionNames.begin(), m_reservedFunctionNames.end());
    std::sort(m_reservedTagNames.begin(), m_reservedTagNames.end());
}

// Process the keyCode and return true if key was handled and
// TextEdit itself shouldn't append it.
bool CodeHelper::processKey(QQuickTextEdit *textEdit, int keyCode, int modifiers)
{
    int position = textEdit->cursorPosition();
    const QString &code = textEdit->text();
    if (m_codeCompletionVisible) {
        // When code completition is visible, some keys have special meening
        if (keyCode == Qt::Key_Escape || keyCode == Qt::Key_Left || keyCode == Qt::Key_Right) {
            setCodeCompletionVisible(false);
            return true;
        } else if (keyCode == Qt::Key_Tab || keyCode == Qt::Key_Return) {
            applyCodeCompletion(textEdit);
            return true;
        } else if (keyCode == Qt::Key_Down) {
            m_codeCompletionModel->nextItem();
            return true;
        } else if (keyCode == Qt::Key_Up) {
            m_codeCompletionModel->previousItem();
            return true;
        } else {
            updateCodeCompletion(textEdit);
        }
    }
    if (keyCode == Qt::Key_Return) {
        // See if we are inside multiline comments
        bool multilineComment = false;
        QString prevCode = code.left(position);
        int codeStartPos = prevCode.lastIndexOf("/*");
        int codeEndPos = prevCode.lastIndexOf("*/");
        if (codeStartPos > codeEndPos)
            multilineComment = true;

        // Get only the previous line before pressing return
        QString singleLine = code.left(position);
        int lineStartPos = singleLine.lastIndexOf('\n');
        if (lineStartPos > -1)
            singleLine = singleLine.remove(0, lineStartPos + 1);

        QString indent = autoIndentGLSLNextLine(singleLine, multilineComment);
        textEdit->insert(position, indent);
        return true;
    } else if (keyCode == Qt::Key_BraceRight) {
        // Get only the current line, without the last '}'
        QString singleLine = code.left(position);
        int lineStartPos = singleLine.lastIndexOf('\n');
        if (lineStartPos > -1)
            singleLine = singleLine.remove(0, lineStartPos + 1);

        if (singleLine.trimmed().isEmpty()) {
            // Line only had '}' so unindent it (max) one step
            int startPos = std::max(int(position - singleLine.size()), position - 4);
            textEdit->remove(startPos, position);
        }
        return false;
    } else if (keyCode == Qt::Key_Tab) {
        // Replace tabs with spaces
        QString indent = QStringLiteral("    ");
        textEdit->insert(position, indent);
        return true;
    } else if ((modifiers & Qt::ControlModifier) && keyCode == Qt::Key_Space) {
        updateCodeCompletion(textEdit, true);
    }
    return false;
}


// Formats GLSL code with simple indent rules.
// Not a full parser so doesn't work with more complex code.
// For that, consider ClangFormat etc.
QString CodeHelper::autoIndentGLSLCode(const QString &code)
{
    QStringList out;
    // Index (indent level) currently
    int index = 0;
    // Index (indent level) for previous line
    int prevIndex = 0;
    // Indent spaces
    QString indent;
    // True when we are inside multiline comments (/* .. */)
    bool multilineComment = false;
    // Increased when command continues to next line (e.g. "if ()")
    int nextLineIndents = 0;
    // Stores indent before nextLineIndents started
    int indentBeforeSingles = 0;

    QStringList codeList = code.split('\n');
    for (const auto &cOrig : codeList) {
        QString c = cOrig.trimmed();

        if (c.isEmpty()) {
            // Lines with only spaces are emptied but kept
            out << c;
            continue;
        }

        bool isPreprocessor = c.startsWith('#');
        if (isPreprocessor) {
            // Preprocesser lines have zero intent
            out << c;
            continue;
        }

        bool isTag = c.startsWith('@');

        // Separate content after "//" to commenPart
        QString commentPart;
        int lineCommentIndex = c.indexOf("//");
        if (lineCommentIndex > -1) {
            commentPart = c.last(c.length() - lineCommentIndex);
            c = c.first(lineCommentIndex);
            // Move spaces from c to comment part (to also trim c)
            while (!c.isEmpty() && c.endsWith(' ')) {
                commentPart.prepend(' ');
                c.chop(1);
            }
        }

        // Multiline comments state
        int startComments = c.count("/*");
        int endComments = c.count("*/");
        int commendsChange = startComments - endComments;
        if (commendsChange > 0)
            multilineComment = true;
        else if (commendsChange < 0)
            multilineComment = false;

        if (multilineComment || endComments > startComments) {
            // Lines inside /* .. */ are not touched
            out << cOrig;
            continue;
        }

        // Check indent for next line
        int indexChange = 0;
        int startBlocks = c.count('{');
        int endBlocks = c.count('}');
        indexChange += startBlocks - endBlocks;
        int startBrackets = c.count('(');
        int endBrackets = c.count(')');
        indexChange += startBrackets - endBrackets;
        index += indexChange;
        index = std::max(index, 0);
        indent.clear();
        int currentIndex = indexChange > 0 ? prevIndex : index;
        if (!isTag) {
            if (currentIndex > 0 && startBlocks > 0 && endBlocks > 0) {
                // Note: "} else {", "} else if () {"
                // Indent one step lower
                currentIndex--;
            } else if (endBrackets > startBrackets) {
                // Note: "variable)"
                // Indent one step higher
                currentIndex++;
            }
            if (!c.startsWith('{'))
                currentIndex += nextLineIndents;

            if (!c.isEmpty() && startBlocks == 0 && endBlocks == 0 && indexChange == 0 && !c.endsWith(';') && !c.endsWith(',') && !c.endsWith('(') && !c.endsWith("*/")) {
                // Note: "if ()", "else if ()", "else", "for (..)"
                // Something that should continue to next line
                nextLineIndents++;
            } else if (nextLineIndents > 0) {
                // Return to indent before e.g. "if (thing) \n if (thing2) \n something;"
                nextLineIndents = indentBeforeSingles;
                indentBeforeSingles = nextLineIndents;
            }
        }

        // Apply indent
        QString singleIndentStep = QStringLiteral("    ");
        for (int i = 0; i < currentIndex; i++)
            indent += singleIndentStep;
        c.prepend(indent);

        out << (c + commentPart);
        prevIndex = index;
    }
    return out.join('\n');
}

// Takes in the previous line (before pressing return key)
// and returns suitable indent string with spaces.
QString CodeHelper::autoIndentGLSLNextLine(const QString &codeLine, bool multilineComment)
{
    int spaces = 0;

    // Check how many spaces previous line had
    for (int i = 0; i < codeLine.size() ; i++) {
        if (codeLine.at(i) == ' ')
            spaces++;
        else
            break;
    }

    QString c = codeLine.trimmed();
    bool isPreprocessor = c.startsWith('#');
    bool isTag = c.startsWith('@');

    // Remove content after "//"
    int lineCommentIndex = c.indexOf("//");
    if (lineCommentIndex > -1) {
        c = c.first(lineCommentIndex);
        c = c.trimmed();
    }

    if (!c.isEmpty() && !isPreprocessor && !isTag && !multilineComment) {
        // Check indent for next line
        int index = 0;
        int indexChange = 0;
        int startBlocks = c.count('{');
        int endBlocks = c.count('}');
        indexChange += startBlocks - endBlocks;
        int startBrackets = c.count('(');
        int endBrackets = c.count(')');
        indexChange += startBrackets - endBrackets;
        if (indexChange > 0) {
            // Note: "{" "if () {", "vec4 x = vec4("
            index++;
        } else if (indexChange < 0 && c.trimmed().size() > 1) {
            // Note: "something; }", but NOT "}" as it has be unindented already
            index--;
        }

        if (index == 0 && !c.isEmpty() && !c.endsWith(';') && !c.endsWith("*/") && !c.endsWith('}')) {
            // Note: "if ()", "else if ()", "else", "for (..)"
            // Something that should continue to next line
            index++;
        }
        spaces += 4 * index;
    }

    QString indent;
    indent.fill(' ', spaces);
    return '\n' + indent;
}

// Get current word under the cursor
// Word is letters, digits and "_".
// Also if word starts with "//" that is included
QString CodeHelper::getCurrentWord(QQuickTextEdit *textEdit)
{
    if (!textEdit)
        return QString();
    int cursorPosition = textEdit->cursorPosition();
    int cPos = cursorPosition - 1;
    int maxPos = textEdit->text().size();
    QString currentWord;
    QChar c = textEdit->getText(cPos, cPos + 1).front();
    while (cPos >= 0 && (c.isLetterOrNumber() || c == '_')) {
        currentWord.prepend(c);
        cPos--;
        c = textEdit->getText(cPos, cPos + 1).front();
    }
    // Special case of "@" tags
    if (cPos >= 1) {
        QString s = textEdit->getText(cPos, cPos + 1);
        if (s == QStringLiteral("@"))
            currentWord.prepend(QStringLiteral("@"));
    }
    cPos = cursorPosition;
    c = textEdit->getText(cPos, cPos + 1).front();
    while (cPos <= maxPos && (c.isLetterOrNumber() || c == '_')) {
        currentWord.append(c);
        cPos++;
        c = textEdit->getText(cPos, cPos + 1).front();
    }
    return currentWord;
}

// Remove the current word under cursor
void CodeHelper::removeCurrentWord(QQuickTextEdit *textEdit)
{
    if (!textEdit)
        return;
    int cursorPosition = textEdit->cursorPosition();
    int cPos = cursorPosition - 1;
    int maxPos = textEdit->text().size();
    int firstPos = 0;
    int lastPos = 0;
    QChar c = textEdit->getText(cPos, cPos + 1).front();
    while (cPos >= 0 && (c.isLetterOrNumber() || c == '_')) {
        cPos--;
        c = textEdit->getText(cPos, cPos + 1).front();
    }
    // Special case of "@" tags
    if (cPos >= 1) {
        QString s = textEdit->getText(cPos, cPos + 1);
        if (s == QStringLiteral("@"))
            cPos--;
    }
    firstPos = cPos + 1;
    cPos = cursorPosition;
    c = textEdit->getText(cPos, cPos + 1).front();
    while (cPos <= maxPos && (c.isLetterOrNumber() || c == '_')) {
        cPos++;
        c = textEdit->getText(cPos, cPos + 1).front();
    }
    lastPos = cPos;
    textEdit->remove(firstPos, lastPos);
}

bool CodeHelper::codeCompletionVisible() const
{
    return m_codeCompletionVisible;
}

void CodeHelper::setCodeCompletionVisible(bool visible)
{
    if (m_codeCompletionVisible == visible)
        return;

    m_codeCompletionVisible = visible;
    if (!m_codeCompletionVisible)
        m_codeCompletionModel->setCurrentIndex(0);

    Q_EMIT codeCompletionVisibleChanged();
}

// Update and show code completion popup
// When force is true, do this without a delay.
void CodeHelper::updateCodeCompletion(QQuickTextEdit *textEdit, bool force)
{
    m_textEdit = textEdit;
    if (force)
        showCodeCompletion();
    else
        m_codeCompletionTimer.start();
}

void CodeHelper::showCodeCompletion()
{
    if (!m_textEdit)
        return;

    QString currentWord = getCurrentWord(m_textEdit);
    QString currentWordCleaned = currentWord;
    bool isComment = false;

    // Check if word is comment/tag
    if (currentWordCleaned.size() >= 2 && currentWordCleaned.left(2) == "//") {
        isComment = true;
        currentWordCleaned = currentWordCleaned.right(currentWordCleaned.size() - 2);
    }

    m_codeCompletionModel->beginResetModel();
    m_codeCompletionModel->clearItems();

    if (!isComment) {
        for (const auto &a : m_reservedArgumentNames) {
            if (a.startsWith(currentWordCleaned, Qt::CaseInsensitive) && a.size() > currentWordCleaned.size())
                m_codeCompletionModel->addItem(a, CodeCompletionModel::TypeArgument);
        }

        for (const auto &a : m_reservedFunctionNames) {
            if (a.startsWith(currentWordCleaned, Qt::CaseInsensitive) && a.size() > currentWordCleaned.size())
                m_codeCompletionModel->addItem(a, CodeCompletionModel::TypeFunction);
        }

        for (const auto &a : m_reservedTagNames) {
            if (a.startsWith(currentWord, Qt::CaseInsensitive) && a.size() > currentWord.size())
                m_codeCompletionModel->addItem(a, CodeCompletionModel::TypeTag);
        }
    }

    m_codeCompletionModel->endResetModel();

    if (!m_codeCompletionModel->m_modelList.isEmpty()) {
        m_codeCompletionModel->setCurrentIndex(0);
        setCodeCompletionVisible(true);
    } else {
        setCodeCompletionVisible(false);
    }
}

CodeCompletionModel *CodeHelper::codeCompletionModel() const
{
    return m_codeCompletionModel;
}

void CodeHelper::applyCodeCompletion(QQuickTextEdit *textEdit)
{
    CodeCompletionModel::ModelData t = m_codeCompletionModel->currentItem();
    if (!t.name.isEmpty()) {
        // Replace the current word with code completion one
        removeCurrentWord(textEdit);
        textEdit->insert(textEdit->cursorPosition(), t.name);
        if (t.type == CodeCompletionModel::TypeFunction) {
            // For functions, place cursor between parentheses, e.g. "sin(|)".
            textEdit->setCursorPosition(textEdit->cursorPosition() - 1);
        }
    }
    setCodeCompletionVisible(false);
}
