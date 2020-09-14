#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "qt_action.h"

namespace Ui {
class MainWindow;
}

/*
主窗口在原qtvcp中是被设计成一个通用的平台，能读取给定的.ui文件与动作，可以支持多种风格的界面。
这里就给示教器设计了一个固定的界面。
有一些控件的事件是在这里绑定的，可以放在自定义的控件绑定，这样代码会更清晰。
*/

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(shared_ptr<_IStat> info, QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    shared_ptr<_IStat> INFO;
    shared_ptr<_Lcnc_Action> ACTION;

    _GStat* STATUS; // Can't use singal-slot in shared_ptr

    void init_ui_2();
    void init_singal();
    void init_status();
    void init_ini();

public slots:
    void mdi_edit_slot(int height);
    void keyboard_show(int height);
    void keyboard_hide();
    void vbar_changed(int min, int max);
    void text_edit_slot();

    void load_program(QString fname);

    //gui signal
    void estop_btn_check(bool checked);
    void machine_on_btn_check(bool checked);
    void all_home_slot();
    void mode_manual_check(bool checked);
    void mode_mdi_check(bool checked);
    void mode_auto_check(bool checked);
    void run_slot();
    void run_toggle_slot(bool data = false);
    void abort_slot();
    void pause_slot();
    void pause_toggle_slot(bool data = false);
    void step_slot();
    void file_edit_slot();
    void file_edit_exit_slot();
    void incr_action(QString text);
    void x_minus_jog_press();
    void x_jog_release();
    void x_plus_jog_press();
    void y_minus_jog_press();
    void y_jog_release();
    void y_plus_jog_press();
    void z_minus_jog_press();
    void z_jog_release();
    void z_plus_jog_press();
    void jog_rate_changed(int data);
    void set_jog_rate_slot();

    //status singal
    void mode_manual_slot();
    void mode_mdi_slot();
    void mode_auto_slot();
    void state_estop_slot();
    void state_estop_reset_slot();
    void state_on_slot();
    void state_off_slot();
    void interp_run_slot();
    void interp_idle_slot();
    void current_position_and_velocity_slot(double* abs_pos, double* tool_pos, double* dtg, double* act_pos, double* act_vel);
    void get_error(string s);
};

#endif // MAINWINDOW_H
