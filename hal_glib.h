#ifndef HAL_GLIB_H
#define HAL_GLIB_H

#include <QObject>

#include <string>
#include <vector>

#include "emcmodule.h"

typedef std::vector<std::string> strings;//TODO:move to header file

class _GStat : public QObject
{
    Q_OBJECT //Emits signals in GObject for python

public:
    _GStat (LinuxcncStat* stat = nullptr);

    LinuxcncStat* stat;
    LinuxcncCommand* cmd;

    unsigned int current_jog_rate = 15;
    unsigned int current_angular_jog_rate = 360;
    unsigned int current_jog_distance = 0;
    char* current_jog_distance_text ;
    unsigned int current_jog_distance_angular = 0;
    char* current_jog_distance_angular_text ;
    short selected_joint = -1;
    char selected_axis ;

    void merge();
    void forced_update();

    struct GStatData {
        int state;
        int mode;
        int interp;
        char* file;
        bool paused;
        int line;
        bool homed;
        int tool_in_spindle;
        int tool_prep_number;
        int motion_mode;
        double override;
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
        bool override_limits_tripped;
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
        std::string g_code;
        std::string m_code;
        CANON_TOOL_TABLE tool_info;
    }old;

private:
    bool status_active = false;
    bool is_all_homed = false;

private slots:
    void update();

signals:
    void periodic();
    void state_estop();
    void state_estop_reset();
    void state_on();
    void state_off();

    void homed();
    void unhomed();
    void all_homed();
    void not_all_homed();
    void override_limits_changed();

    void hard_limits_tripped();

    void mode_manual();
    void mode_auto();
    void mode_mdi();

    void interp_run();
    void interp_idle();
    void interp_pause();
    void interp_reading();
    void interp_waiting();

    void jograte_changed();
    void jograte_angular_changed();
    void jogincrement_changed();
    void jogincrement_angular_changed();

    void joint_selection_changed();
    void axis_selection_changed();

    void program_pause_changed();
    void optional_stop_changed();
    void block_delete_changed();

    void file_loaded();
    void reload_display();
    void line_changed();

    void tool_in_spindle_changed();
    void tool_prep_changed();
    void tool_info_changed();
    void current_tool_offset();

    void motion_mode_changed();
    void spindle_control_changed();
    void current_feed_rate();
    void current_x_rel_position();
    void current_position();

    void current_z_rotation();
    void requested_spindle_speed_changed();
    void actual_spindle_speed_changed();

    void spindle_override_changed();
    void feed_override_changed();
    void rapid_override_changed();
    void max_velocity_override_changed();

    void feed_hold_enable_changed();

    void itime_mode();
    void fpm_mode();
    void fpr_mode();
    void css_mode();
    void rpm_mode();
    void radius_mode();
    void diameter_mode();
    void flood_changed();
    void mist_changed();

    void m_code_changed();
    void g_code_changed();

    void metric_mode_changed();
    void user_system_changed();

    void mdi_line_selected();
    void gcode_line_selected();
    void graphics_line_selected();
    void graphics_gcode_error();
    void graphics_gcode_properties();
    void graphics_view_changed();
    void mdi_history_changed();
    void machine_log_changed();
    void update_machine_log();
    void move_text_lineup();
    void move_text_linedown();
    void dialog_request();
    void focus_overlay_changed();

    void play_sound();
    void virual_keyboard();
    void dro_reference_change_request();
    void show_preference();
    void shutdown();
    void error();
    void general();
    void forced_update_sig();
};

#endif // HAL_GLIB_H
