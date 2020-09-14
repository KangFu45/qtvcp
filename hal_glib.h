#ifndef HAL_GLIB_H
#define HAL_GLIB_H

#include <QObject>

#include <vector>

#include "emcmodule.h"

//TODO: 放在合适的地方
const auto PI = 3.141592653589793238;

/*
原文件：linuxcnc/lib/python/hal_glib.py
对状态缓冲区进行再一次的封装，定义一些基础的信号。
python用gobject来传递处理信号，这里就用qt的信号槽机制。
*/

class _GStat : public QObject
{
    Q_OBJECT //Emits signals in GObject for python

public:
    _GStat (shared_ptr<LinuxcncStat> stat = nullptr);

    shared_ptr<LinuxcncStat> stat;
    shared_ptr<LinuxcncCommand> cmd;

    //TODO:轴的移动速度类型应该是float，但传递一些值时，在Continue手动移动轴时无法停止。
    int current_jog_rate = 15;
    int current_angular_jog_rate = 360;
    float current_jog_distance = 0.0;
    string current_jog_distance_text ;
    int current_jog_distance_angular = 0;
    string current_jog_distance_angular_text ;
    short selected_joint = -1;
    char selected_axis ;

    void merge();
    void forced_update();

    struct GStatData {
        EMC_TASK_STATE_ENUM state;
        EMC_TASK_MODE_ENUM mode;
        EMC_TASK_INTERP_ENUM interp;
        string file;
        bool paused;
        int line;
        bool* homed;
        int tool_in_spindle;
        int tool_prep_number;
        int motion_mode;
        double spindle_or;
        double feed_or;
        double rapid_or;
        double max_velocity_or;
        bool feed_hold;
        int g5x_index;
        bool spindle_enabled;
        int spindle_direction;
        bool block_delete;
        bool optional_stop;
        double actual_spindle_speed;
        int flood;
        int mist;
        double current_z_rotation;
        double* current_tool_offset;
        unsigned char* override_limits;
        bool override_limits_set;
        bool hard_limits_tripped;
        unsigned char* hard_limit_list;
        bool itime;
        bool fpm;
        bool fpr;
        bool css;
        bool rpm;
        bool metric;
        bool radius;
        bool diameter;
        double spindle_speed;
        string g_code;
        string m_code;
        CANON_TOOL_TABLE tool_info;

        double* act_positions;
        double* relp;
        double* dtg;
        double* joint_act_positions;
        double* joint_act_velocitys;
    }fresh;

    void  check_for_modes(int& state, EMC_TASK_MODE_ENUM mode,EMC_TASK_MODE_ENUM& premode);
    EMC_TASK_MODE_ENUM get_current_mode() {return this->fresh.mode;}
    void set_jograte(float upm);
    int get_jograte() {return this->current_jog_rate; }
    void set_jograte_angular(unsigned int rate);
    double get_jograte_angular() {return this->current_angular_jog_rate; }
    double get_jog_increment_angular() {return this->current_jog_distance_angular; }
    void set_jog_increment_angular(unsigned int distance, string text);
    void set_jog_increments(float distance, string text);
    float get_jog_increment() {return this->current_jog_distance; }
    void set_selected_joint(short data);
    int get_selected_joint() {return this->selected_joint; }
    void set_selected_axis(char data);
    char get_selected_axis() {return this->selected_axis; }
    bool is_all_homed_() {return this->is_all_homed; }
    bool is_homing();
    bool machine_is_on() {return this->fresh.state > EMC_TASK_STATE_OFF;}
    bool estop_is_clear() {return this->fresh.state > EMC_TASK_STATE_ESTOP; }
    bool is_man_mode();
    bool is_mdi_mode();
    bool is_auto_mode();
    bool is_on_and_idle();
    bool is_auto_running();
    bool is_auto_paused() {return this->fresh.paused; }
    bool is_interp_running();
    bool is_interp_paused();
    bool is_interp_reading();
    bool is_interp_waiting();
    bool is_interp_idle();
    bool is_file_loaded();
    bool is_metric_mode() {return this->fresh.metric; }
    bool is_spindle_on();
    int get_spindle_speed();
    bool is_joint_mode();
    bool is_status_valid() {return this->status_active; }
    bool is_limits_override_set() {return this->fresh.override_limits_set; }
    bool is_hard_limits_tripped() {return this->fresh.hard_limits_tripped; }
    int get_current_tool();
    void set_tool_touchoff(int tool, char axis, int value);
    void set_axis_origin(char axis, int value);
    void do_jog(int axisnum, int direction, double distance = 0);
    EMC_TRAJ_MODE_ENUM get_jjogmode();
    int jnum_for_axisnum(int axisnum);
    void get_jog_info(int axisnum);
    double* get_probed_position_with_offsets();
    void shutdown_() {emit this->shutdown();}

private:
    bool status_active = false;
    bool is_all_homed = false;

    //helper function
    void STATES(EMC_TASK_STATE_ENUM state);
    void MODES(EMC_TASK_MODE_ENUM mode);
    void INTERP(EMC_TASK_INTERP_ENUM interp);

    double* get_rel_position(double* act_position, double* g5x_offset
                             , double* tool_offset, double* g92_offset);

private slots:
    void update();

signals:
    void periodic();
    void state_estop();
    void state_estop_reset();
    void state_on();
    void state_off();

    void homed(int);
    void unhomed(int);
    void all_homed();
    void not_all_homed(const char*);
    void override_limits_changed(bool, unsigned char*);

    void hard_limits_tripped(bool, unsigned char*);

    void mode_manual();
    void mode_auto();
    void mode_mdi();

    void interp_run();
    void interp_idle();
    void interp_pause();
    void interp_reading();
    void interp_waiting();

    void jograte_changed(float);
    void jograte_angular_changed(int);
    void jogincrement_changed(float, string);
    void jogincrement_angular_changed(int, string);

    void joint_selection_changed(short);
    void axis_selection_changed(char);

    void program_pause_changed(bool);
    void optional_stop_changed(bool);
    void block_delete_changed(bool);

    void file_loaded(const char*);
    void reload_display();
    void line_changed(int);

    void tool_in_spindle_changed(int);
    void tool_prep_changed(int);
    void tool_info_changed();
    void current_tool_offset(double*);

    void motion_mode_changed(int);
    void spindle_control_changed(bool, int);
    void current_feed_rate(double);
    void current_x_rel_position(double);
    //arg1:absolute position
    //arg2:tool position
    //arg2:dtg
    //arg4;joint actual position
    void current_position_and_velocity(double* ,double*, double*, double*, double*);

    void current_z_rotation(double);
    void requested_spindle_speed_changed(double);
    void actual_spindle_speed_changed(double);

    void spindle_override_changed(double);
    void feed_override_changed(double);
    void rapid_override_changed(double);
    void max_velocity_override_changed(double);

    void feed_hold_enable_changed(bool);

    void itime_mode(bool);
    void fpm_mode(bool);
    void fpr_mode(bool);
    void css_mode(bool);
    void rpm_mode(bool);
    void radius_mode(bool);
    void diameter_mode(bool);
    void flood_changed(int);
    void mist_changed(int);

    void m_code_changed(const char*);
    void g_code_changed(const char*);

    void metric_mode_changed(bool);
    void user_system_changed(int);

    void mdi_line_selected(string, string);
    void gcode_line_selected(int);
    void graphics_line_selected(int);
    void graphics_gcode_error();
    void graphics_gcode_properties();
    void graphics_view_changed();
    void mdi_history_changed();
    void machine_log_changed();
    void update_machine_log(string);
    void move_text_lineup();
    void move_text_linedown();
    void dialog_request();
    void focus_overlay_changed();

    void play_sound();
    void virual_keyboard();
    void dro_reference_change_request();
    void show_preference();
    void shutdown();
    void error(string);
    void general();
    void forced_update_sig();

    void file_edit_exit();
};

#endif // HAL_GLIB_H
