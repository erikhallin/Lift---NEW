#ifndef MAIN_SHIP_H
#define MAIN_SHIP_H

#include <iostream>
#include <gl/gl.h>
#include <Box2D/Box2D.h>
#include "particle_engine.h"
#include "MyRayCastCallback.h"
#include "definitions.h"

enum land_phases
{
    lp_init=0,
    lp_fall,
    lp_motor_warm_up,
    lp_stop_fall,
    lp_decline_slow,
    lp_full_stop,
    lp_on_ground,
    lp_complete
};

enum takeoff_phases
{
    tp_init=0,
    tp_to_target_height,
    tp_gear_up,
    tp_blastoff,
    tp_blastoff_fadeout,
    tp_complete
};

class main_ship
{
    public:
        main_ship();

        bool  init(b2World* world_ptr,
                   particle_engine* pPart_eng,
                   float gravity,
                   int texture,
                   float* pMship_led_prog,
                   bool reinit=false,
                   b2Vec2 pos=b2Vec2(0,0));
        int   update(float time_dif);
        bool  draw(void);
        bool  landing_gear_motor_left_lock(bool flag);
        bool  landing_gear_motor_right_lock(bool flag);
        bool  begin_takeoff(void);
        float get_resources(void);
        bool  change_resources(float value);//returns true if amount doesnt go below zero
        float get_fuel(void);
        bool  set_fuel(float new_value);
        bool  change_fuel(float value);
        bool  reset_to_land_pos(void);
        bool  is_landing(void);
        bool  is_takeoff(void);
        int   get_takeoff_phase(void);
        bool  set_landing_gear_motor_speed_left(float motor_speed);
        bool  set_landing_gear_motor_speed_right(float motor_speed);
        bool  set_landing_gear_sensor_flags(bool* flag_landing_gear);
        int   get_auto_pilot(void);
        bool  set_auto_pilot(int player_in_control);
        bool  set_all_players_on_ship(bool flag);
        bool  player_inside(int player_ind,bool inside);
        float get_motor_thrust(void);
        float get_gear_motor_speed(void);
        bool  restock(void);

        b2Body*       get_body_ptr(void);
        b2Fixture*    get_sensor_input_ptr(void);
        b2Fixture*    get_sensor_landing_ptr(void);
        b2Fixture*    get_sensor_landing_gear_left_ptr(void);
        b2Fixture*    get_sensor_landing_gear_right_ptr(void);

        float  m_resources_curr,m_resources_max,m_resources_drawn;
        float  m_fuel_tank_max,m_fuel_tank_curr,m_fuel_tank_drawn;
        float  m_sound_col_timer;


    private:

        float  m_world_gravity_curr;
        float  m_start_pos[2];
        bool   m_player_inside[4];

        //land/takeoff
        bool   m_in_landing_phase,m_in_takeoff_phase,m_landing_gear_delay_done;
        bool   m_landing_gear_left_lock_on,m_landing_gear_right_lock_on,m_auto_land;
        bool*  m_pLanding_gear_touch;
        bool   m_all_players_on_ship;//takeoff possible if true
        int    m_land_phase,m_takeoff_phase,m_player_in_control;
        float  m_landing_dist_max,m_landing_pos_curr,m_landing_start_height_pos,m_landing_fall_speed,m_landing_last_height_pos;
        float  m_motor_thrust_curr,m_landing_target_fall_speed,m_landing_thrust_min;
        float  m_landing_target_height_stop,m_landing_target_height_start_motor,m_landing_target_height_controlled_decline;
        float  m_landing_timer,m_landing_time_limit,m_takeoff_timer,m_takeoff_time_limit;
        float  m_landing_gear_delay_timer,m_landing_gear_delay_limit;
        float  m_balance_extra_thrust_factor,m_tilt_factor_last,m_takeoff_thrust_adjust;
        float  m_landing_dist_to_ground;//,m_landing_extra_stop_force;
        float  m_landing_time,m_takeoff_height_pos,m_takeoff_height_above_ground;
        int    m_texture;

        //lamps
        bool   m_lamp_landing_timer_inc;
        float  m_lamp_landing_timer,m_lamp_landing_time;
        float* m_pMship_led_prog;

        b2World*          m_pWorld;
        particle_engine*  m_pParticle_engine;
        b2Body*           m_pBody;
        b2Fixture*        m_pInput_sensor;
        b2Fixture*        m_pLand_sensor;
        b2Body*           m_pLanding_gear_left;
        b2Body*           m_pLanding_gear_right;
        b2Fixture*        m_pLanding_gear_sensor_left;
        b2Fixture*        m_pLanding_gear_sensor_right;
        b2PrismaticJoint* m_pLanding_motor_join_left;
        b2PrismaticJoint* m_pLanding_motor_join_right;
        b2WeldJoint*      m_pLanding_gear_left_lock;
        b2WeldJoint*      m_pLanding_gear_right_lock;
        bool              m_landstrip_motor_in_use[4];
        b2PrismaticJoint* m_pLandstrip_motor[4];//one per player

};

#endif // MAIN_SHIP_H
