#include "SkillDetailPanel.h"

#include <QCursor>
#include <QLabel>
#include <QRegularExpression>
#include <QTextBrowser>
#include <QVBoxLayout>

SkillDetailPanel::SkillDetailPanel(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto* title = new QLabel("Skill Details", this);
    m_text = new QTextBrowser(this);
    m_text->setOpenLinks(false);

    connect(m_text, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
        const QString value = url.toString();
        if (value.startsWith("dice:")) {
            emit diceExpressionClicked(value.mid(5));
            return;
        }
        if (value.startsWith("help:")) {
            emit helpTermClicked(value.mid(5), QCursor::pos());
        }
    });

    layout->addWidget(title);
    layout->addWidget(m_text);
}

void SkillDetailPanel::setSkill(const SkillNode& node, const QMap<QString, QString>&) {
    QString html;
    html += QString("<h2>%1</h2>").arg(node.name.toHtmlEscaped());
    html += QString("<p><i>%1</i></p>").arg(node.shortDescription.toHtmlEscaped());
    html += QString("<p>%1</p>").arg(decorateLongDescription(node.longDescription));

    if (!node.customFields.isEmpty()) {
        html += "<h3>Custom Fields</h3><ul>";
        for (auto it = node.customFields.begin(); it != node.customFields.end(); ++it) {
            html += QString("<li><b>%1:</b> %2</li>")
                .arg(it.key().toHtmlEscaped(), it.value().toHtmlEscaped());
        }
        html += "</ul>";
    }

    m_text->setHtml(html);
}

void SkillDetailPanel::clear() {
    m_text->clear();
}

QString SkillDetailPanel::decorateLongDescription(const QString& text) const {
    QString html = text.toHtmlEscaped();

    const QRegularExpression dicePattern(R"((\b\d*d\d+(?:[+-]\d*d?\d+)*\b))");
    html.replace(dicePattern, "<a href=\"dice:\\1\" style=\"text-decoration:underline;\">\\1</a>");

    const QRegularExpression helpPattern(R"(\[([^\]]+)\])");
    html.replace(helpPattern, "<a href=\"help:\\1\">[\\1]</a>");

    html.replace("\n", "<br/>");
    return html;
}
