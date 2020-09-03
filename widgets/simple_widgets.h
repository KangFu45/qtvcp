#ifndef SIMPLE_WIDGETS_H
#define SIMPLE_WIDGETS_H

#include <QPushButton>

class Indicated_PushButton : public QPushButton
{
    Q_OBJECT

public:
    Indicated_PushButton(QWidget *parent = Q_NULLPTR);
    void setPara(bool iState
                 , bool tState
                 , QString tString
                 , QString fString
                 , bool draw
                 , QColor onColor
                 , QColor offColor);

    void setText(const QString &text);
    void paintEvent(QPaintEvent * event);
    void paintIndicator();
    //void indicator_update(bool data);
    void setChecked(bool data);
    bool isChecked();
    void toggle_text(bool);

signals:
    void toggled(bool checked);

private slots:
    void toggle();

private:
    bool indicator_state = false;

    bool state_text = false; // use text
    QString true_string = "True";
    QString false_string = "False";

    bool draw_indicator = false; // use indicator like clicked
    int shape = 0; // 0 triangle, 1 circle
    float size = 0.5; //triangle
    int diameter = 10; //circle
    int corner_radius = 5;
    float h_fraction = 0.3;
    float w_fraction = 0.9;
    float right_edge_offset = 0;
    float top_edge_offset = 0;
    QColor on_color = QColor("red");
    QColor off_color = QColor("black");

};

#endif // SIMPLE_WIDGETS_H
