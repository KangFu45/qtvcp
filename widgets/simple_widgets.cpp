#include "simple_widgets.h"

#include <QPainter>

Indicated_PushButton::Indicated_PushButton(QWidget *parent)
    :QPushButton(parent)
{
    connect(this, &Indicated_PushButton::clicked, this, &Indicated_PushButton::toggle);
}

void Indicated_PushButton::setPara(bool iState
             , bool tState
             , QString tString
             , QString fString
             , bool draw
             , QColor onColor
             , QColor offColor)
{
    this->indicator_state = iState;
    this->state_text = tState;
    this->true_string = tString;
    this->false_string = fString;
    this->draw_indicator = draw;
    this->on_color = onColor;
    this->off_color = offColor;
}

void Indicated_PushButton::setText(const QString &text)
{
    if(!this->state_text){
        this->QPushButton::setText(text);
        return;
    }
    //if(this->isCheckable()){
    //    if(this->isChecked())
    //        this->QPushButton::setText(this->true_string);
    //    else
    //        this->QPushButton::setText(this->false_string);
    //}
    //else
    if(this->indicator_state)
        this->QPushButton::setText(this->true_string);
    else
        this->QPushButton::setText(this->false_string);
}

void Indicated_PushButton::setChecked(bool data)
{
    this->indicator_state = data;
    this->toggle_text(false);
    this->update();
}

bool Indicated_PushButton::isChecked()
{
    return this->indicator_state;
}

void Indicated_PushButton::toggle()
{
    this->setChecked(this->indicator_state = !this->indicator_state);
    emit this->toggled(this->indicator_state);
}

void Indicated_PushButton::toggle_text(bool)
{
    if(this->state_text)
        this->setText("");
}

void Indicated_PushButton::paintEvent(QPaintEvent * event)
{
    this->QPushButton::paintEvent(event);
    if(this->draw_indicator)
        this->paintIndicator();
}

void Indicated_PushButton::paintIndicator()
{
    QPainter p(this);
    QColor color;

    if(this->indicator_state)
        color = this->on_color;
    else
        color = this->off_color;

    // triangle
    if(this->shape == 0){
        QRect rect = p.window();
        QPoint top_right = rect.topRight() - QPoint(this->right_edge_offset, -this->top_edge_offset);
        float size1;
        if(this->width() < this->height())
            size1 = this->width() * this->size;
        else
            size1 = this->height() * this->size;

        QLinearGradient gradient(top_right - QPoint(size1,0), top_right);
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1,color);
        p.setBrush(QBrush(gradient));
        p.setPen(color);


        QVector<QPoint> v;
        v.push_back(top_right);
        v.push_back(top_right - QPoint(size1,0));
        v.push_back(top_right + QPoint(0, size1));
        QPolygon triangle(v);

        p.drawLine(triangle.point(1), triangle.point(2));
        p.drawPolygon(triangle);
    }
}
