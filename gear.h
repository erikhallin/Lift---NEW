#ifndef GEAR_H
#define GEAR_H

#include <iostream>
#include <ctime>
#include <vector>
#include <Box2D/Box2D.h>
#include "definitions.h"

enum gear_types
{
    gt_unarmed=0,
    gt_cloak,
    gt_gyro,
    gt_boost,
    gt_turret_rotation,
    gt_turret_aim,
    gt_turret_auto_aim,
    gt_shield,
    gt_cam_control
};

class gear
{
    public:
        gear();
        gear(b2World* world_ptr,int type,float variation_tolerance);

        int   get_type(void);
        bool  set_new_world(b2World* new_world_prt);
        bool  get_shield_data(float shield_data[3]);

        float m_gear_color[3];//temp
        float m_shield_regen_delay,m_shield_hp_max,m_shield_regen_speed;

    private:

        int       m_type;
        b2World*  m_pWorld;

};

#endif // GEAR_H
