#include "appcontroller.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

// Easy one liner for me
// find . \( -name "*.cpp" -o -name "*.h" -o -name "*.qml" -o -name "*.svg" \) |
// \ entr -r sh -c 'cmake --build build && ./build/appDecibel'

// todo : add check for existing song matching new song when scanning
// folder/adding song
// todo : set playlist from localstorage on init
// todo : implement search
// todo : implement shuffle
// todo : implement setting and about
// todo : reduce cpu usage due to heavy ui updating on timer when a song playing
// todo : fix bug song playing twice for a short time when seek
// todo : better indicator to show which song and playlist is playing

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
