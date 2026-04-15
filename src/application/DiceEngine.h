#pragma once

#include <QList>
#include <QString>

class DiceEngine {
public:
    struct RollComponent {
        bool isDice = false;
        int sign = 1;
        int count = 0;
        int sides = 0;
        int constant = 0;
        QList<int> rolls;
        int subtotal = 0;
    };

    struct RollResult {
        bool ok = false;
        QString expression;
        int total = 0;
        QString breakdown;
        QList<RollComponent> components;
    };

    RollResult roll(const QString& expression) const;

private:
    QList<RollComponent> parse(const QString& expression, bool* ok) const;
};
