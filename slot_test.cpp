#include "slot_test.h"

#include <QDebug>

slot_test::slot_test(QObject *parent) : QObject(parent)
{
    this->gstat = new _GStat();
    qDebug()<< "gstat init success!";

    //connect(gstat, &_GStat::periodic, this, &slot_test::update_done);
    //connect(gstat, &_GStat::mode_manual, this, &slot_test::manual);
    //connect(gstat, &_GStat::current_x_rel_position, this, &slot_test::x_rel_position);
    connect(gstat, &_GStat::current_position, this, &slot_test::current_position);
}

void slot_test::update_done()
{
    qDebug()<< "test: update done!";
}

void slot_test::manual()
{
    qDebug()<< "test: choose manual mode!";
}

void slot_test::x_rel_position(double pos)
{
    qDebug()<< "test: current x real postion: "<< pos;
}

void slot_test::current_position(double* p1,double* p2, double* p3, double* p4)
{
    for(int i=0; i<3; ++i)
        qDebug()<<p1[i]<<" "<<p2[i]<<" "<<p3[i]<<" "<<p4[i];
}
