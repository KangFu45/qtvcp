#include "origin_offsetview.h"

#include <QHeaderView>
#include <QFile>
#include <QDebug>

#include <fstream>

int MyTableModel::rowCount(const QModelIndex &parent) const
{
    return 13;
}

int MyTableModel::columnCount(const QModelIndex &parent) const
{
    return 10;
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::EditRole)
        return this->arraydata[index.row()][index.column()];
    if(role == Qt::DisplayRole){
        if(index.column() == 9)
            return QVariant(this->namedata[index.row()].c_str());
        return QVariant(this->arraydata[index.row()][index.column()]);
    }
    return QVariant();
}

Qt::ItemFlags MyTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    if(index.column() == 9 && (index.row() >= 0 || index.row() <= 3))
        return Qt::ItemIsEnabled;
    else if(index.row() == 1 && !(index.column() == 2))
        return Qt::NoItemFlags;
    else
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool MyTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || value.isNull())
        return false;

    if(index.column() == 9)
        this->namedata[index.row()] = value.toString().toStdString();
    else
        this->arraydata[index.row()][index.column()] = value.toFloat();
    emit dataChanged(index, index);
    return true;
}

QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(this->headerdata[section].c_str());
    if(orientation != Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(this->vheaderdata[section].c_str());
    return QVariant();
}

void MyTableModel::sort(int column, Qt::SortOrder order)
{

}


//-----------------------OriginOffsetView-------------------------

OriginOffsetView::OriginOffsetView(QWidget *parent)
    : QTableView(parent)
{
    this->axisletters = new char[9]{'x', 'y', 'z', 'a', 'b', 'c', 'y', 'v', 'w'};
    this->setEnabled(false);
    this->createTable();
}

void OriginOffsetView::hal_init()
{
    this->filename = INFO->PARAMETER_FILE.c_str();
    this->metric_display = STATUS->is_metric_mode();

    connect(STATUS, &_GStat::all_homed, this, &OriginOffsetView::all_homed_slot);
    connect(STATUS, &_GStat::interp_idle, this, &OriginOffsetView::interp_idle_slot);
    connect(STATUS, &_GStat::interp_run, this, &OriginOffsetView::interp_run_slot);
    connect(STATUS, &_GStat::periodic, this, &OriginOffsetView::periodic_check);
    connect(STATUS, &_GStat::metric_mode_changed, this, &OriginOffsetView::metricMode);
    connect(STATUS, &_GStat::user_system_changed, this, &OriginOffsetView::convert_system);
    for(int i=3; i<9; ++i)
        this->hideColumn(i);
}

void OriginOffsetView::createTable()
{
    this->tabledata = new float* [13];
    this->tabledata[0] = new float[9]{0,0,1,0,0,0,0,0,0};
    this->tabledata[1] = new float[9]{0,0,2,0,0,0,0,0,0};
    this->tabledata[2] = new float[9]{0,0,3,0,0,0,0,0,0};
    this->tabledata[3] = new float[9]{0,0,0,0,0,0,0,0,0};
    this->tabledata[4] = new float[9]{0,0,4,0,0,0,0,0,0};
    this->tabledata[5] = new float[9]{0,0,5,0,0,0,0,0,0};
    this->tabledata[6] = new float[9]{0,0,6,0,0,0,0,0,0};
    this->tabledata[7] = new float[9]{0,0,7,0,0,0,0,0,0};
    this->tabledata[8] = new float[9]{0,0,8,0,0,0,0,0,0};
    this->tabledata[9] = new float[9]{0,0,9,0,0,0,0,0,0};
    this->tabledata[10] = new float[9]{0,0,10,0,0,0,0,0,0};
    this->tabledata[11] = new float[9]{0,0,11,0,0,0,0,0,0};
    this->tabledata[12] = new float[9]{0,0,12,0,0,0,0,0,0};

    this->setSelectionMode(QAbstractItemView::SingleSelection);

    strings header = {"X", "Y", "Z", "A", "B", "C", "U", "V", "W", "Name"};
    strings vheader = {"ABS", "Rot", "G92", "Tool", "G54", "G55", "G56", "G57", "G58", "G59", "G59.1", "G59.2", "G59.3"};
    strings name = {"Absolute Position"
                   ,"Rotational Offsets"
                   ,"G92 Offsets"
                   ,"Current Tool"
                   ,"System 1"
                   ,"System 2"
                   ,"System 3"
                   ,"System 4"
                   ,"System 5"
                   ,"System 6"
                   ,"System 7"
                   ,"System 8"
                   ,"System 9"};

    m_tablemodel = new MyTableModel(this->tabledata, header, vheader, name, this);
    this->setModel(this->m_tablemodel);
    connect(this, &OriginOffsetView::clicked, this, &OriginOffsetView::showSelection);

    connect(this, &OriginOffsetView::doubleClicked, this, &OriginOffsetView::edit_slot);

    this->setMinimumSize(100,100);

    QHeaderView * hh = this->horizontalHeader();
    hh->setStretchLastSection(true);
    hh->setMinimumSectionSize(110);

    QHeaderView * vv = this->verticalHeader();
    vv->setMinimumSectionSize(25);

    this->resizeColumnsToContents();
    this->resizeRowsToContents();
    this->setSortingEnabled(false);
}

void OriginOffsetView::showSelection(const QModelIndex &index)
{

}

void OriginOffsetView::periodic_check()
{
    if(this->delay < 9){
        this->delay += 1;
        return;
    }
    else
        this->delay = 0;

    string res = this->ACTION->error_channel->Error_poll();
    if(!res.empty())
        qDebug()<<res.c_str();

    if(!this->filename.isEmpty())
        this->reload_offsets();
}

void OriginOffsetView::reload_offsets()
{
    float** g5x = this->read_file();

    if(!STATUS->is_status_valid())
        this->current_system = "G54";

    double* ap = STATUS->stat->actual_position();
    double* tool = STATUS->stat->tool_offset();
    double* g92 = STATUS->stat->g92_offset();
    double rot = STATUS->stat->rotation_xy();

    if(this->metric_display != INFO->MACHINE_IS_METRIC){
        ap = INFO->convert_units_9(ap);
        tool = INFO->convert_units_9(tool);
        g92 = INFO->convert_units_9(g92);
        //g5x[0] = INFO->convert_units_9(g5x[0]);
        //g5x[1] = INFO->convert_units_9(g5x[1]);
        //g5x[2] = INFO->convert_units_9(g5x[2]);
        //g5x[3] = INFO->convert_units_9(g5x[3]);
        //g5x[4] = INFO->convert_units_9(g5x[4]);
        //g5x[5] = INFO->convert_units_9(g5x[5]);
        //g5x[6] = INFO->convert_units_9(g5x[6]);
        //g5x[7] = INFO->convert_units_9(g5x[7]);
        //g5x[8] = INFO->convert_units_9(g5x[8]);
    }

    double t;
    for(int i=0; i<13; ++i){
        for(int j=0; j<9; ++j){
            if(i==0)
                t = ap[j];
            else if(i==1){
                if(j==2)
                    t=rot;
                else
                    t=0.0;
            }
            else if(i==2)
                t=g92[j];
            else if(i==3)
                t=tool[j];
            else
                t=g5x[i-4][j];
            this->tabledata[i][j] = t;
        }
    }

    emit this->m_tablemodel->layoutChanged();
}

float** OriginOffsetView::read_file() // 9*9
{
    float** g5x = new float* [9];
    g5x[0] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[1] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[2] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[3] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[4] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[5] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[6] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[7] = new float[9]{0,0,0,0,0,0,0,0,0};
    g5x[8] = new float[9]{0,0,0,0,0,0,0,0,0};

    if(this->filename.isEmpty())
        return g5x;
    if(!QFile(this->filename).exists())
        return g5x;

    ifstream fin(this->filename.toStdString(),ios_base::in);
    if(!fin.is_open())
        return g5x;

    string line;
    while (getline(fin, line)) {
       QStringList line1 = QString(line.c_str()).split("\t");
       int param = line1.first().toInt();
       float data = (*(line1.begin()+1)).toFloat();

       if(param >= 5221 && param <= 5229)
           g5x[0][param - 5221] = data;
       else if(param >= 5241 && param <= 5249)
           g5x[1][param - 5241] = data;
       else if(param >= 5261 && param <= 5269)
           g5x[2][param - 5261] = data;
       else if(param >= 5281 && param <= 5289)
           g5x[3][param - 5281] = data;
       else if(param >= 5301 && param <= 5309)
           g5x[4][param - 5301] = data;
       else if(param >= 5321 && param <= 5329)
           g5x[5][param - 5321] = data;
       else if(param >= 5341 && param <= 5349)
           g5x[6][param - 5341] = data;
       else if(param >= 5361 && param <= 5369)
           g5x[7][param - 5361] = data;
       else if(param >= 5381 && param <= 5389)
           g5x[8][param - 5381] = data;
    }
    fin.close();
    return g5x;
}

void OriginOffsetView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    int row = bottomRight.row();
    int col = bottomRight.column();
    float data= this->tabledata[row][col];

    emit this->hidekeyBoard();

    if((row == 1 && col!=2) || row == 0 )
        return;

    if(STATUS->is_status_valid()){
        ACTION->RECORD_CURRENT_MODE();
        if(row == 0){
            //qDebug()<<QString("G10 L2 P0 %1%2").arg(this->axisletters[col]).arg(data).toStdString().c_str();
            ACTION->CALL_MDI(QString("G10 L2 P0 %1%2").arg(this->axisletters[col]).arg(data).toStdString());
        }
        else if(row == 1){
            if(col == 2){
                //qDebug()<<QString("G10 L2 P0 R%1").arg(data).toStdString().c_str();
                ACTION->CALL_MDI(QString("G10 L2 P0 R%1").arg(data).toStdString());
            }
        }
        else if(row == 2){
            //qDebug()<<QString("G92 %1%2").arg(this->axisletters[col]).arg(data).toStdString().c_str();
            ACTION->CALL_MDI(QString("G92 %1%2").arg(this->axisletters[col]).arg(data).toStdString());
        }
        else if(row == 3){
            if(this->current_tool != 0){
                //qDebug()<<QString("G10 L1 P%1 %2%3").arg(this->current_tool).arg(this->axisletters[col]).arg(data).toStdString().c_str();
                ACTION->CALL_MDI(QString("G10 L1 P%1 %2%3").arg(this->current_tool).arg(this->axisletters[col]).arg(data).toStdString());
                ACTION->CALL_MDI("g43");
            }
        }
        else{
            //qDebug()<<QString("G10 L2 P%1 %2%3").arg(row-3).arg(this->axisletters[col]).arg(data).toStdString().c_str();
            ACTION->CALL_MDI(QString("G10 L2 P%1 %2%3").arg(row-3).arg(this->axisletters[col]).arg(data).toStdString());
        }
        ACTION->UPDATE_VAR_FILE();
        ACTION->RESTORE_RECORDED_MODE();
        this->reload_offsets();
    }
}

void OriginOffsetView::convert_system(int data)
{
    strings convert = {"None", "G54", "G55", "G56", "G57", "G58", "G59", "G59.1", "G59.2", "G59.3"};
    this->current_system = convert[data].c_str();
}

void OriginOffsetView::metricMode(bool state)
{
    this->metric_display = state;
}

void OriginOffsetView::edit_slot(const QModelIndex &index)
{
    if(index.flags() & Qt::ItemIsEditable == Qt::ItemIsEditable){
        emit this->needKeyBoard();
    }
}
