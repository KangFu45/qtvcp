#include "hal_glib.h"

#include <QTimer>

_GStat::_GStat (LinuxcncStat* stat)
{
    if(stat == nullptr)
        this->stat = new LinuxcncStat();
    else
        this->stat = stat;

    this->cmd = new LinuxcncCommand();
    this->old.tool_prep_number = 0;

    this->stat->poll();
    this->merge();

    QTimer* timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer,&QTimer::timeout,this,&_GStat::update);
}

void _GStat::merge()
{
    this->old.state = this->stat->task_state();
    this->old.mode = this->stat->task_mode();
    this->old.interp = this->stat->interp_state();
    if(this->stat->call_level() == 0)
        this->old.file = this->stat->file();//manual delete
    this->old.paused = this->stat->paused();
    this->old.line = this->stat->motion_line();
    this->old.homed = this->stat->homed();
    this->old.tool_in_spindle = this->stat->tool_in_spindle();
    this->old.tool_prep_number = -1;//Don't have Pin ,so default;
    this->old.motion_mode = this->stat->motion_mode();
    LinuxcncStat::SpindleData* spindledata = this->stat->spindle();//manual delete
    this->old.override = spindledata[0].override;
    this->old.feed_or = this->stat->feedrate();
    this->old.rapid_or = this->stat->rapidrate();
    this->old.max_velocity_or = this->stat->max_velocity();
    this->old.feed_hold = this->stat->feed_hold_enabled();
    this->old.g5x_index = this->stat->g5x_index();
    this->old.spindle_enabled = spindledata[0].enabled;
    this->old.spindle_direction = spindledata[0].direction;
    this->old.block_delete = this->stat->block_delete();
    this->old.optional_stop = this->stat->optional_stop();
    this->old.actual_spindle_speed = 0;//Don't have Pin
    this->old.flood = this->stat->flood();
    this->old.mist = this->stat->mist();
    this->old.current_z_rotation = this->stat->rotation_xy();
    this->old.current_tool_offset = this->stat->tool_offset();//manual delete
    //overrirde limits / hard limits
    LinuxcncStat::JointData* jointdata = this->stat->joint();
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
    this->old.override_limits = or_limit_list;
    this->old.override_limits_set = or_limit_set;
    this->old.override_limits_tripped = hard_limit;
    this->old.hard_limit_list = hard_limit_list;
    //active G code
    int* gcodedata = this->stat->activegcode();//manual delete
    strings active_gcodes;
    std::string codes = "";
    for(int i=0; i<ACTIVE_G_CODES; ++i){
        if(i%10 == 0)
            active_gcodes.emplace_back(std::string("G"+std::to_string(gcodedata[i]/10)));
        else
            active_gcodes.emplace_back(std::string("G"+std::to_string(gcodedata[i]/10)+"."+std::to_string(gcodedata[i]%10)));
        codes += active_gcodes.back() + " ";
    }
    this->old.g_code = codes;
    //extract specific G code modes
    bool itime , fpm , fpr , css , rpm , metric = false;
    bool radius , diameter = false;
    foreach (const std::string& i, active_gcodes) {
        if(i == "G93") itime = true;
        else if(i == "G94") fpm = true;
        else if(i == "G95") fpr = true;
        else if(i == "G96") css = true;
        else if(i == "G97") rpm = true;
        else if(i == "G21") metric = true;
        else if(i == "G7") diameter = true;
        else if(i == "G8") radius = true;
    }
    this->old.itime = itime;
    this->old.fpm = fpm;
    this->old.fpr = fpr;
    this->old.css = css;
    this->old.rpm = rpm;
    this->old.metric = metric;
    this->old.radius = radius;
    this->old.diameter = diameter;
    this->old.spindle_speed = spindledata->speed;

    //active M codes
    int* mcodedata = this->stat->activemcode();//manual delete
    strings active_mcodes;
    std::string mcodes = "";
    for(int i=0; i<ACTIVE_M_CODES; ++i){
        active_mcodes.emplace_back(std::string("M"+std::to_string(mcodedata[i])));
        mcodes += active_mcodes.back() + " ";
    }
    this->old.m_code = mcodes;
    CANON_TOOL_TABLE* table = this->stat->tool_table();//manual delete
    this->old.tool_info = table[0];

    delete [] spindledata;
    delete [] gcodedata;
    delete [] mcodedata;
    delete [] table;
}

void _GStat::update()
{
    if(!this->stat->poll()){
        this->status_active = false;
        emit this->periodic();
        return;
    }
    this->status_active = true;

}

void _GStat::forced_update()
{

}
