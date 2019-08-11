#ifndef DEF_H
#define DEF_H

#include <sstream>
#include <string>
#include <Box2D/Box2D.h>

#define _Deg2Rad 0.0174532925
#define _Rad2Deg 57.2957795
#define _Met2Pix 20.0
#define _Pix2Met 1.0/20.0
#define _max_particles 10000

using namespace std;

const uint16 _COLCAT_mship=0x0001;
const uint16 _COLCAT_player1=0x0002;
const uint16 _COLCAT_player2=0x0004;
const uint16 _COLCAT_player3=0x0008;
const uint16 _COLCAT_player4=0x0016;
const uint16 _COLCAT_all=0x0032;

const float _version=1.2;
const float _pi=3.14159265359;

const float _tutorial_test_time=2.0;
const float _hud_item_draw_time=1.0;

const float _world_step_time=0.010;//60 FPS
const float _world_velosity_it=200;
const float _world_position_it=200;
const float _world_gravity=9.0;
const int   _world_numof_levels=33;
const float _world_resource_at_enemy_pos_chance=0.2;

const float _training_ring_time=3.0;
const float _training_ring_radius_big=60.0;
const float _training_ring_radius_small=30.0;

const float _screen_shift_thres_min_x=0.05;
const float _screen_shift_thres_min_y=0.10;
const float _screen_shift_thres_max=0.5;//will move cam so that player is inside screen
const float _screen_cam_speed_up=2000.00;
const float _screen_cam_speed_down=0.0001;
const float _screen_cam_player_focus=0.3;

const float _player_damping_lin=1.5;
const float _player_damping_ang=10.0;
const float _player_drone_damping_lin=4.0;
const float _player_drone_damping_ang=3.0;
const float _player_density=1.0;
const float _player_turret_density=0.01;
const float _player_fuel_consumption_factor=0.001;
const float _player_refill_factor_hp=30.0;
const float _player_refill_factor_fuel=30.0;
const float _player_refill_factor_ammo=30.0;
const float _player_ship_to_barrel_dist=0.7;
const float _player_upgrade_cost=10.0;
const float _player_mship_dock_time=1.0;//time before giving up docking
const float _player_mship_control_key_timer=1.0;//hold back for 1 sec to toggle mship control
const int   _player_rope_length_max=10;
const float _player_outside_map_time=10.0;
const float _player_extra_friendly_lift_force=1.2;
const float _player_ship_upgrade_mass_shift=0.05;
const float _player_stuck_time=5;

const float _mship_damping_lin=2.0;
const float _mship_damping_ang=3.0;
const float _mship_density=1.0;
const float _mship_fuel_start=200.0;//200
const float _mship_fuel_max=500.0;//500
const float _mship_resources_start=9.0;//200
const float _mship_resources_max=100.0;//200
const float _mship_scan_range=500.0;//500
const float _mship_travel_speed=0.02;//0.02
const float _mship_travel_pack_up_time_max=1.0;
const float _mship_landing_timeout=50.0;
const float _mship_takeoff_hight_limit_dif=10.0;

const float _object_damping_lin=1.0;
const float _object_damping_ang=0.2;
const float _object_density=0.10;

const float _fuel_density=0.001;
const float _fuel_transfer_speed=0.03;

const float _enemy_damping_lin=1.0;
const float _enemy_damping_ang=1.0;
const float _enemy_density=0.10;
const float _enemy_sound_max_time=1.0;

const float _rope_motor_strength=300.0;
const float _rope_density=0.1;//if rope desity is to low, the joints will break, increase numof step iterations
const float _rope_upper_trans_limit=0.5;
const float _rope_lower_trans_limit=0.15;
const float _rope_part_width=0.05;
const float _rope_part_length=0.2;
const float _rope_max_length=0.1;
const float _rope_motor_max_angle=0.01;
const float _rope_force_limit=30.0;//15
const float _rope_force_limit_fuel=10.0;//15

const float _weapon_genade_damping_lin=0.20;
const float _weapon_genade_damping_ang=1.00;
const float _weapon_mine_damping_lin=1.00;
const float _weapon_mine_damping_ang=1.00;

const float _collision_damage_limit=0.1;

const float _thumbstick_deadzone=10000.0;
const float _thumbstick_deadzone_low=1000.0;

const float _starmap_cam_speed_limit=0.01;
const float _starmap_zoom_min=-0.95;
const float _starmap_zoom_max=3.0;
const float _starmap_zoom_max2=1.0;
const float _starmap_radius_world=20000.0;
const float _starmap_radius_start_planet=8000.0;
const float _starmap_radius_tol_start_planet=1000.0;
const int   _starmap_numof_level_colors=15;
const int   _starmap_numof_level_textures=5;

const float _sound_box_side_rel_dist=0.2;
const float _sound_box_side_shift=2.0;
const float _sound_box_level_outside=0.4;
const float _sound_col_min_level=1.0;
const float _sound_player_col_max_time=0.3;
const int   _sound_chan_motor_mship=10;
const int   _sound_chan_motor_p1=11;
const int   _sound_chan_motor_p2=12;
const int   _sound_chan_motor_p3=13;
const int   _sound_chan_motor_p4=14;
const int   _sound_chan_motor_rope=15;
const int   _sound_chan_motor_turret=16;
const int   _sound_chan_laser_p1=17;
const int   _sound_chan_laser_p2=18;
const int   _sound_chan_laser_p3=19;
const int   _sound_chan_laser_p4=20;
const int   _sound_chan_laser_enemy=21;
const int   _sound_chan_boost_p1=22;
const int   _sound_chan_boost_p2=23;
const int   _sound_chan_boost_p3=24;
const int   _sound_chan_boost_p4=25;
const int   _sound_chan_starmap_travel=26;
const int   _sound_chan_gear_motor_mship=27;
const int   _sound_chan_alarm=28;
const int   _sound_chan_fuel_transfer=29;
const float _sound_alarm_fuel_hp_level=0.10;

enum object_types
{
    ot_fuel=0,
    ot_resource
};

enum directions
{
    dir_up=0,
    dir_right,
    dir_down,
    dir_left
};

enum tutorial_texts
{
    tut_land=0,
    tut_manual,
    tut_fuel,
    tut_enemy,
    tut_return,
    tut_takeoff
};

enum game_states
{
    gs_init=0,
    gs_menu,
    gs_training,
    gs_level_select,
    gs_in_level,
    gs_game_over,
    gs_exit
};

enum training_states
{
    ts_ask=0,
    ts_reply,
    ts_rings_first,
    ts_rings_second
};

enum cam_modes
{
    cm_follow_one=0,
    cm_follow_all,
    cm_follow_other,
    cm_follow_none
};

enum convoy_steps
{
    cs_off=0,
    cs_return_players,
    cs_fade_out_screen,
    cs_dark_screen_delay,
    cs_fade_in_text,
    cs_show_text_delay,
    cs_fade_out_text,
    cs_fade_in_screen
};

enum info_screen_steps
{
    is_off=0,
    is_fade_in,
    is_wait,
    is_fade_out
};

enum help_texts
{
    ht_raise=0,//any button
    ht_eject,//B
    ht_trigger,
    ht_careful,
    ht_towline,//dpad
    ht_connect,//hook connect
    ht_salvage,//input event
    ht_fire,//A
    ht_obtained_gear,
    ht_dock,//dock
    ht_hud1,
    ht_hud2,
    ht_hud3,
    ht_select_gear,//non default gear select
    ht_use_gear,//x
    ht_dock_again,//dock
    ht_recycle_info,
    ht_recycle_now,//recycle gear
    ht_recycle_material,
    ht_ship_upgrade,//dpad
    ht_upgrade_info,
    ht_upgrade_heavier,
    ht_crash_warning,
    ht_drone_eject,
    ht_drone_takeover,
    ht_takeoff,//takeoff
    ht_hook_off,
    ht_fuel_container,
    ht_bring_back_ship,
    ht_map_center,
    ht_map_dots,
    ht_map_yellow_area,
    ht_map_bars,
    ht_map_travel,
    ht_map_land,
    ht_salvaged_ship,
    ht_map_move_cam
};

enum drone_modes
{
    dm_off=0,//ship intact
    dm_on,//drone control instead of ship
    dm_destroyed//no inputs
};

enum collision_types
{
    ct_unknown=0,
    ct_drone_player,
    ct_drone_inputbox
};

enum item_type
{
    it_weapon=0,
    it_gear,
    it_line
};

struct st_body_user_data
{
    st_body_user_data()
    {
        i_id=-1;
        i_subtype=-1;
        f_collision_damage_update=0.0;
        f_projectile_damage_update=0.0;
        f_time_left=1.0;
        b_is_carried=false;
        b_alive=true;
        b_cloaked=false;
        b_to_be_deleted=false;
        b_disconnected_part=false;
    }

    string  s_info;
    int     i_id;//or type
    int     i_subtype;
    float   f_collision_damage_update,f_projectile_damage_update;
    float   f_time_left;
    bool    b_is_carried;//will not be correct if same body is carried by 2 hooks
    b2Body* bp_carrier_body;//pointer to the body that carries this body (players or some enemies)
    void*   vp_this;
    bool    b_alive,b_cloaked,b_to_be_deleted,b_disconnected_part;
};

struct st_int_int_float
{
    st_int_int_float()
    {
        val_i1=0;
        val_i2=0;
        val_f=0.0;
    }
    st_int_int_float(int _i1,int _i2,float _f)
    {
        val_i1=_i1;
        val_i2=_i2;
        val_f=_f;
    }
    int val_i1,val_i2;
    float val_f;
};

struct st_int_float
{
    st_int_float()
    {
        val_i=0;
        val_f=0.0;
    }
    st_int_float(int _i,float _f)
    {
        val_i=_i;
        val_f=_f;
    }
    int val_i;
    float val_f;
};

struct st_float_float_int
{
    st_float_float_int()
    {
        val_f1=0.0;
        val_f2=0.0;
        val_i=0;
    }
    st_float_float_int(int _f1,float _f2,int _i=0)
    {
        val_f1=_f1;
        val_f2=_f2;
        val_i=_i;
    }
    int   val_i;
    float val_f1,val_f2;
};

struct st_collision_event
{
    st_collision_event()
    {
        collision_type=ct_unknown;
        index=-1;
    }
    st_collision_event(b2Body* _bodyA,int type)
    {
        bodyA=_bodyA;
        collision_type=type;
        index=-1;
    }
    st_collision_event(b2Body* _bodyA,b2Body* _bodyB,int type)
    {
        bodyA=_bodyA;
        bodyB=_bodyB;
        collision_type=type;
        index=-1;
    }

    b2Body* bodyA;//drone body
    b2Body* bodyB;//collided player body
    int     collision_type;
    int     index;
};

struct st_terrain_point
{
    st_terrain_point()
    {
        pos_x=pos_y=0.0;
    }
    st_terrain_point(float _x,float _y)
    {
        subpos_x=pos_x=_x;
        subpos_y=pos_y=_y;
    }

    float pos_x,pos_y;
    float subpos_x,subpos_y;
    float height_var;
};

inline string num_to_string(float val)
{
    ostringstream ostr;
    ostr<<val;
    return string( ostr.str() );
}

//for convex hull calculation
inline float cross(const b2Vec2 &p1, const b2Vec2 &p2, const b2Vec2 &p3)
{
	return (p2.x-p1.x)*(p3.y-p1.y)-(p2.y-p1.y)*(p3.x-p1.x);
}

#endif
