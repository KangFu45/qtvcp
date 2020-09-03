#ifndef ORIGIN_OFFSETVIEW_H
#define ORIGIN_OFFSETVIEW_H

#include <QTableView>
#include <QAbstractItemView>
#include <QAbstractTableModel>
#include <QVariant>

#include "qt_action.h"

class MyTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MyTableModel(float** datain, strings _headerdata, strings _vheaderdata, strings names, QObject* parent=nullptr)
        : arraydata(datain), headerdata(_headerdata), vheaderdata(_vheaderdata), namedata(names), QAbstractTableModel(parent) {}

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    float** arraydata; // 13*9
    strings headerdata;
    strings vheaderdata;
    strings namedata;
};

class OriginOffsetView : public QTableView
{
    Q_OBJECT

public:
    OriginOffsetView(QWidget *parent = 0);

    shared_ptr<_Lcnc_Action> ACTION;
    shared_ptr<_IStat> INFO;
    _GStat* STATUS;

    QString filename;
    char* axisletters;
    QString current_system;
    int current_tool = 0;
    bool metric_display = false;
    float** tabledata; // 13*9
    MyTableModel* m_tablemodel;
    int delay = 0;

    void hal_init();
    void createTable();
    void reload_offsets();
    float** read_file(); // 9*9

    void currentToot(int data) {this->current_tool = data;}

signals:
    void needKeyBoard();
    void hidekeyBoard();

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

public slots:
    void showSelection(const QModelIndex &index);
    void periodic_check();

    void all_homed_slot() {this->setEnabled(true);}
    void interp_idle_slot() {this->setEnabled(STATUS->machine_is_on() && STATUS->is_all_homed_());}
    void interp_run_slot() {this->setEnabled(false);}
    void convert_system(int data);
    void metricMode(bool state);

    void edit_slot(const QModelIndex &index);
};

#endif // ORIGIN_OFFSETVIEW_H
