#include "ApplicationController.h"

#include "application/SkillTreeViewModel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

int ApplicationController::run(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);

    SkillTreeViewModel viewModel;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("skillTreeVM", &viewModel);
    engine.load(QUrl("qrc:/Main.qml"));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
