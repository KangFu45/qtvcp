#ifndef SLOT_TEST_H
#define SLOT_TEST_H

#include <QObject>

#include "hal_glib.h"

class slot_test : public QObject
{
    Q_OBJECT
public:
    explicit slot_test(QObject *parent = nullptr);
    _GStat* gstat;

signals:

public slots:
    void update_done();
    void manual();
    void x_rel_position(double pos);
    void current_position(double* p1,double* p2, double* p3, double* p4);
};

#endif // SLOT_TEST_H
