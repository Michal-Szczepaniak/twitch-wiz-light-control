#include "wizcontroller.h"

#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

void logfileHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        qDebug() << msg;
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        qWarning() << msg;
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        qCritical() << msg;
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        qFatal() << msg;
        txt = QString("Fatal: %1").arg(msg);
        abort();
    }
    QFile outFile("log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
    QFile outFile("log");
    outFile.open(QIODevice::WriteOnly);
    outFile.close();
    QGuiApplication app(argc, argv);
    qInstallMessageHandler(logfileHandler);
    app.setOrganizationName("streamlights");
    app.setApplicationName("streamlights");

    QQmlApplicationEngine engine;
    WIZController controller;
    engine.rootContext()->setContextProperty("controller", &controller);

    const QUrl url(QStringLiteral("qrc:/streamlights/Main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() { QCoreApplication::exit(-1); },
    Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
