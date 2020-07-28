#include "hal_glib.h"

#include <math.h>

#include <QTimer>
#include <QDebug>

_GStat::_GStat (shared_ptr<LinuxcncStat> stat)
{
    if(stat == nullptr){
        // The method is right?
        shared_ptr<LinuxcncStat> tmp(new LinuxcncStat);
        this->stat = tmp;
    }
    else
        this->stat = stat;

    shared_ptr<LinuxcncCommand> tmp(new LinuxcncCommand);
    this->cmd = tmp;
    this->fresh.tool_prep_number = 0;

    this->stat->poll();
    this->merge();

    //qDebug()<<"GStat: init stat, stat->polll(), merge() success!";

    QTimer* timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&_GStat::update);
    timer->start(500);
}

void _GStat::STATES(EMC_TASK_STATE_ENUM state)
{
    switch (state) {
    case EMC_TASK_STATE_ESTOP:
        emit this->state_estop();
        break;
    case EMC_TASK_STATE_ESTOP_RESET:
        emit this->state_estop_reset();
        break;
    case EMC_TASK_STATE_ON:
        emit this->state_on();
        break;
    case EMC_TASK_STATE_OFF:
        emit this->state_off();
        break;
    default:
        break;
    }
}

void _GStat::MODES(EMC_TASK_MODE_ENUM mode)
{
    switch (mode) {
    case EMC_TASK_MODE_MANUAL:
        emit this->mode_manual();
        break;
    case EMC_TASK_MODE_AUTO:
        emit this->mode_auto();
        break;
    case EMC_TASK_MODE_MDI:
        emit this->mode_mdi();
        break;
    default:
        break;
    }
}

void _GStat::INTERP(EMC_TASK_INTERP_ENUM interp)
{
    switch (interp) {
    case EMC_TASK_INTERP_WAITING:
        emit this->interp_waiting();
        break;
    case EMC_TASK_INTERP_READING:
        emit this->interp_reading();
        break;
    case EMC_TASK_INTERP_PAUSED:
        emit this->interp_pause();
        break;
    case EMC_TASK_INTERP_IDLE:
        emit this->interp_idle();
        break;
    default:
        break;
    }
}

void _GStat::merge()
{
    this->fresh.state = this->stat->task_state();
    this->fresh.mode = this->stat->task_mode();
    this->fresh.interp = this->stat->interp_state();
    if(this->stat->call_level() == 0)
        this->fresh.file = this->stat->file();//manual delete
    this->fresh.paused = this->stat->paused();
    this->fresh.line = this->stat->motion_line();
    this->fresh.homed = this->stat->homed();
    this->fresh.tool_in_spindle = this->stat->tool_in_spindle();
    this->fresh.tool_prep_number = -1;//Don't have Pin ,so default;
    this->fresh.motion_mode = this->stat->motion_mode();
    LinuxcncStat::SpindleData* spindledata = this->stat->spindle();//manual delete
    this->fresh.spindle_or = spindledata[0].override;
    this->fresh.feed_or = this->stat->feedrate();
    this->fresh.rapid_or = this->stat->rapidrate();
    this->fresh.max_velocity_or = this->stat->max_velocity();
    this->fresh.feed_hold = this->stat->feed_hold_enabled();
    this->fresh.g5x_index = this->stat->g5x_index();
    this->fresh.spindle_enabled = spindledata[0].enabled;
    this->fresh.spindle_direction = spindledata[0].direction;
    this->fresh.block_delete = this->stat->block_delete();
    this->fresh.optional_stop = this->stat->optional_stop();
    this->fresh.actual_spindle_speed = 0;//Don't have Pin
    this->fresh.flood = this->stat->flood();
    this->fresh.mist = this->stat->mist();
    this->fresh.current_z_rotation = this->stat->rotation_xy();
    this->fresh.current_tool_offset = this->stat->tool_offset();//manual delete
    //overrirde limits / hard limits
    LinuxcncStat::JointData* jointdata = this->stat->joint();//manual delete
    int num = this->stat->joints();
    unsigned char* or_limit_list = new unsigned char[num];//manual delete
    bool or_limit_set = false;
    bool hard_limit = false;
    unsigned char* hard_limit_list = new unsigned char[num*2];//manual delete
    for(int i=0; i<num; ++i){
        or_limit_list[i] = jointdata[i].override_limits;
        or_limit_set = or_limit_set || jointdata[i].override_limits;
        hard_limit = hard_limit || jointdata[i].min_hard_limit || jointdata[i].max_hard_limit;
        hard_limit_list[i] = jointdata[i].min_hard_limit;
        hard_limit_list[i+num] = jointdata[i].max_hard_limit;
    }
    this->fresh.override_limits = or_limit_list;
    this->fresh.override_limits_set = or_limit_set;
    this->fresh.hard_limits_tripped = hard_limit;
    this->fresh.hard_limit_list = hard_limit_list;
    //active G code
    int* gcodedata = this->stat->activegcode();//manual delete
    strings active_gcodes;
    string codes = "";
    for(int i=0; i<ACTIVE_G_CODES; ++i){
        if(i%10 == 0)
            active_gcodes.emplace_back(string("G"+to_string(gcodedata[i]/10)));
        else
            active_gcodes.emplace_back(string("G"+to_string(gcodedata[i]/10)+"."+to_string(gcodedata[i]%10)));
        codes += active_gcodes.back() + " ";
    }
    this->fresh.g_code = codes;
    //extract specific G code modes
    bool itime , fpm , fpr , css , rpm , metric = false;
    bool radius , diameter = false;
    foreach (const string& i, active_gcodes) {
        if(i == "G93") itime = true;
        else if(i == "G94") fpm = true;
        else if(i == "G95") fpr = true;
        else if(i == "G96") css = true;
        else if(i == "G97") rpm = true;
        else if(i == "G21") metric = true;
        else if(i == "G7") diameter = true;
        else if(i == "G8") radius = true;
    }
    this->fresh.itime = itime;
    this->fresh.fpm = fpm;
    this->fresh.fpr = fpr;
    this->fresh.css = css;
    this->fresh.rpm = rpm;
    this->fresh.metric = metric;
    this->fresh.radius = radius;
    this->fresh.diameter = diameter;
    this->fresh.spindle_speed = spindledata->speed;

    //active M codes
    int* mcodedata = this->stat->activemcode();//manual delete
    strings active_mcodes;
    string mcodes = "";
    for(int i=0; i<ACTIVE_M_CODES; ++i){
        active_mcodes.emplace_back(string("M"+to_string(mcodedata[i])));
        mcodes += active_mcodes.back() + " ";
    }
    this->fresh.m_code = mcodes;
    CANON_TOOL_TABLE* table = this->stat->tool_table();//manual delete
    this->fresh.tool_info = table[0];

    delete [] spindledata;
    delete [] gcodedata;
    delete [] mcodedata;
    delete [] table;
    delete [] jointdata;

    this->fresh.act_positions = nullptr;
    this->fresh.relp = nullptr;
    this->fresh.dtg = nullptr;
    this->fresh.joint_act_positions = nullptr;
}

void _GStat::update()
{
    //qDebug()<<"GStat: update() start!";
    if(!this->stat->poll()){
        this->status_active = false;
        emit this->periodic();
        return;
    }
    //qDebug()<<"GStat: stat->poll() success!";

    this->status_active = true;
    GStatData rot = this->fresh;
    this->merge();

    if(this->fresh.state != rot.state){
        if(this->fresh.state > EMC_TASK_STATE_ESTOP)
            emit this->state_estop_reset();
        else
            emit this->state_estop();
        emit this->state_off();
        emit this->interp_idle();
    }

    if(this->fresh.state != rot.state){
        if(rot.state == EMC_TASK_STATE_ON && this->fresh.state < EMC_TASK_STATE_ON)
            emit this->state_off();
        this->STATES(this->fresh.state);
        if(this->fresh.state == EMC_TASK_STATE_ON){
            rot.mode = EMC_TASK_MODE_NULL;
            rot.interp = EMC_TASK_INTERP_NULL;
        }
    }

    if(this->fresh.mode != rot.mode)
        MODES(this->fresh.mode);

    if(this->fresh.interp != rot.interp){
        if(!rot.interp || rot.interp == EMC_TASK_INTERP_IDLE)
            emit this->interp_run();
        INTERP(this->fresh.interp);
    }

    //paused
    if( this->fresh.paused != rot.paused)
        emit this->program_pause_changed(this->fresh.paused);
    //block delete
    if(this->fresh.block_delete != rot.block_delete)
        emit this->block_delete_changed(this->fresh.block_delete);
    //optional_stop
    if(this->fresh.optional_stop != rot.optional_stop)
        emit this->block_delete_changed(this->fresh.block_delete);
    //file changed
    if(this->fresh.file != rot.file){
        //if interpreter is reading or waiting, the new file
        //is a remap procedure, with the following test we
        //partly avoid emitting a signal in that case, which would cause
        //a reload of the preview and sourceview widgets.  A signal could
        //still be emitted if aborting a program shortly after it ran an
        //external file subroutine, but that is fixed by not updating the
        //file name if call level != 0 in the merge() function above.
        //do avoid that a signal is emited in that case, causing
        //a reload of the preview and sourceview widgets
        if(this->stat->interp_state() == EMC_TASK_INTERP_IDLE)
            emit this->file_loaded(this->fresh.file.c_str());
    }
    //ToDo : Find a way to avoid signal when the line changed due to
    //       a remap procedure, because the signal do highlight a wrong
    //       line in the code
    // current line
    if(this->fresh.line != rot.line)
        emit this->line_changed(this->fresh.line);

    if(this->fresh.tool_in_spindle != rot.tool_in_spindle)
        emit this->tool_in_spindle_changed(this->fresh.tool_in_spindle);

    if(this->fresh.tool_prep_number != rot.tool_prep_number)
        emit this->tool_prep_changed(this->fresh.tool_prep_number);

    if(this->fresh.motion_mode != rot.motion_mode)
        emit this->motion_mode_changed(this->fresh.motion_mode);

    //if the homed status has changed
    //check number of homed joints against number of available joints
    //if they are equal send the all-homed signal
    //else send the not-all-homed signal (with a string of unhomed joint numbers)
    //if a joint is homed send 'homed' (with a string of homed joint number)
    if(this->fresh.homed != rot.homed){
        int homed_joints = 0;
        string unhomed_joints = "";
        bool* homeds = this->stat->homed();//manual delete
        for(int i=0; i<this->stat->joints(); ++i){
            if(homeds[i]){
                ++homed_joints;
                emit this->homed(i);
            }
            else{
                emit this->unhomed(i);
                unhomed_joints += to_string(i);
            }
        }
        if(homed_joints == this->stat->joints()){
            this->is_all_homed = true;
            emit this->all_homed();
        }
        else{
            this->is_all_homed = false;
            emit this->not_all_homed(unhomed_joints.c_str());
        }
        delete [] homeds;
    }

    //override limits
    if(this->fresh.override_limits != rot.override_limits)
        emit this->override_limits_changed(this->fresh.override_limits_set, this->fresh.override_limits);
    //hard limits tripped
    if(this->fresh.hard_limit_list != rot.hard_limit_list)
        emit this->hard_limits_tripped(this->fresh.hard_limits_tripped, this->fresh.hard_limit_list);
    //current velocity
    emit this->current_feed_rate(this->stat->current_vel()*60.0);

    //all manual delete
    this->fresh.act_positions = this->stat->actual_position();
    //double* positions = this->stat->position();
    double* g5x_offsets = this->stat->g5x_offset();
    double* tool_offsets = this->stat->tool_offset();
    double* g92_offsets = this->stat->g92_offset();
    this->fresh.joint_act_positions = this->stat->joint_actual_position();
    this->fresh.dtg = this->stat->dtg();
    //x relative position
    emit this->current_x_rel_position(this->fresh.act_positions[0]-g5x_offsets[0]-tool_offsets[0]-g92_offsets[0]);

    //calculate position offsets (native units)
    this->fresh.relp = this->get_rel_position(this->fresh.act_positions, g5x_offsets, tool_offsets, g92_offsets);
    emit this->current_position(this->fresh.act_positions, this->fresh.relp,
                                this->fresh.dtg, this->fresh.joint_act_positions);//manual delete ptr in slot function

    //spindle control
    if((this->fresh.spindle_enabled != rot.spindle_enabled)
            || (this->fresh.spindle_direction != rot.spindle_direction))
        emit this->spindle_control_changed(this->fresh.spindle_enabled, this->fresh.spindle_direction);
    //requested spindle speed
    if(this->fresh.spindle_speed != rot.spindle_speed)
        emit this->requested_spindle_speed_changed(this->fresh.spindle_speed);
    //actual spindle speed
    if(this->fresh.actual_spindle_speed != rot.actual_spindle_speed)
        emit this->actual_spindle_speed_changed(this->fresh.actual_spindle_speed);
    //spindle override
    if(this->fresh.spindle_or != rot.spindle_or)
        emit this->spindle_override_changed(this->fresh.spindle_or*100);
    //feed override
    if(this->fresh.feed_or != rot.feed_or)
        emit this->feed_override_changed(this->fresh.feed_or*100);
    //rapid override
    if(this->fresh.rapid_or != rot.rapid_or)
        emit this->rapid_override_changed(this->fresh.feed_or*100);
    //max-velocity override
    if(this->fresh.max_velocity_or != rot.max_velocity_or)
        emit this->max_velocity_override_changed(this->fresh.max_velocity_or*60);
    //feed hold
    if(this->fresh.feed_hold != rot.feed_hold)
        emit this->feed_hold_enable_changed(this->fresh.feed_hold);
    //mist
    if(this->fresh.mist != rot.mist)
        emit this->mist_changed(this->fresh.mist);
    //flood
    if(this->fresh.flood != rot.flood)
        emit this->flood_changed(this->fresh.flood);
    //rotation around Z
    if(this->fresh.current_z_rotation != rot.current_z_rotation)
        emit this->current_z_rotation(this->fresh.current_z_rotation);
    //current tool offsets
    if(this->stat->pose_equal(this->fresh.current_tool_offset , rot.current_tool_offset))
        emit this->current_tool_offset(this->fresh.current_tool_offset);

    //***********Gcodes************************
    //G codes
    if(this->fresh.g_code != rot.g_code)
        emit this->g_code_changed(this->fresh.g_code.c_str());
    //metric mode g21
    if(this->fresh.metric != rot.metric)
        emit this->metric_mode_changed(this->fresh.metric);
    //G5x (active user system)
    if(this->fresh.g5x_index != rot.g5x_index)
        emit this->user_system_changed(this->fresh.g5x_index);
    //inverse time mode g93
    if(this->fresh.itime != rot.itime)
        emit this->itime_mode(this->fresh.itime);
    //feed per minute mode g94
    if(this->fresh.fpm != rot.fpm)
        emit this->fpm_mode(this->fresh.fpm);
    //feed per revolution mode g95
    if(this->fresh.fpr != rot.fpr)
        emit this->fpr_mode(this->fresh.fpr);
    //css mode g96
    if(this->fresh.css != rot.css)
        emit this->css_mode(this->fresh.css);
    //rpm mode g97
    if(this->fresh.rpm != rot.rpm)
        emit this->rpm_mode(this->fresh.rpm);
    //radius mode g8
    if(this->fresh.radius != rot.radius)
        emit this->radius_mode(this->fresh.radius);
    //diameter mode g7
    if(this->fresh.diameter != rot.diameter)
        emit this->diameter_mode(this->fresh.diameter);

    //***********Mcodes**********************
    //M codes
    if(this->fresh.m_code != rot.m_code)
        emit this->m_code_changed(this->fresh.m_code.c_str());
    //if(this->fresh.tool_info != rot.tool_info)
    //    emit this->tool_info_changed();

    delete [] g5x_offsets;
    delete [] tool_offsets;
    delete [] g92_offsets;

    //delete old ptr
    delete [] rot.current_tool_offset;
    delete [] rot.override_limits;
    delete [] rot.hard_limit_list;

    if(rot.act_positions != nullptr) delete rot.act_positions;
    if(rot.relp != nullptr) delete rot.relp;
    if(rot.dtg != nullptr) delete rot.dtg;
    if(rot.joint_act_positions != nullptr) delete rot.joint_act_positions;

    // AND DONE...
    emit this->periodic();
}

void _GStat::forced_update()
{

}

//*****************helper function*********************

static double radians(double angle){
    return ( angle / 180.0 ) * PI;
}

double* _GStat::get_rel_position(double* act_position, double* g5x_offset
                         , double* tool_offset, double* g92_offset)
{
    double x, y, z, a, b, c, u, v, w;
    x = act_position[0] - g5x_offset[0] - tool_offset[0];
    y = act_position[1] - g5x_offset[1] - tool_offset[1];
    z = act_position[2] - g5x_offset[2] - tool_offset[2];
    a = act_position[3] - g5x_offset[3] - tool_offset[3];
    b = act_position[4] - g5x_offset[4] - tool_offset[4];
    c = act_position[5] - g5x_offset[5] - tool_offset[5];
    u = act_position[6] - g5x_offset[6] - tool_offset[6];
    v = act_position[7] - g5x_offset[7] - tool_offset[7];
    w = act_position[8] - g5x_offset[8] - tool_offset[8];

    if(this->stat->rotation_xy() != 0){
        double t = radians(this->stat->rotation_xy());
        double xr = x*cos(t) - y*sin(t);
        double yr = x*sin(t) - y*cos(t);
        x = xr;
        y = yr;
    }

    double* relp = new double[9];//manual delete

    relp[0] = x - g92_offset[0];
    relp[1] = y - g92_offset[1];
    relp[2] = z - g92_offset[2];
    relp[3] = a - g92_offset[3];
    relp[4] = b - g92_offset[4];
    relp[5] = c - g92_offset[5];
    relp[6] = u - g92_offset[6];
    relp[7] = v - g92_offset[7];
    relp[8] = w - g92_offset[8];
    return relp;
}

//check for requied modes
//fail if mode is 0
//fail if machine is busy
//true if all ready in mode
//None if possible to change
// state == 1 and true ,state == 0 and none ,state == -1 and false
void _GStat::check_for_modes(int& state, EMC_TASK_MODE_ENUM mode, EMC_TASK_MODE_ENUM& premode)
{
    this->stat->poll();
    premode = this->stat->task_mode();

    if(this->stat->task_mode() == mode) {
        state = 1;
        return;
    }
    if(this->stat->task_mode() == EMC_TASK_MODE_AUTO
            && this->stat->interp_state() != EMC_TASK_INTERP_IDLE){
        state = 0;
        return;
    }
    state = -1;
}

void _GStat::set_jograte(unsigned int upm)
{
    this->current_jog_rate = upm;
    emit this->jograte_changed(upm);
}

void _GStat::set_jograte_angular(unsigned int rate)
{
    this->current_angular_jog_rate = rate;
    emit this->jograte_angular_changed(rate);
}

void _GStat::set_jog_increment_angular(unsigned int distance, string text)
{
    this->current_jog_distance_angular = distance;
    this->current_jog_distance_angular_text = text;
    emit this->jogincrement_angular_changed(distance, text);
}

void _GStat::set_jog_increments(unsigned int distance, string text)
{
    this->current_jog_distance  = distance;
    this->current_jog_distance_text = text;
    emit this->jogincrement_changed(distance, text);
}

void _GStat::set_selected_joint(short data)
{
    this->selected_joint = data;
    emit this->joint_selection_changed(data);
}

void _GStat::set_selected_axis(char data)
{
    this->selected_axis = data;
    emit this->axis_selection_changed(data);
}

bool _GStat::is_homing()
{
    LinuxcncStat::JointData* data = this->stat->joint();
    int num = this->stat->joints();
    for(int i=0; i<num; ++i){
        if(data[i].homing)
            return true;
    }
    delete [] data;
    return false;
}

bool _GStat::is_man_mode()
{
    this->stat->poll();
    return this->stat->task_mode() == EMC_TASK_MODE_MANUAL;
}

bool _GStat::is_mdi_mode()
{
    this->stat->poll();
    return this->stat->task_mode() == EMC_TASK_MODE_MDI;
}

bool _GStat::is_auto_mode()
{
    this->stat->poll();
    return this->stat->task_mode() == EMC_TASK_MODE_AUTO;
}
bool _GStat::is_on_and_idle()
{
    this->stat->poll();
    return this->stat->task_state() > EMC_TASK_STATE_OFF
            && this->stat->interp_state() == EMC_TASK_INTERP_IDLE;
}

bool _GStat::is_auto_running()
{
    this->stat->poll();
    return this->stat->task_mode() == EMC_TASK_MODE_AUTO
            && this->stat->interp_state() != EMC_TASK_INTERP_IDLE;
}

bool _GStat::is_interp_running()
{
    this->stat->poll();
    return this->stat->interp_state() != EMC_TASK_INTERP_IDLE;
}

bool _GStat::is_interp_paused()
{
    this->stat->poll();
    return this->stat->interp_state() == EMC_TASK_INTERP_PAUSED;
}

bool _GStat::is_interp_reading()
{
    this->stat->poll();
    return this->stat->interp_state() == EMC_TASK_INTERP_READING;
}

bool _GStat::is_interp_waiting()
{
    this->stat->poll();
    return this->stat->interp_state() == EMC_TASK_INTERP_WAITING;
}

bool _GStat::is_interp_idle()
{
    this->stat->poll();
    return this->stat->interp_state() == EMC_TASK_INTERP_IDLE;
}

bool _GStat::is_file_loaded()
{
    this->stat->poll();
    if(this->stat->file().empty())
        return false;
    else
        return true;
}

bool _GStat::is_spindle_on()
{
    //TODO:
    return false;
}

int _GStat::get_spindle_speed()
{
    return 0;
}

bool _GStat::is_joint_mode()
{
    this->stat->poll();
    return this->stat->motion_mode() == EMC_TRAJ_MODE_FREE;
}

int _GStat::get_current_tool()
{
    this->stat->poll();
    return this->stat->tool_in_spindle();
}

void _GStat::set_tool_touchoff(int tool, char axis, int value)
{
    EMC_TASK_MODE_ENUM premode;
    string m("G10 L10 P");
    m += to_string(tool);
    m += " ";
    m += axis;
    m += to_string(value);

    this->stat->poll();
    if(this->stat->task_mode() != EMC_TASK_MODE_MDI){
        premode = this->stat->task_mode();
        this->cmd->mode(EMC_TASK_MODE_MDI);
        this->cmd->wait_complete();
    }
    this->cmd->mdi(m);
    this->cmd->wait_complete();
    this->cmd->mdi("g43");
    this->cmd->wait_complete();
    if(premode)
        this->cmd->mode(premode);
}

void _GStat::set_axis_origin(char axis, int value)
{
    EMC_TASK_MODE_ENUM premode;
    string m("G10 L20 P0 ");
    m += axis;
    m += to_string(value);

    this->stat->poll();
    if(this->stat->task_mode() != EMC_TASK_MODE_MDI){
        premode = this->stat->task_mode();
        this->cmd->mode(EMC_TASK_MODE_MDI);
        this->cmd->wait_complete();
    }
    this->cmd->mdi(m);
    this->cmd->wait_complete();
    if(premode)
        this->cmd->mode(premode);
    emit this->reload_display();
}

void _GStat::do_jog(int axisnum, int direction, double distance)
{

}

EMC_TASK_MODE_ENUM _GStat::get_jjogmode()
{
    return this->stat->task_mode();
}

int _GStat::jnum_for_axisnum(int axisnum)
{
    return 0;
}

void _GStat::get_jog_info(int axisnum)
{

}

double* _GStat::get_probed_position_with_offsets()
{
    return nullptr;
}
