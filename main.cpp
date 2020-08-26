#include <QCoreApplication>
#include <QDebug>

#include <iostream>

#include "slot_test.h"
#include "qt_istat.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(argc<2) {cerr << "Usage: " << argv[0] << " NMLFILE\n"; abort();}
    const char* inifile = argv[1];

    qDebug() << inifile << "\n";

    _IStat* stat = new _IStat(inifile);

    slot_test* slot = new slot_test();

    //LinuxcncStat stat1;
    //
    //double* data;
    //int num;
    //LinuxcncStat::JointData* data1;
    //while(1){
    //    usleep(500*1000);
    //    stat1.poll();
    //    num = stat1.joints();
    //    data1 = stat1.joint();
    //    for(int i =0; i<num; ++i)
    //        qDebug()<<data1[i].homed<<" "<<data1[i].input<<" "<<data1[i].max_position_limit<<" "<<data1[i].velocity;
    //}

    //LinuxcncCommand command1;
    //char* cmd = "g1 x5 f100";
    //cin >> cmd;
    //qDebug()<<cmd <<"\n";
    //command1.home(cmd);
    //command1.mode(EMC_TASK_MODE_MDI);
    //command1.mdi(cmd,20);

    return a.exec();
}
