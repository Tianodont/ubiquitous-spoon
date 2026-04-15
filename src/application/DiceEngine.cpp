#include "DiceEngine.h"

#include <QRandomGenerator>
#include <QRegularExpression>

QList<DiceEngine::RollComponent> DiceEngine::parse(const QString& expression, bool* ok) const {
    QList<RollComponent> components;
    const QString normalized = expression.trimmed().remove(' ');
    if (normalized.isEmpty()) {
        *ok = false;
        return {};
    }

    QRegularExpression tokenPattern(R"(([+-]?)(\d*d\d+|\d+))");
    QRegularExpressionMatchIterator it = tokenPattern.globalMatch(normalized);

    int consumed = 0;
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        if (match.capturedStart() != consumed) {
            *ok = false;
            return {};
        }

        RollComponent component;
        component.sign = (match.captured(1) == "-") ? -1 : 1;

        const QString token = match.captured(2);
        const int dIndex = token.indexOf('d');
        if (dIndex >= 0) {
            component.isDice = true;
            const QString left = token.left(dIndex);
            component.count = left.isEmpty() ? 1 : left.toInt();
            component.sides = token.mid(dIndex + 1).toInt();
            if (component.count <= 0 || component.sides <= 0 || component.count > 100 || component.sides > 1000) {
                *ok = false;
                return {};
            }
        } else {
            component.constant = token.toInt();
        }

        consumed = match.capturedEnd();
        components.append(component);
    }

    *ok = (consumed == normalized.size() && !components.isEmpty());
    return *ok ? components : QList<RollComponent>{};
}

DiceEngine::RollResult DiceEngine::roll(const QString& expression) const {
    RollResult result;
    result.expression = expression;

    bool ok = false;
    result.components = parse(expression, &ok);
    if (!ok) {
        return result;
    }

    QStringList pieces;
    for (RollComponent& component : result.components) {
        QString term;
        if (component.isDice) {
            int subtotal = 0;
            QStringList rollText;
            for (int i = 0; i < component.count; ++i) {
                const int value = QRandomGenerator::global()->bounded(1, component.sides + 1);
                component.rolls.append(value);
                subtotal += value;
                rollText << QString::number(value);
            }
            component.subtotal = subtotal * component.sign;
            term = QString("(%1)").arg(rollText.join("+"));
        } else {
            component.subtotal = component.constant * component.sign;
            term = QString::number(component.constant);
        }

        if (pieces.isEmpty()) {
            pieces << (component.sign < 0 ? "-" + term : term);
        } else {
            pieces << (component.sign < 0 ? "-" + term : "+" + term);
        }
        result.total += component.subtotal;
    }

    result.breakdown = pieces.join("");
    result.ok = true;
    return result;
}
