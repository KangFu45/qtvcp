#ifndef MDI_LINE_H
#define MDI_LINE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

#include "qt_action.h"

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget *parent = Q_NULLPTR) : QLineEdit(parent) {}

signals:
    void needKeyBoard(int height);

protected:
    void mousePressEvent(QMouseEvent* event);
};

class MDILine : public QWidget
{
    Q_OBJECT

public:
    MDILine(QWidget *parent = 0);

    shared_ptr<_Lcnc_Action> ACTION;
    shared_ptr<_IStat> INFO;
    _GStat* STATUS;

    void hal_init();
    void line_up();
    void line_down();

    LineEdit* m_textLine;
    QPushButton* m_sendBtn;

public slots:
    void submit();
    void external_line_selected(string text, string filename);
    void keyPressEvent(QKeyEvent *event);

    void setEnable() {this->setEnabled(true);}
    void setDisable() {this->setEnabled(false);}
    void inter_idle_slot();
    void inter_run_slot();
    void all_homed_slot();
};

#endif // MDI_LINE_H
