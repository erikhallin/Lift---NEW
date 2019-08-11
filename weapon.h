#ifndef WEAPON_H
#define WEAPON_H

#include <iostream>
#include <ctime>
#include <vector>
#include <Box2D/Box2D.h>
#include "MyRayCastCallback.h"
#include "definitions.h"

using namespace std;

enum weapon_types
{
    wt_unarmed=0,
    wt_pea,
    wt_spread,
    wt_burst,
    wt_rapid,
    wt_rocket,
    wt_grenade,
    wt_cannon,
    wt_beam,
    wt_mine
};

enum weapon_subtypes
{
    wst_normal=0,
    wst_impact,
    wst_timed,
    wst_second_impact,
    wst_second_timed//timer starts after first impact
};

class weapon
{
    public:
        weapon();
        weapon(b2World* world_ptr,int type,float variation_tolerance,int subtype=0);

        bool  is_ready_to_fire(void);
        int   fire_weapon(b2Vec2 pos,b2Vec2 fire_direction,float time_dif=0);
        int   get_type(void);
        bool  get_beam_pos(b2Vec2& start_pos,b2Vec2& end_pos);
        float get_ammo_cost(void);
        bool  set_new_world(b2World* new_world_prt);

        float m_weapon_color[3];//temp

        int       m_type,m_subtype,m_burst_counter,m_burst_counter_max;
        float     m_bullet_damage,m_bullet_speed;
        float     m_ammo_cost_per_shot,m_spread_factor,m_grenade_timer,m_range_max,m_beam_damage;

    private:

        //time
        float m_time_last_fired,m_time_when_ready_to_fire,m_fire_cooldown,m_fire_cooldown_part;


        bool      m_beam_pos_updated;
        b2Vec2    m_vBeam_pos_start,m_vBeam_pos_end;



        b2World*  m_pWorld;
};

#endif // WEAPON_H
