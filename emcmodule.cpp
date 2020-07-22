#include "emcmodule.h"

#include <iostream>

//------------------LinuxcncIni-----------------------

LinuxcncIni::LinuxcncIni(const char* inifile)
{
    if(this->i == nullptr)
        this->i = new IniFile();

    if(!this->i->Open(inifile)) { std::cerr << "error: IniFile.Open(" <<inifile <<") failed!\n"; abort();}
}

const char* LinuxcncIni::Ini_find(const char *tag, const char *section)
{
    int num = 1;
    const char* out = this->i->Find(section,tag,num);
    if(out == nullptr) return nullptr;
    return out;
}

std::vector<const char*> LinuxcncIni::Ini_findall(const char *tag, const char *section)
{
    const char* out;
    int num = 1;

    std::vector<const char*> result;
    while (1) {
        out = this->i->Find(section,tag,num);
        if(out == nullptr) break;
        result.emplace_back(out);
        ++num;
    }
    return result;
}

//--------------------------------------------------------

static char* get_nmlfile() { return EMC2_DEFAULT_NMLFILE; }

//--------------------
static bool check_stat(RCS_STAT_CHANNEL* emcStatusBuffer){
    if(!emcStatusBuffer->valid()){
        std::cerr << "error: emcStatusBuffer invalid err=" << emcStatusBuffer->error_type;
        return false;
    }
    return true;
}

LinuxcncStat::LinuxcncStat()
{
    char* file = get_nmlfile();

    RCS_STAT_CHANNEL* c = new RCS_STAT_CHANNEL(emcFormat, "emcStatus", "xemc", file);
    if(!c) {std::cerr << "error: linuxcnc stat init failed!\n"; return;}

    this->c = c;
}

bool LinuxcncStat::poll()
{
    if(!check_stat(this->c)) return false;
    if(this->c->peek() == EMC_STAT_TYPE){
        EMC_STAT* emcStatus = static_cast<EMC_STAT*>(this->c->get_address());
        memcpy((char*)&this->status,emcStatus,sizeof(EMC_STAT));
    }
    return true;
}

static int* int_array(int* arr, int sz){
    int* res = new int[sz];
    for(int i = 0; i < sz; ++i)
        res[i] = arr[i];
    return res;
}

static double* double_array(double* arr, int sz){
    double* res = new double[sz];
    for(int i = 0; i < sz; ++i)
        res[i] = arr[i];
    return res;
}

static double* pose(const EmcPose& p){
    double* res = new double[9];
    res[0] = p.tran.x;
    res[1] = p.tran.y;
    res[2] = p.tran.z;
    res[3] = p.a;
    res[4] = p.b;
    res[5] = p.c;
    res[6] = p.u;
    res[7] = p.v;
    res[8] = p.w;
    return res;
}
double* LinuxcncStat::actual_position(){
    return pose(this->status.motion.traj.actualPosition);
}

double* LinuxcncStat::ain(){
    return double_array(this->status.motion.analog_input, EMCMOT_MAX_AIO);
}

double* LinuxcncStat::aout(){
    return double_array(this->status.motion.analog_output, EMCMOT_MAX_AIO);
}

LinuxcncStat::JointData LinuxcncStat::Stat_joint_one(int jointno){
    LinuxcncStat::JointData data;
    data.jointType = this->status.motion.joint[jointno].jointType;
    data.units = this->status.motion.joint[jointno].units;
    data.backlash = this->status.motion.joint[jointno].backlash;
    data.min_position_limit = this->status.motion.joint[jointno].minPositionLimit;
    data.max_position_limit = this->status.motion.joint[jointno].maxPositionLimit;
    data.max_ferror = this->status.motion.joint[jointno].maxFerror;
    data.min_ferror = this->status.motion.joint[jointno].minFerror;
    data.ferror_current = this->status.motion.joint[jointno].ferrorCurrent;
    data.ferror_highMark = this->status.motion.joint[jointno].ferrorHighMark;
    data.output = this->status.motion.joint[jointno].output;
    data.input = this->status.motion.joint[jointno].input;
    data.velocity = this->status.motion.joint[jointno].velocity;
    data.inpos = this->status.motion.joint[jointno].input;
    data.homing = this->status.motion.joint[jointno].homing;
    data.homed = this->status.motion.joint[jointno].homed;
    data.fault = this->status.motion.joint[jointno].fault;
    data.enabled = this->status.motion.joint[jointno].enabled;
    data.min_soft_limit = this->status.motion.joint[jointno].minSoftLimit;
    data.max_soft_limit = this->status.motion.joint[jointno].maxSoftLimit;
    data.min_hard_limit = this->status.motion.joint[jointno].minHardLimit;
    data.max_hard_limit = this->status.motion.joint[jointno].maxHardLimit;
    data.override_limits = this->status.motion.joint[jointno].overrideLimits;
    return data;
}

LinuxcncStat::JointData* LinuxcncStat::joint()
{
    LinuxcncStat::JointData* data = new LinuxcncStat::JointData[EMCMOT_MAX_JOINTS];
    for(int i=0; i<EMCMOT_MAX_JOINTS; ++i)
        data[i] = this->Stat_joint_one(i);
    return data;
}

LinuxcncStat::AxisData LinuxcncStat::Stat_axis_one(int axisno)
{
    LinuxcncStat::AxisData data;
    data.velocity = this->status.motion.axis[axisno].velocity;
    data.min_position_limit = this->status.motion.axis[axisno].minPositionLimit;
    data.max_position_limit = this->status.motion.axis[axisno].maxPositionLimit;
    return data;
}

LinuxcncStat::AxisData* LinuxcncStat::axis()
{
    LinuxcncStat::AxisData* data = new LinuxcncStat::AxisData[EMCMOT_MAX_AXIS];
    for(int i=0; i<EMCMOT_MAX_AXIS; ++i)
        data[i] = this->Stat_axis_one(i);
    return data;
}

LinuxcncStat::SpindleData LinuxcncStat::Stat_spindle_one(int spindleno)
{
    LinuxcncStat::SpindleData data;
    data.brake = this->status.motion.spindle[spindleno].brake;
    data.direction = this->status.motion.spindle[spindleno].direction;
    data.enabled = this->status.motion.spindle[spindleno].enabled;
    data.override_enabled = this->status.motion.spindle[spindleno].spindle_override_enabled;
    data.speed = this->status.motion.spindle[spindleno].speed;
    data.override = this->status.motion.spindle[spindleno].spindle_scale;
    data.homed = this->status.motion.spindle[spindleno].homed;
    data.orient_state = this->status.motion.spindle[spindleno].orient_state;
    data.orient_fault = this->status.motion.spindle[spindleno].orient_fault;
    return data;
}

LinuxcncStat::SpindleData* LinuxcncStat::spindle()
{
    LinuxcncStat::SpindleData* data = new LinuxcncStat::SpindleData[EMCMOT_MAX_SPINDLES];
    for(int i=0; i<EMCMOT_MAX_SPINDLES; ++i)
        data[i] = this->Stat_spindle_one(i);
    return data;
}

int* LinuxcncStat::din(){
    return int_array(this->status.motion.synch_di, EMCMOT_MAX_AIO);
}

int* LinuxcncStat::dout(){
    return int_array(this->status.motion.synch_do, EMCMOT_MAX_AIO);
}

int* LinuxcncStat::activegcode(){
    return int_array(this->status.task.activeGCodes, ACTIVE_G_CODES);
}

bool* LinuxcncStat::homed()
{
    bool* res = new bool[EMCMOT_MAX_JOINTS];
    for(int i=0; i<EMCMOT_MAX_JOINTS; ++i)
        res[i] = this->status.motion.joint[i].homed;
    return res;
}

int* LinuxcncStat::limit()
{
    int* res = new int[EMCMOT_MAX_JOINTS];
    for(int i=0; i<EMCMOT_MAX_JOINTS; ++i){
        int v = 0;
        if(this->status.motion.joint[i].minHardLimit) v |= 1;
        if(this->status.motion.joint[i].maxHardLimit) v |= 2;
        if(this->status.motion.joint[i].minSoftLimit) v |= 4;
        if(this->status.motion.joint[i].maxSoftLimit) v |= 8;
        res[i] = v;
    }
    return res;
}

int* LinuxcncStat::activemcode(){
    return int_array(this->status.task.activeMCodes, ACTIVE_M_CODES);
}

double* LinuxcncStat::g5x_offset(){
    return pose(this->status.task.g5x_offset);
}

int LinuxcncStat::g5x_index(){
    return this->status.task.g5x_index;
}

double* LinuxcncStat::g92_offset(){
    return pose(this->status.task.g92_offset);
}

double* LinuxcncStat::position(){
    return pose(this->status.motion.traj.position);
}

double* LinuxcncStat::dtg(){
    return pose(this->status.motion.traj.dtg);
}

double* LinuxcncStat::joint_position()
{
    double* res = new double[EMCMOT_MAX_JOINTS];
    for(int i=0; i<EMCMOT_MAX_JOINTS; ++i)
        res[i] = this->status.motion.joint[i].output;
    return res;
}

double* LinuxcncStat::joint_actual_position()
{
    double* res = new double[EMCMOT_MAX_JOINTS];
    for(int i=0; i<EMCMOT_MAX_JOINTS; ++i)
        res[i] = this->status.motion.joint[i].input;
    return res;
}

double* LinuxcncStat::probed_position(){
    return pose(this->status.motion.traj.probedPosition);
}

double* LinuxcncStat::settings(){
    return double_array(this->status.task.activeSettings,ACTIVE_SETTINGS);
}

double* LinuxcncStat::tool_offset(){
    return pose(this->status.task.toolOffset);
}

CANON_TOOL_TABLE* LinuxcncStat::tool_table()
{
    CANON_TOOL_TABLE* res = new CANON_TOOL_TABLE[CANON_POCKETS_MAX];
    for(int i=0; i<CANON_POCKETS_MAX; ++i)
        res[i] = this->status.io.tool.toolTable[i];
    return res;
}

int LinuxcncStat::axes(){
    return this->status.motion.traj.deprecated_axes;
}

//-------------------------------------------


//-------------------LinuxcncCommand---------------------
#define EMC_COMMAND_TIMEOUT 5.0  // how long to wait until timeout
#define EMC_COMMAND_DELAY   0.01 // how long to sleep between checks

LinuxcncCommand::LinuxcncCommand()
{
    char* file = get_nmlfile();

    RCS_CMD_CHANNEL* c = new RCS_CMD_CHANNEL(emcFormat, "emcCommand", "xemc", file);
    if(!c){ std::cerr<<"error: new RCS_CMD_CHANNEL failed!"; return;}

    RCS_STAT_CHANNEL* s = new RCS_STAT_CHANNEL(emcFormat, "emcStatus", "xemc", file);
    if(!s){std::cerr<<"error: new RCS_STAT_CHANNEL failed!"; return;}

    this->c = c;
    this->s = s;
}

int LinuxcncCommand::emcWaitCommandComplete(double timeout)
{
    double start = etime();

        do {
            double now = etime();
            if(this->s->peek() == EMC_STAT_TYPE) {
               EMC_STAT *stat = (EMC_STAT*)this->s->get_address();
               int serial_diff = stat->echo_serial_number - this->serial;
               if (serial_diff > 0) {
                    return RCS_DONE;
               }
               if (serial_diff == 0 &&
                   ( stat->status == RCS_DONE || stat->status == RCS_ERROR )) {
                    return stat->status;
               }
            }
            esleep(fmin(timeout - (now - start), EMC_COMMAND_DELAY));
        } while (etime() - start < timeout);
        return -1;
}

int LinuxcncCommand::emcSendCommand(RCS_CMD_MSG& cmd)
{
    if (this->c->write(&cmd)) {
            return -1;
        }
        this->serial = cmd.serial_number;

        double start = etime();
        while (etime() - start < EMC_COMMAND_TIMEOUT) {
            EMC_STAT *stat = (EMC_STAT*)this->s->get_address();
            int serial_diff = stat->echo_serial_number - this->serial;
            if(this->s->peek() == EMC_STAT_TYPE &&
               serial_diff >= 0) {
                    return 0;
               }
            esleep(EMC_COMMAND_DELAY);
        }
        return -1;
}

bool LinuxcncCommand::mdi(char* cmd, int len)
{
    EMC_TASK_PLAN_EXECUTE m;
    if(unsigned(len) > sizeof(m.command) - 1) {std::cerr<<"error: MDI commands limited to" <<sizeof(m.command) - 1<<" characters!"; return false;}

    rtapi_strxcpy(m.command, cmd);
    emcSendCommand(m);
    return true;
}

bool LinuxcncCommand::mode(EMC_TASK_MODE_ENUM mode)
{
    EMC_TASK_SET_MODE m;
    m.mode = mode;
    switch(m.mode) {
            case EMC_TASK_MODE_MDI:
            case EMC_TASK_MODE_MANUAL:
            case EMC_TASK_MODE_AUTO:
                break;
            default:
                std::cerr<< "error: Mode should be MODE_MDI, MODE_MANUAL, or MODE_AUTO";
                return false;
    }
    emcSendCommand(m);
    return true;
}

void LinuxcncCommand::home(int joint)
{
    EMC_JOINT_HOME m;
    m.joint = joint;
    emcSendCommand(m);
}

void LinuxcncCommand::unhome(int joint)
{
    EMC_JOINT_UNHOME m;
    m.joint = joint;
    emcSendCommand(m);
}

void LinuxcncCommand::debug(int level)
{
    EMC_SET_DEBUG d;
    d.debug = level;
    emcSendCommand(d);
}

void LinuxcncCommand::teleop_enable(int enable)
{
    EMC_TRAJ_SET_TELEOP_ENABLE en;
    en.enable = enable;
    emcSendCommand(en);
}

void LinuxcncCommand::traj_mode(EMC_TRAJ_MODE_ENUM mode)
{
    EMC_TRAJ_SET_MODE mo;
    mo.mode = mode;
    emcSendCommand(mo);
}

bool LinuxcncCommand::state(EMC_TASK_STATE_ENUM state)
{
    EMC_TASK_SET_STATE m;
    m.state = state;
    switch(m.state){
        case EMC_TASK_STATE_ESTOP:
        case EMC_TASK_STATE_ESTOP_RESET:
        case EMC_TASK_STATE_ON:
        case EMC_TASK_STATE_OFF:
            break;
        default:
            std::cerr<< "error: Machine state should be STATE_ESTOP, STATE_ESTOP_RESET, STATE_ON, or STATE_OFF ";
            return false;
    }
    emcSendCommand(m);
    return true;
}

void LinuxcncCommand::feedrate(double scale)
{
    EMC_TRAJ_SET_SCALE m;
    m.scale = scale;
    emcSendCommand(m);
}

void LinuxcncCommand::rapidrate(double scale)
{
    EMC_TRAJ_SET_RAPID_SCALE m;
    m.scale = scale;
    emcSendCommand(m);
}

void LinuxcncCommand::maxvel(double vel)
{
    EMC_TRAJ_SET_MAX_VELOCITY m;
    m.velocity = vel;
    emcSendCommand(m);
}

void LinuxcncCommand::spindleoverride(double scale, int spindle)
{
    EMC_TRAJ_SET_SPINDLE_SCALE m;
    m.scale = scale;
    m.spindle = spindle;
    emcSendCommand(m);
}

bool LinuxcncCommand::spindle(int dir, double arg1, double arg2)
{
    switch (dir) {
    case LOCAL_SPINDLE_FORWARD:
    case LOCAL_SPINDLE_REVERSE:
    {
        EMC_SPINDLE_ON m;
        m.speed = dir * arg1;
        m.spindle = (int)arg2;
        emcSendCommand(m);
    }
        break;
    case LOCAL_SPINDLE_INCREASE:
    {
        EMC_SPINDLE_INCREASE m;
        m.spindle = (int)arg1;
        emcSendCommand(m);
    }
        break;
    case LOCAL_SPINDLE_DECREASE:
    {
        EMC_SPINDLE_DECREASE m;
        m.spindle = (int)arg1;
        emcSendCommand(m);
    }
        break;
    case LOCAL_SPINDLE_CONSTANT:
    {
        EMC_SPINDLE_CONSTANT m;
        m.spindle = (int)arg1;
        emcSendCommand(m);
    }
        break;
    case LOCAL_SPINDLE_OFF:
    {
        EMC_SPINDLE_OFF m;
        m.spindle = (int)arg1;
        emcSendCommand(m);
    }
        break;
    default:
        std::cerr<<"error: Spindle direction should be SPINDLE_FORWARD, SPINDLE_REVERSE, SPINDLE_OFF, SPINDLE_INCREASE, SPINDLE_DECREASE, or SPINDLE_CONSTANT";
        return false;
    }
    return true;
}

void LinuxcncCommand::tool_offset(int toolno, double tranz, double tranx, double diameter,
                 double frontangle, double backangle, int orientation)
{
    EMC_TOOL_SET_OFFSET m;
    m.toolno = toolno;
    m.offset.tran.z = tranz;
    m.offset.tran.x = tranx;
    m.diameter = diameter;
    m.frontangle = frontangle;
    m.backangle = backangle;
    m.orientation = orientation;
    emcSendCommand(m);
}

bool LinuxcncCommand::mist(int dir)
{
    switch (dir) {
    case LOCAL_MIST_ON:
    {
        EMC_COOLANT_MIST_ON m;
        emcSendCommand(m);
    }
        break;
    case LOCAL_MIST_OFF:
    {
        EMC_COOLANT_MIST_OFF m;
        emcSendCommand(m);
    }
        break;
    default:
        std::cerr<<"error: Mist should be MIST_ON or MIST_OFF";
        return false;
    }
    return true;
}

bool LinuxcncCommand::flood(int dir)
{
    switch (dir) {
    case LOCAL_FLOOD_ON:
    {
        EMC_COOLANT_FLOOD_ON m;
        emcSendCommand(m);
    }
        break;
    case LOCAL_FLOOD_OFF:
    {
        EMC_COOLANT_FLOOD_OFF m;
        emcSendCommand(m);
    }
        break;
    default:
        std::cerr<<"error: FLOOD should be FLOOD_ON or FLOOD_OFF";
        return false;
    }
    return true;
}

bool LinuxcncCommand::brake(int dir, int spindle)
{
    switch (dir) {
    case LOCAL_BRAKE_ENGAGE:
    {
        EMC_SPINDLE_BRAKE_ENGAGE m;
        m.spindle = spindle;
        emcSendCommand(m);
    }
        break;
    case LOCAL_BRAKE_RELEASE:
    {
        EMC_SPINDLE_BRAKE_RELEASE m;
        m.spindle = spindle;
        emcSendCommand(m);
    }
        break;
    default:
        std::cerr<<"error: BRAKE should be BRAKE_ENGAGE or BRAKE_RELEASE";
        return false;
    }
    return true;
}

void LinuxcncCommand::load_tool_table()
{
    EMC_TOOL_LOAD_TOOL_TABLE m;
    m.file[0] = '\0';
    emcSendCommand(m);
}

void LinuxcncCommand::abort()
{
    EMC_TASK_ABORT m;
    emcSendCommand(m);
}

void LinuxcncCommand::task_plan_synch()
{
    EMC_TASK_PLAN_SYNCH synch;
    emcSendCommand(synch);
}

void LinuxcncCommand::override_limits()
{
    EMC_JOINT_OVERRIDE_LIMITS m;
    m.joint = 0;
    emcSendCommand(m);
}

bool LinuxcncCommand::jog(int fn, int ja_value, int jjogmode, double vel, double inc)
{
    if(fn == LOCAL_JOG_STOP){
        EMC_JOG_STOP abort;
        abort.joint_or_axis = ja_value;
        abort.jjogmode = jjogmode;
        emcSendCommand(abort);
    }else if(fn == LOCAL_JOG_CONTINUOUS){
        EMC_JOG_CONT cont;
        cont.joint_or_axis = ja_value;
        cont.vel = vel;
        cont.jjogmode = jjogmode;
        emcSendCommand(cont);
    }else if(fn ==LOCAL_JOG_INCREMENT){
        EMC_JOG_INCR incr;
        incr.joint_or_axis = ja_value;
        incr.vel = vel;
        incr.incr = inc;
        incr.jjogmode = jjogmode;
        emcSendCommand(incr);
    }else{
        std::cerr<<"error: jog() first argument must be JOG_xxx";
        return false;
    }
    return true;
}

void LinuxcncCommand::reset_interpreter()
{
    EMC_TASK_PLAN_INIT m;
    emcSendCommand(m);
}

bool LinuxcncCommand::program_open(char* file, int len)
{
    EMC_TASK_PLAN_CLOSE m0;
    emcSendCommand(m0);

    EMC_TASK_PLAN_OPEN m;
    if(unsigned(len) > sizeof(m.file) - 1){
        std::cerr << "error: File name limited to "<< sizeof(m.file) - 1 <<" characters";
        return false;
    }
    rtapi_strxcpy(m.file, file);
    emcSendCommand(m);
    return true;
}

bool LinuxcncCommand::emcauto(int fn, int line)
{
    switch (fn) {
    case LOCAL_AUTO_RUN:
    {
        EMC_TASK_PLAN_RUN run;
        run.line = line;
        emcSendCommand(run);
    }
        break;
    case LOCAL_AUTO_PAUSE:
    {
        EMC_TASK_PLAN_PAUSE pause;
        emcSendCommand(pause);
    }
        break;
    case LOCAL_AUTO_RESUME:
    {
        EMC_TASK_PLAN_REVERSE resume;
        emcSendCommand(resume);
    }
        break;
    case LOCAL_AUTO_STEP:
    {
        EMC_TASK_PLAN_STEP step;
        emcSendCommand(step);
    }
        break;
    case LOCAL_AUTO_REVERSE:
    {
        EMC_TASK_PLAN_REVERSE reverse;
        emcSendCommand(reverse);
    }
        break;
    case LOCAL_AUTO_FORWARD:
    {
        EMC_TASK_PLAN_FORWARD forward;
        emcSendCommand(forward);
    }
        break;
    default:
        std::cerr <<"error: Unexpected argument "<<fn<<" to command.auto";
        return false;
    }
    return true;
}

void LinuxcncCommand::set_optional_stop(int state)
{
    EMC_TASK_PLAN_SET_OPTIONAL_STOP m;
    m.state = state;
    emcSendCommand(m);
}

void LinuxcncCommand::set_block_delete(int state)
{
    EMC_TASK_PLAN_SET_BLOCK_DELETE m;
    m.state = state;
    emcSendCommand(m);
}

void LinuxcncCommand::set_min_limit(int joint, double limit)
{
    EMC_JOINT_SET_MIN_POSITION_LIMIT m;
    m.joint = joint;
    m.limit = limit;
    emcSendCommand(m);
}

void LinuxcncCommand::set_max_limit(int joint, double limit)
{
    EMC_JOINT_SET_MAX_POSITION_LIMIT m;
    m.joint = joint;
    m.limit = limit;
    emcSendCommand(m);
}

void LinuxcncCommand::set_feed_override(bool mode)
{
    EMC_TRAJ_SET_FO_ENABLE m;
    m.mode = mode;
    emcSendCommand(m);
}

void LinuxcncCommand::set_spindle_override(bool mode, int spindle)
{
    EMC_TRAJ_SET_SO_ENABLE m;
    m.mode = mode;
    m.spindle = spindle;
    emcSendCommand(m);
}

void LinuxcncCommand::set_feed_hold(bool mode)
{
    EMC_TRAJ_SET_FH_ENABLE m;
    m.mode = mode;
    emcSendCommand(m);
}

void LinuxcncCommand::set_adaptive_feed(bool status)
{
    EMC_MOTION_ADAPTIVE m;
    m.status = status;
    emcSendCommand(m);
}

void LinuxcncCommand::set_digital_output(bool index, bool start)
{
    EMC_MOTION_SET_DOUT m;
    m.index = index;
    m.start = start;
    m.now = 1;
    emcSendCommand(m);
}

void LinuxcncCommand::set_analog_output(bool index, double start)
{
    EMC_MOTION_SET_AOUT m;
    m.index = index;
    m.start = start;
    m.now = 1;
    emcSendCommand(m);
}

int LinuxcncCommand::wait_complete()
{
    double timeout = EMC_COMMAND_TIMEOUT;
    //TODO:
    return emcWaitCommandComplete(timeout);
}

//-------------------------------------------------

//------------------LinuxcncError------------------

LinuxcncError::LinuxcncError()
{
    char *file = get_nmlfile();

    NML* c = new NML(emcFormat, "emcError", "xemc", file);
    if(!c){std::cerr<<"error: new NML failed!"; return;}

    this->c = c;
}

char* LinuxcncError::Error_poll()
{
    if(this->c->valid()){std::cerr<<"error: Error buffer invalid!"; return NULL;}
    NMLTYPE type = this->c->read();
    if(type == 0) return nullptr;

    //don't return error type now
    char* r;
#define PASTE(a,b) a ## b
#define _TYPECASE(tag, type, f) \
    case tag: { \
        char error_string[LINELEN]; \
        strncpy(error_string, ((type*)this->c->get_address())->f, LINELEN-1); \
        error_string[LINELEN-1] = 0; \
        r = error_string;\
        break; \
    }
#define TYPECASE(x, f) _TYPECASE(PASTE(x, _TYPE), x, f)
    switch (type) {
        TYPECASE(EMC_OPERATOR_ERROR, error)
        TYPECASE(EMC_OPERATOR_TEXT, text)
        TYPECASE(EMC_OPERATOR_DISPLAY, display)
        TYPECASE(NML_ERROR, error)
        TYPECASE(NML_TEXT, text)
        TYPECASE(NML_DISPLAY, display)
    default:
        {
            char error_string[256];
            snprintf(error_string, sizeof(error_string), "unrecognized error %" PRId32, type);
            r = error_string;
            break;
        }
    }
    return r;
}
