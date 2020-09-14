#ifndef EMCMODULE_H
#define EMCMODULE_H

/*
改写的文件：linuxcnc/src/emc/usr_intf/axis/extensions/emcmodule.cc
原文件是作c++转python接口模块，是管理GUI的nml缓冲区的接口，原linuxcnc界面
均使用python，现示教器无法安装pyqt5的开发环境，所以示教器界面开发使用c++。
管理GUI的3个nml缓冲区（start，command，error），参照nml通信方式初始化与读写
即可，指定nml配置文件及对应的缓冲区，一般无需作其他更改。
若需要增加消息，可参考linuxcnc官方文档进行增加，一般开发新功能时才需要，如：长电三轴增加的工具坐标系转换功能
*/

#define ULAPI

#include <inttypes.h>
#include "config.h"
#include "rcs.hh"
#include "emc.hh"
#include "emc_nml.hh"
#include "kinematics.h"
#include "config.h"
#include "inifile.hh"
#include "timer.hh"
#include "nml_oi.hh"
#include "rcs_print.hh"
#include <rtapi_string.h>

#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <memory>

using namespace std;

//TODO: 放在合适的地方
typedef vector<string> strings;

#define QT_NO_GEOM_VARIANT

#ifndef T_BOOL
// The C++ standard probably doesn't specify the amount of storage for a 'bool',
// and on some systems it might be more than one byte.  However, on x86 and
// x86-64, sizeof(bool) == 1.  When a counterexample is found, this must be
// replaced with the result of a configure test.
#define T_BOOL T_UBYTE
#endif

#define LOCAL_SPINDLE_FORWARD (1)
#define LOCAL_SPINDLE_REVERSE (-1)
#define LOCAL_SPINDLE_OFF (0)
#define LOCAL_SPINDLE_INCREASE (10)
#define LOCAL_SPINDLE_DECREASE (11)
#define LOCAL_SPINDLE_CONSTANT (12)

#define LOCAL_MIST_ON (1)
#define LOCAL_MIST_OFF (0)

#define LOCAL_FLOOD_ON (1)
#define LOCAL_FLOOD_OFF (0)

#define LOCAL_BRAKE_ENGAGE (1)
#define LOCAL_BRAKE_RELEASE (0)

#define LOCAL_JOG_STOP (0)
#define LOCAL_JOG_CONTINUOUS (1)
#define LOCAL_JOG_INCREMENT (2)

#define LOCAL_AUTO_RUN (0)
#define LOCAL_AUTO_PAUSE (1)
#define LOCAL_AUTO_RESUME (2)
#define LOCAL_AUTO_STEP (3)
#define LOCAL_AUTO_REVERSE (4)
#define LOCAL_AUTO_FORWARD (5)

/* This definition of offsetof avoids the g++ warning
 * 'invalid offsetof from non-POD type'.
 */
#pragma GCC diagnostic ignored "-Winvalid-offsetof"

//配置文件管理类，即.ini文件
class LinuxcncIni
{
public:
    LinuxcncIni(const char* inifile);
    ~LinuxcncIni() { delete i; }

    string Ini_find(const char *tag, const char *section, string def = "");
    strings Ini_findall(const char *tag, const char *section);

    IniFile* i =  nullptr;
};

//状态缓冲区类，只读
class LinuxcncStat
{
public:
    LinuxcncStat();
    ~LinuxcncStat() { delete c; }

    //缓冲区刷新
    bool poll();

public:
    //Stat_members in emcmodule for python
    //stat
    int echo_serial_number() { return this->status.echo_serial_number; }
    int state() {return this->status.status; }

    //task
    EMC_TASK_MODE_ENUM task_mode() {return this->status.task.mode; }
    EMC_TASK_STATE_ENUM task_state() {return this->status.task.state; }
    int exec_state() {return this->status.task.execState; }
    EMC_TASK_INTERP_ENUM interp_state() {return this->status.task.interpState; }
    int call_level() {return this->status.task.callLevel; }
    int read_line() {return this->status.task.readLine; }
    int motion_line() {return this->status.task.motionLine; }
    int current_line() {return this->status.task.currentLine; }
    string file() {return this->status.task.file; }
    string command() {return this->status.task.command; }
    int program_units() {return this->status.task.programUnits; }
    int interpreter_errcode() {return this->status.task.interpreter_errcode; }
    bool optional_stop() {return this->status.task.optional_stop_state; }
    bool block_delete() {return this->status.task.block_delete_state; }
    int task_paused() {return this->status.task.task_paused; }
    bool input_timeout() {return this->status.task.input_timeout; }
    double rotation_xy() {return this->status.task.rotation_xy; }
    double delay_left() {return this->status.task.delayLeft; }
    int queued_mdi_commands() {return this->status.task.queuedMDIcommands; }

    //motion
    double linear_units() {return this->status.motion.traj.linearUnits; }
    double angular_units() {return this->status.motion.traj.angularUnits; }
    double cycle_time() {return this->status.motion.traj.cycleTime; }
    int joints() {return this->status.motion.traj.axes; }
    //int spindles() {return this->status.motion.traj.spindles; }
    int axis_mask() {return this->status.motion.traj.axis_mask; }
    EMC_TRAJ_MODE_ENUM motion_mode() {return this->status.motion.traj.mode; }
    bool enabled() {return this->status.motion.traj.enabled; }
    bool inpos() {return this->status.motion.traj.inpos; }
    int queue() {return this->status.motion.traj.queue; }
    int active_queue() {return this->status.motion.traj.activeQueue; }
    bool queue_full() {return this->status.motion.traj.queueFull; }
    int id() {return this->status.motion.traj.id; }
    bool paused() {return this->status.motion.traj.paused; }
    double feedrate() {return this->status.motion.traj.scale; }
    double rapidrate() {return this->status.motion.traj.rapid_scale; }
    double velocity() {return this->status.motion.traj.velocity; }
    double acceleration() {return this->status.motion.traj.acceleration; }
    double max_velocity() {return this->status.motion.traj.maxVelocity; }
    double max_acceleration() {return this->status.motion.traj.maxAcceleration; }
    bool probe_tripped() {return this->status.motion.traj.probe_tripped; }
    bool probing() {return this->status.motion.traj.probing; }
    int probe_val() {return this->status.motion.traj.probeval; }
    int kinematics_type() {return this->status.motion.traj.kinematics_type; }
    int motion_type() {return this->status.motion.traj.motion_type; }
    double distance_to_go() {return this->status.motion.traj.distance_to_go; }
    double current_vel() {return this->status.motion.traj.current_vel; }
    bool feed_override_enabled() {return this->status.motion.traj.feed_override_enabled; }
    bool adaptive_feed_enabled() {return this->status.motion.traj.adaptive_feed_enabled; }
    bool feed_hold_enabled() {return this->status.motion.traj.feed_hold_enabled; }
    int num_extrajoints() {return this->status.motion.numExtraJoints; }

    //io
    //io.tool
    int pocket_prepped() {return this->status.io.tool.pocketPrepped; }
    int tool_in_spindle() {return this->status.io.tool.toolInSpindle; }
    //io.cooland
    int mist() {return this->status.io.coolant.mist; }
    int flood() {return this->status.io.coolant.flood; }
    //io.aux
    int estop() {return this->status.io.aux.estop; }
    //io.lube
    int lube() {return this->status.io.lube.on; }
    int lube_level() {return this->status.io.lube.level; }

    int debug() {return this->status.debug; }

    //Stat_getsetlist in emcmodule for python
    struct JointData {//copy EMC_JOINT_STAT , don't have Dict data type in C++
        unsigned char jointType;
        double units;
        double backlash;
        double min_position_limit;
        double max_position_limit;
        double max_ferror;
        double min_ferror;

        // dynamic status
        double ferror_current;
        double ferror_highMark;
        double output;
        double input;
        double velocity;
        bool inpos;
        bool homing;
        bool homed;
        bool fault;
        bool enabled;
        bool min_soft_limit;
        bool max_soft_limit;
        bool min_hard_limit;
        bool max_hard_limit;
        bool override_limits;
    };

    struct AxisData {
        double min_position_limit;
        double max_position_limit;
        double velocity;
    };

    struct SpindleData {
        double speed;
        double override;
        double css_maximum;
        double css_factor;
        int direction;
        int brake;
        int increasing;
        int enabled;
        int orient_state;
        int orient_fault;
        bool override_enabled;
        bool homed;
    };

    double* actual_position();
    double* ain();
    double* aout();
    JointData* joint();
    AxisData* axis();
    SpindleData* spindle();
    int* din();
    int* dout();
    int* activegcode();
    bool* homed();
    int* limit();
    int* activemcode();
    double* g5x_offset();
    int g5x_index();
    double* g92_offset();
    double* position();
    double* dtg();
    double* joint_position();
    double* joint_actual_position();
    double* joint_actual_velocity();
    double* probed_position();
    double* settings();
    double* tool_offset();
    CANON_TOOL_TABLE* tool_table();
    int axes();

    bool pose_equal(double* p1, double* p2);

private:
    RCS_STAT_CHANNEL* c = nullptr;
    EMC_STAT status;

    JointData Stat_joint_one(int jointno);
    AxisData Stat_axis_one(int axisno);
    SpindleData Stat_spindle_one(int spindleno);
};

//命令缓冲区，读写
/*
TODO:当缓冲区为REMOTE时，
在不归零时无法移动轴，报轴未选中的错。
在MDI模式下移动轴后，无法在手动Continue模式下移动，报底层运动模式错误，给定距离手动移动后，可进行Continue移动。
在AUTO模式下执行运动后，手动模式下不工作，也是报底层运动模式的错，重复上条错误的过程又能恢复正常。
缓冲区为LOCAL时无上述错误，与硬件无关。
*/
class LinuxcncCommand
{
public:
    LinuxcncCommand();
    ~LinuxcncCommand() { delete c; }

private:
    RCS_CMD_CHANNEL* c = nullptr;
    RCS_STAT_CHANNEL* s = nullptr;
    int serial;

    int emcWaitCommandComplete(double timeout);
    int emcSendCommand(RCS_CMD_MSG& cmd);

public:
    //Command_methods in emcmodule for python
    void debug(int level);
    //FIX, remote gui, After move axis in mdi mode,
    //then move axis for continue in manual mode, maybe motion buffer is error
    void teleop_enable(bool enable);
    void traj_mode(EMC_TRAJ_MODE_ENUM mode);
    bool state(EMC_TASK_STATE_ENUM state);
    bool mdi(string cmd);
    bool mode(EMC_TASK_MODE_ENUM mode);
    void feedrate(double scale);
    void rapidrate(double scale);
    void maxvel(double vel);
    void spindleoverride(double scale, int spindle);
    bool spindle(int dir, double arg1, int arg2);
    void tool_offset(int toolno, double tranz, double tranx, double diameter,
                     double frontangle, double backangle, int orientation);
    bool mist(int dir);
    bool flood(int dir);
    bool brake(int dir, int spindle);
    void load_tool_table();
    void abort();
    void task_plan_synch();
    void override_limits();
    void home(int joint);
    void unhome(int joint);
    bool jog(int fn, int jjogmode, int ja_value, double vel=0, double inc=0);
    void reset_interpreter();
    bool program_open(const char* file, int len);
    bool emcauto(int fn, int line = -1);
    void set_optional_stop(bool state);
    void set_block_delete(int state);
    void set_min_limit(int joint, double limit);
    void set_max_limit(int joint, double limit);
    void set_feed_override(bool mode);
    void set_spindle_override(bool mode, int spindle);
    void set_feed_hold(bool mode);
    void set_adaptive_feed(bool status);
    void set_digital_output(bool index, bool start);
    void set_analog_output(bool index, double start);

    int wait_complete(int timeout = 0);
};

//底层错误类，只读，只报底层的错，界面的错需要另外处理
//TODO:缓冲区为REMOTE时，无错误信息，有错误类型，缓冲区为LOCAL无上述错误，与硬件无关。
class LinuxcncError
{
public:
    LinuxcncError();
    ~LinuxcncError() { delete c;}

    string Error_poll();

private:
    NML* c = nullptr;
};

#endif // EMCMODULE_H
