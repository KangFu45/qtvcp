#include "mdi_line.h"

#include <QFile>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QDebug>

void LineEdit::mousePressEvent(QMouseEvent* event)
{
    emit this->needKeyBoard(280 - event->y());
}

MDILine::MDILine(QWidget *parent) : QWidget(parent)
{
    m_textLine = new LineEdit;
    m_sendBtn = new QPushButton("Send");
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_textLine);
    layout->addWidget(m_sendBtn);
    this->setLayout(layout);
}

void MDILine::hal_init()
{
    connect(STATUS, &_GStat::state_off,this, &MDILine::setEnable);
    connect(STATUS, &_GStat::state_estop, this, &MDILine::setDisable);
    connect(STATUS, &_GStat::interp_idle, this, &MDILine::inter_idle_slot);
    connect(STATUS, &_GStat::interp_run, this, &MDILine::inter_run_slot);
    connect(STATUS, &_GStat::all_homed, this, &MDILine::all_homed_slot);
    connect(STATUS, &_GStat::mdi_line_selected, this, &MDILine::external_line_selected);
    connect(this->m_sendBtn, &QPushButton::clicked, this, &MDILine::submit);
}

void MDILine::line_up()
{
    emit STATUS->move_text_lineup();
}

void MDILine::line_down()
{
    emit STATUS->move_text_linedown();
}

void MDILine::submit()
{
    QString text = this->m_textLine->text();
    if(text.indexOf("\n") == -1)
        text += "\n";
    if(text.isEmpty())
        return;
    else{
        ACTION->CALL_MDI(text.toStdString());
        QFile file(INFO->MDI_HISTORY_PATH.c_str());
        if(file.open(QIODevice::Append)){
            file.write(text.toUtf8());
            file.close();
        }
        emit STATUS->mdi_history_changed();
    }
}

void MDILine::external_line_selected(string text, string filename)
{
    if(filename == INFO->MDI_HISTORY_PATH)
        this->m_textLine->setText(text.c_str());
}

void MDILine::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Up)
        this->line_up();
    else if(event->key() == Qt::Key_Down)
        this->line_down();
}

void MDILine::inter_idle_slot()
{
    //qDebug()<<"inter_idle_slot: "<<(STATUS->machine_is_on() && (STATUS->is_all_homed_() || INFO->NO_HOME_REQUIRED));
    this->setEnabled(STATUS->machine_is_on() && (STATUS->is_all_homed_() || INFO->NO_HOME_REQUIRED));
}

void MDILine::inter_run_slot()
{
    //qDebug()<<"inter_run_slot: "<<(!STATUS->is_auto_mode());
    this->setEnabled(!STATUS->is_auto_mode());
}

void MDILine::all_homed_slot()
{
    //qDebug()<<"all_homed_slot: "<<(STATUS->machine_is_on());
    this->setEnabled(STATUS->machine_is_on());
}
