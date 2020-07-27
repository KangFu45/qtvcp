#ifndef QT_ACTION_H
#define QT_ACTION_H

#include "qt_istat.h"
#include "hal_glib.h"

class _Lcnc_Action
{
public:
    _Lcnc_Action(shared_ptr<LinuxcncCommand> _cmd) {}

    shared_ptr<_IStat> INFO;
    shared_ptr<_GStat> STATUS;

    shared_ptr<LinuxcncCommand> cmd;
    string prefilter_path;

    void SET_ESTOP_STATE();
    void SET_MACHINE_STATE();
    void SET_MACHINE_HOMING(int joint);
    void SET_MACHINE_UNHOMED(int joint);
    void SET_AUTO_MODE();
    void TOGGLE_LIMITS_OVERRIDE();
    void SET_MDI_MODE();
    void SET_MANUAL_MODE();
    void CALL_MDI(string code);
    void CALL_MDI_WAIT(string code, unsigned int time = 5);
    void CALL_INI_MDI(unsigned int number);
    void CALL_OWORD(string code, unsigned int time = 5);
    void UPDATE_VAR_FILE();
    void OPEN_PROGRAM(string fname);
    void SAVE_PROGRAM(string source, string fname);
    void SET_AXIS_ORIGIN(char axis, int value);
    void SET_TOOL_OFFSET(char axis, int value, bool fixture = false);
    void SET_DIRECT_TOOL_OFFSET(char axis, int value);
    void RUN(unsigned int line = 0);
    void STEP();
    void ABORT();
    void PAUSE();
    void SET_MAX_VELOCITY_RATE(double rate) {this->cmd->maxvel(rate / 60.0);}//?
    void SET_RAPID_RATE(double rate) {this->cmd->rapidrate(rate / 100.0);}
    void SET_FEED_RATE(double rate) {this->cmd->feedrate(rate / 100.0);}
    void SET_SPINDLE_RATE(double rate, unsigned short number = 0) {this->cmd->spindleoverride(rate / 100.0, number);}
    void SET_JOG_RATE(int rate) {this->STATUS->set_jograte(rate); }

};

#endif // QT_ACTION_H
