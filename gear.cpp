#include "gear.h"

gear::gear()
{
    //ctor
}

gear::gear(b2World* world_ptr,int type,float variation_tolerance)
{
    m_pWorld=world_ptr;
    m_type=type;

    //give random color temp
    m_gear_color[0]=float(rand()%500)/1000.0+0.5;
    m_gear_color[1]=float(rand()%500)/1000.0+0.5;
    m_gear_color[2]=float(rand()%500)/1000.0+0.5;

    //shield data
    m_shield_regen_delay=5.0;
    m_shield_hp_max=30.0;
    m_shield_regen_speed=0.2;

    //random variance
    m_shield_regen_delay+=m_shield_regen_delay*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
    m_shield_hp_max+=m_shield_hp_max*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
    m_shield_regen_speed+=m_shield_regen_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
}

int gear::get_type(void)
{
    return m_type;
}

bool gear::set_new_world(b2World* new_world_prt)
{
    m_pWorld=new_world_prt;

    return true;
}

bool gear::get_shield_data(float shield_data[3])
{
    shield_data[0]=m_shield_regen_delay;
    shield_data[1]=m_shield_hp_max;
    shield_data[2]=m_shield_regen_speed;

    return (m_type==gt_shield);
}
