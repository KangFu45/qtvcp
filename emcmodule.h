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

private:
    RCS_STAT_CHANNEL* c = NULL;
    EMC_STAT status;

public:
    //Stat_members in emcmodule for python
    //stat
    int echo_serial_number() { return this->status.echo_serial_number; }
    int state() {return this->status.status; }
    //task
    int task_mode() {return this->status.task.mode; }

    //Stat_getsetlist in emcmodule for python
    double* actual_position();
    double* ain();
    double* aout();
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
