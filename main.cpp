#include "appcontroller.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// Easy one liner for me
// find . \( -name "*.cpp" -o -name "*.h" -o -name "*.qml" -o -name "*.svg" \) |
// \ entr -r sh -c 'cmake --build build && ./build/appDecibel'

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  app.setWindowIcon(QIcon(":/assets/icons/app-icon.svg"));

  // Set application metadata
  QCoreApplication::setApplicationName("Decibel");
  QCoreApplication::setOrganizationName("MyOrg");

  // Debug output for graphics information
  qDebug() << "Graphics API:" << app.platformName();

  // Create the app controller
  AppController controller;

  // Connect cleanup before app quits
  QObject::connect(&app, &QGuiApplication::aboutToQuit, &controller,
                   &AppController::cleanup);

  // Create QML engine
  QQmlApplicationEngine engine;

  // Expose controller to QML
  engine.rootContext()->setContextProperty("backend", &controller);

  // Load the main QML file
  engine.loadFromModule("Decibel", "Main");

  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
