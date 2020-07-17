#include <QCoreApplication>
#include <QDebug>

#include <iostream>

#include "emcmodule.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(argc<2) {std::cerr << "Usage: " << argv[0] << " NMLFILE\n"; abort();}
    const char* inifile = argv[1];

    qDebug() << inifile << "\n";

    LinuxcncIni ini(inifile);

    qDebug() << ini.Ini_find("DISPLAY","INCREMENTS") <<"\n";

    //LinuxcncStat stat1;
    //
    //double* pos;
    //while(1){
    //    usleep(100*1000);
    //    stat1.poll();
    //    pos = stat1.actual_position();
    //    qDebug() << pos[0] <<" "<< pos[1] <<" "<< pos[2] <<"\n";
    //}

    LinuxcncCommand command1;
    char* cmd = "g1 x5 f100";
    //std::cin >> cmd;
    //qDebug()<<cmd <<"\n";
    //command1.home(cmd);
    command1.mode(EMC_TASK_MODE_MDI);
    command1.mdi(cmd,20);

    return a.exec();
}
