#ifndef HAL_GLIB_H
#define HAL_GLIB_H

#include <QObject>

#include "emcmodule.h"

class _GStat : public QObject
{
    Q_OBJECT //Emits signals in GObject for python

public:
    _GStat (LinuxcncStat* stat = nullptr);

    LinuxcncStat* stat;
    LinuxcncCommand* cmd;

    unsigned int current_jog_rate = 15;
    unsigned int current_angular_jog_rate = 360;
    unsigned int current_jog_distance = 0;
    char* current_jog_distance_text ;
    unsigned int current_jog_distance_angular = 0;
    char* current_jog_distance_angular_text ;
    short selected_joint = -1;
    char selected_axis ;

    void merge();
    void update();
    void forced_update();

private:
    bool status_active = false;
    bool is_all_homed = false;

signals:
    void periodic();
    void state_estop();
    void state_estop_reset();
    void state_on();
    void state_off();

    void homed();
    void unhomed();
    void all_homed();
    void not_all_homed();
    void override_limits_changed();

    void hard_limits_tripped();

    void mode_manual();
    void mode_auto();
    void mode_mdi();

    void interp_run();
    void interp_idle();
    void interp_pause();
    void interp_reading();
    void interp_waiting();

    void jograte_changed();
    void jograte_angular_changed();
    void jogincrement_changed();
    void jogincrement_angular_changed();
};

#endif // HAL_GLIB_H
