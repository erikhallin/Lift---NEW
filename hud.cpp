#include "hud.h"

hud::hud()
{
    m_show_hud_time_limit=20.0;
    m_show_hud_slide_time_limit=2.0;
    m_force_show_rel_min=0.1;
    m_screen_offset_pix=15.0;
}

bool hud::init(vector<player>& vec_players,int screen_w,int screen_h,int tex_decal,int tex_menu,int tex_text,float* pCam_pos)
{
    m_pVec_players=&vec_players;
    m_screen_width=screen_w;
    m_screen_height=screen_h;
    m_draw_bar_frame=false;
    m_tex_decal=tex_decal;
    m_tex_text=tex_text;
    m_tex_menu=tex_menu;
    m_draw_tutorial=true;
    m_draw_convoy_text=false;
    m_convoy_text_fade_level=0.0;
    m_pCam_pos=pCam_pos;
    m_text_hook_off_shown=m_text_fuel_box_shown=m_text_out_of_fuel_shown=m_text_out_of_hp_shown=false;
    m_show_text_out_of_fuel=m_show_text_out_of_hp=false;
    m_text_out_of_fuel_show_timer=m_text_out_of_fuel_show_time=30.0;
    m_text_out_of_hp_show_timer=m_text_out_of_hp_show_time=30.0;

    //tutorial objectives
    for(int i=0;i<6;i++)
    {
        m_draw_tut_text[i]=false;
        m_tut_text_fade_level[i]=0.0;
        m_tut_comleted[i]=false;
        m_tut_text_offset_y[i]=0.0;
    }
    m_draw_tut_text[0]=true;
    //tutorial help text
    m_start_show_text_delay=3.0;
    for(int i=0;i<37;i++)
    {
        m_draw_tut_moretext[i]=false;
        m_tut_moretext_fade_level[i]=0.0;
        m_tut_more_comleted[i]=false;
        m_tut_moretext_time_left[i]=0.0;
    }
    //set time left
    m_tut_moretext_time_left[2]=6.0;
    m_tut_moretext_time_left[3]=5.0;
    m_tut_moretext_time_left[8]=3.0;
    m_tut_moretext_time_left[10]=3.0;
    m_tut_moretext_time_left[11]=3.0;
    m_tut_moretext_time_left[12]=3.0;
    m_tut_moretext_time_left[16]=3.0;
    m_tut_moretext_time_left[18]=3.0;
    m_tut_moretext_time_left[20]=3.0;
    m_tut_moretext_time_left[21]=3.0;
    m_tut_moretext_time_left[22]=3.0;
    //m_tut_moretext_time_left[23]=3.0;
    //m_tut_moretext_time_left[24]=3.0;
    m_tut_moretext_time_left[26]=5.0;
    m_tut_moretext_time_left[27]=7.0;
    m_tut_moretext_time_left[29]=5.0;
    m_tut_moretext_time_left[30]=5.0;
    m_tut_moretext_time_left[31]=5.0;
    m_tut_moretext_time_left[32]=5.0;
    m_tut_moretext_time_left[ht_salvaged_ship]=5.0;
    m_tut_moretext_time_left[ht_map_move_cam]=5.0;
    m_tut_moretext_fade_level[ht_takeoff]=-3.0;
    m_tut_moretext_fade_level[ht_map_travel]=-3.0;

    m_show_hud_timer[0]=m_show_hud_timer[1]=m_show_hud_timer[2]=m_show_hud_timer[3]=0.0;
    m_show_hud_slide_timer[0]=m_show_hud_slide_timer[1]=m_show_hud_slide_timer[2]=m_show_hud_slide_timer[3]=0.0;
    m_hud_slide_in[0]=m_hud_slide_in[1]=m_hud_slide_in[2]=m_hud_slide_in[3]=false;

    //get weapon/gear
    for(int player_i=0;player_i<(int)(*m_pVec_players).size();player_i++)
    {
        weapon* player_weapon=(*m_pVec_players)[player_i].get_weapon_ptr();
        m_vpWeapon[player_i]=player_weapon;

        gear* player_gear=(*m_pVec_players)[player_i].get_gear_ptr();
        m_vpGear[player_i]=player_gear;

        //TEMP color
        set_gear_color( player_i, player_gear->m_gear_color );
        set_weapon_color( player_i, player_weapon->m_weapon_color );

        //reset trigger values
        m_vPlayer_trigger_value[player_i][0]=0.0;
        m_vPlayer_trigger_value[player_i][1]=0.0;
        m_vPlayer_thruster_bar_value[player_i]=0.0;

        m_draw_selected_item_timer[player_i]=0;
        m_draw_selected_item_type[player_i]=it_weapon;
    }


    return true;
}

bool hud::update(float time_dif)
{
    //update timer
    for(int player_i=0;player_i<4;player_i++)
    {
        //get weapon
        weapon* player_weapon=(*m_pVec_players)[player_i].get_weapon_ptr();
        m_vpWeapon[player_i]=player_weapon;

        //get gear
        gear* player_gear=(*m_pVec_players)[player_i].get_gear_ptr();
        m_vpGear[player_i]=player_gear;

        //TEMP colors
        set_gear_color( player_i, player_gear->m_gear_color );
        set_weapon_color( player_i, player_weapon->m_weapon_color );

        //display item timer
        if(m_draw_selected_item_timer[player_i]>0.0)
        {
            m_draw_selected_item_timer[player_i]-=time_dif;
            if(m_draw_selected_item_timer[player_i]<0.0) m_draw_selected_item_timer[player_i]=0.0;
        }


        if(m_hud_slide_in[player_i])//increase value to slide in
        {
            m_show_hud_slide_timer[player_i]+=time_dif*3.0;
            if(m_show_hud_slide_timer[player_i]>=m_show_hud_slide_time_limit)
            {
                m_show_hud_slide_timer[player_i]=m_show_hud_slide_time_limit;
                m_hud_slide_in[player_i]=false;
            }
        }
        else//hud either already shown or not at all
        {
            if(m_show_hud_timer[player_i]>0) m_show_hud_timer[player_i]-=time_dif;//first show timer
            else if(m_show_hud_slide_timer[player_i]>=0) m_show_hud_slide_timer[player_i]-=time_dif;//then slide timer
        }

        //force show HUD if any value is below set limit
        if( (*m_pVec_players)[player_i].get_rel_hp()  <m_force_show_rel_min ||
            (*m_pVec_players)[player_i].get_rel_fuel()<m_force_show_rel_min ||
            (*m_pVec_players)[player_i].get_rel_ammo()<m_force_show_rel_min )
        {
            if(!m_hud_slide_in[player_i])//shown or not at all
            {
                if(m_show_hud_timer[player_i]<=0.0)//not at all, start slide in
                {
                    m_hud_slide_in[player_i]=true;
                }
            }
        }

        //update thruster value
        float thruster_target_value=(m_vPlayer_trigger_value[player_i][0]+m_vPlayer_trigger_value[player_i][1])*0.5;
        if(m_vPlayer_thruster_bar_value[player_i]<thruster_target_value)
        {
            m_vPlayer_thruster_bar_value[player_i]=thruster_target_value;
            //m_vPlayer_thruster_bar_value[player_i]+=time_dif*2.0;
        }
        if(m_vPlayer_thruster_bar_value[player_i]>thruster_target_value)
        {
            m_vPlayer_thruster_bar_value[player_i]-=time_dif*2.0;
            if(m_vPlayer_thruster_bar_value[player_i]<0.0) m_vPlayer_thruster_bar_value[player_i]=0.0;
        }
    }

    //update tutorial text
    if(m_draw_tutorial)
    {
        /*//test if done
        if(m_tut_comleted[tut_land] &&
           m_tut_comleted[tut_manual] &&
           m_tut_comleted[tut_fuel] &&
           m_tut_comleted[tut_enemy] &&
           !m_draw_tut_text[tut_return] &&
           m_draw_tut_moretext[ht_drone_takeover])
        {
            m_draw_tut_text[tut_return]=true;
        }*/

        //out of fuel timer
        if(m_show_text_out_of_fuel)
        {
            m_text_out_of_fuel_show_timer-=time_dif;
            if(m_text_out_of_fuel_show_timer<0) m_show_text_out_of_fuel=false;
        }

        //out of hp timer
        if(m_show_text_out_of_hp)
        {
            m_text_out_of_hp_show_timer-=time_dif;
            if(m_text_out_of_hp_show_timer<0) m_show_text_out_of_hp=false;
        }

        //text start delay
        if(m_start_show_text_delay>0.0)
        {
            m_start_show_text_delay-=time_dif;
            {
                if(m_start_show_text_delay<=0.0)
                {
                    m_start_show_text_delay=0.0;
                    m_draw_tut_moretext[0]=true;
                }
            }
        }

        float fade_speed=1.0;
        for(int i=0;i<6;i++)
        {
            //turn on test
            if(m_draw_tut_text[i] && m_tut_text_fade_level[i]<1.0)
            {
                m_tut_text_fade_level[i]+=time_dif*fade_speed;
                if(m_tut_text_fade_level[i]>1.0) m_tut_text_fade_level[i]=1.0;
            }
            //turn off test
            else if(!m_draw_tut_text[i] && m_tut_text_fade_level[i]>0.0)
            {
                m_tut_text_fade_level[i]-=time_dif*fade_speed*0.5;
                if(m_tut_text_fade_level[i]<0.0)
                {
                    //done
                    m_tut_text_fade_level[i]=0.0;

                    //extra y shift to pass on
                    float extra_y_shift=m_tut_text_offset_y[i];

                    //shift start pos of the text box below
                    float row_height=30.0;
                    float text_height=row_height;
                    if(i==2) text_height=row_height*4.0;
                    else if(i==3) text_height=row_height*3.0;

                    //is there a text box below
                    for(int i2=i+1;i2<6;i2++)
                    {
                        if(m_tut_text_fade_level[i2]>0.0)
                        {
                            //found text box below, add to y shift value
                            m_tut_text_offset_y[i2]+=text_height+extra_y_shift;
                            break;
                        }
                    }
                }
            }
        }

        //move text box up
        for(int i=0;i<6;i++)
        {
            if(m_tut_text_offset_y[i]>0.0)
            {
                //linear
                float shift_speed=30.0;
                //non-linear end
                if(m_tut_text_offset_y[i]<15.0)
                {
                    shift_speed=m_tut_text_offset_y[i]*2.0;
                    if(shift_speed<1.0) shift_speed=1.0;
                }

                m_tut_text_offset_y[i]-=time_dif*shift_speed;
                //done
                if(m_tut_text_offset_y[i]<0.0) m_tut_text_offset_y[i]=0.0;
            }
        }

        //help text
        for(int i=0;i<37;i++)
        {
            //turn on test
            if(m_draw_tut_moretext[i] && m_tut_moretext_fade_level[i]<1.0)
            {
                //cout<<i<<", "<<m_tut_moretext_fade_level[i]<<endl;

                //exception test
                if(i==5 && (m_draw_tut_text[0] || m_draw_tut_text[1]) )//connect towline message and landing/manual message
                {
                    //pause until manual have been shown
                }
                else//update as normal
                {
                    m_tut_moretext_fade_level[i]+=time_dif*fade_speed*0.6;
                    if(m_tut_moretext_fade_level[i]>1.0)
                    {
                        m_tut_moretext_fade_level[i]=1.0;

                        //if already completed, turn off
                        if(m_tut_more_comleted[i]) m_draw_tut_moretext[i]=false;
                    }
                }
            }

            //turn off test
            if(!m_draw_tut_moretext[i] && m_tut_moretext_fade_level[i]>0.0)
            {
                m_tut_moretext_fade_level[i]-=time_dif*fade_speed*0.6;
                if(m_tut_moretext_fade_level[i]<=0.0)
                {
                    //done
                    m_tut_moretext_fade_level[i]=0.0;
                    m_tut_more_comleted[i]=true;

                    //show next test, if next text should be triggered
                    //if(i==0) m_draw_tut_moretext[1]=true;
                    if(i==0) m_draw_tut_moretext[10]=true;
                    if(i==1) m_draw_tut_moretext[2]=true;
                    if(i==2) m_draw_tut_moretext[3]=true;
                    if(i==3) m_draw_tut_moretext[7]=true;
                    if(i==7) m_draw_tut_moretext[4]=true;
                    if(i==4) m_draw_tut_moretext[5]=true;
                    if(i==5) m_draw_tut_moretext[6]=true;
                    if(i==6) m_draw_tut_moretext[8]=true;
                    if(i==8) m_draw_tut_moretext[9]=true;
                    //if(i==9) m_draw_tut_moretext[10]=true;
                    if(i==9) m_draw_tut_moretext[13]=true;
                    if(i==10) m_draw_tut_moretext[11]=true;
                    if(i==11) m_draw_tut_moretext[12]=true;
                    //if(i==12) m_draw_tut_moretext[13]=true;
                    if(i==12) m_draw_tut_moretext[1]=true;
                    if(i==13) m_draw_tut_moretext[14]=true;
                    if(i==14) m_draw_tut_moretext[15]=true;
                    if(i==15) m_draw_tut_moretext[16]=true;
                    if(i==16) m_draw_tut_moretext[17]=true;
                    if(i==17) m_draw_tut_moretext[18]=true;
                    if(i==18) m_draw_tut_moretext[19]=true;
                    if(i==19) m_draw_tut_moretext[20]=true;
                    if(i==20) m_draw_tut_moretext[21]=true;
                    if(i==21) m_draw_tut_moretext[22]=true;
                    if(i==22) m_draw_tut_moretext[23]=true;
                    if(i==23) m_draw_tut_moretext[24]=true;
                    if(i==24) m_draw_tut_moretext[28]=true;
                    if(i==28) m_draw_tut_moretext[ht_salvaged_ship]=true;
                    if(i==ht_map_center) m_draw_tut_moretext[ht_map_dots]=true;
                    if(i==ht_map_dots) m_draw_tut_moretext[ht_map_yellow_area]=true;
                    if(i==ht_map_yellow_area) m_draw_tut_moretext[ht_map_bars]=true;
                    if(i==ht_map_bars) m_draw_tut_moretext[ht_map_move_cam]=true;
                    if(i==ht_map_move_cam) m_draw_tut_moretext[ht_map_travel]=true;
                    if(i==ht_map_travel) m_draw_tut_moretext[ht_map_land]=true;

                    //complete last mission if done
                    //if(i==25) m_tut_more_comleted[25]=true;
                }
            }

            //time delay to turn off
            if(m_draw_tut_moretext[i] && m_tut_moretext_fade_level[i]==1.0 && m_tut_moretext_time_left[i]>0.0)
            {
                m_tut_moretext_time_left[i]-=time_dif;
                if(m_tut_moretext_time_left[i]<=0.0)
                {
                    m_tut_moretext_time_left[i]=0.0;
                    m_draw_tut_moretext[i]=false;
                }
            }
        }
    }
    else if(m_draw_convoy_text)
    {
        float fade_speed=1.0;
        //turn on
        if(m_convoy_text_fade_level<1.0)
        {
            m_convoy_text_fade_level+=time_dif*fade_speed;
            if(m_convoy_text_fade_level>1.0) m_convoy_text_fade_level=1.0;
        }
    }

    return true;
}

bool hud::draw()
{
    float screen_pos_x,screen_pos_y,box_length,bar_length,bar_height,bar_length_max;
    float screen_start_pos_x,screen_start_pos_y;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);//ritar svart men fadear
    //glBlendFunc(GL_ONE,GL_ONE);//ritar ej svart men fadear inte
    //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);//fadear det svarta men inte vida
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);//ok
    glBindTexture(GL_TEXTURE_2D, m_tex_decal);

    //draw selected item
    for(int player_i=0;player_i<4;player_i++)
    {
        if(m_draw_selected_item_timer[player_i]>0.0)
        {
            glPushMatrix();
            b2Vec2 player_pos=(*m_pVec_players)[player_i].get_body_ptr()->GetPosition();
            glTranslatef(player_pos.x*_Met2Pix-m_pCam_pos[0] ,player_pos.y*_Met2Pix-40.0-m_pCam_pos[1], 0.0);

            float texture_size=0.021484375;
            float texture_pos_y=texture_size*3.0;//row 3

            int item_type=m_vpWeapon[player_i]->get_type();
            if(m_draw_selected_item_type[player_i]==it_gear)
            {
                item_type=m_vpGear[player_i]->get_type();
                texture_pos_y=texture_size*4.0;//row 4
            }
            if(m_draw_selected_item_type[player_i]==it_line)
            {
                item_type=14;//fuel
                if( (*m_pVec_players)[player_i].m_line_type_tow ) item_type=13;//tow
                texture_pos_y=texture_size*4.0;//row 4
            }
            float texture_pos_x=float(item_type)*texture_size;

            screen_pos_x=0;
            screen_pos_y=0;
            float box_length=0.01*(float)m_screen_height;

            glColor4f(1,1,1,m_draw_selected_item_timer[player_i]/_hud_item_draw_time);
            glBegin(GL_QUADS);
            glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
            glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
            glTexCoord2f(texture_pos_x,texture_pos_y);
            glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
            glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
            glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
            glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
            glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
            glEnd();

            glPopMatrix();
        }
    }
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    //player 1
    int player_id=0;
    if(m_show_hud_slide_timer[player_id]>0.0)
    {
        glPushMatrix();
        //move sliding translation
        float slide_length=0.04*(float)m_screen_height*(1.0-m_show_hud_slide_timer[player_id]/m_show_hud_slide_time_limit);
        glTranslatef(0.0,-slide_length,0.0);
        //get player hud window pos
        //screen_start_pos_x=0.03*(float)m_screen_width; //rel pos
        //screen_start_pos_y=0.02*(float)m_screen_height;
        screen_start_pos_x=m_screen_offset_pix+0.005*(float)m_screen_height; //abs pos, rel depending on box size
        screen_start_pos_y=m_screen_offset_pix+0.005*(float)m_screen_height;

        float texture_size=0.021484375; //22 pixels (1:1 ratio if screen is 1080p) or 21.6 : 0.02109375

        //draw player id box
        float texture_pos_x=float(player_id)*texture_size;
        float texture_pos_y=texture_size*0.0;//row 0
        screen_pos_x=screen_start_pos_x+0.0*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(0.7,0.7,0.7,0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw weapon box
        int weapon_type=m_vpWeapon[player_id]->get_type();
        texture_pos_x=float(weapon_type)*texture_size;
        texture_pos_y=texture_size*1.0;//row 1
        screen_pos_x=screen_start_pos_x+0.03*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_weapon_color_p1[0],m_weapon_color_p1[1],m_weapon_color_p1[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw gear box
        int gear_type=m_vpGear[player_id]->get_type();
        texture_pos_x=float(gear_type)*texture_size;
        texture_pos_y=texture_size*2.0;//row 2
        screen_pos_x=screen_start_pos_x+0.06*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_gear_color_p1[0],m_gear_color_p1[1],m_gear_color_p1[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        //draw hp bar
        glLineWidth(1);
        screen_pos_x=screen_start_pos_x+0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y-0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_hp_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_hp();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.1,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.8,0.2,0.2,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.8,0.3,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }


        //draw fuel bar
        screen_pos_x=screen_start_pos_x+0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_fuel_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_fuel();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.1,0.3,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.2,0.8,0.2,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.8,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw ammo bar
        screen_pos_x=screen_start_pos_x+0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_ammo_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_ammo();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.2,0.2,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.4,0.4,0.8,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.3,0.8);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw thruster bar
        screen_pos_x=screen_start_pos_x-box_length;
        screen_pos_y=screen_start_pos_y+0.0165*(float)m_screen_height;
        bar_length_max=0.00065*(float)m_screen_height*(*m_pVec_players)[player_id].m_motor_thrust_power_max;
        bar_length=bar_length_max * m_vPlayer_thruster_bar_value[player_id];
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.3,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.7,0.7,0.7,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.2,0.2,0.2);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        glPopMatrix();
    }
    //player 2
    player_id=1;
    if(m_show_hud_slide_timer[player_id]>0.0)
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        float slide_length=0.04*(float)m_screen_height*(1.0-m_show_hud_slide_timer[player_id]/m_show_hud_slide_time_limit);
        glTranslatef(0.0,-slide_length,0.0);
        //get player hud window pos
        //screen_start_pos_x=0.97*(float)m_screen_width;
        screen_start_pos_x=(float)m_screen_width-m_screen_offset_pix-0.005*(float)m_screen_height; //abs pos, rel depending on box size
        screen_start_pos_y=m_screen_offset_pix+0.005*(float)m_screen_height;

        float texture_size=0.021484375; //22 pixels (1:1 ratio if screen is 1080p) or 21.6 : 0.02109375

        //draw player id box
        float texture_pos_x=float(player_id)*texture_size;
        float texture_pos_y=texture_size*0.0;//row 0
        screen_pos_x=screen_start_pos_x+0.0*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(0.7,0.7,0.7,0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw weapon box
        int weapon_type=m_vpWeapon[player_id]->get_type();
        texture_pos_x=float(weapon_type)*texture_size;
        texture_pos_y=texture_size*1.0;//row 1
        screen_pos_x=screen_start_pos_x-0.03*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_weapon_color_p2[0],m_weapon_color_p2[1],m_weapon_color_p2[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw gear box
        int gear_type=m_vpGear[player_id]->get_type();
        texture_pos_x=float(gear_type)*texture_size;
        texture_pos_y=texture_size*2.0;//row 2
        screen_pos_x=screen_start_pos_x-0.06*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_gear_color_p2[0],m_gear_color_p2[1],m_gear_color_p2[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        //draw hp bar
        glLineWidth(1);
        screen_pos_x=screen_start_pos_x-0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y-0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_hp_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_hp();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.1,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.8,0.2,0.2,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.8,0.3,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }


        //draw fuel bar
        screen_pos_x=screen_start_pos_x-0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_fuel_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_fuel();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.1,0.3,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.2,0.8,0.2,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.8,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw ammo bar
        screen_pos_x=screen_start_pos_x-0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_ammo_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_ammo();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.2,0.2,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.4,0.4,0.8,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.3,0.8);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw thruster bar
        screen_pos_x=screen_start_pos_x+box_length;
        screen_pos_y=screen_start_pos_y+0.0165*(float)m_screen_height;
        bar_length_max=0.00065*(float)m_screen_height*(*m_pVec_players)[player_id].m_motor_thrust_power_max;
        bar_length=bar_length_max * m_vPlayer_thruster_bar_value[player_id];
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.3,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.7,0.7,0.7,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.2,0.2,0.2);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        glPopMatrix();
    }
    //player 3
    player_id=2;
    if(m_show_hud_slide_timer[player_id]>0.0)
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        float slide_length=0.04*(float)m_screen_height*(1.0-m_show_hud_slide_timer[player_id]/m_show_hud_slide_time_limit);
        glTranslatef(0.0,slide_length,0.0);
        //get player hud window pos
        //screen_start_pos_x=0.03*(float)m_screen_width;
        screen_start_pos_x=m_screen_offset_pix+0.005*(float)m_screen_height; //abs pos, rel depending on box size
        screen_start_pos_y=(float)m_screen_height-m_screen_offset_pix-0.005*(float)m_screen_height;

        float texture_size=0.021484375; //22 pixels (1:1 ratio if screen is 1080p) or 21.6 : 0.02109375

        //draw player id box
        float texture_pos_x=float(player_id)*texture_size;
        float texture_pos_y=texture_size*0.0;//row 0
        screen_pos_x=screen_start_pos_x+0.0*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(0.7,0.7,0.7,0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw weapon box
        int weapon_type=m_vpWeapon[player_id]->get_type();
        texture_pos_x=float(weapon_type)*texture_size;
        texture_pos_y=texture_size*1.0;//row 1
        screen_pos_x=screen_start_pos_x+0.03*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_weapon_color_p3[0],m_weapon_color_p3[1],m_weapon_color_p3[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw gear box
        int gear_type=m_vpGear[player_id]->get_type();
        texture_pos_x=float(gear_type)*texture_size;
        texture_pos_y=texture_size*2.0;//row 2
        screen_pos_x=screen_start_pos_x+0.06*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_gear_color_p3[0],m_gear_color_p3[1],m_gear_color_p3[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        //draw hp bar
        glLineWidth(1);
        screen_pos_x=screen_start_pos_x+0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y-0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_hp_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_hp();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.1,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.8,0.2,0.2,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.8,0.3,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }


        //draw fuel bar
        screen_pos_x=screen_start_pos_x+0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_fuel_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_fuel();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.1,0.3,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.2,0.8,0.2,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.8,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw ammo bar
        screen_pos_x=screen_start_pos_x+0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_ammo_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_ammo();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.2,0.2,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.4,0.4,0.8,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.3,0.8);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw thruster bar
        screen_pos_x=screen_start_pos_x-box_length;
        screen_pos_y=screen_start_pos_y-0.0165*(float)m_screen_height;
        bar_length_max=0.00065*(float)m_screen_height*(*m_pVec_players)[player_id].m_motor_thrust_power_max;
        bar_length=bar_length_max * m_vPlayer_thruster_bar_value[player_id];
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.3,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y+bar_height);
        glColor4f(0.7,0.7,0.7,0.9);
        glVertex2d(screen_pos_x+bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.2,0.2,0.2);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x+bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        glPopMatrix();
    }
    //player 4
    player_id=3;
    if(m_show_hud_slide_timer[player_id]>0.0)
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        float slide_length=0.04*(float)m_screen_height*(1.0-m_show_hud_slide_timer[player_id]/m_show_hud_slide_time_limit);
        glTranslatef(0.0,slide_length,0.0);
        //get player hud window pos
        //screen_start_pos_x=0.97*(float)m_screen_width;
        screen_start_pos_x=(float)m_screen_width-m_screen_offset_pix-0.005*(float)m_screen_height; //abs pos, rel depending on box size
        screen_start_pos_y=(float)m_screen_height-m_screen_offset_pix-0.005*(float)m_screen_height;

        float texture_size=0.021484375; //22 pixels (1:1 ratio if screen is 1080p) or 21.6 : 0.02109375

        //draw player id box
        float texture_pos_x=float(player_id)*texture_size;
        float texture_pos_y=texture_size*0.0;//row 0
        screen_pos_x=screen_start_pos_x+0.0*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(0.7,0.7,0.7,0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw weapon box
        int weapon_type=m_vpWeapon[player_id]->get_type();
        texture_pos_x=float(weapon_type)*texture_size;
        texture_pos_y=texture_size*1.0;//row 1
        screen_pos_x=screen_start_pos_x-0.03*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_weapon_color_p4[0],m_weapon_color_p4[1],m_weapon_color_p4[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        //draw gear box
        int gear_type=m_vpGear[player_id]->get_type();
        texture_pos_x=float(gear_type)*texture_size;
        texture_pos_y=texture_size*2.0;//row 2
        screen_pos_x=screen_start_pos_x-0.06*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        box_length=0.01*(float)m_screen_height;
        glColor4f(1,1,1,1);
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(texture_pos_x+texture_size*0.5,texture_pos_y+texture_size*0.5);
        glVertex2d(screen_pos_x,screen_pos_y);
        glColor4f(m_gear_color_p4[0],m_gear_color_p4[1],m_gear_color_p4[2],0.7);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y);
        glVertex2d(screen_pos_x-box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y);
        glVertex2d(screen_pos_x+box_length,screen_pos_y+box_length);
        glTexCoord2f(texture_pos_x+texture_size,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x+box_length,screen_pos_y-box_length);
        glTexCoord2f(texture_pos_x,texture_pos_y+texture_size);
        glVertex2d(screen_pos_x-box_length,screen_pos_y-box_length);
        glEnd();

        glDisable(GL_TEXTURE_2D);

        //draw hp bar
        glLineWidth(1);
        screen_pos_x=screen_start_pos_x-0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y-0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_hp_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_hp();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.1,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.8,0.2,0.2,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.8,0.3,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }


        //draw fuel bar
        screen_pos_x=screen_start_pos_x-0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.00*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_fuel_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_fuel();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.1,0.3,0.1,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.2,0.8,0.2,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.8,0.3);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw ammo bar
        screen_pos_x=screen_start_pos_x-0.082*(float)m_screen_height;
        screen_pos_y=screen_start_pos_y+0.0065*(float)m_screen_height;
        bar_length_max=0.0010*(float)m_screen_height*(*m_pVec_players)[player_id].m_ammo_max;
        bar_length=bar_length_max * (*m_pVec_players)[player_id].get_rel_ammo();
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.2,0.2,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.4,0.4,0.8,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.3,0.3,0.8);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        //draw thruster bar
        screen_pos_x=screen_start_pos_x+box_length;
        screen_pos_y=screen_start_pos_y-0.0165*(float)m_screen_height;
        bar_length_max=0.00065*(float)m_screen_height*(*m_pVec_players)[player_id].m_motor_thrust_power_max;
        bar_length=bar_length_max * m_vPlayer_thruster_bar_value[player_id];
        bar_height=0.002*(float)m_screen_height;
        glColor4f(0.3,0.3,0.3,0.9);
        glBegin(GL_QUADS);//fill bar
        glVertex2d(screen_pos_x,screen_pos_y+bar_height);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y+bar_height);
        glColor4f(0.7,0.7,0.7,0.9);
        glVertex2d(screen_pos_x-bar_length,screen_pos_y-bar_height);
        glVertex2d(screen_pos_x,screen_pos_y-bar_height);
        glEnd();
        if(m_draw_bar_frame)
        {
            glColor3f(0.2,0.2,0.2);
            glBegin(GL_LINE_STRIP);//frame
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y+bar_height);
            glVertex2d(screen_pos_x-bar_length_max,screen_pos_y-bar_height);
            glVertex2d(screen_pos_x,screen_pos_y-bar_height);
            glEnd();
        }

        glPopMatrix();
    }

    //draw screen text
    if(m_draw_tutorial)
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_tex_menu);
        //glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
        //texture coord
        float tex_pos_y_min[6]={ (1024.0-373.0)/1024.0,
                                 (1024.0-403.0)/1024.0,
                                 (1024.0-433.0)/1024.0,
                                 (1024.0-553.0)/1024.0,
                                 (1024.0-643.0)/1024.0,
                                 (1024.0-673.0)/1024.0 };
        float tex_pos_y_max[6]={ (1024.0-403.0)/1024.0,
                                 (1024.0-433.0)/1024.0,
                                 (1024.0-553.0)/1024.0,
                                 (1024.0-643.0)/1024.0,
                                 (1024.0-673.0)/1024.0,
                                 (1024.0-703.0)/1024.0 };
        float tex_pos_x_min=0.0;
        float tex_pos_x_max=970.0/1024.0;
        //screen coord
        float screen_pos_x=0.0;
        float screen_pos_y=50.0;
        float row_height=30.0;
        float row_width=970.0;
        //go to text pos
        glTranslatef(screen_pos_x,screen_pos_y,0.0);

        for(int i=0;i<6;i++)
        {
            if(m_tut_text_fade_level[i]>0.0)
            {
                //extra translate
                glTranslatef(0.0,m_tut_text_offset_y[i],0.0);

                //set text height
                float text_height=row_height;
                if(i==2) text_height=row_height*4.0;
                else if(i==3) text_height=row_height*3.0;

                //draw
                glColor4f(m_tut_text_fade_level[i],m_tut_text_fade_level[i],m_tut_text_fade_level[i],m_tut_text_fade_level[i]);
                glBegin(GL_QUADS);
                glTexCoord2f(tex_pos_x_max,tex_pos_y_min[i]);
                glVertex2f(row_width,0.0);
                glTexCoord2f(tex_pos_x_max,tex_pos_y_max[i]);
                glVertex2f(row_width,text_height);
                glTexCoord2f(tex_pos_x_min,tex_pos_y_max[i]);
                glVertex2f(0.0,text_height);
                glTexCoord2f(tex_pos_x_min,tex_pos_y_min[i]);
                glVertex2f(0.0,0.0);
                glEnd();

                glTranslatef(0.0,text_height,0.0);
            }
        }

        glPopMatrix();

        //draw more text
        glBindTexture(GL_TEXTURE_2D, m_tex_text);
        for(int i=0;i<37;i++)
        {
            if(m_tut_moretext_fade_level[i]>0.0)
            {
                //cout<<i<<endl;
                glPushMatrix();

                //move to screen pos (starmap pos)
                if(i==29||i==30||i==31||i==32||i==33||i==34||i==36)
                {
                    if(i==ht_map_yellow_area) glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.3,0.0);
                    else if(i==ht_map_bars)   glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.05,0.0);
                    else                      glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.4,0.0);
                }
                //move fuel box
                else if(i==27)
                {
                    glTranslatef(-m_pCam_pos[0],-m_pCam_pos[1],0);
                    glTranslatef(m_fuel_box_pos.x*_Met2Pix-512.0,m_fuel_box_pos.y*_Met2Pix-150.0,0.0);
                }
                //move to player pos
                else if(i==2||i==3||i==4||i==5||i==7||i==9||i==14||i==15||i==22||i==23||i==24||i==25||i==26||i==28)
                {
                    b2Vec2 player_pos=m_player_body_ptr->GetPosition();
                    //float angle=m_player_body_ptr->GetAngle()*_Rad2Deg;

                    //test if outside screen
                    float translate_val[2]={player_pos.x*_Met2Pix-512.0-m_pCam_pos[0] ,player_pos.y*_Met2Pix+150.0-m_pCam_pos[1]};
                    if(i==26)
                    {
                        translate_val[0]=player_pos.x*_Met2Pix-512.0-m_pCam_pos[0];
                        translate_val[1]=player_pos.y*_Met2Pix-150.0-m_pCam_pos[1];
                    }

                    //cap values inside screen
                    if( translate_val[0] < 0.0 )
                    {
                        translate_val[0]=0.0;
                    }
                    if( translate_val[0] > m_screen_width-1024.0 )
                    {
                        translate_val[0]=m_screen_width-1024.0;
                    }
                    if( translate_val[1] < 30.0 )
                    {
                        translate_val[1]=30.0;
                    }
                    if( translate_val[1] > m_screen_height-30.0 )
                    {
                        translate_val[1]=m_screen_height-30.0;
                    }

                    glTranslatef(translate_val[0],translate_val[1],0);
                    //glRotatef(angle,0,0,1);
                }
                //move to mship
                else
                {
                    b2Vec2 mship_pos=m_mship_body_ptr->GetPosition();
                    //float angle=m_mship_body_ptr->GetAngle()*_Rad2Deg;
                    glTranslatef(-m_pCam_pos[0],-m_pCam_pos[1],0);
                    if(i==25)
                     glTranslatef(mship_pos.x*_Met2Pix-512.0,mship_pos.y*_Met2Pix-180.0,0.0);
                    else
                     glTranslatef(mship_pos.x*_Met2Pix-512.0,mship_pos.y*_Met2Pix-150.0,0.0);
                    //glRotatef(angle,0,0,1);
                }

                float texture_pos_y_min=(1024.0-(float)i*27.0)/1024.0;
                float texture_pos_y_max=(1024.0-(float)i*27.0-27.0)/1024.0;

                //draw
                glColor4f(m_tut_moretext_fade_level[i],m_tut_moretext_fade_level[i],m_tut_moretext_fade_level[i],m_tut_moretext_fade_level[i]);
                glBegin(GL_QUADS);
                glTexCoord2f(1.0,texture_pos_y_min);
                glVertex2f(1024.0,0.0);
                glTexCoord2f(1.0,texture_pos_y_max);
                glVertex2f(1024.0,27.0);
                glTexCoord2f(0.0,texture_pos_y_max);
                glVertex2f(0.0,27.0);
                glTexCoord2f(0.0,texture_pos_y_min);
                glVertex2f(0.0,0.0);
                glEnd();

                //extra row text
                if(i==23)
                {
                    glBindTexture(GL_TEXTURE_2D,m_tex_menu);
                    glBegin(GL_QUADS);
                    glTexCoord2f(1.0,(1024.0-78.0)/1024.0);
                    glVertex2f(831.0,30.0);
                    glTexCoord2f(1.0,(1024.0-108.0)/1024.0);
                    glVertex2f(831.0,60.0);
                    glTexCoord2f(385.0/1024.0,(1024.0-108.0)/1024.0);
                    glVertex2f(192.0,60.0);
                    glTexCoord2f(385.0/1024.0,(1024.0-78.0)/1024.0);
                    glVertex2f(192.0,30.0);
                    glEnd();
                    glBindTexture(GL_TEXTURE_2D, m_tex_text);
                }

                glPopMatrix();
            }
        }

        //glDisable(GL_BLEND);

    }
    else if(m_draw_convoy_text)//test if drawing convoy text
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_tex_menu);
        //glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);

        float tex_pos_x_min=0.0;
        float tex_pos_x_max=970.0/1024.0;
        float tex_pos_y_min=(1024.0-764.0)/1024.0;
        float tex_pos_y_max=(1024.0-824.0)/1024.0;
        float text_height=60.0;
        float row_width=970.0;
        float screen_pos_x=0.0;
        float screen_pos_y=50.0;

        //go to text pos
        glTranslatef(screen_pos_x,screen_pos_y,0.0);
        //draw
        glColor4f(m_convoy_text_fade_level,m_convoy_text_fade_level,m_convoy_text_fade_level,m_convoy_text_fade_level);
        glBegin(GL_QUADS);
        glTexCoord2f(tex_pos_x_max,tex_pos_y_min);
        glVertex2f(row_width,0.0);
        glTexCoord2f(tex_pos_x_max,tex_pos_y_max);
        glVertex2f(row_width,text_height);
        glTexCoord2f(tex_pos_x_min,tex_pos_y_max);
        glVertex2f(0.0,text_height);
        glTexCoord2f(tex_pos_x_min,tex_pos_y_min);
        glVertex2f(0.0,0.0);
        glEnd();

        //glDisable(GL_BLEND);
        glPopMatrix();
    }

    //out of fuel text
    if(m_show_text_out_of_fuel)
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_tex_menu);
        //glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);

        float tex_pos_x_min=0.0;
        float tex_pos_x_max=1.0;
        float tex_pos_y_min=(1024.0-884.0)/1024.0;
        float tex_pos_y_max=(1024.0-884.0-60.0)/1024.0;
        float text_height=60.0;
        float row_width=1024.0;

        float fade_level=1.0;
        if(m_text_out_of_fuel_show_timer>m_text_out_of_fuel_show_time-3.0)//fade in
        {
            fade_level=1.0-(m_text_out_of_fuel_show_timer-(m_text_out_of_fuel_show_time-3.0))/3.0;
        }
        else if(m_text_out_of_fuel_show_timer>3)
        {
            fade_level=1;
        }
        else//fade out
        {
            fade_level=m_text_out_of_fuel_show_timer/3.0;
        }

        float translate_val[2]={m_text_out_of_fuel_pos[0]-m_pCam_pos[0],m_text_out_of_fuel_pos[1]-m_pCam_pos[1]};
        //cap values inside screen
        if( translate_val[0] < 0.0 )
        {
            translate_val[0]=0.0;
        }
        if( translate_val[0] > m_screen_width-1024.0 )
        {
            translate_val[0]=m_screen_width-1024.0;
        }
        if( translate_val[1] < 90.0 )
        {
            translate_val[1]=90.0;
        }
        if( translate_val[1] > m_screen_height-60.0 )
        {
            translate_val[1]=m_screen_height-60.0;
        }

        glTranslatef(translate_val[0],translate_val[1],0.0);

        glColor4f(fade_level,fade_level,fade_level,fade_level);
        glBegin(GL_QUADS);
        glTexCoord2f(tex_pos_x_max,tex_pos_y_min);
        glVertex2f(row_width,0.0);
        glTexCoord2f(tex_pos_x_max,tex_pos_y_max);
        glVertex2f(row_width,text_height);
        glTexCoord2f(tex_pos_x_min,tex_pos_y_max);
        glVertex2f(0.0,text_height);
        glTexCoord2f(tex_pos_x_min,tex_pos_y_min);
        glVertex2f(0.0,0.0);
        glEnd();

        glPopMatrix();
    }

    //out of hp text
    if(m_show_text_out_of_hp)
    {
        glPushMatrix();
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_tex_menu);
        //glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);

        float tex_pos_x_min1=686.0/1024.0;
        float tex_pos_x_max1=(686.0+334.0)/1024.0;
        float tex_pos_y_min1=(1024.0-3.0)/1024.0;
        float tex_pos_y_max1=(1024.0-3.0-30.0)/1024.0;
        float text_height1=30.0;
        float row_width1=334.0;

        float tex_pos_x_min2=334.0/1024.0;
        float tex_pos_x_max2=1.0;
        float tex_pos_y_min2=(1024.0-884.0)/1024.0;
        float tex_pos_y_max2=(1024.0-884.0-30.0)/1024.0;
        float text_height2=30.0;
        float row_width2=1024.0-334.0;

        float tex_pos_x_min3=0.0;
        float tex_pos_x_max3=1.0;
        float tex_pos_y_min3=(1024.0-884.0-30.0)/1024.0;
        float tex_pos_y_max3=(1024.0-884.0-60.0)/1024.0;
        float text_height3=30.0;
        float row_width3=1024.0;

        float fade_level=1.0;
        if(m_text_out_of_hp_show_timer>m_text_out_of_hp_show_time-3.0)//fade in
        {
            fade_level=1.0-(m_text_out_of_hp_show_timer-(m_text_out_of_hp_show_time-3.0))/3.0;
        }
        else if(m_text_out_of_hp_show_timer>3)
        {
            fade_level=1;
        }
        else//fade out
        {
            fade_level=m_text_out_of_hp_show_timer/3.0;
        }

        float translate_val[2]={m_text_out_of_hp_pos[0]-m_pCam_pos[0],m_text_out_of_hp_pos[1]-m_pCam_pos[1]};
        //cap values inside screen
        if( translate_val[0] < 0.0 )
        {
            translate_val[0]=0.0;
        }
        if( translate_val[0] > m_screen_width-1024.0 )
        {
            translate_val[0]=m_screen_width-1024.0;
        }
        if( translate_val[1] < 90.0 )
        {
            translate_val[1]=90.0;
        }
        if( translate_val[1] > m_screen_height-60.0 )
        {
            translate_val[1]=m_screen_height-60.0;
        }

        glTranslatef(translate_val[0],translate_val[1],0.0);

        glColor4f(fade_level,fade_level,fade_level,fade_level);
        glBegin(GL_QUADS);
        //first row first part
        glTexCoord2f(tex_pos_x_max1,tex_pos_y_min1);
        glVertex2f(row_width1,0.0);
        glTexCoord2f(tex_pos_x_max1,tex_pos_y_max1);
        glVertex2f(row_width1,text_height1);
        glTexCoord2f(tex_pos_x_min1,tex_pos_y_max1);
        glVertex2f(0.0,text_height1);
        glTexCoord2f(tex_pos_x_min1,tex_pos_y_min1);
        glVertex2f(0.0,0.0);
        //first row second part
        glTexCoord2f(tex_pos_x_max2,tex_pos_y_min2);
        glVertex2f(row_width1+row_width2,0.0);
        glTexCoord2f(tex_pos_x_max2,tex_pos_y_max2);
        glVertex2f(row_width1+row_width2,text_height2);
        glTexCoord2f(tex_pos_x_min2,tex_pos_y_max2);
        glVertex2f(row_width1+0.0,text_height2);
        glTexCoord2f(tex_pos_x_min2,tex_pos_y_min2);
        glVertex2f(row_width1+0.0,0.0);
        //second row
        glTexCoord2f(tex_pos_x_max3,tex_pos_y_min3);
        glVertex2f(row_width3,30.0);
        glTexCoord2f(tex_pos_x_max3,tex_pos_y_max3);
        glVertex2f(row_width3,30.0+text_height3);
        glTexCoord2f(tex_pos_x_min3,tex_pos_y_max3);
        glVertex2f(0.0,30.0+text_height3);
        glTexCoord2f(tex_pos_x_min3,tex_pos_y_min3);
        glVertex2f(0.0,30.0);
        glEnd();

        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    return true;
}

bool hud::show_hud(int player_id)
{
    m_show_hud_timer[player_id]=m_show_hud_time_limit;
    m_hud_slide_in[player_id]=true;

    return true;
}

bool hud::set_weapon_color(int player_id,float color[3])
{
    //update weapon
    weapon* player_weapon=(*m_pVec_players)[player_id].get_weapon_ptr();
    m_vpWeapon[player_id]=player_weapon;

    switch(player_id)
    {
        case 0:
        {
            m_weapon_color_p1[0]=color[0];
            m_weapon_color_p1[1]=color[1];
            m_weapon_color_p1[2]=color[2];
        }break;

        case 1:
        {
            m_weapon_color_p2[0]=color[0];
            m_weapon_color_p2[1]=color[1];
            m_weapon_color_p2[2]=color[2];
        }break;

        case 2:
        {
            m_weapon_color_p3[0]=color[0];
            m_weapon_color_p3[1]=color[1];
            m_weapon_color_p3[2]=color[2];
        }break;

        case 3:
        {
            m_weapon_color_p4[0]=color[0];
            m_weapon_color_p4[1]=color[1];
            m_weapon_color_p4[2]=color[2];
        }break;
    }

    return true;
}

bool hud::set_gear_color(int player_id,float color[3])
{
    //update gear
    gear* player_gear=(*m_pVec_players)[player_id].get_gear_ptr();
    m_vpGear[player_id]=player_gear;

    switch(player_id)
    {
        case 0:
        {
            m_gear_color_p1[0]=color[0];
            m_gear_color_p1[1]=color[1];
            m_gear_color_p1[2]=color[2];
        }break;

        case 1:
        {
            m_gear_color_p2[0]=color[0];
            m_gear_color_p2[1]=color[1];
            m_gear_color_p2[2]=color[2];
        }break;

        case 2:
        {
            m_gear_color_p3[0]=color[0];
            m_gear_color_p3[1]=color[1];
            m_gear_color_p3[2]=color[2];
        }break;

        case 3:
        {
            m_gear_color_p4[0]=color[0];
            m_gear_color_p4[1]=color[1];
            m_gear_color_p4[2]=color[2];
        }break;
    }

    return true;
}

bool hud::set_player_trigger_value_left(int player_id,float thrust_power)
{
    m_vPlayer_trigger_value[player_id][0]=thrust_power;

    return true;
}

bool hud::set_player_trigger_value_right(int player_id,float thrust_power)
{
    m_vPlayer_trigger_value[player_id][1]=thrust_power;

    return true;
}

bool hud::set_draw_tutorial_text(int value,bool flag)
{
    if(value==-1)
    {
        m_draw_tutorial=flag;
    }
    else if(value==-2)
    {
        //convoy text
        m_draw_convoy_text=flag;
        m_convoy_text_fade_level=0.0;
    }
    else
    {
        //do not reshow if already completed
        if(m_tut_comleted[value] && flag) ;//ignore
        else m_draw_tut_text[value]=flag;

        //mission complete test
        if(!flag)
        {
            m_tut_comleted[value]=true;

            //force enemy mission if not discoverd any enemies and other missions are completed
            if(m_tut_comleted[tut_land] &&
               m_tut_comleted[tut_manual] &&
               m_tut_comleted[tut_fuel] &&
               !m_tut_comleted[tut_enemy] )
            {
                //start enemy (triggered if fuel is completed)
                m_draw_tut_text[tut_enemy]=true;
            }
        }
    }

    return true;
}

bool hud::get_draw_tutorial_text(int value)
{
    return m_draw_tut_text[value];
}

bool hud::set_mship_ptr(b2Body* mship_body_ptr)
{
    m_mship_body_ptr=mship_body_ptr;

    return true;
}

bool hud::set_player_ptr(b2Body* player_body_ptr)
{
    m_player_body_ptr=player_body_ptr;

    return true;
}

b2Body* hud::get_player_ptr(void)
{
    return m_player_body_ptr;
}

bool hud::set_draw_tutorial_helptext(int value,bool flag)
{
    //special case, activate show text
    if( (value==ht_takeoff || value==ht_hook_off || value==ht_fuel_container ||
         value==ht_drone_eject || value==ht_map_center) && !flag)
    {
        m_draw_tut_moretext[value]=true;

        if(value==ht_hook_off && m_text_hook_off_shown) return true;
        else if(value==ht_hook_off) m_text_hook_off_shown=true;

        if(value==ht_fuel_container && m_text_fuel_box_shown) return true;
        else if(value==ht_fuel_container) m_text_fuel_box_shown=true;

        if(value==ht_drone_eject)
        {
            m_tut_moretext_fade_level[ht_drone_eject]=0.0;
            m_tut_more_comleted[ht_drone_eject]=false;
            m_tut_moretext_time_left[ht_drone_eject]=0.0;
        }

        if(value==ht_map_center)
        {
            m_tut_moretext_fade_level[ht_takeoff]=0.0;
            m_tut_moretext_time_left[ht_takeoff]=0.0;
            m_draw_tut_moretext[ht_takeoff]=false;
        }

        return true;
    }

    //do not reshow if already completed
    if(m_tut_more_comleted[value] && flag)
    {
        /*//dont ignore this
        if(value==ht_eject)
        {
            m_tut_more_comleted[value]=flag;
        }*/

        return false;//ignore if already done
    }
    else
    {
        m_tut_more_comleted[value]=flag;
        //turn of text if showing now
        if(flag)
         m_draw_tut_moretext[value]=false;

        //unshow drone takeover
        if(value==ht_drone_takeover && !flag)
        {
            m_draw_tut_moretext[ht_drone_takeover]=false;
            m_tut_moretext_fade_level[ht_drone_takeover]=0.0;
            m_tut_more_comleted[ht_drone_takeover]=false;
            m_tut_moretext_time_left[ht_drone_takeover]=0.0;
        }

        /*//fast unshow takeoff message
        if(value==ht_takeoff && flag)
        {
            m_draw_tut_moretext[ht_takeoff]=false;
            m_tut_moretext_fade_level[ht_takeoff]=0.0;
            m_tut_moretext_time_left[ht_takeoff]=0.0;
        }*/
    }

    return true;
}

bool hud::get_draw_tutorial_helptext(int value)
{
    return m_draw_tut_moretext[value];
}

bool hud::is_helptext_completed(int mission_ind)
{
    //specific test
    if(mission_ind!=-1)
    {
        return m_tut_more_comleted[mission_ind];
    }

    //all test
    for(int i=0;i<25;i++)
    {
        if(!m_tut_more_comleted[i]) return false;
    }
    if(!m_tut_more_comleted[ht_bring_back_ship] || !m_draw_tut_moretext[ht_salvaged_ship]) return false;

    return true;
}

bool hud::show_out_of_fuel(float pos[2])
{
    if(m_text_out_of_fuel_shown) return false;

    m_text_out_of_fuel_pos[0]=pos[0];
    m_text_out_of_fuel_pos[1]=pos[1];

    m_show_text_out_of_fuel=true;
    m_text_out_of_fuel_shown=true;

    return true;
}

bool hud::show_out_of_hp(float pos[2])
{
    if(m_text_out_of_hp_shown) return false;

    m_text_out_of_hp_pos[0]=pos[0];
    m_text_out_of_hp_pos[1]=pos[1];

    m_show_text_out_of_hp=true;
    m_text_out_of_hp_shown=true;

    return true;
}

bool hud::draw_selected_item(int item_type,int player_ind)
{
    m_draw_selected_item_timer[player_ind]=_hud_item_draw_time;
    m_draw_selected_item_type[player_ind]=item_type;

    return true;
}
