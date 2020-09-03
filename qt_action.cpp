#include "qt_action.h"

#include <iostream>
#include <fstream>

#include <QDebug>

#include "logs.h"

extern src::severity_logger< severity_level > slog;

_Lcnc_Action::_Lcnc_Action(shared_ptr<_IStat> info)
{
    STATUS = new _GStat;
    INFO = info;
    this->cmd = STATUS->cmd;
    shared_ptr<LinuxcncError> error(new LinuxcncError);
    this->error_channel = error;
    this->error_channel->Error_poll();
}

void _Lcnc_Action::SET_ESTOP_STATE(bool state)
{
    if(state)
        this->cmd->state(EMC_TASK_STATE_ESTOP);
    else
        this->cmd->state(EMC_TASK_STATE_ESTOP_RESET);
}

void _Lcnc_Action::SET_MACHINE_STATE(bool state)
{
    if(state)
        this->cmd->state(EMC_TASK_STATE_ON);
    else
        this->cmd->state(EMC_TASK_STATE_OFF);
}

void _Lcnc_Action::SET_MACHINE_HOMING(int joint)
{
    this->ensure_mode(EMC_TASK_MODE_MANUAL);
    this->cmd->teleop_enable(false);
    if(!bool(INFO->HOME_ALL_FLAG) && joint == -1){
        if(!this->home_all_warning_flag){
            this->home_all_warning_flag = true;
            emit STATUS->error("Home-all not available according to INI Joint Home sequence Set the joint sequence in the INI or modify the screen for individual home buttons to avoid this warning Press again to home Z axis Joint");
        }
        else{
            if(STATUS->is_all_homed_()){
                this->home_all_warning_flag = false;
                return;
            }
            // so linuxcnc is misonfigured or the Screen is built wrong (needs individual home buttons)
            // now we will fake individual home buttons by homing joints one at a time
            // but always start will Z - on a mill it's safer
            int zj = INFO->GET_JOINT_FROM_NAME.find('Z')->second;
            bool* homeds = STATUS->stat->homed();//manual delete
            if(!homeds[zj]){
                BOOST_LOG_SEV(slog, normal) << "Homing Joint: "<< zj;
                this->cmd->home(zj);
                emit STATUS->error("Home-all not available according to INI Joint Home sequence Press again to home next Joint");
                return;
            }
            int length = INFO->AVAILABLE_JOINTS.size();
            int* seqs= INFO->JOINT_SEQUENCE_LIST;
            for(int num = 0; num<length; ++num){
                int j = seqs[num];
                // at the end so all homed
                if(num == length - 1)
                    this->home_all_warning_flag = false;
                // one from end but end is already homed
                if(num == length - 2 && homeds[zj])
                    this->home_all_warning_flag = false;
                // Z joint is homed first outside this for loop
                if(j == zj) continue;
                // ok home it then stop and wait for next button push
                if(!homeds[j]){
                    BOOST_LOG_SEV(slog, normal) << "Homing Joint: "<< j;
                    this->cmd->home(j);
                    if(this->home_all_warning_flag)
                        emit STATUS->error("Home-all not available according to INI Joint Home sequence Press again to home next Joint");
                    break;
                }
            }

            delete [] homeds;
        }
    }
    else{
        BOOST_LOG_SEV(slog, normal) << "Homing Joint: "<< joint;
        this->cmd->home(joint);
    }
}

void _Lcnc_Action::SET_MACHINE_UNHOMED(int joint)
{
    this->ensure_mode(EMC_TASK_MODE_MANUAL);
    this->cmd->teleop_enable(false);
    this->cmd->unhome(joint);
}

void _Lcnc_Action::SET_AUTO_MODE()
{
    this->ensure_mode(EMC_TASK_MODE_AUTO);
}

// if called while on hard limit will set the flag and allow machine on
// if called with flag set and now off hard limits - resets the flag
void _Lcnc_Action::TOGGLE_LIMITS_OVERRIDE()
{
    if(STATUS->is_limits_override_set() && STATUS->is_hard_limits_tripped())
        emit STATUS->error("Can Not Reset Limits Override - Still On Hard Limits");
    else if(!(STATUS->is_limits_override_set()) && STATUS->is_hard_limits_tripped())
    {
        emit STATUS->error("Hard Limits Are Overridden!");
        this->cmd->override_limits();
    }
    else{
        emit STATUS->error("Hard Limits Are Reset To Active!");
        this->cmd->override_limits();
    }
}

void _Lcnc_Action::SET_MDI_MODE()
{
    this->ensure_mode(EMC_TASK_MODE_MDI);
}

void _Lcnc_Action::SET_MANUAL_MODE()
{
    this->ensure_mode(EMC_TASK_MODE_MANUAL);
}

void _Lcnc_Action::CALL_MDI(string code)
{
    this->ensure_mode(EMC_TASK_MODE_MDI);
    this->cmd->mdi(code);
}

int _Lcnc_Action::CALL_MDI_WAIT(string code, unsigned int time)
{
    BOOST_LOG_SEV(slog, normal) << "MDI_WAIT_COMMAND= " << code <<", maxt = " <<time;
    this->ensure_mode(EMC_TASK_MODE_MDI);
    //log.debug('MDI_COMMAND: {}'.format(l))
    this->cmd->mdi(code);
    int result = this->cmd->wait_complete(time);
    if(result == -1){
        BOOST_LOG_SEV(slog, normal) << "MDI_WAIT_COMMAND timeout past "<<time <<"sec. Error: "<<result;
        this->ABORT();
    }
    else if(result == RCS_DONE){
        BOOST_LOG_SEV(slog, normal) << "MDI_WAIT_COMMAND RCS error: " <<result;
        return -1;
    }
    string res = this->error_channel->Error_poll();
    if (!res.empty()){
        emit STATUS->error("error_channel().poll()");
        BOOST_LOG_SEV(slog, error) << "MDI_WAIT_COMMAND Error channel: "<<res;
        return -1;
    }
    return 0;
}

void _Lcnc_Action::CALL_INI_MDI(unsigned int number)
{
}

int _Lcnc_Action::CALL_OWORD(string code, unsigned int time )
{
    BOOST_LOG_SEV(slog, normal) << "OWORD_COMMAND= "<<code;
    this->ensure_mode(EMC_TASK_MODE_MDI);
    this->cmd->mdi(code);
    STATUS->stat->poll();
    while (STATUS->stat->exec_state() == EMC_TASK_EXEC_WAITING_FOR_MOTION_AND_IO
           || STATUS->stat->exec_state() == EMC_TASK_EXEC_WAITING_FOR_MOTION) {
        int result = this->cmd->wait_complete(time);
        if(result == -1){
            BOOST_LOG_SEV(slog, error) << "Oword timeout oast () Error = #"<<result;
            this->ABORT();
        }
        else if(result == RCS_ERROR){
            BOOST_LOG_SEV(slog, error) << "Oword RCS Error = "<<result;
            return -1;
        }
        string res = this->error_channel->Error_poll();
        if (!res.empty()){
            emit STATUS->error("error_channel().poll()");
            BOOST_LOG_SEV(slog, error) << "Oword Error: "<<res;
            return -1;
        }
        STATUS->stat->poll();
    }
    //TODO:
    //int result = this->cmd->wait_complete(time);
    //if(result == -1 || result == RCS_ERROR || this->error_channel->Error_poll().empty())
}

void _Lcnc_Action::UPDATE_VAR_FILE()
{
    this->ensure_mode(EMC_TASK_MODE_MANUAL);
    this->ensure_mode(EMC_TASK_MODE_MDI);
}

void _Lcnc_Action::OPEN_PROGRAM(string fname)
{
    this->prefilter_path = fname;
    this->ensure_mode(EMC_TASK_MODE_AUTO);
    // just .ngc file
    BOOST_LOG_SEV(slog, normal) << "Load program "<< fname;
    this->cmd->program_open(fname.c_str(), fname.size());
    if(STATUS->stat->file() == fname)
        emit STATUS->file_loaded(fname.c_str());
}

void _Lcnc_Action::SAVE_PROGRAM(string source, string fname)
{
    if(source.empty()) return;
    //if(fname.find(".") == -1) fname += ".ngc";

    ofstream outfile;
    outfile.open(fname.c_str(), ios_base::out | ios_base::trunc);
    if(outfile.is_open()){
        outfile<< source.c_str();
        STATUS->update_machine_log("Saved: " + fname);
        outfile.close();
    }
    //TODO: con't open execute
}

void _Lcnc_Action::SET_AXIS_ORIGIN(char axis, int value)
{
    string m1("XYZABCUVW");
    if((axis == ' ') || m1.find(axis) == m1.npos){
        BOOST_LOG_SEV(slog, warning) << "Couldn't set orgin -axis "<< axis <<"not recognized";
        return;
    }
    string m("G10 L20 P0 ");
    m += axis;
    m += to_string(value);

    EMC_TASK_MODE_ENUM premode = this->ensure_mode(EMC_TASK_MODE_MDI);
    this->cmd->mdi(m);
    this->cmd->wait_complete();
    this->ensure_mode(premode);
    this->RELOAD_DISPLAY();
}

// Adjust tool offsets so current position ends up the given value
void _Lcnc_Action::SET_TOOL_OFFSET(char axis, int value, bool fixture)
{
    int lnum = 10+int(fixture);
    string m("G10 L");
    m += lnum;
    m += " P";
    m += to_string(STATUS->stat->tool_in_spindle());
    m += " ";
    m += axis;
    m += to_string(value);

    EMC_TASK_MODE_ENUM premode = this->ensure_mode(EMC_TASK_MODE_MDI);
    this->cmd->mdi(m);
    this->cmd->wait_complete();
    this->cmd->mdi("G43");
    this->cmd->wait_complete();
    this->ensure_mode(premode);
    this->RELOAD_DISPLAY();
}

// Set actual tool offset in tool table to the given value
void _Lcnc_Action::SET_DIRECT_TOOL_OFFSET(char axis, int value)
{
    string m("G10 L1 P");
    m += to_string(STATUS->get_current_tool());
    m += " ";
    m += axis;
    m += to_string(value);

    EMC_TASK_MODE_ENUM premode = this->ensure_mode(EMC_TASK_MODE_MDI);
    this->cmd->mdi(m);
    this->cmd->wait_complete();
    this->cmd->mdi("G43");
    this->cmd->wait_complete();
    this->ensure_mode(premode);
    this->RELOAD_DISPLAY();
}

void _Lcnc_Action::RUN(unsigned int line)
{
    if(!STATUS->is_auto_mode())
        this->ensure_mode(EMC_TASK_MODE_AUTO);
    if(STATUS->is_auto_paused() && line == 0){
        this->cmd->emcauto(LOCAL_AUTO_STEP);
        return;
    }
    else if(!STATUS->is_auto_running())
        this->cmd->emcauto(LOCAL_AUTO_RUN, line);
}

void _Lcnc_Action::STEP()
{
    if(STATUS->is_auto_running() && !STATUS->is_auto_paused()){
        this->cmd->emcauto(LOCAL_AUTO_PAUSE);
        return;
    }
    if(STATUS->is_auto_paused())
        this->cmd->emcauto(LOCAL_AUTO_STEP);
}

void _Lcnc_Action::ABORT(){
    this->ensure_mode(EMC_TASK_MODE_AUTO);
    this->cmd->abort();
}

void _Lcnc_Action::PAUSE()
{
    if(!STATUS->stat->paused())
        this->cmd->emcauto(LOCAL_AUTO_PAUSE);
    else{
        BOOST_LOG_SEV(slog, normal) << "resume";
        this->cmd->emcauto(LOCAL_AUTO_RESUME);
    }
}

void _Lcnc_Action::SET_JOG_INCR(double incr, string text)
{
    STATUS->set_jog_increments(incr, text);
    for(int i=0; i<STATUS->stat->joints(); ++i)
        this->STOP_JOG(i);
}

void _Lcnc_Action::SET_JOG_INCR_ANGULAR(double incr, string text)
{
    STATUS->set_jog_increment_angular(incr, text);
    for(int i=0; i<STATUS->stat->joints(); ++i)
        this->STOP_JOG(i);
}

void _Lcnc_Action::SET_SPINDLE_FASTER(int number)
{
    return;
}

void _Lcnc_Action::SET_SPINDLE_SLOWER(int number)
{
    return;
}

void _Lcnc_Action::SET_SPINDLE_STOP(int number)
{
    return;
}

void _Lcnc_Action::SET_USER_SYSTEM(string system)
{
    // system must fix format G54 or g54
    string j = system.substr(1);
    if(j == "54" || j == "55" || j == "56" || j == "57" || j == "58"  || j == "59"
            || j == "59.1" || j == "59.2" || j == "59.3"){
        EMC_TASK_MODE_ENUM premode = this->ensure_mode(EMC_TASK_MODE_MDI);
        this->cmd->mdi(string("G") + j);
        this->cmd->wait_complete();
        this->ensure_mode(premode);
    }
}

void _Lcnc_Action::ZERO_G92_OFFSET()
{
    this->CALL_MDI("G92.1");
    this->RELOAD_DISPLAY();
}

void _Lcnc_Action::ZERO_ROTATIONAL_OFFSET()
{
    this->CALL_MDI("G10 L2 P0 R0");
    this->RELOAD_DISPLAY();
}

void _Lcnc_Action::ZERO_G5X_OFFSET(int num)
{
    EMC_TASK_MODE_ENUM premode = this->ensure_mode(EMC_TASK_MODE_MDI);
    string clear_command("G10 L2 P");
    clear_command += to_string(num);
    clear_command += " R0";
    foreach (const char& a, INFO->AVAILABLE_AXES) {
        clear_command += " ";
        clear_command += a;
        clear_command += "0";
    }

    this->cmd->mdi(clear_command);
    this->cmd->wait_complete();
    this->ensure_mode(premode);
    this->RELOAD_DISPLAY();
}

EMC_TASK_MODE_ENUM _Lcnc_Action::RECORD_CURRENT_MODE()
{
    EMC_TASK_MODE_ENUM mode = STATUS->get_current_mode();
    this->last_mode = mode;
    return mode;
}

void _Lcnc_Action::RESTORE_RECORDED_MODE()
{
    this->ensure_mode(this->last_mode);
}

void _Lcnc_Action::SET_SELECTED_JOINT(int data)
{
    STATUS->set_selected_joint(data);
}

void _Lcnc_Action::SET_SELECTED_AXIS(int data)
{
    STATUS->set_selected_axis(data);
}

// jog based on STATUS's rate and distance
// use joint number for joint or letter for axis jogging
void _Lcnc_Action::DO_JOG(int joint_axis, int direction)
{
    bool angular = false;
    LinuxcncStat::JointData* data = STATUS->stat->joint();
    if(int(data[joint_axis].jointType) == EMC_ANGULAR)
        angular = true;

    double distance, rate;
    if(angular){
        distance = STATUS->get_jog_increment_angular();
        rate = STATUS->get_jograte_angular()/60;
    }
    else{
        distance = STATUS->get_jog_increment();
        rate = STATUS->get_jograte();
        //rate = STATUS->get_jograte()/60;
    }
    this->JOG(joint_axis, direction, rate, distance);
}

// jog based on given variables
// checks for jog joint mode first
// 1==> joint jog, 0==> axis jog
void _Lcnc_Action::JOG(int jointnum, int direction, double rate, double distance)
{
    int j_or_a;
    bool jjogmode = this->get_jog_info(jointnum, j_or_a);
    if(direction == 0)
        this->cmd->jog(LOCAL_JOG_STOP, int(jjogmode), j_or_a);
    else{
        if(distance == 0)
            this->cmd->jog(LOCAL_JOG_CONTINUOUS, int(jjogmode), j_or_a, direction * rate);
        else
            this->cmd->jog(LOCAL_JOG_INCREMENT, int(jjogmode), j_or_a, direction * rate, distance);
    }
}
void _Lcnc_Action::STOP_JOG(int jointnum)
{
    if(STATUS->machine_is_on() && STATUS->is_man_mode()){
        int j_or_a;
        bool jjogmode = this->get_jog_info(jointnum, j_or_a);
        this->cmd->jog(LOCAL_JOG_STOP, jjogmode, j_or_a);
    }
}

void _Lcnc_Action::SET_GRAPHICS_VIEW(string view)
{
    return;
}

void _Lcnc_Action::ADJUST_GRAPHICS_PAN(int x, int y)
{
    return;
}

void _Lcnc_Action::ADJUST_GRAPHICS_ROTATE(int x, int y)
{
    return;
}

void _Lcnc_Action::SHUT_SYSTEM_DOWN_PROMPT()
{
    return;
}

void _Lcnc_Action::SHUT_SYSTEM_DOWN_NOW()
{
    return;
}

void _Lcnc_Action::UPDATE_MACHINE_LOG(string text, int option)
{
    return;
}

void _Lcnc_Action::CALL_DIALOG(string command)
{
    return;
}

void _Lcnc_Action::HIDE_POINTER()
{
    return;
}

//*******************Action helper functions***************************

// num_state == -1 or != num for None
// 1==> joint jog, 0==> axis jog
bool _Lcnc_Action::get_jog_info(int num, int& num_state)
{
    if(STATUS->stat->motion_mode() == EMC_TRAJ_MODE_FREE){
        this->jnum_check(num, num_state);
        return true;
    }
    num_state = num;
    return false;
}

void _Lcnc_Action::jnum_check(int num, int& num_state)
{
    // identity=1,serial=2,parallel=3,custom=4
    if(STATUS->stat->kinematics_type() != KINEMATICS_IDENTITY)
        BOOST_LOG_SEV(slog, warning) << "Joint jogging not supported for non-identity kinematics";
    if(num > INFO->JOINT_COUNT){
        BOOST_LOG_SEV(slog, error) << "computed joint number="<<num <<"exceeds jointcount="<<INFO->JOINT_COUNT;
        num_state = -1;
        return;
    }
    vector<unsigned short> joints = INFO->AVAILABLE_JOINTS;
    bool in = false;
    for(unsigned short& joint : joints){
        if(joint == num)
            in = true;
    }
    if(!in){
        BOOST_LOG_SEV(slog, warning) << "Joint "<<num <<"is not in available joints ";
        num_state = -1;
        return;
    }
    num_state = num;
}

EMC_TASK_MODE_ENUM _Lcnc_Action::ensure_mode(EMC_TASK_MODE_ENUM mode)
{
    int state;
    EMC_TASK_MODE_ENUM premode;
    STATUS->check_for_modes(state, mode, premode);
    // TODO:Don't have return value and some modes in all instances , so only execute false state
    if(state == -1){
        this->cmd->mode(mode);
        this->cmd->wait_complete();
    }
    return premode;
}

void _Lcnc_Action::open_filter_program(string fname)
{

}

void _Lcnc_Action::load_filter_result(string fname)
{

}

void _Lcnc_Action::mktemp()
{

}

void _Lcnc_Action::ENSURE_TRAJ_FREE_MODE()
{
    if(STATUS->get_jjogmode() != EMC_TRAJ_MODE_TELEOP)
        this->cmd->traj_mode(EMC_TRAJ_MODE_TELEOP);
}
