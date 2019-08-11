#ifndef STAR_MAP_H
#define STAR_MAP_H

#include <iostream>
#include <sstream>
#include <vector>
#include <gl/gl.h>
#include <Box2D/Box2D.h>
#include <math.h>
#include "definitions.h"
#include "sound.h"

using namespace std;

struct st_planet
{
    st_planet()
    {
        pos[0]=pos[1]=0.0;
        fuel_content=0;
        enemy_content=0;
        dist_to_closest_planet=0;
        world_initialized=false;
        brightness=1.0;
        flash_speed=1.0;
        level_index=-1;
    }
    /*st_planet(float pos_[2],float fuel_,float enemy_)
    {
        pos[0]=pos_[0];
        pos[1]=pos_[1];
        fuel_content=fuel_;
        enemy_content=enemy_;
        dist_to_closest_planet=0;
        world_initialized=false;
    }*/

    float pos[2];
    float fuel_content;
    float enemy_content;
    bool  world_initialized;
    float dist_to_closest_planet;
    float brightness;
    float flash_speed;
    int   level_index;
    float level_color[3];
    int   level_texture_index;

    b2World* pWorld;
};

class star_map
{
    public:
        star_map();

        bool  init(int screen_size[2],int texture,sound* pSound,string file_data="");
        int   update(float time_dif,float& mship_fuel);//mship_fuel will be updated when travelling
        bool  draw(void);
        bool  move_cam_pos(float move_value[3],bool extra_zoom=true);//sent variable will be updated woth current pos, x,y,zoom
        bool  get_cam_pos(float cam_pos[3]);
        bool  planet_selection(int direction);
        bool  planet_go_to(void);
        int   get_planet_now(void);
        int   get_planet_now_level_index(void);
        bool  set_planet_now_level_index(int level_index);
        int   get_curr_planet_level_fuel(void);
        bool  change_curr_planet_level_fuel(int value);
        int   get_curr_planet_level_enemy(void);
        bool  change_curr_planet_level_enemy(int value);
        float get_dist_to_closest_planet_with_fuel(void);
        bool  move_cam_towards_planet_now(float progression);
        bool  get_curr_planet_level_color(float color[3]);
        int   get_curr_planet_level_texture(void);
        bool  is_travelling(void);
        bool  reset_view(void);
        bool  make_new_goal(void);
        bool  save_data(string& game_data);

    private:

        float m_stored_time;//0-360
        int   m_screen_width,m_screen_height;
        float m_max_planet_dist;
        float m_min_planet_dist;
        int   m_max_numof_planets,m_min_numof_planets;
        float m_max_numof_enemy_content,m_max_numof_fuel_content;
        int   m_numof_planets,m_planet_now,m_planet_selected,m_planet_selection_ind,m_planet_goal,m_planet_start;
        float m_cam_pos[2],m_cam_speed[2],m_cam_pos_old[2],m_zoom_level;
        bool  m_planet_travel_now,m_travel_pack_up_now,m_travel_done_set_up_now,m_travel_towards_goal;
        float m_travel_time,m_travel_time_max,m_travel_pack_up_time,m_travel_done_set_up_time;
        int   m_travel_planet_start,m_travel_planet_end;
        float m_mship_fuel,m_travel_fuel_cost;
        float m_curr_planet_enemy_content_drag,m_curr_planet_fuel_content_drag;
        int   m_texture;
        float m_world_rotation,m_radar_sweep_time;

        vector<st_planet> m_vec_planets;
        vector<int> m_vec_planets_ind_within_scan_range;

        sound* m_pSound;

        bool calc_planets_within_range(float range);
};

#endif // STAR_MAP_H
