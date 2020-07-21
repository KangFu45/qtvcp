#include "hal_glib.h"

_GStat::_GStat (LinuxcncStat* stat)
{
    if(stat == nullptr)
        this->stat = new LinuxcncStat();
    else
        this->stat = stat;


}

void _GStat::merge()
{

}

void _GStat::update()
{

}

void _GStat::forced_update()
{

}
