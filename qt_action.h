#ifndef QT_ACTION_H
#define QT_ACTION_H

#include "qt_istat.h"
#include "hal_glib.h"

class _Lcnc_Action
{
public:
    _Lcnc_Action(shared_ptr<_IStat> info);

    shared_ptr<_IStat> INFO;
    _GStat* STATUS;

    shared_ptr<LinuxcncCommand> cmd;
    shared_ptr<LinuxcncError> error_channel;
    string prefilter_path;

    void SET_ESTOP_STATE(bool state);
    void SET_MACHINE_STATE(bool state);
    void SET_MACHINE_HOMING(int joint);
    void SET_MACHINE_UNHOMED(int joint);
    void SET_AUTO_MODE();
    void ENSURE_TRAJ_FREE_MODE();
    void TOGGLE_LIMITS_OVERRIDE();
    void SET_MDI_MODE();
    void SET_MANUAL_MODE();
    void CALL_MDI(string code);
    int CALL_MDI_WAIT(string code, unsigned int time = 5);
    void CALL_INI_MDI(unsigned int number);
    int CALL_OWORD(string code, unsigned int time = 5);
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
    void SET_JOG_RATE(float rate) {this->STATUS->set_jograte(rate); }
    void SET_JOG_RATE_ANGULAR(int rate) {this->STATUS->set_jograte_angular(rate);}
    void SET_JOG_INCR(double incr, string text);
    void SET_JOG_INCR_ANGULAR(double incr, string text);
    void SET_SPINDLE_ROTATION(int direction = 1, double rpm = 100, int number = -1) {this->cmd->spindle(direction, rpm, number);}
    void SET_SPINDLE_FASTER(int number = 0);
    void SET_SPINDLE_SLOWER(int number = 0);
    void SET_SPINDLE_STOP(int number = 0);
    void SET_USER_SYSTEM(string system);
    void ZERO_G92_OFFSET();
    void ZERO_ROTATIONAL_OFFSET();
    void ZERO_G5X_OFFSET(int num);
    EMC_TASK_MODE_ENUM RECORD_CURRENT_MODE();
    void RESTORE_RECORDED_MODE();
    void SET_SELECTED_JOINT(int data);
    void SET_SELECTED_AXIS(int data);
    void DO_JOG(int joint_axis, int direction);
    void JOG(int jointnum, int direction, double rate, double distance = 0);
    void STOP_JOG(int jointnum);
    void TOGGLE_FLOOD() {this->cmd->flood(this->STATUS->stat->flood());}
    void SET_FLOOD_ON() {this->cmd->flood(1);}
    void SET_FLOOD_OFF() {this->cmd->flood(0);}
    void TOGGLE_MIST() {this->cmd->mist(this->STATUS->stat->flood());}
    void SET_MIST_ON() {this->cmd->mist(1);}
    void SET_MIST_OFF() {this->cmd->mist(0);}
    void RELOAD_TOOLTABLE() {this->cmd->load_tool_table();}
    void TOGGLE_OPTIONAL_STOP() {this->cmd->set_optional_stop(this->STATUS->stat->optional_stop());}
    void SET_OPTIONAL_STOP_ON() {this->cmd->set_optional_stop(true);}
    void SET_OPTIONAL_STOP_OFF() {this->cmd->set_optional_stop(false);}
    void RELOAD_DISPLAY() {emit this->STATUS->reload_display();}
    void SET_GRAPHICS_VIEW(string view);
    void SET_GRAPHICS_GRID_SIZE(int size) {emit this->STATUS->graphics_view_changed();}//?
    void ADJUST_GRAPHICS_PAN(int x, int y);
    void ADJUST_GRAPHICS_ROTATE(int x, int y);
    void SHUT_SYSTEM_DOWN_PROMPT();
    void SHUT_SYSTEM_DOWN_NOW();
    void UPDATE_MACHINE_LOG(string text, int option = 0);
    void CALL_DIALOG(string command);
    void HIDE_POINTER();
    EMC_TASK_MODE_ENUM ensure_mode(EMC_TASK_MODE_ENUM mode);

private:

    bool home_all_warning_flag = false;
    EMC_TASK_MODE_ENUM last_mode;

    bool get_jog_info(int num, int& num_state);
    void jnum_check(int num, int& num_state);
    void open_filter_program(string fname);
    void load_filter_result(string fname);
    void mktemp();
};


#endif // QT_ACTION_H
