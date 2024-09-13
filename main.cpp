#include "fcmpwidget.h"

#include <QApplication>
#include <QIcon>

// Add SpdLog
#include "spdlog/spdlog.h"
#include "spdlog/sinks/daily_file_sink.h"

QString GetApplicationVersion()
{
    QString strVersion("1.10.9.24");
    return strVersion;
}

int main(int argc, char *argv[])
{
    // see:
    // https://github.com/gabime/spdlog/wiki/0.-FAQ

    // Create a daily logger - a new file is created every day on 1:10am
    auto logger = spdlog::daily_logger_mt(/*"daily_logger"*/"dl", "logs/fcmp.txt", 1, 10);
    logger->set_level(spdlog::level::info);
    logger->flush_on(spdlog::level::info);
    spdlog::set_default_logger(logger);

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/client3.jpg"));
    a.setApplicationVersion(::GetApplicationVersion());

    spdlog::info("Application 'FCmp' starting!");

    FCmpWidget w;
    w.show();
    return a.exec();
}
