#ifndef EMCMODULE_H
#define EMCMODULE_H

#endif // EMCMODULE_H

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

class LinuxcncIni
{
public:
    LinuxcncIni(const char* inifile);
    ~LinuxcncIni() { delete i; }

    const char* Ini_find(const char *tag, const char *section);
    std::vector<const char*> Ini_findall(const char *tag, const char *section);

    IniFile* i =  NULL;
};

class LinuxcncStat
{
public:
    LinuxcncStat();
    ~LinuxcncStat() { delete c; }

    bool poll();

public:
    //Stat_members in emcmodule for python
    //stat
    int echo_serial_number() { return this->status.echo_serial_number; }
    int state() {return this->status.status; }

    //task
    int task_mode() {return this->status.task.mode; }
    int task_state() {return this->status.task.state; }
    int exec_state() {return this->status.task.execState; }
    int interp_state() {return this->status.task.interpState; }
    int call_level() {return this->status.task.callLevel; }
    int read_line() {return this->status.task.readLine; }
    int motion_line() {return this->status.task.motionLine; }
    int current_line() {return this->status.task.currentLine; }
    char* file() {return this->status.task.file; }
    char* command() {return this->status.task.command; }
    int program_units() {return this->status.task.programUnits; }
    int interpreter_errcode() {return this->status.task.interpreter_errcode; }
    bool optional_stop() {return this->status.task.optional_stop_state; }
    bool block_delete() {return this->status.task.block_delete_state; }
    int task_paused() {return this->status.task.task_paused; }
    bool input_timeout() {return this->status.task.input_timeout; }
    double rotation_xy() {return this->status.task.rotation_xy; }
    double x_rotation_normal() {return this->status.task.x_rotation_normal; }
    double y_rotation_normal() {return this->status.task.y_rotation_normal; }
    double z_rotation_normal() {return this->status.task.z_rotation_normal; }
    double delay_left() {return this->status.task.delayLeft; }
    int queued_mdi_commands() {return this->status.task.queuedMDIcommands; }

    //motion
    double linear_units() {return this->status.motion.traj.linearUnits; }
    double angular_units() {return this->status.motion.traj.angularUnits; }
    double cycle_time() {return this->status.motion.traj.cycleTime; }
    int joints() {return this->status.motion.traj.joints; }
    int spindles() {return this->status.motion.traj.spindles; }
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
        unsigned char inpos;
        unsigned char homing;
        unsigned char homed;
        unsigned char fault;
        unsigned char enabled;
        unsigned char min_soft_limit;
        unsigned char max_soft_limit;
        unsigned char min_hard_limit;
        unsigned char max_hard_limit;
        unsigned char override_limits;
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
    double* probed_position();
    double* settings();
    double* tool_offset();
    CANON_TOOL_TABLE* tool_table();
    int axes();

private:
    RCS_STAT_CHANNEL* c = NULL;
    EMC_STAT status;

    JointData Stat_joint_one(int jointno);
    AxisData Stat_axis_one(int axisno);
    SpindleData Stat_spindle_one(int spindleno);
};

class LinuxcncCommand
{
public:
    LinuxcncCommand();
    ~LinuxcncCommand() { delete c; }

private:
    RCS_CMD_CHANNEL* c = NULL;
    RCS_STAT_CHANNEL* s = NULL;
    int serial;

    int emcWaitCommandComplete(double timeout);
    int emcSendCommand(RCS_CMD_MSG& cmd);

public:
    //Command_methods in emcmodule for python
    bool mdi(char* cmd, int len);
    bool mode(EMC_TASK_MODE_ENUM mode);
    void home(int joint);
    void unhome(int joint);
};

class LinuxcncError
{
public:
    LinuxcncError();
    ~LinuxcncError() { delete c;}

    char* Error_poll();

private:
    NML* c = NULL;
};
