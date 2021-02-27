#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "hmiworker.h"


void LogOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s", localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s", localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
    QVariant a;
}


int main(int argc, char *argv[])
{
    qInstallMessageHandler(LogOutput);
	QDir::setCurrent(QStringLiteral(CONFIG_WORKING_DIR));

    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);


//    QQmlApplicationEngine engine;
//    const QUrl url(QStringLiteral("qrc:/main.qml"));
//    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
//                     &app, [url](QObject *obj, const QUrl &objUrl) {
//        if (!obj && url == objUrl)
//            QCoreApplication::exit(-1);
//    }, Qt::QueuedConnection);
//    engine.load(url);


    HMIWorker *worker = new HMIWorker;
    // QThread* thread = new QThread;
    // worker->moveToThread(thread);
    // QObject::connect(thread, SIGNAL(started()), worker, SLOT(start()));
	// QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
    // thread->start();
	worker->start();


    return app.exec();
}
