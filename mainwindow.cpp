#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "widgets/mdi_line.h"
#include "widgets/virtualkeyboardwidget.h"

#include <QDebug>
#include <QScrollBar>

MainWindow::MainWindow(shared_ptr<_IStat> info, QWidget *parent) :
    INFO(info),
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    shared_ptr<_Lcnc_Action> action(new _Lcnc_Action(INFO));
    ACTION = action;
    STATUS = ACTION->STATUS;

    ui->setupUi(this);

    this->ui->GcodeDisplayWidget->set_auto_show_mdi(true);
    this->ui->GcodeDisplayWidget->INFO = INFO;
    this->ui->GcodeDisplayWidget->STATUS = STATUS;

    this->ui->MDILineWidget->INFO = INFO;
    this->ui->MDILineWidget->STATUS = STATUS;
    this->ui->MDILineWidget->ACTION = ACTION;

    this->ui->GcodeEditWidget->ACTION = ACTION;
    this->ui->GcodeEditWidget->STATUS = STATUS;
    this->ui->GcodeEditWidget->m_editor->STATUS = STATUS;
    this->ui->GcodeEditWidget->m_editor->INFO = INFO;

    this->ui->OffsetTableView->ACTION = ACTION;
    this->ui->OffsetTableView->STATUS = STATUS;
    this->ui->OffsetTableView->INFO = INFO;

    init_ui_2();
    init_ini();
    init_singal();
    init_status();
}

MainWindow::~MainWindow()
{
    if( CVirtualKeyBoardWidget::GetInstance().isVisible() )
            CVirtualKeyBoardWidget::GetInstance().close();
    delete ui;
}

void MainWindow::init_ui_2()
{
    this->ui->estop_btn->setPara(true, true, "ESTOP SET", "ESTOP RESET", true, QColor(Qt::black), QColor(Qt::green));
    this->ui->machine_on_btn->setPara(true, true, "MACHINE ON", "MACHINE OFF", true, QColor(Qt::green), QColor(Qt::black));
    this->ui->mode_manual_btn->setPara(true, false, "", "", true, QColor(Qt::green), QColor(Qt::black));
    this->ui->mode_mdi_btn->setPara(true, false, "", "", true, QColor(Qt::green), QColor(Qt::black));
    this->ui->mode_auto_btn->setPara(true, false, "", "", true, QColor(Qt::green), QColor(Qt::black));
    this->ui->run_btn->setPara(true, false, "", "", true, QColor(Qt::green), QColor(Qt::black));
    this->ui->pause_btn->setPara(true, false, "", "", true, QColor(Qt::yellow), QColor(Qt::black));

    // g5x_offset choose
    // TODO: to setup
    this->ui->coord_comboBox->addItems({"ABS", "DTG", "G54"});

    this->ui->GcodeEditWidget->setHidden(true);

    this->ui->jog_rate_sli->setRange(int(INFO->MIN_LINEAR_JOG_VEL), int(INFO->MAX_LINEAR_JOG_VEL));
    this->ui->jog_rate_sli->setValue(STATUS->get_jograte() * 60);
    this->ui->jog_rate_lab->setText(QString::number(STATUS->get_jograte() * 60));
}

void MainWindow::init_ini()
{
    //TRAJ:LINEAR_UNITS
    if(INFO->MACHINE_IS_METRIC){
        this->ui->pos_unit_lab->setText("mm");
        this->ui->vel_unit_lab->setText("mm/s");
    }
    else{
        this->ui->pos_unit_lab->setText("inch");
        this->ui->vel_unit_lab->setText("inch/s");
    }

    foreach (const string& t, INFO->JOG_INCREMENTS) {
        this->ui->inc_comboBox->addItem(t.c_str());
    }

    //TRAJ:COOEDINATES
    this->ui->x_frame->setVisible(false);
    this->ui->y_frame->setVisible(false);
    this->ui->z_frame->setVisible(false);
    //this->ui->u_frame->setVisible(false);
    foreach (const char& t, INFO->AVAILABLE_AXES) {
        if(t == 'X')
            this->ui->x_frame->setVisible(true);
        else if(t == 'Y')
            this->ui->y_frame->setVisible(true);
        else if(t == 'Z')
            this->ui->z_frame->setVisible(true);
        //else if(t == 'U')
        //    this->ui->u_frame->setVisible(true);
        //...A,B,C,V,W
    }
}

void MainWindow::init_status()
{   
    //TODO: to complete

    EMC_TASK_MODE_ENUM mode = STATUS->get_current_mode();
    switch (mode) {
    case EMC_TASK_MODE_MANUAL:
        emit STATUS->mode_manual();
        break;
    case EMC_TASK_MODE_MDI:
        emit STATUS->mode_mdi();
        break;
    case EMC_TASK_MODE_AUTO:
        emit STATUS->mode_auto();
        break;
    default:
        break;
    }

    if(STATUS->is_all_homed_())
        emit STATUS->all_homed();

    // power
    if(STATUS->machine_is_on()){
        //qDebug()<<"machine on: true";
        this->ui->machine_on_btn->setChecked(true);
        this->state_on_slot();
    }
    else{
        //qDebug()<<"machine on: false";
        this->ui->machine_on_btn->setChecked(false);//invert
        this->state_off_slot();
    }

    // check all status in begin
    // estop
    if(STATUS->estop_is_clear()){
        //qDebug()<<"estop reset: true";
        this->ui->estop_btn->setChecked(false);//invert
        this->state_estop_reset_slot();
    }
    else{
        //qDebug()<<"estop reset: false";
        this->ui->estop_btn->setChecked(true);//invert
        this->state_estop_slot();
    }

    // interp
    this->ui->run_btn->setChecked(STATUS->is_auto_running());
    this->ui->pause_btn->setChecked(STATUS->is_auto_paused());


}

void MainWindow::init_singal()
{
    this->ui->GcodeDisplayWidget->hal_init();
    this->ui->MDILineWidget->hal_init();
    this->ui->GcodeEditWidget->hal_init();
    this->ui->OffsetTableView->hal_init();

    connect(this->ui->estop_btn,&Indicated_PushButton::toggled,this,&MainWindow::estop_btn_check);
    connect(this->ui->machine_on_btn,&Indicated_PushButton::toggled,this,&MainWindow::machine_on_btn_check);
    connect(this->ui->home_all_btn,&QPushButton::clicked,this,&MainWindow::all_home_slot);
    connect(this->ui->mode_manual_btn,&Indicated_PushButton::toggled,this,&MainWindow::mode_manual_check);
    connect(this->ui->mode_mdi_btn,&Indicated_PushButton::toggled,this,&MainWindow::mode_mdi_check);
    connect(this->ui->mode_auto_btn,&Indicated_PushButton::toggled,this,&MainWindow::mode_auto_check);
    connect(this->ui->MDILineWidget->m_textLine,&LineEdit::needKeyBoard,this,&MainWindow::mdi_edit_slot);
    connect(this->ui->MDILineWidget->m_textLine,&LineEdit::editingFinished,this,&MainWindow::keyboard_hide);
    connect(this->ui->scrollArea->verticalScrollBar(),&QScrollBar::rangeChanged,this,&MainWindow::vbar_changed);
    connect(this->ui->FileManagerWidget, &FileManager::load, this, &MainWindow::load_program);
    connect(this->ui->run_btn, &QPushButton::clicked, this, &MainWindow::run_slot);
    connect(this->ui->run_btn, &Indicated_PushButton::toggled, this, &MainWindow::run_toggle_slot);
    connect(this->ui->pause_btn, &QPushButton::clicked, this, &MainWindow::pause_slot);
    connect(this->ui->pause_btn, &Indicated_PushButton::toggled, this, &MainWindow::pause_toggle_slot);
    connect(this->ui->abort_btn, &QPushButton::clicked, this, &MainWindow::abort_slot);
    connect(this->ui->step_btn, &QPushButton::clicked, this, &MainWindow::step_slot);
    connect(this->ui->FileManagerWidget->m_editBtn, &QPushButton::clicked, this, &MainWindow::file_edit_slot);
    connect(this->ui->GcodeEditWidget->m_searchText, &LineEdit::needKeyBoard, this, &MainWindow::keyboard_show);
    connect(this->ui->GcodeEditWidget->m_searchText,&LineEdit::editingFinished,this,&MainWindow::keyboard_hide);
    connect(this->ui->GcodeEditWidget->m_replaceText, &LineEdit::needKeyBoard, this, &MainWindow::keyboard_show);
    connect(this->ui->GcodeEditWidget->m_replaceText,&LineEdit::editingFinished,this,&MainWindow::keyboard_hide);
    connect(this->ui->GcodeEditWidget->m_exitAction,&QAction::triggered,this,&MainWindow::keyboard_hide);
    connect(this->ui->GcodeEditWidget->m_keyBoardAct,&QAction::triggered,this,&MainWindow::text_edit_slot);
    connect(this->ui->inc_comboBox,&QComboBox::currentTextChanged,this,&MainWindow::incr_action);
    connect(this->ui->x_minus_jogbutton,&QPushButton::pressed,this,&MainWindow::x_minus_jog_press);
    connect(this->ui->x_minus_jogbutton,&QPushButton::released,this,&MainWindow::x_jog_release);
    connect(this->ui->x_plus_jogbutton,&QPushButton::pressed,this,&MainWindow::x_plus_jog_press);
    connect(this->ui->x_plus_jogbutton,&QPushButton::released,this,&MainWindow::x_jog_release);
    connect(this->ui->y_minus_jogbutton,&QPushButton::pressed,this,&MainWindow::y_minus_jog_press);
    connect(this->ui->y_minus_jogbutton,&QPushButton::released,this,&MainWindow::y_jog_release);
    connect(this->ui->y_plus_jogbutton,&QPushButton::pressed,this,&MainWindow::y_plus_jog_press);
    connect(this->ui->y_plus_jogbutton,&QPushButton::released,this,&MainWindow::y_jog_release);
    connect(this->ui->z_minus_jogbutton,&QPushButton::pressed,this,&MainWindow::z_minus_jog_press);
    connect(this->ui->z_minus_jogbutton,&QPushButton::released,this,&MainWindow::z_jog_release);
    connect(this->ui->z_plus_jogbutton,&QPushButton::pressed,this,&MainWindow::z_plus_jog_press);
    connect(this->ui->z_plus_jogbutton,&QPushButton::released,this,&MainWindow::z_jog_release);
    connect(this->ui->jog_rate_sli,&QSlider::valueChanged,this,&MainWindow::jog_rate_changed);
    connect(this->ui->jog_rate_sli,&QSlider::sliderReleased,this,&MainWindow::set_jog_rate_slot);
    connect(this->ui->OffsetTableView,&OriginOffsetView::needKeyBoard,this,&MainWindow::text_edit_slot);
    connect(this->ui->OffsetTableView,&OriginOffsetView::hidekeyBoard,this,&MainWindow::keyboard_hide);

    connect(STATUS,&_GStat::mode_manual,this,&MainWindow::mode_manual_slot);
    connect(STATUS,&_GStat::mode_mdi,this,&MainWindow::mode_mdi_slot);
    connect(STATUS,&_GStat::mode_auto,this,&MainWindow::mode_auto_slot);
    connect(STATUS,&_GStat::state_estop,this,&MainWindow::state_estop_slot);
    connect(STATUS,&_GStat::state_estop_reset,this,&MainWindow::state_estop_reset_slot);
    connect(STATUS,&_GStat::state_on,this,&MainWindow::state_on_slot);
    connect(STATUS,&_GStat::state_off,this,&MainWindow::state_off_slot);
    connect(STATUS,&_GStat::interp_idle,this,&MainWindow::interp_idle_slot);
    connect(STATUS,&_GStat::interp_run,this,&MainWindow::interp_run_slot);
    connect(STATUS,&_GStat::program_pause_changed,this,&MainWindow::pause_toggle_slot);
    connect(STATUS,&_GStat::file_edit_exit,this,&MainWindow::file_edit_exit_slot);
    connect(STATUS,&_GStat::current_position_and_velocity,this,&MainWindow::current_position_and_velocity_slot);
    connect(STATUS,&_GStat::error,this,&MainWindow::get_error);
}

void MainWindow::mdi_edit_slot(int height)
{
    if(!STATUS->is_mdi_mode()) ACTION->SET_MDI_MODE();
    this->keyboard_show(height);
}

void MainWindow::keyboard_show(int height)
{
    if(!CVirtualKeyBoardWidget::GetInstance().isVisible())
        CVirtualKeyBoardWidget::GetInstance().ShowVirtualKeyBoard();

    //placeholder_lift
    if(this->ui->PlaceholderWidget->height() == 0)
        this->ui->PlaceholderWidget->setFixedHeight(height);
}

void MainWindow::keyboard_hide()
{
    if(this->ui->PlaceholderWidget->height() != 0)
        this->ui->PlaceholderWidget->setFixedHeight(0);
    if(CVirtualKeyBoardWidget::GetInstance().isVisible())
        CVirtualKeyBoardWidget::GetInstance().Hide();
}

void MainWindow::vbar_changed(int min, int max)
{
    this->ui->scrollArea->verticalScrollBar()->setValue(max);
}

void MainWindow::estop_btn_check(bool checked)
{
    if(checked){
        ACTION->SET_ESTOP_STATE(true);
    }
    else{
        ACTION->SET_ESTOP_STATE(false);
    }
}

void MainWindow::machine_on_btn_check(bool checked)
{
    if(checked)
        ACTION->SET_MACHINE_STATE(true);
    else
        ACTION->SET_MACHINE_STATE(false);
}

void MainWindow::all_home_slot()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->SET_MACHINE_HOMING(0);
    ACTION->SET_MACHINE_HOMING(1);
    ACTION->SET_MACHINE_HOMING(2);
}

void MainWindow::mode_manual_slot()
{
    if(STATUS->is_interp_idle()){
        if(!this->ui->mode_manual_btn->isChecked()) this->ui->mode_manual_btn->setChecked(true);
        if(this->ui->mode_mdi_btn->isChecked()) this->ui->mode_mdi_btn->setChecked(false);
        if(this->ui->mode_auto_btn->isChecked()) this->ui->mode_auto_btn->setChecked(false);
        //auto mode widget hide
        if(!this->ui->AutoControlFrame->isHidden()) this->ui->AutoControlFrame->setHidden(true);
        if(!this->ui->FileManagerWidget->isHidden()) this->ui->FileManagerWidget->setHidden(true);
        if(!this->ui->GcodeEditWidget->isHidden()) this->ui->GcodeEditWidget->setHidden(true);
    }
}

void MainWindow::mode_mdi_slot()
{
    if(STATUS->is_interp_idle()){
        if(this->ui->mode_manual_btn->isChecked()) this->ui->mode_manual_btn->setChecked(false);
        if(!this->ui->mode_mdi_btn->isChecked())this->ui->mode_mdi_btn->setChecked(true);
        if(this->ui->mode_auto_btn->isChecked())this->ui->mode_auto_btn->setChecked(false);
        if(!this->ui->AutoControlFrame->isHidden()) this->ui->AutoControlFrame->setHidden(true);
        if(!this->ui->FileManagerWidget->isHidden()) this->ui->FileManagerWidget->setHidden(true);
        if(!this->ui->GcodeEditWidget->isHidden()) this->ui->GcodeEditWidget->setHidden(true);
    }
}

void MainWindow::mode_auto_slot()
{
    if(STATUS->is_interp_idle()){
        if(this->ui->mode_manual_btn->isChecked())this->ui->mode_manual_btn->setChecked(false);
        if(this->ui->mode_mdi_btn->isChecked())this->ui->mode_mdi_btn->setChecked(false);
        if(!this->ui->mode_auto_btn->isChecked())this->ui->mode_auto_btn->setChecked(true);
        if(this->ui->AutoControlFrame->isHidden()) this->ui->AutoControlFrame->setHidden(false);
        if(this->ui->FileManagerWidget->isHidden()) this->ui->FileManagerWidget->setHidden(false);
    }
}

void MainWindow::state_estop_slot()
{
    this->state_off_slot();
    this->ui->machine_on_btn->setEnabled(false);
    this->ui->machine_on_btn->setChecked(false);
}

void MainWindow::state_estop_reset_slot()
{
    this->ui->machine_on_btn->setEnabled(true);
}

void MainWindow::state_on_slot()
{
    this->ui->JogButtonsWidget->setEnabled(true);
    this->ui->coord_comboBox->setEnabled(true);
    this->ui->MDILineWidget->setEnabled(true);
    this->ui->AutoControlFrame->setEnabled(true);
}

void MainWindow::state_off_slot()
{
    this->ui->JogButtonsWidget->setEnabled(false);
    this->ui->coord_comboBox->setEnabled(false);
    this->ui->MDILineWidget->setEnabled(false);
    this->ui->AutoControlFrame->setEnabled(false);
}

void MainWindow::current_position_and_velocity_slot(double* abs_pos, double* tool_pos, double* dtg, double* act_pos, double* act_vel)
{
    QString coord = this->ui->coord_comboBox->currentText();
    double* pos;
    if(coord == "ABS")
        pos = abs_pos;
    else if(coord == "DTG")
        pos = dtg;
    else if(coord == "G54")
        pos = tool_pos;

    foreach (const char& t, INFO->AVAILABLE_AXES) {
        if(t == 'X' && this->ui->x_frame->isVisible()){
            this->ui->x_pos_lab->setText(QString::number(pos[0],'f',3));
            this->ui->x_vel_lab->setText(QString::number(act_vel[0]));
        }
        else if(t == 'Y' && this->ui->y_frame->isVisible()){
            this->ui->y_pos_lab->setText(QString::number(pos[1],'f',3));
            this->ui->y_vel_lab->setText(QString::number(act_vel[1]));
        }
        else if(t == 'Z' && this->ui->z_frame->isVisible()){
            this->ui->z_pos_lab->setText(QString::number(pos[2],'f',3));
            this->ui->z_vel_lab->setText(QString::number(act_vel[2]));
        }
        //...A,B,C,V,W
    }

}

void MainWindow::mode_manual_check(bool checked)
{
    if(checked){
        if(!STATUS->is_man_mode() && STATUS->is_interp_idle()){
            ACTION->SET_MANUAL_MODE();
            emit STATUS->mode_manual();
        }
        else
            this->ui->mode_manual_btn->setChecked(false);
    }
    else{
        if(STATUS->is_man_mode())
            this->ui->mode_manual_btn->setChecked(true);
    }
}

void MainWindow::mode_mdi_check(bool checked)
{
    if(checked){
        if(!STATUS->is_mdi_mode() && STATUS->is_interp_idle() ){
            ACTION->SET_MDI_MODE();
            emit STATUS->mode_mdi();
        }
        else
            this->ui->mode_mdi_btn->setChecked(false);
    }
    else{
        if(STATUS->is_mdi_mode())
            this->ui->mode_mdi_btn->setChecked(true);
    }
}

void MainWindow::mode_auto_check(bool checked)
{
    if(checked){
        if(!STATUS->is_auto_mode() && STATUS->is_interp_idle() ){
            ACTION->SET_AUTO_MODE();
            emit STATUS->mode_auto();
        }
        else
            this->ui->mode_auto_btn->setChecked(false);
    }
    else{
        if(STATUS->is_auto_mode())
            this->ui->mode_auto_btn->setChecked(true);
    }
}

void MainWindow::load_program(QString fname)
{
    ACTION->OPEN_PROGRAM(fname.toStdString());
    emit STATUS->update_machine_log("loaded: " + fname.toStdString());
}

void MainWindow::run_slot()
{
    ACTION->RUN();
}

void MainWindow::run_toggle_slot(bool data)
{
    // keep this run status
    if(STATUS->is_auto_running() != this->ui->run_btn->isChecked())
        this->ui->run_btn->setChecked(STATUS->is_auto_running());
}

void MainWindow::interp_run_slot()
{
    this->run_toggle_slot(true);
    this->ui->home_all_btn->setEnabled(false);
    this->ui->JogButtonsWidget->setEnabled(false);
    this->ui->pause_btn->setEnabled(true);
    this->ui->step_btn->setEnabled(true);
    this->ui->abort_btn->setEnabled(true);
}

void MainWindow::interp_idle_slot()
{
    this->run_toggle_slot(false);
    bool on = STATUS->machine_is_on();
    this->ui->home_all_btn->setEnabled(on);
    this->ui->JogButtonsWidget->setEnabled(on);
    this->ui->run_btn->setEnabled(on && (STATUS->is_all_homed_() || INFO->NO_HOME_REQUIRED));
    this->ui->pause_btn->setEnabled(false);
    this->ui->step_btn->setEnabled(false);
    this->ui->abort_btn->setEnabled(false);
}

void MainWindow::abort_slot()
{
    ACTION->ABORT();
}

void MainWindow::pause_slot()
{
    ACTION->PAUSE();
}

void MainWindow::pause_toggle_slot(bool data)
{
    // keep this run status
    if(STATUS->is_auto_paused() != this->ui->pause_btn->isChecked())
        this->ui->pause_btn->setChecked(STATUS->is_auto_paused());
}

void MainWindow::step_slot()
{
    ACTION->STEP();
}

void MainWindow::file_edit_slot()
{
    if(STATUS->is_on_and_idle()){
        if(!this->ui->AutoControlFrame->isHidden()) this->ui->AutoControlFrame->setHidden(true);
        if(!this->ui->FileManagerWidget->isHidden()) this->ui->FileManagerWidget->setHidden(true);

        this->ui->GcodeEditWidget->editMode();
        this->ui->GcodeEditWidget->setHidden(false);
    }
}

void MainWindow::file_edit_exit_slot()
{
    this->ui->GcodeEditWidget->setHidden(true);
    if(STATUS->is_auto_mode())
        this->mode_auto_slot();
}

void MainWindow::text_edit_slot()
{
    // just open keyboard
    if(!CVirtualKeyBoardWidget::GetInstance().isVisible()){
        CVirtualKeyBoardWidget::GetInstance().ShowVirtualKeyBoard();
        //placeholder_lift
        this->ui->PlaceholderWidget->setFixedHeight(240);
    }
    else{
        CVirtualKeyBoardWidget::GetInstance().Hide();
        this->ui->PlaceholderWidget->setFixedHeight(0);
    }
}

void MainWindow::incr_action(QString text)
{
    float incr;
    if(text == "Continuous")
        incr = 0.0;
    else{
        float d = text.left(text.indexOf(" ")).toFloat();
        if(d <= 0) return;
        incr = INFO->convert_metric_to_machine(d);
    }
    ACTION->SET_JOG_INCR(incr, text.toStdString());
}

void MainWindow::x_minus_jog_press()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->DO_JOG(0, -1);
}

void MainWindow::x_jog_release()
{
    if(STATUS->get_jog_increment() != 0.0) return;
    //ACTION->DO_JOG(0, 0);
    ACTION->STOP_JOG(0);
}

void MainWindow::x_plus_jog_press()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->DO_JOG(0, 1);
}

void MainWindow::y_minus_jog_press()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->DO_JOG(1, -1);
}

void MainWindow::y_jog_release()
{
    if(STATUS->get_jog_increment() != 0.0) return;
    ACTION->DO_JOG(1, 0);
}

void MainWindow::y_plus_jog_press()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->DO_JOG(1, 1);
}

void MainWindow::z_minus_jog_press()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->DO_JOG(2, -1);
}

void MainWindow::z_jog_release()
{
    if(STATUS->get_jog_increment() != 0.0) return;
    ACTION->DO_JOG(2, 0);
}

void MainWindow::z_plus_jog_press()
{
    ACTION->ensure_mode(EMC_TASK_MODE_MANUAL);
    ACTION->DO_JOG(2, 1);
}

void MainWindow::jog_rate_changed(int data)
{
    this->ui->jog_rate_lab->setText(QString::number(data));
}

void MainWindow::set_jog_rate_slot()
{
    ACTION->SET_JOG_RATE(this->ui->jog_rate_sli->value() / 60.0);
}

void MainWindow::get_error(string s)
{
    qDebug()<<s.c_str();
}
