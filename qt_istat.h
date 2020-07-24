#ifndef QT_ISTAT_H
#define QT_ISTAT_H

#include "emcmodule.h"

class _IStat
{
public:
    _IStat(const char* inipath);

    void update();

    LinuxcncIni* inifile;

    bool LINUXCNC_IS_RUNNING = true;
    string MDI_HISTORY_PATH = "~/.axis_mdi_history";
    string QTVCP_LOG_HISTORY_PATH = "~/qtvcp.log";
    string MACHINE_LOG_HISTORY_PATH = "~/.machine_log_history";
    string PREFERENCE_PATH = "~/.Preferences";
    string SUB_PATH;
    strings SUB_PATH_LIST;
    string IMAGE_PATH;
    string LIB_PATH;

    bool MACHINE_IS_LATHE = false;
    bool MACHINE_IS_METRIC = false;
    double MACHINE_UNIT_CONVERSION = 1.0;
    double MACHINE_UNIT_CONVERSION_9 = 1.0;//?
    vector<char> AVAILABLE_AXES = {'X','Y','Z'};
    vector<unsigned short> AVAILABLE_JOINTS = {0,1,2};
    map<unsigned short, char> Get_NAME_FROM_JOINT = {{0,'X'}, {1,'Y'}, {2,'Z'}};
    map<char, unsigned short> GET_JOINT_FROM_NAME = {{'X',0}, {'Y',1}, {'Z',2}};
    bool NO_HOME_REQUIRED = false;//?
    strings JOG_INCREMENTS;
    strings ANGULAR_INCREMENTS;

    int MAX_LINEAR_VELOCITY = 60;
    float DEFAULT_LINEAR_VELOCITY = 15.0;

    unsigned short AVAILABLE_SPINDLES = 1;
    int DEFAULT_SPINDLE_SPEED = 200;
    int MAX_SPINDLE_SPEED = 2500;
    float MAX_FEED_OVERRIDE = 1.5;
    float MAX_SPINDLE_OVERRIDE = 1.5;
    float MIN_SPINDLE_OVERRIDE = 0.5;

    string INI_MACROS;
    string PARAMETER_FILE;
    int HOME_ALL_FLAG;
    int* JOINT_SEQUENCE_LIST;
    strings JOINT_TYPE;
    int* JOINT_SEQUENCE;
    vector<char> TRAJ_COORDINATES;
    int JOINT_COUNT;
    float DEFAULT_LINEAR_JOG_VEL;
    float MIN_LINEAR_JOG_VEL;
    float MAX_LINEAR_JOG_VEL;
    float DEFAULT_ANGULAR_JOG_VEL;
    float MIN_ANGULAR_JOG_VEL;
    float MAX_ANGULAR_JOG_VEL;
    float MAX_TRAJ_VELOCITY;
    strings MDI_COMMAND_LIST;
    string TOOL_FILE_PATH;
    string POSTGUI_HALFILE_PATH;

private:
    string get_error_safe_setting(const char* heading, const char* detail, string def = "");

};

#endif // QT_ISTAT_H
