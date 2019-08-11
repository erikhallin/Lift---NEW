#ifndef PLAYER_H
#define PLAYER_H

#include <iostream>//temp
#include <vector>
#include <Box2D/Box2D.h>
#include <gl/gl.h>
#include "MyRayCastCallback.h"
#include "definitions.h"
#include "particle_engine.h"
#include "weapon.h"
#include "gear.h"
#include "sound.h"

using namespace std;

class player
{
    public:
        player();

        bool       init(b2World* world_ptr,b2Body* mship_ptr,particle_engine* pPart_eng,sound* pSound,int texture,
                        int player_id,bool reinit=false,b2Vec2 pos=b2Vec2(0,0));
        b2Body*    get_body_ptr(void);
        b2Fixture* get_rope_hook_sensor(void);
        b2Body*    get_connected_body_ptr(void);
        bool       set_focus_point(float _x,float _y);//Where user wants to set camera focus
        bool       get_focus_point(float focus_point[2]);
        bool       set_rope_motor(float motor_speed);
        bool       shift_player_pos(float x,float y,bool absolute_pos=false);//returns hook status
        bool       update(float time_dif,float view_pos[4]);
        bool       draw(void);
        bool       player_hook_connected(void);
        bool       hook_connect(b2Body* pBody_to_connect);
        bool       hook_disconnect(void);
        bool       is_hook_off(void);
        bool       connected_to_mship(void);
        bool       is_spawning(void);
        bool       disconnect_from_mship(void);
        bool       change_ship_mass(float mass_change,bool reset_mass=false);
        bool       reset_motion(void);

        float      get_rel_hp(void);
        float      get_rel_fuel(void);
        float      get_rel_ammo(void);
        bool       change_hp(float value_dif);
        bool       change_fuel(float value_dif);
        bool       change_ammo(float value_dif);

        int        dock_player_to_mship(void);
        int        get_mship_dock_status(void);//0-not docked, 1-in docking progress, 2-docked
        bool       change_turret_rotation(float change);//in deg
        float      get_turret_angle(void);//in deg
        bool       set_turret_angle(float angle);//in deg
        bool       destroy_body_and_joints(void);
        //bool       move_and_reset_player(void);
        bool       spawn_from_mship(void);
        bool       raise_from_mship(void);
        bool       get_rope_lock_status(void);
        bool       lifting_a_player_ship(void);
        bool       erase_lost_ropes(void);
        int        get_rope_length(void);
        bool       set_pos(b2Vec2 pos);

        //weapon
        bool       fire_turret(float time_dif,bool ammo_test=true);
        bool       set_current_weapon(weapon* pCurr_weapon);
        bool       use_default_weapon(void);
        weapon*    get_weapon_ptr(void);
        weapon*    get_default_weapon_ptr(void);
        int        get_weapon_index(void);
        bool       set_weapon_index(int value);
        //gear
        bool       set_current_gear(gear* pCurr_gear);
        bool       use_default_gear(void);
        gear*      get_gear_ptr(void);
        gear*      get_default_gear_ptr(void);
        int        get_gear_index(void);
        bool       set_gear_index(int value);

        //drone
        int     get_drone_mode(void);
        int     set_drone_mode(int mode,bool mode_recall=false);
        b2Body* get_player_drone_body_ptr(void);
        bool    set_player_drone_body_ptr(b2Body* body_ptr);

        //line
        bool    m_line_type_tow;//false=fuel line
        int     m_fuel_line_to_player_ind;

        //weapon and gear values
        bool       m_key_trigger_weapon_swap,m_using_default_weapon,m_key_trigger_gear_swap,m_using_default_gear;
        bool       m_key_trigger_use_gear,m_cloak_target_off,m_turret_aim_on,m_gyro_on,m_shield_broken;
        float      m_cloak_timer,m_cloak_delay,m_boost_timer,m_boost_delay,m_boost_multiplyer;
        float      m_turret_rotation_speed_slow,m_turret_rotation_speed_fast,m_turret_range;
        float      m_gyro_tilt_limit,m_gyro_fall_speed_limit;
        float      m_shield_regen_timer,m_shield_regen_delay,m_shield_hp_curr,m_shield_hp_max,m_shield_regen_speed;

        //misc
        float      m_motor_thrust_power_max,m_hp_curr,m_hp_max,m_fuel_curr,m_fuel_max,m_ammo_curr,m_ammo_max;
        float      m_key_hold_time_back,m_key_hold_time_start;
        bool       m_key_trigger_dpad,m_key_trigger_LB,m_key_trigger_RB,m_key_trigger_start,m_key_trigger_back,m_skip_tutorial_flag;
        bool       m_key_trigger_thumbstick_left,m_key_trigger_thumbstick_right,m_manual_flag,m_takeoff_flag;
        bool       m_key_trigger_a,m_key_trigger_b;
        bool       m_key_trigger_line_swap;
        float      m_drone_spawn_time,m_drone_spawn_timer;
        float      m_outside_map_timer;
        bool       m_force_led_on;
        float      m_sound_col_timer;
        int        m_upgrade_counter;
        b2Vec2     m_body_pos_old;
        float      m_stuck_timer;


    private:

        int                m_id;
        b2Body*            m_pBody;
        b2Body*            m_pMain_ship_body;
        b2World*           m_pWorld;
        b2Body*            m_pHook_body;
        b2Fixture*         m_pHook_sensor;
        b2Body*            m_pBody_connected_to;
        b2WeldJoint*       m_pHook_joint;
        b2WeldJoint*       m_pMship_lock_joint;
        b2RopeJoint*       m_pRope_lock_joint;
        b2PrismaticJoint*  m_pLand_adjust_joint;
        b2PrismaticJoint*  m_pLand_raise_joint;
        //b2RevoluteJoint*   m_turret_joint;
        int                m_carrier_player_id;
        particle_engine*   m_pParticle_engine;
        b2Vec2             m_vBeam_start,m_vBeam_end;

        weapon*            m_pWeapon_curr;
        weapon*            m_pWeapon_default;
        gear*              m_pGear_curr;
        gear*              m_pGear_default;
        int                m_weapon_index,m_gear_index;

        int                m_texture;

        bool   m_hook_connected,m_rope_lock_on,m_mship_lock_on,m_mship_land_adjusting,m_hook_off,m_draw_beam,m_is_spawning;

        float  m_turret_rotation,m_ship_mass_factor,m_dock_adjust_timer,m_barrel_length,m_angle_time,m_led_glow;

        sound* m_pSound;

        //drone
        int     m_drone_mode;
        b2Body* m_pBody_drone;

        //rope
        b2PrismaticJoint*         m_rope_motor_joint;
        vector<b2Body*>           m_vec_rope_bodies;//last element in vector is closest to ship
        vector<b2RopeJoint*>      m_vec_rope_joints;//last element in vector is closest to ship
        //vector<b2RevoluteJoint*>      m_vec_rope_joints;//last element in vector is closest to ship
        //vector<b2DistanceJoint*>  m_vec_rope_joints;//last element in vector is closest to ship
        vector< vector<b2Body*> > m_vec_vec_disconnected_ropes;

        float m_rel_focus_point[2];
};

#endif // PLAYER_H
