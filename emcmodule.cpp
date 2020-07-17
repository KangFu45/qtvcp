#include "emcmodule.h"

#include <iostream>

//------------------LinuxcncIni-----------------------

LinuxcncIni::LinuxcncIni(const char* inifile)
{
    if(this->i == NULL)
        this->i = new IniFile();

    if(!this->i->Open(inifile)) { std::cerr << "error: IniFile.Open(" <<inifile <<") failed!\n"; abort();}
}

const char* LinuxcncIni::Ini_find(const char *tag, const char *section)
{
    int num = 1;
    const char* out = this->i->Find(section,tag,num);
    if(out == NULL) return NULL;
    return out;
}

std::vector<const char*> LinuxcncIni::Ini_findall(const char *tag, const char *section)
{
    const char* out;
    int num = 1;

    std::vector<const char*> result;
    while (1) {
        out = this->i->Find(section,tag,num);
        if(out == NULL) break;
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
    if(!this->c) {std::cerr << "error: linuxcnc stat init failed!\n"; return;}

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
                std::cerr<< "erroe: Mode should be MODE_MDI, MODE_MANUAL, or MODE_AUTO";
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
    if(type == 0) return "";

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
