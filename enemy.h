#ifndef ENEMY_H
#define ENEMY_H

#include <iostream>
#include <vector>
#include <string>
#include <gl/gl.h>
#include <Box2D/Box2D.h>
#include "MyRayCastCallback.h"
#include "MyQueryCallback.h"
#include "particle_engine.h"
#include "weapon.h"
#include "gear.h"
#include "sound.h"
#include "definitions.h"

using namespace std;

enum enemy_types
{
    et_init=0,
    et_default,

    et_burst_bot,       //w burst
    et_auto_flat,       //w auto
    et_flipper,         //w spread
    et_rocket_tank,     //w rocket
    et_grenade_ship,    //w grenade
    et_cannon_tank,     //w cannon
    et_miner,           //w mine
    et_beamer,          //w beam

    et_cloaker,         //i cloak               (push to activate/deactivate)
    et_scanner,         //i gyro                (push to activate/deactivate)
    et_lifter,          //i boost               (push to activate/deactivate, use when thuster at maximum)
    et_stand_turret,    //i rot turret inc/dec  (push to activate/deactivate, returns turret angel to straight ahead)
    et_flying_turret,   //i rot turret dir      (push to activate/deactivate)
    et_aim_bot,         //i auto aim            (push to activate/deactivate)
    et_rammer           //i shield              (push to activate/deactivate)
};

enum plan_phases
{
    pp_idle=0,
    pp_attack,
    pp_run_away,
    pp_lift,
    pp_go_to,
    pp_patrol
};

class enemy
{
    public:
        enemy();

        bool     init(b2World* pWorld,sound* pSound,particle_engine* pPart_eng,b2Vec2 pos,int type,int texture);
        int      update(float time_dif,float view_pos[4]);//0 dead, 1 alive, 2 convoy ship reached target
        bool     draw(void);
        b2Body*  get_body_ptr(void);
        bool     set_target_pos(b2Vec2 target_pos);
        bool     change_hp(float value_dif);
        bool     set_current_weapon(weapon* pCurr_weapon);
        bool     fire_turret(int sound_box,int weapon_id=0,float time_dif=0.0);
        weapon*  get_weapon_ptr(void);
        gear*    get_gear_ptr(void);
        bool     hook_status(void);
        bool     hook_disconnect(void);
        bool     force_hook(b2Body* object_ptr,b2Vec2 shift_pos);
        b2Body*  get_connected_body(void);
        bool     update_player_bodies_ptr(void);
        bool     set_convoy_pos(b2Vec2 end_pos);
        bool     delete_equipment(void);
        bool     scanner_alarm(b2Vec2 scanner_pos,b2Body* target_body);

        float    m_sound_col_timer;

    private:

        int              m_type,m_plan_phase;
        float            m_size,m_hp_max,m_hp_curr;
        b2World*         m_pWorld;
        b2Body*          m_pBody;
        b2Body*          m_pBody_connected_to;
        particle_engine* m_pParticle_engine;
        sound*           m_pSound;
        vector<b2Body*>  m_vec_pPlayer_bodies;
        b2Body*          m_pTarget_body;
        weapon*          m_pWeapon_curr;
        gear*            m_pGear_curr;
        b2RopeJoint*     m_hook_joint;
        b2Vec2           m_hooked_pos;
        vector<b2Vec2>   m_vec_checkpoints;
        b2Vec2           m_vBeam_start,m_vBeam_end;
        float            m_sound_timer;

        b2Vec2  m_ship_target_pos;
        float   m_tilt_limit_max,m_tilt_limit_ok;
        float   m_height_limit_max,m_height_limit_ok,m_height_above_target;
        float   m_offside_limit_max,m_offside_limit_ok;
        float   m_ai_think_timer,m_ai_think_delay,m_ai_think_time_default;
        float   m_ai_sight_range,m_ai_detection_range;//will detect players within detect_range and follow to out of sight_range
        float   m_height_above_ground,m_height_above_ground_min;
        float   m_ai_aim_angle_tolerance,m_fire_ship_to_barrel_dist;
        float   m_measure_ground_dist_timer,m_measure_ground_dist_delay,m_lift_target_movement_sens;
        bool    m_above_target_flag,m_target_hooked,m_body_flipped,m_flip_target_left,m_target_on_right_side,m_target_swap_side;
        bool    m_draw_beam,m_play_beam_sound,m_is_dead,m_cloak_target_off,m_patrol_right,m_shield_on,m_shield_target_on;
        float   m_hook_distance,m_thurst_regulator,m_thrust_smooth,m_counter_rotation_limit,m_speed_limit_min;
        int     m_movement_phase,m_patrol_bad_pos_counter,m_patrol_bad_pos_max;
        float   m_ai_flee_step_length,m_ai_low_hp_flee_limit,m_ai_flee_think_factor;
        float   m_turret_angle,m_turret_angle_min,m_turret_angle_max,m_turret_turn_speed;
        float   m_tilt_adjust_delay,m_tilt_adjust_timer,m_turret_flip_timer,m_turret_flip_delay;
        float   m_fire_range_min,m_ram_reset_distance,m_cloak_timer,m_cloak_delay;
        float   m_pref_height_above_ground,m_pref_height_above_ground_tol,m_patrol_next_time,m_patrol_timer;
        float   m_pure_idle_timer,m_pure_idle_delay,m_shield_timer,m_shield_delay;
        float   m_speed_limit_up,m_led_timer,m_led_time,m_led_glow;
        bool    m_led_flip,m_led_flip2;

        int     m_texture;

        bool    m_convoy_mode;
        b2Vec2  m_convoy_end_pos;
};

#endif // ENEMY_H
