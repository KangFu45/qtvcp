#include "qt_istat.h"

#include <QFileInfo>

_IStat::_IStat(const char *inipath)
{
    QFileInfo fi(inipath);
    if(!fi.isFile()){
        this->LINUXCNC_IS_RUNNING = false;
        return;
    }

    this->inifile = new LinuxcncIni(inipath);
    this->update();
}

void _IStat::update()
{
    this->MDI_HISTORY_PATH = this->inifile->Ini_find("DISPLA", "MDI_HISTORY_FILE", "~/.axis_mdi_history");
    this->QTVCP_LOG_HISTORY_PATH = this->inifile->Ini_find("DISPLAY", "LOG_FILE", "~/qtvcp.log");
    this->MACHINE_LOG_HISTORY_PATH = this->inifile->Ini_find("DISPLAY", "MACHINE_LOG_PATH", "~/.machine_log_history");
    this->PREFERENCE_PATH = this->inifile->Ini_find("DISPLAY", "PREFERENCE_FILE_PATH");
    this->SUB_PATH = this->inifile->Ini_find("RS274NGC", "SUBROUTINE_PATH");
    //TODO: MACRO_PATH
    this->INI_MACROS = this->inifile->Ini_find("DISPLAY", "MACRO");
    //TODO: optimize
    this->MACHINE_IS_LATHE = this->inifile->Ini_find("DISPLAY", "LATHE") == "true";
    //TODO: PROGRAM_FILTERS
    this->PARAMETER_FILE = this->inifile->Ini_find("RS274NGC", "PARAMETER_FILE");
    string units = this->inifile->Ini_find("TRAJ", "LINEAR_UNITS");
    if(units.empty()){
        units = this->inifile->Ini_find("AXIS_0", "UNITS");
        if(units.empty())
            units = "inch";
    }
    // set up the conversion arrays based on what units we discovered
    if(units=="mm" || units=="metric" || units=="1.0"){
        this->MACHINE_IS_METRIC = true;
        this->MACHINE_UNIT_CONVERSION = 1.0/25.4;
        //this->MACHINE_UNIT_CONVERSION_9
    } else{
        this->MACHINE_IS_METRIC = false;
        this->MACHINE_UNIT_CONVERSION = 25.4;
        //this->MACHINE_UNIT_CONVERSION_9
    }
    string axes = this->inifile->Ini_find("TRAJ", "COORDINATES");
    //TODO: this->AVAILABLE_AXES

    // home all check
    this->HOME_ALL_FLAG = 1;
    // set Home All Flage only if ALL joints specify a HOME_SEQUENCE
    int jointcount = this->AVAILABLE_JOINTS.size();
    this->JOINT_SEQUENCE_LIST = new int[jointcount];
    for(int i=0; i<jointcount; ++i){
        string seq = this->inifile->Ini_find(string("JOINT_"+to_string(i)).c_str(), "HOME_SEQUENCE");
        if(!seq.empty()){
            seq = "-1";
            this->HOME_ALL_FLAG = 0;
        }
        this->JOINT_SEQUENCE_LIST[i] = stoi(seq);
    }
    // joint sequence/type
    this->JOINT_SEQUENCE = new int[jointcount];
    for(int i=0; i<jointcount; ++i){
        const char* section = string("JOINT_"+to_string(i)).c_str();
        this->JOINT_TYPE.emplace_back(this->inifile->Ini_find(section, "TYPE", "LINEAR"));
        this->JOINT_SEQUENCE[i] = stoi(this->inifile->Ini_find(section, "HOME_SEQUENCE", "0"));
    }
    //TODO: JOINT_SYNCH_LIST

    // jogging incrments
    if(this->MACHINE_IS_METRIC)
        this->JOG_INCREMENTS = {"Continuous", ".001 mm", ".01 mm", ".1 mm", "1 mm"};
    else
        this->JOG_INCREMENTS = {"Continuous", ".0001 in", ".001 in", ".01 in", ".1 in"};

    // angular jogging increments
    this->ANGULAR_INCREMENTS = {"Continuous", "1", "45", "180", "360"};

    //TODO: GRID_INCRMENTS

    this->TRAJ_COORDINATES = this->AVAILABLE_AXES;

    this->JOINT_COUNT = stoi(this->inifile->Ini_find("KINS", "JOINTS", "0"));
    this->DEFAULT_LINEAR_JOG_VEL = stof(this->get_error_safe_setting("DISPLAY", "DEFAULT_LINEAR_VELOCITY", "1")) * 60;
    this->MIN_LINEAR_JOG_VEL = stof(this->get_error_safe_setting("DISPLAY", "MIN_LINEAR_VELOCITY", "1")) * 60;
    this->MAX_LINEAR_JOG_VEL = stof(this->get_error_safe_setting("DISPLAY", "MAX_LINEAR_VELOCITY", "5")) * 60;
    this->DEFAULT_ANGULAR_JOG_VEL = stof(this->get_error_safe_setting("DISPLAY", "DEFAULT_ANGULAR_VELOCITY", "6")) * 60;
    this->MIN_ANGULAR_JOG_VEL = stof(this->get_error_safe_setting("DISPLAY", "MIN_ANGULAR_VELOCITY", "1")) * 60;
    this->MAX_ANGULAR_JOG_VEL = stof(this->get_error_safe_setting("DISPLAY", "MAX_ANGULAR_VELOCITY", "60")) * 60;

    this->AVAILABLE_SPINDLES = stoi(this->inifile->Ini_find("TRAJ", "SPINDLES", "1"));
    //TODO: 'DEFAULT_SPINDLE_{}_SPEED

    // check legacy
    //TODO: DEFAULT_SPINDLE_SPEED

    this->MAX_FEED_OVERRIDE = stof(this->get_error_safe_setting("DISPLAY", "MAX_FEED_OVERRIDE", "1.5")) * 100;
    this->MAX_TRAJ_VELOCITY = stof(this->get_error_safe_setting("TRAJ", "MAX_LINEAR_VELOCITY",
                                                                this->get_error_safe_setting("AXIS_X", "MAX_VELOCITY", "5"))) * 60;

    this->MDI_COMMAND_LIST = this->inifile->Ini_findall("MDI_COMMAND_LIST", "MDI_COMMAND");
    this->TOOL_FILE_PATH = this->get_error_safe_setting("EMCIO", "TOOL_TABLE");
    this->POSTGUI_HALFILE_PATH = this->inifile->Ini_find("HAL", "POSTGUI_HALFILE");
}

//********************helper function*******************

string _IStat:: get_error_safe_setting(const char* heading, const char* detail, string def)
{
    string res = this->inifile->Ini_find(heading, detail);
    if(res.empty())
        return def;//log.waring()
    return res;
}

