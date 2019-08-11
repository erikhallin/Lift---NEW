#ifndef HUD_H
#define HUD_H

#include <iostream>
#include <vector>
#include <gl/gl.h>

#include "player.h"
#include "weapon.h"
#include "gear.h"

using namespace std;

class hud
{
    public:
        hud();

        bool init(vector<player>& vec_players,int screen_w,int screen_h,int tex_decal,int tex_menu,int tex_text,float* pCam_pos);
        bool update(float time_dif);
        bool draw(void);
        bool show_hud(int player_id);
        bool set_weapon_color(int player_id,float color[3]);
        bool set_gear_color  (int player_id,float color[3]);
        bool set_player_trigger_value_left (int player_id,float thrust_power);
        bool set_player_trigger_value_right(int player_id,float thrust_power);
        bool set_draw_tutorial_text(int value,bool flag);
        bool get_draw_tutorial_text(int value);
        bool set_mship_ptr(b2Body* mship_body_ptr);
        bool set_player_ptr(b2Body* player_body_ptr);
        b2Body* get_player_ptr(void);
        bool set_draw_tutorial_helptext(int value,bool flag);
        bool get_draw_tutorial_helptext(int value);
        bool is_helptext_completed(int mission_ind=-1);
        bool show_out_of_fuel(float pos[2]);
        bool show_out_of_hp(float pos[2]);
        bool draw_selected_item(int item_type,int player_ind);

        bool   m_text_hook_off_shown,m_text_fuel_box_shown,m_text_out_of_fuel_shown,m_text_out_of_hp_shown;
        b2Vec2 m_fuel_box_pos;

    private:

        float m_screen_width,m_screen_height;
        float m_show_hud_timer[4],m_show_hud_time_limit,m_show_hud_slide_timer[4],m_show_hud_slide_time_limit;
        bool  m_hud_slide_in[4];
        bool  m_draw_bar_frame,m_draw_tutorial,m_draw_convoy_text,m_draw_tut_text[6],m_tut_comleted[6];
        bool  m_draw_tut_moretext[37],m_tut_more_comleted[37];
        float m_force_show_rel_min,m_screen_offset_pix;
        int   m_tex_decal,m_tex_text,m_tex_menu;
        float m_tut_text_fade_level[6],m_tut_text_offset_y[6],m_convoy_text_fade_level;
        float m_tut_moretext_fade_level[37],m_tut_moretext_time_left[37],m_start_show_text_delay;
        float* m_pCam_pos;
        float m_text_out_of_fuel_pos[2],m_text_out_of_fuel_show_time,m_text_out_of_fuel_show_timer;
        float m_text_out_of_hp_pos[2],m_text_out_of_hp_show_time,m_text_out_of_hp_show_timer;
        bool  m_show_text_out_of_fuel,m_show_text_out_of_hp;
        float m_draw_selected_item_timer[4];
        int   m_draw_selected_item_type[4];

        weapon* m_vpWeapon[4];
        gear*   m_vpGear[4];
        b2Body* m_mship_body_ptr;
        b2Body* m_player_body_ptr;

        float m_vPlayer_trigger_value[4][2];//left,right
        float m_vPlayer_thruster_bar_value[4];

        float m_weapon_color_p1[3];
        float m_weapon_color_p2[3];
        float m_weapon_color_p3[3];
        float m_weapon_color_p4[3];
        float m_gear_color_p1[3];
        float m_gear_color_p2[3];
        float m_gear_color_p3[3];
        float m_gear_color_p4[3];

        vector<player>* m_pVec_players;

};

#endif // HUD_H
