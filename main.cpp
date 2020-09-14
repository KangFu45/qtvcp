#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>

#include <iostream>

#include "qt_action.h"
#include "logs.h"

src::severity_logger< severity_level > slog;
const char* file = "/home/fukang/333.log";

int main(int argc, char *argv[])
{
    //这是qt调用虚拟键盘的方式，但示教器不能用
    //qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    QApplication a(argc, argv);

    if(argc<2) {std::cerr << "Usage: " << argv[0] << " NMLFILE\n"; abort();}
    const char* inifile = argv[1];

    shared_ptr<_IStat> info(new _IStat(inifile));

    typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
    boost::shared_ptr< text_sink > pSink(new text_sink);

    {
        text_sink::locked_backend_ptr pBackend = pSink->locked_backend();

        pBackend->auto_flush(true);

        boost::shared_ptr< std::ofstream > pStream(new std::ofstream(file));
        assert(pStream->is_open());
        pBackend->add_stream(pStream);
    }

    logging::core::get()->add_sink(pSink);

    //src::logger lg;

    pSink->set_formatter(expr::stream
                         << expr::attr< unsigned int >("RecordID")
                         << " [" << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%d.%m.%Y %H:%M:%S.%f")
                         << "] [" << expr::attr< severity_level >("Severity")
                         << "] ["
                         << expr::smessage << "]");

    attrs::counter< unsigned int > RecordID(1);
    logging::core::get()->add_global_attribute("RecordID", RecordID);

    attrs::local_clock TimeStamp;
    logging::core::get()->add_global_attribute("TimeStamp", TimeStamp);

    BOOST_LOG_FUNC();

    pSink->set_filter(
                expr::attr< severity_level >("Severity").or_default(normal) >= normal);

    BOOST_LOG_SEV(slog, normal) << "Load inifile: " << inifile;

    //QDesktopWidget* desk = QApplication::desktop();

    MainWindow w(info);
    w.setFixedSize(800,480);
    w.show();

    return a.exec();
}
