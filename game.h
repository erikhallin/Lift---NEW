#ifndef GAME_H
#define GAME_H

#include <SOIL/SOIL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
#include <vector>
#include <stdlib.h>
#include <Box2D/Box2D.h>
#include <gl/gl.h>
#include "gamepad.h"
#include "definitions.h"
#include "player.h"
#include "object.h"
#include "MyContactListener.h"
#include "MyRayCastCallback.h"
#include "MyQueryCallback.h"
#include "particle_engine.h"
#include "main_ship.h"
#include "hud.h"
#include "enemy.h"
#include "star_map.h"
#include "sound.h"
#include "base64.h"
#include "files_in_text.h"
#include "level_data.h"

using namespace std;

class game
{
    public:
        game();

        bool pre_init(int screen_size[2]);//to show loading screen
        bool init(bool keys[256],int screen_size[2],bool test_level,bool load_sound_and_texture=true);
        bool update(bool& quit_flag);
        bool draw(void);
        bool reset(void);

    private:

        string  m_save_filename,m_save_key;
        bool    m_load_game,m_save_file_present;

        bool*   m_pKeys;
        bool    m_gamepad_connected[4];
        bool    m_player_active[4];
        bool    m_run_test_level,m_on_tutorial_level,m_waiting_for_convoy;
        gamepad m_gamepad[4];
        int     m_input_reroute[4];
        int     m_screen_width,m_screen_height;
        float   m_fuel_curr;
        float   m_waiting_for_convoy_screen_fade_level,m_waiting_for_convoy_text_fade_level;
        float   m_waiting_for_convoy_text_time,m_waiting_for_convoy_text_timer;
        float   m_waiting_for_convoy_screen_timer;
        bool    m_show_manual,m_show_info,m_show_lost,m_show_gameover,m_info_shown;
        float   m_screen_info_fade_level,m_lost_screen_fade_level,m_gameover_screen_fade_level;
        int     m_last_played_level,m_planets_visited_counter;
        int     m_convoy_phase,m_info_screen_phase,m_lost_screen_phase,m_gameover_screen_phase;
        float   m_fps_frame_count,m_fps_measure_interval,m_fps_measured_time;
        float   m_level_highest_point,m_tutorial_test_timer;
        float   m_sound_alarm_delay,m_sound_alarm_delay_timer;

        //triggers
        bool    m_key_trigger_keyboard,m_key_trigger_n,m_key_trigger_b,m_key_trigger_tab;
        bool    m_key_trigger_m,m_key_trigger_esc;

        star_map m_Starmap;
        b2World* m_pWorld;

        vector<int>      m_vec_played_levels;
        vector<b2Body*>  m_vec_terrain;
        vector<b2Body*>  m_vec_projectiles_to_remove;
        vector<player>   m_vec_players;
        vector<object>   m_vec_objects;
        vector<enemy*>   m_vec_pEnemies;
        vector<weapon*>  m_vec_pWeapon_stored;
        vector<gear*>    m_vec_pGear_stored;
        vector<st_int_int_float>    m_vec_stars;
        vector<st_collision_event>  m_vec_collision_events;
        vector< vector<st_terrain_point> > m_vec_vec_terrain_points;
        vector< vector<st_terrain_point> > m_vec_vec_cave_points;
        main_ship*       m_pMain_ship;
        hud              m_hud;
        vector<st_int_int_float> m_vec_training_rings_p1;
        vector<st_int_int_float> m_vec_training_rings_p2;
        vector<st_int_int_float> m_vec_training_rings_p3;
        vector<st_int_int_float> m_vec_training_rings_p4;
        st_int_int_float         m_arr_training_rings_hook[4];

        int    m_id_counter_object;

        int    m_game_state,m_training_state;
        int    m_came_mode,m_cam_player_to_follow,m_cam_enemy_to_follow;
        int    m_numof_levels;

        float  m_time_last_cycle,m_time_this_cycle;
        float  m_cam_pos[2],m_cam_speed[2];
        float  m_player_start_pos[2];
        float  m_world_max_x,m_world_offside_limit,m_world_gravity_curr;
        float  m_level_soft_borders[2],m_level_hard_borders[2],m_level_static_fade_distance;
        float  m_level_sky_height,m_level_bottom_height;
        bool   m_level_loaded,m_player_input_enabled,m_mship_landed,m_fade_off;
        int    m_master_player_controller;
        float  m_screen_fade_prog;
        float  m_mship_led_prog;
        bool   m_player_stuck[4];
        float  m_draw_stuck_timer;

        MyContactListener m_myContactListenerInstance;//for collision detection
        particle_engine*  m_pParticle_engine;

        int        m_pEvent_flag_hook[4];//one per player
        bool*      m_pEvent_flag_input_box;
        bool*      m_pEvent_flag_landing_gear;
        //bool     m_pRope_hook_sensor_flag[4];//one per player
        bool       m_pMship_landing_sensor_flag[4];//one per player
        b2Body*    m_ppBody_in_mship_input[1];
        b2Body*    m_pBody_in_mship_input_void;//dummmy
        b2Body*    m_ppPlayer_bodies[4];
        b2Fixture* m_ppRope_hook_sensors[4];
        b2Body*    m_ppBody_to_connect[4];
        b2Body*    m_pBody_to_connect_p1;//dummmy
        b2Body*    m_pBody_to_connect_p2;//dummmy
        b2Body*    m_pBody_to_connect_p3;//dummmy
        b2Body*    m_pBody_to_connect_p4;//dummmy

        //texture
        int      m_tex_decal,m_tex_menu,m_tex_manual,m_tex_info,m_tex_lost,m_tex_goal,m_tex_text,m_tex_moretext;
        int      m_tex_mask;
        int      m_tex_terrains[_starmap_numof_level_textures];

        //sound
        sound*   m_pSound;
        int      m_music_source_id,m_music_id_playing;
        bool     m_music_intro_playing,m_music_on,m_music_was_off_at_level_start;

        bool load_textures(void);
        bool load_sounds(void);
        bool set_sound_loops(void);
        bool init_box2d(void);
        bool load_level_data(int level_ind,
                             vector<st_float_float_int>& vec_enemy_pos,
                             vector<st_float_float_int>& vec_object_pos);
        bool draw_static(float set_fade_level=-1);
        bool add_explotion(b2Body* body_ptr);
        bool unload_level(void);
        bool load_selected_level(void);
        bool draw_cave(float cam_pos[2]);
        bool draw_terrain(float cam_pos[2]);
        bool draw_projectiles(void);
        bool require_convoy_test(void);
        bool create_convoy(void);
        bool lost_test(void);
        bool save_game_to_file(void);
        bool load_game_from_file(void);
};

#endif // GAME_H
