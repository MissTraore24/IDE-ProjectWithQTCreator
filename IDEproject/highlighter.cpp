#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // Format des mots-clés C++
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList cppKeywords = {"\\bint\\b", "\\bfloat\\b", "\\bdouble\\b", "\\breturn\\b", "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b"};
    foreach (const QString &pattern, cppKeywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Format des mots-clés Python
    QStringList pythonKeywords = {"\\bdef\\b", "\\bclass\\b", "\\bimport\\b", "\\bfrom\\b", "\\bas\\b", "\\breturn\\b", "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b"};
    foreach (const QString &pattern, pythonKeywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Format des mots-clés JavaScript
    QStringList jsKeywords = {"\\bfunction\\b", "\\bvar\\b", "\\blet\\b", "\\bconst\\b", "\\breturn\\b", "\\bif\\b", "\\belse\\b", "\\bfor\\b", "\\bwhile\\b"};
    foreach (const QString &pattern, jsKeywords) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Format des commentaires (valable pour C++, Python, et JavaScript)
    commentFormat.setForeground(Qt::green);
    rule.pattern = QRegularExpression("//[^\n]*|#.*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Format des chaînes de caractères (valable pour C++, Python, et JavaScript)
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*\"|'.*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Format des fonctions
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text) {
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
