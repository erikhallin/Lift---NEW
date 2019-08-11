#include "game.h"

game::game()
{
    m_game_state=gs_init;
}

bool game::pre_init(int screen_size[2])
{
    string s_decode=base64_decode( load_base64_file(file_texture_loading) );
    unsigned char* texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    int tex_loading = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
    );

    /*//load image from file
    int tex_loading=SOIL_load_OGL_texture
	(
		"texture\\loading.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);*/

	if(tex_loading==0)
	{
	    cout<<"ERROR: Could not load texture\n";
	    return false;
	}

    //draw image
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex_loading);
    glColor3f(1,1,1);
    glTranslatef( ((float)screen_size[0]-318.0)*0.5, ((float)screen_size[1]-86.0)*0.5, 0.0  );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0,1.0);
    glVertex2f(0.0,0.0);
    glTexCoord2f(0.0,0.0);
    glVertex2f(0.0,86.0);
    glTexCoord2f(1.0,0.0);
    glVertex2f(318.0,86.0);
    glTexCoord2f(1.0,1.0);
    glVertex2f(318.0,0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    return true;
}

bool game::init(bool keys[256],int screen_size[2],bool test_level,bool load_sound_and_texture)
{
    cout<<"\nInitializing Game\n";
    srand(time(0));

    m_time_this_cycle=m_time_last_cycle=(float)clock()/(float)CLOCKS_PER_SEC;
    m_fps_frame_count=0.0;
    m_fps_measure_interval=2.0;
    m_fps_measured_time=0.0;

    m_pKeys=keys;
    m_screen_width=screen_size[0];
    m_screen_height=screen_size[1];
    m_level_loaded=false;
    m_player_input_enabled=true;
    m_mship_landed=false;
    m_key_trigger_keyboard=m_key_trigger_n=m_key_trigger_b=m_key_trigger_tab=false;
    m_last_played_level=-1;
    m_master_player_controller=0;//TEMP
    m_run_test_level=test_level;
    m_on_tutorial_level=!test_level;
    m_screen_fade_prog=0.99;
    m_waiting_for_convoy_screen_fade_level=m_waiting_for_convoy_text_fade_level=0.0;
    m_waiting_for_convoy=false;
    m_waiting_for_convoy_text_time=3.0;//5 sec
    m_waiting_for_convoy_text_timer=0.0;
    m_waiting_for_convoy_screen_timer=0.0;
    m_convoy_phase=cs_off;
    m_fade_off=false;
    m_show_manual=false;
    m_planets_visited_counter=0;
    m_level_highest_point=0.0;
    m_cam_enemy_to_follow=0;
    m_show_info=false;
    m_info_screen_phase=is_off;
    m_screen_info_fade_level=0.0;
    m_show_lost=false;
    m_lost_screen_phase=is_off;
    m_lost_screen_fade_level=0.0;
    m_show_gameover=false;
    m_gameover_screen_phase=is_off;
    m_gameover_screen_fade_level=0.0;
    m_tutorial_test_timer=_tutorial_test_time;
    m_fuel_curr=_mship_fuel_start;
    m_info_shown=false;
    m_mship_led_prog=0.0;
    m_music_source_id=30;
    m_music_intro_playing=true;
    m_music_id_playing=ogg_music0_intro;
    m_key_trigger_m=false;
    m_key_trigger_esc=true;//true to avoid direct quit after restart in menu
    m_music_on=true;
    m_music_was_off_at_level_start=false;
    m_sound_alarm_delay=10.0,
    m_sound_alarm_delay_timer=0;
    m_player_stuck[0]=m_player_stuck[1]=m_player_stuck[2]=m_player_stuck[3]=false;
    m_draw_stuck_timer=0;
    m_save_filename="save.dat";
    m_save_key="n3DsaOd8273r6wb37bskj32h";
    m_load_game=false;
    m_training_state=ts_ask;
    //test if a save is present
    ifstream save_file(m_save_filename.c_str());
    if(save_file!=0)
    {
        save_file.close();
        cout<<"A save file have been found\n";
        m_save_file_present=true;
    }
    else m_save_file_present=false;

    //values to be reset for each level
    m_cam_pos[0]=0.0;
    m_cam_pos[1]=0.0;
    m_cam_speed[0]=0.0;
    m_cam_speed[1]=0.0;
    m_world_max_x=0.0;
    m_world_offside_limit=0.0;
    m_came_mode=cm_follow_one;//who to follow
    //if(test_level) m_came_mode=cm_follow_none;
    m_cam_player_to_follow=0;
    m_level_soft_borders[0]=-1.0;//left soft border
    m_level_soft_borders[1]= 1.0;//right soft border
    m_level_hard_borders[0]=-2.0;//left hard border
    m_level_hard_borders[1]= 2.0;//right hard border
    m_level_static_fade_distance=m_screen_width*0.5;
    m_level_sky_height=1000;
    m_id_counter_object=0;
    m_pEvent_flag_hook[0]=m_pEvent_flag_hook[1]=m_pEvent_flag_hook[2]=m_pEvent_flag_hook[3]=ev_idle;
    m_pMship_landing_sensor_flag[0]=m_pMship_landing_sensor_flag[1]=m_pMship_landing_sensor_flag[2]=m_pMship_landing_sensor_flag[3]=false;

    //init particle engine
    m_pParticle_engine=new particle_engine();
    m_pParticle_engine->init();

    //init gamepads and players
    for(int i=0;i<4;i++)
    {
        m_input_reroute[i]=i;
        m_gamepad[i]=gamepad(i);
        if( m_gamepad[i].IsConnected() )
         m_gamepad_connected[i]=true;
        else
         m_gamepad_connected[i]=false;

        m_player_active[i]=false;
    }

    //load sounds and texture, and some allocated variables
    if(load_sound_and_texture)
    {
        //load sounds
        if(!load_sounds())
        {
            cout<<"ERROR: Could not load sounds\n";
            return false;
        }

        //load textures
        if(!load_textures())
        {
            cout<<"ERROR: Could not load textures\n";
            return false;
        }

        //variable allocation
        m_pEvent_flag_input_box=new bool();
        m_pEvent_flag_landing_gear=new bool[2];
    }
    //enable sounds
    m_pSound->enable_sound(true);
    //set sound loops
    set_sound_loops();
    //start music
    m_pSound->stopSound(m_music_source_id); //stop if other music was playing on this channel
    m_pSound->playSimpleSound(ogg_music0_intro,1.0,m_music_source_id);

    //load box2d/level TEMP (for loading menu world)
    if( !init_box2d() )
    {
        cout<<"ERROR: Could not initialize Box2D\n";
        return false;
    }
    else m_level_loaded=true;

    /*//extra weapon temp
    for(int i=2;i<=wt_mine;i++)
     m_vec_pWeapon_stored.push_back( new weapon(m_pWorld,i,1.0) );

    //extra gear temp
    for(int i=1;i<gt_cam_control;i++)
     m_vec_pGear_stored.push_back( new gear(m_pWorld,i,1.0) );*/

    //init starmap
    if(!m_Starmap.init(screen_size,m_tex_menu,m_pSound))
    {
        cout<<"ERROR: Could not initialize starmap\n";
        return false;
    }
    //set start planet to tutorial level (-2)
    m_Starmap.set_planet_now_level_index(-2);

    //init HUD
    m_hud.init(m_vec_players,m_screen_width,m_screen_height,m_tex_decal,m_tex_menu,m_tex_text,m_cam_pos);
    //turn of tutorial if testlevel
    if(m_run_test_level) m_hud.set_draw_tutorial_text(-1,false);

    //gen stars (menu)
    int numof_stars=100;
    for(int i=0;i<numof_stars;i++)
    {
        int x_pos=rand()%m_screen_width;
        int y_pos=rand()%(m_screen_height);
        float light=float(rand()%500)/1000.0+0.5;

        m_vec_stars.push_back( st_int_int_float(x_pos,y_pos,light) );
    }
    //20% extra low
    numof_stars=int(float(numof_stars)*0.1);
    for(int i=0;i<numof_stars;i++)
    {
        int x_pos=rand()%m_screen_width;
        int y_pos=(rand()%int((float)m_screen_height*0.30))+m_screen_height;
        float light=float(rand()%500)/1000.0+0.2;

        m_vec_stars.push_back( st_int_int_float(x_pos,y_pos,light) );
    }


    m_game_state=gs_menu;
    //m_game_state=gs_level_select;
    if(m_run_test_level) m_game_state=gs_in_level;

    cout<<"Game Initialization Complete\n\n";

    //cout<<"Unload level test\n";//TEMP
    //unload_level();

    return true;
}

bool game::update(bool& quit_flag)
{
    m_time_last_cycle=m_time_this_cycle;//store time for last cycle
    m_time_this_cycle=(float)clock()/(float)CLOCKS_PER_SEC;//get time now, in sec
    float time_dif=m_time_this_cycle-m_time_last_cycle;

    //static timedif, update will only be called after 20ms have passed
    time_dif=_world_step_time;

    //quit test
    switch(m_game_state)
    {
        case gs_menu://quit directly if ESC
        {
            if(m_pKeys[27])
            {
                if(!m_key_trigger_esc)
                {
                    m_key_trigger_esc=true;
                    quit_flag=true;
                }
            }
            else m_key_trigger_esc=false;
        }break;

        case gs_level_select://can not quit from this state, go to menu
        {
            if(m_pKeys[27])
            {
                if(!m_key_trigger_esc)
                {
                    m_key_trigger_esc=true;

                    reset();
                    return false;
                }
            }
            else m_key_trigger_esc=false;
        }break;

        case gs_training://can not quit from this state, go to menu
        {
            if(m_pKeys[27])
            {
                if(!m_key_trigger_esc)
                {
                    m_key_trigger_esc=true;

                    reset();
                    return false;
                }
            }
            else m_key_trigger_esc=false;
        }break;

        case gs_in_level://quit if in manual view
        {
            //quit game if ESC is pressed
            if(m_show_manual || m_show_lost)
            {
                if(m_pKeys[27])
                {
                    //reduce countdown timer to quit
                    m_vec_players[0].m_key_hold_time_start-=time_dif*2.0;//assume player 1
                    if(m_vec_players[0].m_key_hold_time_start<0.0)
                    {
                        m_vec_players[0].m_key_hold_time_start=_player_mship_control_key_timer;
                        //cout<<"Quit game\n";
                        //quit_flag=true;//quit game
                        m_key_trigger_esc=true;

                        reset();
                        return false;//to menu
                    }

                    if(!m_key_trigger_esc && m_show_lost)//one press
                    {
                        m_key_trigger_esc=true;
                        //quit_flag=true;//quit game
                        reset();
                        return false;
                    }
                }
                else
                {
                    m_key_trigger_esc=false;

                    //reduce timer
                    if(m_vec_players[0].m_key_hold_time_start<=_player_mship_control_key_timer)
                    {
                        m_vec_players[0].m_key_hold_time_start+=time_dif;
                        if(m_vec_players[0].m_key_hold_time_start>_player_mship_control_key_timer)
                         m_vec_players[0].m_key_hold_time_start=_player_mship_control_key_timer;
                    }
                }
            }
            else//show manual with esc
            {
                //show manual by esc
                if(m_pKeys[27])
                {
                    if(!m_key_trigger_esc && m_screen_fade_prog==0.0)//one press
                    {
                        m_key_trigger_esc=true;
                        m_show_manual=true;
                    }
                }
                else m_key_trigger_esc=false;
            }
        }break;

        case gs_game_over://quit directly if ESC
        {
            if(m_pKeys[27]) quit_flag=true;
        }break;
    }

    /*//fps calc
    m_fps_measured_time+=time_dif;
    m_fps_frame_count+=1;
    if(m_fps_measured_time>m_fps_measure_interval)
    {
        //display FPS
        float fps=m_fps_frame_count/m_fps_measured_time;
        cout<<"Updates per sec: "<<fabs(fps)<<endl;
        m_fps_measured_time=0.0;
        m_fps_frame_count=0.0;
    }*/

    //time lock, assume never to reach more than 1000 FPS to avoid negative time_dif (>100 should not be possible)
    //if(time_dif<0.001) time_dif=0.001;
    //cout<<"Time dif: "<<time_dif<<endl;

    //music intro test
    if(m_music_intro_playing && m_music_on)
    {
        //test if end of intro
        if( m_pSound->get_source_status(m_music_source_id) )
        {
            m_music_intro_playing=false;
            //play loop part
            m_pSound->playSimpleSound(m_music_id_playing+1,1.0,m_music_source_id,true);
            //cout<<"Music loop start\n";
        }
    }

    //toogle music, m
    if(m_pKeys[77])
    {
        if(!m_key_trigger_m)
        {
            m_key_trigger_m=true;
            //toogle music
            m_music_on=!m_music_on;
            if(m_music_on)
            {
                m_pSound->resume_source(m_music_source_id);
                m_pSound->enable_music(true);

                if(m_music_was_off_at_level_start)
                {
                    m_music_was_off_at_level_start=false;

                    //start music
                    bool play_loop=!m_music_intro_playing;
                    switch(m_game_state)
                    {
                        case gs_menu:         m_pSound->playSimpleSound(m_music_id_playing,1.0,m_music_source_id,play_loop); break;
                        case gs_level_select: m_pSound->playSimpleSound(ogg_starmap_noise,1.0,m_music_source_id,true);       break;
                        case gs_in_level:     m_pSound->playSimpleSound(m_music_id_playing,1.0,m_music_source_id,play_loop); break;
                        //dont start the music in game over state
                    }

                }

            }
            else
            {
                m_pSound->pause_source(m_music_source_id);
                m_pSound->enable_music(false);
            }
        }
    }
    else m_key_trigger_m=false;

    //alarm timer
    if(m_sound_alarm_delay_timer>0) m_sound_alarm_delay_timer-=time_dif;

    /*//TEMP keyboard input
    b2Body* player_body_temp=m_vec_players[0].get_body_ptr();
    if(m_pKeys[38])//up
    {
        player_body_temp->ApplyForce( b2Vec2(0,-10), player_body_temp->GetWorldCenter(), true );
    }
    if(m_pKeys[40])//down
    {
        player_body_temp->ApplyForce( b2Vec2(0,10), player_body_temp->GetWorldCenter(), true );
    }
    if(m_pKeys[37])//left
    {
        float magnitude=500.0*time_dif;
        b2Vec2 force = b2Vec2( cos(player_body_temp->GetAngle()-_pi*0.5) * magnitude , sin(player_body_temp->GetAngle()-_pi*0.5) * magnitude );
        player_body_temp->ApplyForce(force, player_body_temp->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
        //add particle to engine
        m_pParticle_engine->add_particle(m_vec_players[0].get_body_ptr()->GetWorldPoint( b2Vec2(-0.4,0.3) ),force);
    }
    if(m_pKeys[39])//right
    {
        float magnitude=500.0*time_dif;
        b2Vec2 force = b2Vec2( cos(player_body_temp->GetAngle()-_pi*0.5) * magnitude , sin(player_body_temp->GetAngle()-_pi*0.5) * magnitude );
        player_body_temp->ApplyForce(force, player_body_temp->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
        //add particle to engine
        m_pParticle_engine->add_particle(m_vec_players[0].get_body_ptr()->GetWorldPoint( b2Vec2(0.4,0.3) ),force);
    }

    if(m_pKeys[45])//ins
    {
        player_body_temp->ApplyTorque( 0.80, true );
    }
    if(m_pKeys[46])//del
    {
        player_body_temp->ApplyTorque( -0.80, true );
    }

    //move cam with numpad
    if(m_pKeys[ 98]) m_cam_pos[1]+=10.0;
    if(m_pKeys[100]) m_cam_pos[0]-=10.0;
    if(m_pKeys[102]) m_cam_pos[0]+=10.0;
    if(m_pKeys[104]) m_cam_pos[1]-=10.0;

    //select enemy/player to follow
    if(m_pKeys[66])//b
    {
        if(!m_key_trigger_b)
        {
            m_key_trigger_b=true;
            //enemy
            m_cam_enemy_to_follow--;
            if(m_cam_enemy_to_follow<0) m_cam_enemy_to_follow=m_vec_pEnemies.size()-1;
            //player
            m_cam_player_to_follow--;
            if(m_cam_player_to_follow<0) m_cam_player_to_follow=m_vec_players.size()-1;
        }
    }
    else m_key_trigger_b=false;
    if(m_pKeys[78])//n
    {
        if(!m_key_trigger_n)
        {
            m_key_trigger_n=true;
            //enamy
            m_cam_enemy_to_follow++;
            if(m_cam_enemy_to_follow>=(int)m_vec_pEnemies.size() ) m_cam_enemy_to_follow=0;
            //player
            m_cam_player_to_follow++;
            if(m_cam_player_to_follow>=(int)m_vec_players.size() ) m_cam_player_to_follow=0;
        }
    }
    else m_key_trigger_n=false;

    //select view type
    if(m_pKeys[9])//tab
    {
        if(!m_key_trigger_tab)
        {
            m_key_trigger_tab=true;
            m_came_mode++;
            if(m_came_mode>cm_follow_none) m_came_mode=cm_follow_one;

            cout<<"Cam mode: "<<m_came_mode<<endl;
        }
    }
    else m_key_trigger_tab=false;*/

    //get gamepad data
    st_gamepad_data gamepad_data[4];
    st_gamepad_data gamepad_data_copy[4];
    for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
    {
        if( m_gamepad[gamepad_i].IsConnected() )
        {
            //test if new connection
            if(!m_gamepad_connected[gamepad_i])
            {
                //test if that player slot is unused (inactive)
                //if other players have used this slot, the controls would not react if not resetted
                if(!m_player_active[gamepad_i])
                {
                    m_input_reroute[ gamepad_i ]=gamepad_i;
                    cout<<"Input reroute: Control "<<m_input_reroute[ gamepad_i ]+1<<" is now controlling ship "<<gamepad_i+1<<endl;
                }
            }

            m_gamepad_connected[gamepad_i]=true;
            gamepad_data_copy[gamepad_i]=gamepad_data[gamepad_i]=m_gamepad[gamepad_i].GetState();


        }
        else//lost controller
        {
            if( m_gamepad_connected[gamepad_i] )//had connection and lost it
            {
                m_gamepad_connected[gamepad_i]=false;
                cout<<"Lost connection to controller: "<<gamepad_i+1<<endl;
            }
        }
    }

    /*//cycle active gamepad, use gamepad 1 to control other players, TEMP
    if(m_pKeys[13])//enter
    {
        if(!m_key_trigger_keyboard && m_game_state==gs_in_level)
        {
            m_key_trigger_keyboard=true;
            //next player to control
            m_master_player_controller++;
            if(m_master_player_controller>3) m_master_player_controller=0;
            cout<<"Player 1 now controls player: "<<m_master_player_controller+1<<endl;

            //TEMP force convoy
            m_hud.set_draw_tutorial_text(-2,true);
            m_waiting_for_convoy=true;
        }
    }
    else m_key_trigger_keyboard=false;*/

    //swap gamepad data
    if(m_master_player_controller!=0)
    {
        st_gamepad_data gamepad_empty=gamepad_data[m_master_player_controller];
        gamepad_data[m_master_player_controller]=gamepad_data[0];
        gamepad_data[0]=gamepad_empty;
    }

    //input rerouting
    for(int i=0;i<4;i++)
    {
        if(m_input_reroute[i]!=i)
        {
            //cout<<"input reroute: Control "<<m_input_reroute[i]+1<<" to player "<<i+1<<endl;
            gamepad_data[i]=gamepad_data_copy[ m_input_reroute[i] ];

            //clear input from other controller, if were to join (wrong, could be in use for other player)
            //gamepad_data[m_input_reroute[i]]=st_gamepad_data();
        }
    }

    //state dependent update
    switch(m_game_state)
    {
        case gs_menu:
        {
            float view_pos[4]={m_cam_pos[0],m_cam_pos[1],
                               m_cam_pos[0]+m_screen_width,m_cam_pos[1]+m_screen_height};//top left, down right

            //update particle engine
            m_pParticle_engine->update(time_dif);

            //handle gamepad input to add/control players and to start game
            //update gamepad input
            for(int player_i=0;player_i<4;player_i++)
            {
                if(!m_gamepad_connected[player_i]) continue;//dont read from disconnected gamepad

                //if player not active, stop here
                //if(!m_player_active[player_i]) continue;

                //if START, go to tutorial level
                if(gamepad_data[player_i].button_start)
                {
                    if(!m_vec_players[player_i].m_key_trigger_start && !m_fade_off)
                    {
                        m_vec_players[player_i].m_key_trigger_start=true;
                        m_fade_off=true;
                        m_load_game=false;
                    }
                }
                else m_vec_players[player_i].m_key_trigger_start=false;

                //if BACK, try to load saved data
                if(gamepad_data[player_i].button_back)
                {
                    if(!m_vec_players[player_i].m_key_trigger_start && !m_fade_off)
                    {
                        m_vec_players[player_i].m_key_trigger_start=true;
                        //test if a save file exists
                        ifstream save_file(m_save_filename.c_str());
                        if(save_file!=0)
                        {
                            save_file.close();
                            //save file exists, raise flag to load data
                            m_load_game=true;
                            m_fade_off=true;
                        }


                    }
                }
                else m_vec_players[player_i].m_key_trigger_start=false;
            }

            /*//master input, keyboard
            if(m_pKeys[13])//enter
            {
                if(!m_key_trigger_keyboard && m_screen_fade_prog<1.0)
                {
                    m_key_trigger_keyboard=true;
                    //go to level select
                    m_fade_off=true;
                }
            }
            else m_key_trigger_keyboard=false;*/

            /*//key arrows cam move
            if(m_pKeys[37])
            {
                m_cam_pos[0]-=10.0;
            }
            if(m_pKeys[38])
            {
                m_cam_pos[1]-=10.0;
            }
            if(m_pKeys[39])
            {
                m_cam_pos[0]+=10.0;
            }
            if(m_pKeys[40])
            {
                m_cam_pos[1]+=10.0;

                cout<<"cam_pos: "<<m_cam_pos[0]<<"\t"<<m_cam_pos[1]<<endl;
            }*/

            //update cam pos
            if(m_screen_fade_prog<1.0 && m_fade_off)//move cam up
            {
                //linear
                m_cam_pos[1]-=1.5;
            }
            else//cam goes down
            {
                if(m_cam_pos[1]<850)
                {
                    //linear
                    if(m_cam_pos[1]<500)
                     m_cam_pos[1]+=0.5;
                    //exp
                    else
                    {
                        float diff=(850.0-m_cam_pos[1])*0.002;
                        //if(diff<0.1) diff=0.1;
                        m_cam_pos[1]+=diff;
                    }
                }
            }
            //rotating motion
            float rot_radius=0.05;
            float rot_speed=0.1;
            m_cam_pos[0]+=cosf(m_time_this_cycle*rot_speed)*rot_radius;
            m_cam_pos[1]+=sinf(m_time_this_cycle*rot_speed*0.5)*rot_radius;

            for(int player_i=0;player_i<4;player_i++)
            {
                /*if(!m_gamepad_connected[player_i] && m_master_player_controller==0)
                 continue;//dont read from disconnected gamepad, unless master control*/

                //reset key back
                if(m_vec_players[player_i].m_key_trigger_back && !gamepad_data[player_i].button_back)
                 m_vec_players[player_i].m_key_trigger_back=false;

                if(!m_player_input_enabled) break;//player input not read
                if(m_vec_players[player_i].get_rel_hp()<=0.0) continue;//no input if no HP

                //if player not active, activation test
                if(!m_player_active[player_i])
                {
                    //activate by pushing A,Y,X,B,LB,RB,START,BACK,DPAD
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                       gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                       /*gamepad_data[player_i].button_start ||*/ gamepad_data[player_i].button_back ||
                       gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                       gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up )
                    {
                        //spawn from top of screen
                        m_player_active[player_i]=true;
                        //disconnect from mship
                        m_pMain_ship->player_inside(player_i,false);
                        m_vec_players[player_i].disconnect_from_mship();
                        //move to top of the screen
                        b2Body* body_ptr=m_vec_players[player_i].get_body_ptr();
                        b2Vec2 new_pos(0,0);
                        switch(player_i)
                        {
                            case 0: new_pos.Set( 95,m_cam_pos[1]*_Pix2Met-0.5); break;
                            case 1: new_pos.Set(105,m_cam_pos[1]*_Pix2Met-0.5); break;
                            case 2: new_pos.Set(115,m_cam_pos[1]*_Pix2Met-0.5); break;
                            case 3: new_pos.Set(125,m_cam_pos[1]*_Pix2Met-0.5); break;
                        }
                        body_ptr->SetTransform( new_pos,0.0 );
                    }
                    else continue;//player not active
                }

                //get body
                b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                //st_body_user_data* player_data=(st_body_user_data*)player_body->GetUserData();

                //force hook led on
                if(gamepad_data[player_i].button_Y && !m_vec_players[player_i].is_hook_off())
                 m_vec_players[player_i].m_force_led_on=true;

                //rope control
                //no dock or rope control if cloaked
                if(m_vec_players[player_i].m_cloak_timer==m_vec_players[player_i].m_cloak_delay &&
                   m_vec_players[player_i].m_cloak_target_off)
                {
                    //rope control, not if connected to mship
                    if(gamepad_data[player_i].dpad_up ^ gamepad_data[player_i].dpad_down &&
                       !m_vec_players[player_i].connected_to_mship() )
                    {
                        if(gamepad_data[player_i].dpad_down) m_vec_players[player_i].set_rope_motor(_rope_motor_strength*time_dif);
                        else                                 m_vec_players[player_i].set_rope_motor(-_rope_motor_strength*time_dif);


                        //play sound
                        int rope_length=m_vec_players[player_i].get_rope_length();
                        //cout<<rope_length<<endl;
                        if( (m_vec_players[player_i].get_rope_lock_status() && gamepad_data[player_i].dpad_up) ||
                            (rope_length>=_player_rope_length_max && gamepad_data[player_i].dpad_down) )
                        {
                            ;//dont play sound
                        }
                        else//play
                        {
                            //calc sound area
                            b2Vec2 pos=player_body->GetPosition();
                            int sound_box=0;//sound off
                            float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                            if(pos.x*_Met2Pix>view_pos[0] &&
                               pos.x*_Met2Pix<view_pos[2] &&
                               pos.y*_Met2Pix>view_pos[1] &&
                               pos.y*_Met2Pix<view_pos[3] )
                            {
                                sound_box=1;//on screen
                            }
                            else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                    pos.x*_Met2Pix<view_pos[0] &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=2;//left side
                            }
                            else if(pos.x*_Met2Pix>view_pos[2] &&
                                    pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=3;//right side
                            }
                            else if(pos.x*_Met2Pix>view_pos[0] &&
                                    pos.x*_Met2Pix<view_pos[2] &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[1] )
                            {
                                sound_box=4;//top side
                            }
                            else if(pos.x*_Met2Pix>view_pos[0] &&
                                    pos.x*_Met2Pix<view_pos[2] &&
                                    pos.y*_Met2Pix>view_pos[3] &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=5;//top side
                            }

                            if(sound_box!=0)
                            {
                                float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                    0,1,0, 0,0,-1, 0,0,0,
                                                    1,  1,  0};
                                switch(sound_box)
                                {
                                    case 0: break;//no sound
                                    case 1:
                                    {
                                        sound_data[14]=0;
                                    }break;
                                    case 2://left
                                    {
                                        sound_data[12]=-_sound_box_side_shift;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 3://right
                                    {
                                        sound_data[12]=_sound_box_side_shift;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 4:
                                    {
                                        sound_data[12]=0;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 5:
                                    {
                                        sound_data[12]=0;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                }

                                //test if already playing
                                if( m_pSound->get_volume(_sound_chan_motor_rope) < sound_data[19] )
                                m_pSound->updateSound(_sound_chan_motor_rope,sound_data);
                            }
                        }
                    }
                    else m_vec_players[player_i].set_rope_motor(0.0);//turn rope motor off

                    //hook/land control
                    if(gamepad_data[player_i].button_Y)//activate hook sensor (hold)
                    {
                        if(!m_vec_players[player_i].player_hook_connected())//if not already connected to any object
                        {
                            //test if hook is connectable to objects
                            if(m_pEvent_flag_hook[player_i]==ev_connectable)
                            {
                                //cout<<"Hook connectable\n";
                                //make connection
                                if(m_vec_players[player_i].hook_connect(m_ppBody_to_connect[player_i]))//connection made if true
                                {
                                    //cout<<"Hook connection made\n";
                                    //notify carried body
                                    st_body_user_data* data=(st_body_user_data*)m_ppBody_to_connect[player_i]->GetUserData();
                                    data->b_is_carried=true;
                                    data->bp_carrier_body=player_body;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_hook_connect,1.0);
                                }
                                m_pEvent_flag_hook[player_i]=ev_idle;//reset
                            }
                            //else cout<<"Hook not connectable\n";
                        }
                    }

                    //disconnect hook/disconnect from main ship
                    if(gamepad_data[player_i].button_B && !gamepad_data[player_i].button_Y)
                    {
                        //if hook connected, disconnect
                        if(m_vec_players[player_i].player_hook_connected())//only if connected
                        {
                            //destroy hook joint
                            m_vec_players[player_i].hook_disconnect();

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,1.0);
                        }
                    }
                }

                //thrusters
                if(gamepad_data[player_i].button_left_trigger)
                {
                    //linear sens
                    float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                               (float)gamepad_data[player_i].trigger_left/255.0*time_dif;

                    b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                    player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                    //add particle to engine
                    m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.3,0.3) ),force);
                }
                if(gamepad_data[player_i].button_right_trigger)
                {
                    //linear sens
                    float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                               (float)gamepad_data[player_i].trigger_right/255.0*time_dif;

                    b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                    player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                    //add particle to engine
                    m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.3,0.3) ),force);
                }
                //report trigger value to HUD
                m_hud.set_player_trigger_value_left (player_i,(float)gamepad_data[player_i].trigger_left/255.0);
                m_hud.set_player_trigger_value_right(player_i,(float)gamepad_data[player_i].trigger_right/255.0);
                //set motor sound
                if(gamepad_data[player_i].button_left_trigger || gamepad_data[player_i].button_right_trigger)
                {
                    //play sound

                    //calc sound area
                    b2Vec2 pos=player_body->GetPosition();
                    int sound_box=0;//sound off
                    float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                    if(pos.x*_Met2Pix>view_pos[0] &&
                       pos.x*_Met2Pix<view_pos[2] &&
                       pos.y*_Met2Pix>view_pos[1] &&
                       pos.y*_Met2Pix<view_pos[3] )
                    {
                        sound_box=1;//on screen
                    }
                    else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                            pos.x*_Met2Pix<view_pos[0] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=2;//left side
                    }
                    else if(pos.x*_Met2Pix>view_pos[2] &&
                            pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=3;//right side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[1] )
                    {
                        sound_box=4;//top side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[3] &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=5;//top side
                    }

                    if(sound_box!=0)
                    {
                        float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                            0,1,0, 0,0,-1, 0,0,0,
                                            1,  1,  0};
                        switch(sound_box)
                        {
                            case 0: break;//no sound
                            case 1:
                            {
                                sound_data[14]=0;
                            }break;
                            case 2://left
                            {
                                sound_data[12]=-_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 3://right
                            {
                                sound_data[12]=_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 4:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 5:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                        }

                        int channel_id=0;
                        switch(player_i)
                        {
                            case 0: channel_id=_sound_chan_motor_p1; break;
                            case 1: channel_id=_sound_chan_motor_p2; break;
                            case 2: channel_id=_sound_chan_motor_p3; break;
                            case 3: channel_id=_sound_chan_motor_p4; break;
                        }
                        float sound_volume=((float)gamepad_data[player_i].trigger_left+
                                            (float)gamepad_data[player_i].trigger_right)/510.0;
                        //if( gamepad_data[player_i].trigger_left<gamepad_data[player_i].trigger_right )
                        // sound_volume=(float)gamepad_data[player_i].trigger_right/255.0;
                        float sound_pitch=0.5+sound_volume*0.5;
                        sound_data[18]=sound_pitch;
                        sound_data[19]*=sound_volume;

                        m_pSound->updateSound(channel_id,sound_data);
                    }
                }
                else//stop sound
                {
                    int channel_id=0;
                    switch(player_i)
                    {
                        case 0: channel_id=_sound_chan_motor_p1; break;
                        case 1: channel_id=_sound_chan_motor_p2; break;
                        case 2: channel_id=_sound_chan_motor_p3; break;
                        case 3: channel_id=_sound_chan_motor_p4; break;
                    }
                    float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                          0,1,0, 0,0,0, 0,0,0,
                                          1,  0,  0};
                    m_pSound->updateSound(channel_id,sound_data);
                }

                //fire
                if(gamepad_data[player_i].button_A)
                {
                    if( m_vec_players[player_i].fire_turret(time_dif,false) )//no ammo required
                    {
                        float pos[2]={m_vec_players[player_i].get_body_ptr()->GetPosition().x*_Met2Pix,
                                      m_vec_players[player_i].get_body_ptr()->GetPosition().y*_Met2Pix};
                        //play weapon specific sound
                        switch(m_vec_players[player_i].get_weapon_ptr()->get_type())
                        {
                            case wt_pea:     m_pSound->play_sound_w_test(wav_weapon_pea,pos); break;
                            case wt_spread:  m_pSound->play_sound_w_test(wav_weapon_spread,pos); break;
                            case wt_burst:   m_pSound->play_sound_w_test(wav_weapon_pea,pos); break;
                            case wt_rapid:   m_pSound->play_sound_w_test(wav_weapon_pea,pos); break;
                            case wt_rocket:  m_pSound->play_sound_w_test(wav_weapon_rocket,pos); break;
                            case wt_grenade: m_pSound->play_sound_w_test(wav_weapon_grenade,pos); break;
                            case wt_cannon:  m_pSound->play_sound_w_test(wav_weapon_cannon,pos); break;
                            case wt_beam:    m_pSound->play_sound_w_test(wav_weapon_laser,pos); break;
                            case wt_mine:    m_pSound->play_sound_w_test(wav_weapon_mine,pos); break;
                        }
                    }
                }

                //test if body is outside window
                b2Vec2 player_pos=player_body->GetPosition();
                //float player_angle=player_body->GetAngle();
                //float min_y=(m_cam_pos[1]+m_screen_height)*_Pix2Met+5.0;
                //float max_y=m_cam_pos[1]*_Pix2Met-5.0;
                float min_x=m_cam_pos[0]*_Pix2Met;
                float max_x=(m_cam_pos[0]+m_screen_width)*_Pix2Met;
                float wrap_border_size=2.0;
                //test height (off)
                /*if(player_pos.y<max_y)
                {
                    m_vec_players[player_i].shift_player_pos( player_pos.x, max_y, true );
                }
                else if(player_pos.y>min_y)//below screen (if shifted underground)
                {
                    m_vec_players[player_i].shift_player_pos( player_pos.x, max_y, true );
                }*/

                //test if within x-limits (move only if rope is in)
                if(player_pos.x<min_x-wrap_border_size && m_vec_players[player_i].get_rope_lock_status() )
                {
                    m_vec_players[player_i].shift_player_pos( min_x+m_screen_width*_Pix2Met+wrap_border_size,
                                                              player_pos.y, true );
                }
                else if(player_pos.x>max_x+wrap_border_size && m_vec_players[player_i].get_rope_lock_status() )
                {
                    m_vec_players[player_i].shift_player_pos( max_x-m_screen_width*_Pix2Met-wrap_border_size,
                                                              player_pos.y, true );
                }
            }

            //update players
            for(int player_i=0;player_i<4;player_i++)
            {
                m_vec_players[player_i].update(time_dif,view_pos);
            }

            //update objects
            for(int object_i=0;object_i<(int)m_vec_objects.size();object_i++)
            {
                m_vec_objects[object_i].update();
            }

            //remove collided projectiles
            if(!m_vec_projectiles_to_remove.empty())
            {
                //cout<<"Removing projectiles...";
                for(int proj_i=0;proj_i<(int)m_vec_projectiles_to_remove.size();proj_i++)
                {
                    st_body_user_data* data=(st_body_user_data*)m_vec_projectiles_to_remove[proj_i]->GetUserData();
                    b2Vec2 pos=m_vec_projectiles_to_remove[proj_i]->GetPosition();

                    //if rocket, grenade or mine
                    if(data->i_id==wt_rocket || data->i_id==wt_grenade || data->i_id==wt_mine)
                    {
                        //test if grenade should explode (not if timed or first bounce)
                        if(data->i_id==wt_grenade && data->i_subtype!=wst_impact && data->i_subtype!=wst_timed)
                        {
                            switch(data->i_subtype)
                            {
                                case wst_second_impact:
                                {
                                    //change to impact, will explode on next impact
                                    data->i_subtype=wst_impact;
                                }break;

                                case wst_second_timed:
                                {
                                    //change to timed
                                    data->i_subtype=wst_timed;
                                }break;
                            }

                            //will not explode this time
                            continue;
                        }

                        //add explosion
                        add_explotion(m_vec_projectiles_to_remove[proj_i]);
                    }

                    //add visual explosion
                    m_pParticle_engine->add_explosion(pos,20,300,0.2);

                    //play sound
                    if(true)
                    {
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(pos.x*_Met2Pix>view_pos[0] &&
                           pos.x*_Met2Pix<view_pos[2] &&
                           pos.y*_Met2Pix>view_pos[1] &&
                           pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                pos.x*_Met2Pix<view_pos[0] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(pos.x*_Met2Pix>view_pos[2] &&
                                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[3] &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            //select sound
                            if(data->i_id==wt_rocket || data->i_id==wt_grenade || data->i_id==wt_mine)
                            {
                                //rand sound feature
                                sound_data[18]=(float)(rand()%200)/500+0.6;
                                m_pSound->playWAVE(wav_bullet_explosion,sound_data);
                            }
                            else m_pSound->playWAVE(wav_bullet_hit,sound_data);
                        }
                    }


                    //delete data
                    //delete data;
                    //remove body (OLD, removed in master remover)
                    //m_pWorld->DestroyBody(m_vec_projectiles_to_remove[proj_i]);
                    //mark for removal
                    data->b_to_be_deleted=true;
                }
                //clear vector
                m_vec_projectiles_to_remove.clear();
                //cout<<"done\n";
            }

            //update screen fade
            if(m_screen_fade_prog>0.0 && !m_fade_off)
            {
                float fade_speed=0.15;
                m_screen_fade_prog-=time_dif*fade_speed;
                //done
                if(m_screen_fade_prog<0.0)
                {
                    m_screen_fade_prog=0.0;
                }
            }
            if(m_screen_fade_prog<1.0 && m_fade_off)
            {
                float fade_speed=0.2;
                m_screen_fade_prog+=time_dif*fade_speed;
                //done
                if(m_screen_fade_prog>1.0)
                {
                    m_screen_fade_prog=1.0;
                    m_fade_off=false;

                    //m_game_state=gs_level_select; go to menu

                    if(m_load_game)
                    {
                        load_game_from_file();
                        m_game_state=gs_level_select;
                        m_on_tutorial_level=false;
                        m_hud.set_draw_tutorial_text(-1,false);
                    }
                    else//create new game
                    {
                        //ask if training required
                        m_game_state=gs_training;
                    }
                }
            }

            //master destroy body, menu
            b2Body* tmp=m_pWorld->GetBodyList();
            while(tmp)
            {
                st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                if(data->b_to_be_deleted)
                {
                    b2Body* body_to_remove=tmp;
                    tmp=tmp->GetNext();
                    //no joint checks, could not be done in menu
                    delete data;
                    m_pWorld->DestroyBody(body_to_remove);
                }
                else tmp=tmp->GetNext();
            }

            m_pWorld->Step( _world_step_time,_world_velosity_it,_world_position_it );

        }break;

        case gs_training:
        {
            switch(m_training_state)
            {
                case ts_ask:
                {
                    //check for player reply
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        //if player not active, activation test
                        if(m_gamepad_connected[player_i] && m_screen_fade_prog<0.8)
                        {
                            //reply by pushing A or B
                            if( gamepad_data[player_i].button_A )
                            {
                                if( !m_vec_players[player_i].m_key_trigger_a )
                                {
                                    m_vec_players[player_i].m_key_trigger_a=true;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_gear_enable,1.0);

                                    //run tutorial
                                    m_training_state=ts_reply;
                                    m_screen_fade_prog=0.9;
                                    break;
                                }

                            }
                            else m_vec_players[player_i].m_key_trigger_a=false;

                            if( gamepad_data[player_i].button_B )
                            {
                                if( !m_vec_players[player_i].m_key_trigger_b )
                                {
                                    m_vec_players[player_i].m_key_trigger_b=true;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_gear_enable,1.0);

                                    //skip tutorial
                                    m_fade_off=true;
                                    m_on_tutorial_level=false;
                                    //turn off tutorial text in hud
                                    m_hud.set_draw_tutorial_text(-1,false);
                                    break;
                                }
                            }
                            else m_vec_players[player_i].m_key_trigger_b=false;


                            //XXX TEMP skip training with start
                            if( gamepad_data[player_i].button_start )
                            {
                                load_selected_level();
                                m_game_state=gs_in_level;
                                break;
                            }
                            //XXX
                        }
                    }
                }break;

                case ts_reply:
                {
                    //check for player reply
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        //if player not active, activation test
                        if(m_gamepad_connected[player_i] && !m_fade_off )
                        {
                            //test A
                            if( gamepad_data[player_i].button_A )
                            {
                                if( !m_vec_players[player_i].m_key_trigger_a )
                                {
                                    m_vec_players[player_i].m_key_trigger_a=true;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_gear_enable,1.0);

                                    //start the training
                                    m_fade_off=true;
                                    break;
                                }
                            }
                            else m_vec_players[player_i].m_key_trigger_a=false;

                            //test B
                            if( gamepad_data[player_i].button_B && !m_fade_off )
                            {
                                if( !m_vec_players[player_i].m_key_trigger_b )
                                {
                                    m_vec_players[player_i].m_key_trigger_b=true;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_gear_enable,1.0);

                                    //start the training
                                    m_fade_off=true;
                                    break;
                                }
                            }
                            else m_vec_players[player_i].m_key_trigger_b=false;

                            //test other keys
                            if( (gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                                 gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                                 gamepad_data[player_i].button_start || gamepad_data[player_i].button_back ||
                                 gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                                 gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up ) && !m_fade_off )
                            {
                                //play sound
                                m_pSound->playSimpleSound(wav_gear_enable,1.0);

                                //start the training
                                m_fade_off=true;
                                break;
                            }

                        }
                    }
                }break;

                case ts_rings_second:
                {
                    //test if complete
                    bool training_complete=true;
                    bool any_active_player=false;
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if( m_player_active[player_i] && m_gamepad_connected[player_i] && !m_fade_off )
                        {
                            any_active_player=true;

                            switch(player_i)
                            {
                                case 0: if( m_vec_training_rings_p1.back().val_f>0.0 ) training_complete=false; break;
                                case 1: if( m_vec_training_rings_p2.back().val_f>0.0 ) training_complete=false; break;
                                case 2: if( m_vec_training_rings_p3.back().val_f>0.0 ) training_complete=false; break;
                                case 3: if( m_vec_training_rings_p4.back().val_f>0.0 ) training_complete=false; break;
                            }
                        }
                    }
                    if(training_complete && any_active_player)
                    {
                        m_fade_off=true;
                    }
                }//no break, use same update

                case ts_rings_first:
                {
                    float view_pos[4]={m_cam_pos[0],m_cam_pos[1],
                                       m_cam_pos[0]+m_screen_width,m_cam_pos[1]+m_screen_height};//top left, down right
                    /*//rotating motion
                    float rot_radius=0.05;
                    float rot_speed=0.1;
                    m_cam_pos[0]+=cosf(m_time_this_cycle*rot_speed)*rot_radius;
                    m_cam_pos[1]+=sinf(m_time_this_cycle*rot_speed*0.5)*rot_radius;*/

                    //update particle engine
                    m_pParticle_engine->update(time_dif);

                    //player ship input
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        //if player not active, activation test
                        if(!m_player_active[player_i])
                        {
                            //activate by pushing A,Y,X,B,LB,RB,START,BACK,DPAD
                            if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                               gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                               gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                               gamepad_data[player_i].button_start || gamepad_data[player_i].button_back ||
                               gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                               gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up )
                            {
                                //cout<<"player activiated\n";

                                //spawn from top of screen
                                m_player_active[player_i]=true;
                                //disconnect from mship
                                m_pMain_ship->player_inside(player_i,false);
                                m_vec_players[player_i].disconnect_from_mship();
                                //move to top of the screen
                                b2Body* body_ptr=m_vec_players[player_i].get_body_ptr();
                                b2Vec2 new_pos(0,0);
                                switch(player_i)
                                {
                                    case 2: new_pos.Set( 95,m_cam_pos[1]*_Pix2Met-0.5); break;
                                    case 0: new_pos.Set(105,m_cam_pos[1]*_Pix2Met-0.5); break;
                                    case 1: new_pos.Set(115,m_cam_pos[1]*_Pix2Met-0.5); break;
                                    case 3: new_pos.Set(125,m_cam_pos[1]*_Pix2Met-0.5); break;
                                }
                                body_ptr->SetTransform( new_pos,0.0 );

                                //play sound
                                m_pSound->playSimpleSound(wav_enemy_ship_detected,1.0);
                            }
                            else continue;//player not active
                        }

                        //get body
                        b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                        //st_body_user_data* player_data=(st_body_user_data*)player_body->GetUserData();

                        //rope control
                        if(gamepad_data[player_i].dpad_up ^ gamepad_data[player_i].dpad_down &&
                           !m_vec_players[player_i].connected_to_mship() )
                        {
                            if(gamepad_data[player_i].dpad_down) m_vec_players[player_i].set_rope_motor(_rope_motor_strength*time_dif);
                            else                                 m_vec_players[player_i].set_rope_motor(-_rope_motor_strength*time_dif);


                            //play sound
                            int rope_length=m_vec_players[player_i].get_rope_length();
                            //cout<<rope_length<<endl;
                            if( (m_vec_players[player_i].get_rope_lock_status() && gamepad_data[player_i].dpad_up) ||
                                (rope_length>=_player_rope_length_max && gamepad_data[player_i].dpad_down) )
                            {
                                ;//dont play sound
                            }
                            else//play
                            {
                                //calc sound area
                                b2Vec2 pos=player_body->GetPosition();
                                int sound_box=0;//sound off
                                float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                                if(pos.x*_Met2Pix>view_pos[0] &&
                                   pos.x*_Met2Pix<view_pos[2] &&
                                   pos.y*_Met2Pix>view_pos[1] &&
                                   pos.y*_Met2Pix<view_pos[3] )
                                {
                                    sound_box=1;//on screen
                                }
                                else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                        pos.x*_Met2Pix<view_pos[0] &&
                                        pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                        pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                {
                                    sound_box=2;//left side
                                }
                                else if(pos.x*_Met2Pix>view_pos[2] &&
                                        pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                        pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                        pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                {
                                    sound_box=3;//right side
                                }
                                else if(pos.x*_Met2Pix>view_pos[0] &&
                                        pos.x*_Met2Pix<view_pos[2] &&
                                        pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                        pos.y*_Met2Pix<view_pos[1] )
                                {
                                    sound_box=4;//top side
                                }
                                else if(pos.x*_Met2Pix>view_pos[0] &&
                                        pos.x*_Met2Pix<view_pos[2] &&
                                        pos.y*_Met2Pix>view_pos[3] &&
                                        pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                {
                                    sound_box=5;//top side
                                }

                                if(sound_box!=0)
                                {
                                    float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                        0,1,0, 0,0,-1, 0,0,0,
                                                        1,  1,  0};
                                    switch(sound_box)
                                    {
                                        case 0: break;//no sound
                                        case 1:
                                        {
                                            sound_data[14]=0;
                                        }break;
                                        case 2://left
                                        {
                                            sound_data[12]=-_sound_box_side_shift;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                        case 3://right
                                        {
                                            sound_data[12]=_sound_box_side_shift;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                        case 4:
                                        {
                                            sound_data[12]=0;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                        case 5:
                                        {
                                            sound_data[12]=0;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                    }

                                    //test if already playing
                                    if( m_pSound->get_volume(_sound_chan_motor_rope) < sound_data[19] )
                                    m_pSound->updateSound(_sound_chan_motor_rope,sound_data);
                                }
                            }
                        }
                        else m_vec_players[player_i].set_rope_motor(0.0);//turn rope motor off

                        //thrusters
                        if(gamepad_data[player_i].button_left_trigger)
                        {
                            //linear sens
                            float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                                       (float)gamepad_data[player_i].trigger_left/255.0*time_dif;

                            b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                            player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                            //add particle to engine
                            m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.3,0.3) ),force);
                        }
                        if(gamepad_data[player_i].button_right_trigger)
                        {
                            //linear sens
                            float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                                       (float)gamepad_data[player_i].trigger_right/255.0*time_dif;

                            b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                            player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                            //add particle to engine
                            m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.3,0.3) ),force);
                        }
                        //set motor sound
                        if(gamepad_data[player_i].button_left_trigger || gamepad_data[player_i].button_right_trigger)
                        {
                            //play sound

                            //calc sound area
                            b2Vec2 pos=player_body->GetPosition();
                            int sound_box=0;//sound off
                            float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                            if(pos.x*_Met2Pix>view_pos[0] &&
                               pos.x*_Met2Pix<view_pos[2] &&
                               pos.y*_Met2Pix>view_pos[1] &&
                               pos.y*_Met2Pix<view_pos[3] )
                            {
                                sound_box=1;//on screen
                            }
                            else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                    pos.x*_Met2Pix<view_pos[0] &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=2;//left side
                            }
                            else if(pos.x*_Met2Pix>view_pos[2] &&
                                    pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=3;//right side
                            }
                            else if(pos.x*_Met2Pix>view_pos[0] &&
                                    pos.x*_Met2Pix<view_pos[2] &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[1] )
                            {
                                sound_box=4;//top side
                            }
                            else if(pos.x*_Met2Pix>view_pos[0] &&
                                    pos.x*_Met2Pix<view_pos[2] &&
                                    pos.y*_Met2Pix>view_pos[3] &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=5;//top side
                            }

                            if(sound_box!=0)
                            {
                                float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                    0,1,0, 0,0,-1, 0,0,0,
                                                    1,  1,  0};
                                switch(sound_box)
                                {
                                    case 0: break;//no sound
                                    case 1:
                                    {
                                        sound_data[14]=0;
                                    }break;
                                    case 2://left
                                    {
                                        sound_data[12]=-_sound_box_side_shift;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 3://right
                                    {
                                        sound_data[12]=_sound_box_side_shift;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 4:
                                    {
                                        sound_data[12]=0;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 5:
                                    {
                                        sound_data[12]=0;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                }

                                int channel_id=0;
                                switch(player_i)
                                {
                                    case 0: channel_id=_sound_chan_motor_p1; break;
                                    case 1: channel_id=_sound_chan_motor_p2; break;
                                    case 2: channel_id=_sound_chan_motor_p3; break;
                                    case 3: channel_id=_sound_chan_motor_p4; break;
                                }
                                float sound_volume=((float)gamepad_data[player_i].trigger_left+
                                                    (float)gamepad_data[player_i].trigger_right)/510.0;
                                //if( gamepad_data[player_i].trigger_left<gamepad_data[player_i].trigger_right )
                                // sound_volume=(float)gamepad_data[player_i].trigger_right/255.0;
                                float sound_pitch=0.5+sound_volume*0.5;
                                sound_data[18]=sound_pitch;
                                sound_data[19]*=sound_volume;

                                m_pSound->updateSound(channel_id,sound_data);
                            }
                        }
                        else//stop sound
                        {
                            int channel_id=0;
                            switch(player_i)
                            {
                                case 0: channel_id=_sound_chan_motor_p1; break;
                                case 1: channel_id=_sound_chan_motor_p2; break;
                                case 2: channel_id=_sound_chan_motor_p3; break;
                                case 3: channel_id=_sound_chan_motor_p4; break;
                            }
                            float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                                  0,1,0, 0,0,0, 0,0,0,
                                                  1,  0,  0};
                            m_pSound->updateSound(channel_id,sound_data);
                        }

                        //test if body is outside window
                        b2Vec2 player_pos=player_body->GetPosition();
                        //float player_angle=player_body->GetAngle();
                        float min_y=(m_cam_pos[1]+m_screen_height)*_Pix2Met+2.0;
                        float max_y=m_cam_pos[1]*_Pix2Met-2.0;
                        float min_x=m_cam_pos[0]*_Pix2Met;
                        float max_x=(m_cam_pos[0]+m_screen_width)*_Pix2Met;
                        float wrap_border_size=2.0;
                        //test height
                        if(player_pos.y<max_y)
                        {
                            m_vec_players[player_i].shift_player_pos( player_pos.x, max_y, true );
                        }
                        else if(player_pos.y>min_y)//below screen (if shifted underground)
                        {
                            m_vec_players[player_i].shift_player_pos( player_pos.x, max_y, true );
                        }

                        //test if within x-limits
                        if(player_pos.x<min_x-wrap_border_size )
                        {
                            m_vec_players[player_i].shift_player_pos( min_x+m_screen_width*_Pix2Met+wrap_border_size,
                                                                      player_pos.y, true );
                        }
                        else if(player_pos.x>max_x+wrap_border_size )
                        {
                            m_vec_players[player_i].shift_player_pos( max_x-m_screen_width*_Pix2Met-wrap_border_size,
                                                                      player_pos.y, true );
                        }

                        //ring test, find target pos and dist
                        if(m_player_active[player_i])
                        {
                            vector<st_int_int_float>* pVec_rings;
                            switch(player_i)
                            {
                                case 0: pVec_rings=&m_vec_training_rings_p1; break;
                                case 1: pVec_rings=&m_vec_training_rings_p2; break;
                                case 2: pVec_rings=&m_vec_training_rings_p3; break;
                                case 3: pVec_rings=&m_vec_training_rings_p4; break;
                            }
                            float target_x=-1;
                            float target_y=-1;
                            float radius=_training_ring_radius_big;
                            int ring_ind=-1;
                            for(unsigned int ring_i=0;ring_i<(*pVec_rings).size();ring_i++)
                            {
                                if( (*pVec_rings)[ring_i].val_f>0.0 )
                                {
                                    target_x=(*pVec_rings)[ring_i].val_i1;
                                    target_y=(*pVec_rings)[ring_i].val_i2;
                                    if(ring_i>2) radius=_training_ring_radius_small;
                                    ring_ind=ring_i;
                                    //cout<<"target ring found: "<<player_i<<endl;
                                    //cout<<" T "<<target_x<<"\t"<<target_y<<endl;
                                    //cout<<" P "<<player_pos.x*_Met2Pix<<"\t"<<player_pos.y*_Met2Pix<<endl;
                                    break;
                                }
                            }
                            //inside ring test
                            if(target_x!=-1 && target_y!=-1)
                            {
                                float dist=sqrt( (player_pos.x*_Met2Pix-target_x)*(player_pos.x*_Met2Pix-target_x)+
                                                 (player_pos.y*_Met2Pix-target_y)*(player_pos.y*_Met2Pix-target_y) );
                                //hook ring test
                                bool special_rule=true;
                                if(ring_ind==5)
                                {
                                    special_rule=false;
                                    b2Vec2 hook_pos=m_vec_players[player_i].get_rope_hook_sensor()->GetBody()->GetPosition();
                                    float hook_ring_dist=sqrt( (hook_pos.x*_Met2Pix-m_arr_training_rings_hook[player_i].val_i1)*
                                                               (hook_pos.x*_Met2Pix-m_arr_training_rings_hook[player_i].val_i1)+
                                                               (hook_pos.y*_Met2Pix-m_arr_training_rings_hook[player_i].val_i2)*
                                                               (hook_pos.y*_Met2Pix-m_arr_training_rings_hook[player_i].val_i2) );
                                    if(hook_ring_dist<_training_ring_radius_small)
                                    {
                                        special_rule=true;//hook inside ring
                                    }

                                }

                                if(dist<radius && special_rule)
                                {
                                    //cout<<"ring dec\n";
                                    (*pVec_rings)[ring_ind].val_f-=time_dif;
                                    if( (*pVec_rings)[ring_ind].val_f<0 )
                                    {
                                        (*pVec_rings)[ring_ind].val_f=0.0;

                                        //play sound
                                        m_pSound->playSimpleSound(wav_enemy_ship_detected,1.0);

                                        //test if ready with first rings
                                        if(ring_ind==4) m_training_state=ts_rings_second;
                                    }
                                    //hook ring
                                    if(ring_ind==5)
                                    {
                                        m_arr_training_rings_hook[player_i].val_f-=time_dif;
                                        if(m_arr_training_rings_hook[player_i].val_f<0.0) m_arr_training_rings_hook[player_i].val_f=0.0;
                                    }
                                }
                                else
                                {
                                    //resetting value
                                    //cout<<"ring inc\n";
                                    (*pVec_rings)[ring_ind].val_f+=time_dif;
                                    if( (*pVec_rings)[ring_ind].val_f>_training_ring_time )
                                     (*pVec_rings)[ring_ind].val_f=_training_ring_time;
                                    //hook ring
                                    if(ring_ind==5)
                                    {
                                        m_arr_training_rings_hook[player_i].val_f+=time_dif;
                                        if(m_arr_training_rings_hook[player_i].val_f>_training_ring_time)
                                         m_arr_training_rings_hook[player_i].val_f=_training_ring_time;
                                    }
                                }
                            }
                            //else no targets left
                        }
                    }


                }break;

            }


            //update screen fade
            if(m_screen_fade_prog>0.0 && !m_fade_off)
            {
                float fade_speed=0.15;
                m_screen_fade_prog-=time_dif*fade_speed;
                //done
                if(m_screen_fade_prog<0.0)
                {
                    m_screen_fade_prog=0.0;
                }
            }
            if(m_screen_fade_prog<1.0 && m_fade_off)
            {
                float fade_speed=0.2;
                m_screen_fade_prog+=time_dif*fade_speed;
                //done
                if(m_screen_fade_prog>1.0)
                {
                    m_screen_fade_prog=1.0;
                    m_fade_off=false;

                    //m_game_state=gs_level_select; go to menu

                    switch(m_training_state)
                    {
                        case ts_rings_second:
                        {
                            //go to tutorial mission
                            //load tutorial level
                            if(!load_selected_level())
                            {
                                //could not load level, back to menu...
                                cout<<"ERROR: Could not load the tutorial mission\n";
                                m_game_state=gs_menu;
                            }
                            else m_game_state=gs_in_level;
                        }break;

                        case ts_reply:
                        {
                            //start the training
                            m_training_state=ts_rings_first;
                            m_fade_off=false;

                            //set training rings
                            int hook_ring_height_dif=100;
                            m_vec_training_rings_p1.clear();
                            m_vec_training_rings_p2.clear();
                            m_vec_training_rings_p3.clear();
                            m_vec_training_rings_p4.clear();
                            for(int player_i=0;player_i<4;player_i++)
                            {
                                switch(player_i)
                                {
                                    case 0:
                                    {
                                        m_vec_training_rings_p1.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.4+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p1.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.4+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.3+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p1.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.4+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.7+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p1.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.4+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.4+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p1.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.4+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.6+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p1.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.4+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        //place hook ring
                                        m_arr_training_rings_hook[player_i]=st_int_int_float( m_vec_training_rings_p1.back().val_i1,
                                                                                              m_vec_training_rings_p1.back().val_i2+hook_ring_height_dif,
                                                                                              _training_ring_time );

                                    }break;

                                    case 1:
                                    {
                                        m_vec_training_rings_p2.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.6+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p2.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.6+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.3+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p2.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.6+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.7+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p2.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.6+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.4+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p2.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.6+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.6+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p2.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.6+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        //place hook ring
                                        m_arr_training_rings_hook[player_i]=st_int_int_float( m_vec_training_rings_p2.back().val_i1,
                                                                                              m_vec_training_rings_p2.back().val_i2+hook_ring_height_dif,
                                                                                              _training_ring_time );

                                    }break;

                                    case 2:
                                    {
                                        m_vec_training_rings_p3.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.2+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p3.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.2+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.3+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p3.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.2+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.7+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p3.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.2+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.4+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p3.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.2+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.6+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p3.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.2+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        //place hook ring
                                        m_arr_training_rings_hook[player_i]=st_int_int_float( m_vec_training_rings_p3.back().val_i1,
                                                                                              m_vec_training_rings_p3.back().val_i2+hook_ring_height_dif,
                                                                                              _training_ring_time );

                                    }break;

                                    case 3:
                                    {
                                        m_vec_training_rings_p4.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.8+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p4.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.8+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.3+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p4.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.8+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.7+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p4.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.8+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.4+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p4.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.8+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.6+rand()%60-30,
                                                                                          _training_ring_time) );
                                        m_vec_training_rings_p4.push_back( st_int_int_float(m_cam_pos[0]+m_screen_width*0.8+rand()%60-30,
                                                                                          m_cam_pos[1]+m_screen_height*0.5+rand()%60-30,
                                                                                          _training_ring_time) );
                                        //place hook ring
                                        m_arr_training_rings_hook[player_i]=st_int_int_float( m_vec_training_rings_p4.back().val_i1,
                                                                                              m_vec_training_rings_p4.back().val_i2+hook_ring_height_dif,
                                                                                              _training_ring_time );
                                    }break;
                                }


                            }

                            //play sound
                            m_pSound->playSimpleSound(wav_enemy_ship_detected,1.0);


                        }break;

                        case ts_ask:
                        {
                            //skip tutorial, to starmap
                            m_game_state=gs_level_select;
                        }break;

                    }
                }
            }

            m_pWorld->Step( _world_step_time,_world_velosity_it,_world_position_it );

        }break;

        case gs_level_select:
        {
            //gameover screen progress
            if(m_show_gameover && !m_fade_off)
            {
                switch(m_gameover_screen_phase)
                {
                    case is_off:
                    {
                        m_gameover_screen_phase=is_fade_in;
                    }break;

                    case is_fade_in:
                    {
                        float fade_speed=0.2;
                        if(m_gameover_screen_fade_level<1.0) m_gameover_screen_fade_level+=time_dif*fade_speed;
                        if(m_gameover_screen_fade_level>=1.0)
                        {
                            m_gameover_screen_fade_level=1.0;
                            m_gameover_screen_phase=is_wait;
                        }
                    }break;

                    case is_wait:
                    {
                        ;//wait for player input
                    }break;

                    case is_fade_out:
                    {
                        float fade_speed=0.3;
                        if(m_gameover_screen_fade_level>0.0) m_gameover_screen_fade_level-=time_dif*fade_speed;
                        if(m_gameover_screen_fade_level<=0.0)
                        {
                            m_gameover_screen_fade_level=0.0;
                            m_gameover_screen_phase=is_off;
                            m_show_gameover=false;
                            m_game_state=gs_level_select;

                            //refuel,restock mship
                            m_pMain_ship->restock();
                            //add new weapons
                            for(int weap_i=wt_spread;weap_i<=wt_mine;weap_i++)
                            {
                                m_vec_pWeapon_stored.push_back( new weapon(m_pWorld,weap_i,1.0) );
                            }
                            //add new gear
                            for(int gear_i=gt_cloak;gear_i<=gt_shield;gear_i++)
                            {
                                m_vec_pGear_stored.push_back( new gear(m_pWorld,gear_i,1.0) );
                            }

                            //mark new goal
                            m_Starmap.make_new_goal();
                            cout<<"New goal station selected\n";

                            return true;

                            /*//TEMP restart
                            //reset game
                            reset();
                            //init game
                            cout<<"Restarting game\n";
                            return false;*/
                        }
                    }break;
                }
            }

            //info screen progress
            if(m_show_info)
            {
                switch(m_info_screen_phase)
                {
                    case is_off:
                    {
                        m_info_screen_phase=is_fade_in;
                    }break;

                    case is_fade_in:
                    {
                        float fade_speed=1.0;
                        if(m_screen_info_fade_level<1.0) m_screen_info_fade_level+=time_dif*fade_speed;
                        if(m_screen_info_fade_level>=1.0)
                        {
                            m_screen_info_fade_level=1.0;
                            m_info_screen_phase=is_wait;
                        }
                    }break;

                    case is_wait:
                    {
                        ;//wait for player input
                    }break;

                    case is_fade_out:
                    {
                        float fade_speed=2.0;
                        if(m_screen_info_fade_level>0.0) m_screen_info_fade_level-=time_dif*fade_speed;
                        if(m_screen_info_fade_level<=0.0)
                        {
                            m_screen_info_fade_level=0.0;
                            m_info_screen_phase=is_off;
                            m_show_info=false;
                            //go to level select state
                            //m_game_state=gs_level_select;//no remain in starmap

                            //unzoom and move to center
                            m_Starmap.reset_view();

                            //show next tutorial text
                            //m_hud.set_draw_tutorial_helptext(ht_map_travel,false);
                        }
                    }break;
                }
            }

            //skip info screen test
            if(m_show_info && m_info_screen_phase==is_wait)
            {
                //test only if unshow manual button is pressed
                for(int player_i=0;player_i<4;player_i++)
                {
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                       gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                       gamepad_data[player_i].button_start || gamepad_data[player_i].button_back ||
                       gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                       gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up )
                    {
                        m_info_screen_phase=is_fade_out;
                    }
                }
            }
            if(m_show_info) break;//no more tests

            //skip gameover screen test
            if(m_show_gameover && m_gameover_screen_phase==is_wait)
            {
                for(int player_i=0;player_i<4;player_i++)
                {
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                       gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                       gamepad_data[player_i].button_start || gamepad_data[player_i].button_back ||
                       gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                       gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up )
                    {
                        m_gameover_screen_phase=is_fade_out;
                    }
                }
            }

            if(m_show_gameover && !m_fade_off) break;//stop time

            //update particle engine
            m_pParticle_engine->update(time_dif);

            //handle gamepad input for selecting level
            //update gamepad input
            for(int player_i=0;player_i<4;player_i++)
            {
                //if(!m_gamepad_connected[player_i]) continue;//dont read from disconnected gamepad

                //if player not active, stop here
                //if(!m_player_active[player_i]) continue;

                //move starmap cam
                if(gamepad_data[player_i].thumbstick_left_x>_thumbstick_deadzone_low||
                   gamepad_data[player_i].thumbstick_left_x<-_thumbstick_deadzone_low)
                {
                    bool ignore_move=false;
                    if(m_on_tutorial_level && !m_hud.is_helptext_completed(ht_map_bars))
                     ignore_move=true;

                    if(!ignore_move)
                    {
                        float move_value[3]={0.0,0.0,0.0};
                        move_value[0]=gamepad_data[player_i].thumbstick_left_x/32768.0*time_dif*200.0;
                        m_Starmap.move_cam_pos(move_value,false);
                    }
                }
                if(gamepad_data[player_i].thumbstick_left_y>_thumbstick_deadzone_low||
                   gamepad_data[player_i].thumbstick_left_y<-_thumbstick_deadzone_low)
                {
                    bool ignore_move=false;
                    if(m_on_tutorial_level && !m_hud.is_helptext_completed(ht_map_bars))
                     ignore_move=true;

                    if(!ignore_move)
                    {
                        float move_value[3]={0.0,0.0,0.0};
                        move_value[1]=-gamepad_data[player_i].thumbstick_left_y/32768.0*time_dif*200.0;
                        m_Starmap.move_cam_pos(move_value,false);
                    }
                }
                //zoom starmap
                if(gamepad_data[player_i].thumbstick_right_y>_thumbstick_deadzone_low||
                   gamepad_data[player_i].thumbstick_right_y<-_thumbstick_deadzone_low)
                {
                    bool ignore_move=false;
                    if(m_on_tutorial_level && !m_hud.is_helptext_completed(ht_map_bars))
                     ignore_move=true;

                    if(!ignore_move)
                    {
                        float cam_pos_curr[3];
                        m_Starmap.get_cam_pos(cam_pos_curr);
                        float zoom_sens=(1.0+cam_pos_curr[2]);
                        float move_value[3]={0.0,0.0,0.0};
                        move_value[2]=gamepad_data[player_i].thumbstick_right_y/32768.0*time_dif*zoom_sens;
                        m_Starmap.move_cam_pos(move_value,false);
                    }
                }
                //planet selection in starmap
                if(gamepad_data[player_i].dpad_left && !m_vec_players[player_i].m_key_trigger_dpad)
                {
                    m_vec_players[player_i].m_key_trigger_dpad=true;
                    m_Starmap.planet_selection(dir_left);
                }
                if(gamepad_data[player_i].dpad_right && !m_vec_players[player_i].m_key_trigger_dpad)
                {
                    m_vec_players[player_i].m_key_trigger_dpad=true;
                    m_Starmap.planet_selection(dir_right);
                }
                if(gamepad_data[player_i].dpad_up && !m_vec_players[player_i].m_key_trigger_dpad)
                {
                    m_vec_players[player_i].m_key_trigger_dpad=true;
                    m_Starmap.planet_selection(dir_up);
                }
                if(gamepad_data[player_i].dpad_down && !m_vec_players[player_i].m_key_trigger_dpad)
                {
                    m_vec_players[player_i].m_key_trigger_dpad=true;
                    m_Starmap.planet_selection(dir_down);
                }
                if(!gamepad_data[player_i].dpad_left && !gamepad_data[player_i].dpad_right &&
                   !gamepad_data[player_i].dpad_up && !gamepad_data[player_i].dpad_down)
                {
                    //reset trigger
                    m_vec_players[player_i].m_key_trigger_dpad=false;
                }

                //go to selected planet
                if(gamepad_data[player_i].button_A)
                {
                    bool ignore=false;
                    if(m_on_tutorial_level && !m_hud.is_helptext_completed(ht_map_move_cam))
                     ignore=true;

                    if(!ignore)
                    {
                        bool travel_ok=m_Starmap.planet_go_to();

                        if(m_on_tutorial_level)
                        {
                            if(m_hud.get_draw_tutorial_helptext(ht_map_travel) && travel_ok)
                            {
                                m_hud.set_draw_tutorial_helptext(ht_map_travel,true);
                            }
                        }
                    }
                }

                //if START, go to selected level
                if(gamepad_data[player_i].button_start)
                {
                    if( !m_vec_players[player_i].m_key_trigger_start && m_screen_fade_prog==0.0 && !m_Starmap.is_travelling() )
                    {
                        m_vec_players[player_i].m_key_trigger_start=true;

                        //load selected level
                        //by fading
                        m_fade_off=true;

                        if(m_on_tutorial_level)
                        {
                            //cancel land if not on land mission
                            if(m_hud.get_draw_tutorial_helptext(ht_map_land))
                             m_hud.set_draw_tutorial_helptext(ht_map_land,true);
                            else
                            {
                                //canceled
                                m_fade_off=false;
                            }
                        }
                    }
                }
                else m_vec_players[player_i].m_key_trigger_start=false;
            }

            //update starmap
            m_fuel_curr=m_pMain_ship->get_fuel();//will be updated in starmap when travelling
            int ret_val=m_Starmap.update(time_dif,m_fuel_curr);//if any fuel was consumed, the value is updated
            switch(ret_val)
            {
                case 2://goal reached
                {
                    //fade off and show goal screen
                    m_show_gameover=true;
                    m_fade_off=true;
                }break;
            }
            m_pMain_ship->set_fuel(m_fuel_curr);//send new fuel value to mship

            /*//master input, keyboard
            if(m_pKeys[13])//enter
            {
                if(!m_key_trigger_keyboard)
                {
                    //go to level select
                    m_key_trigger_keyboard=true;
                    m_fade_off=true;
                }
            }
            else m_key_trigger_keyboard=false;*/

            //tutorial events
            if(m_on_tutorial_level)
            {
                //update hud
                if(!m_show_info) m_hud.update(time_dif);

                //test if info text should be shown
                if(m_hud.get_draw_tutorial_helptext(ht_map_travel) && !m_info_shown)
                {
                    m_show_info=true;
                    m_info_shown=true;
                }
            }

            //update screen fade
            if(m_screen_fade_prog>0.0 && !m_fade_off)
            {
                //zoom out
                float zoom_speed=m_screen_fade_prog;
                if(zoom_speed>0.7) zoom_speed=1.0;
                float move_value[3]={0.0,0.0,-0.025*zoom_speed};
                m_Starmap.move_cam_pos(move_value);

                float fade_speed=0.4;
                m_screen_fade_prog-=time_dif*fade_speed;

                //done
                if(m_screen_fade_prog<0.0)
                {
                    m_screen_fade_prog=0.0;

                    //start starmap tutorial
                    if(m_on_tutorial_level)
                    {
                        m_hud.set_draw_tutorial_helptext(ht_map_center,false);
                    }
                }

                //set starmap noise volume
                m_pSound->set_volume(m_music_source_id,1.0-m_screen_fade_prog);
            }
            if(m_screen_fade_prog<1.0 && m_fade_off)
            {
                //zoom in
                float zoom_speed=2.0-m_screen_fade_prog;
                float move_value[3]={0.0,0.0,0.030*zoom_speed};
                //move towards the planet
                m_Starmap.move_cam_towards_planet_now(m_screen_fade_prog);
                m_Starmap.move_cam_pos(move_value);

                float fade_speed=1.0;
                if(m_show_gameover) fade_speed=0.5;

                m_screen_fade_prog+=time_dif*fade_speed;

                //set starmap noise volume
                float volume=1.0-m_screen_fade_prog;
                if(volume<0) volume=0;
                if(volume>1) volume=1;
                m_pSound->set_volume(m_music_source_id,volume);

                //done
                if(m_screen_fade_prog>1.0)
                {
                    m_screen_fade_prog=1.0;
                    m_fade_off=false;

                    //start music
                    if(!m_music_on) m_music_was_off_at_level_start=true;//remember to start music if resumed later
                    m_pSound->stopSound(m_music_source_id);
                    int music_id=ogg_music0_intro;
                    do
                    {
                        int rand_val=rand()%(9);
                        //cout<<"-- "<<rand_val<<endl;
                        switch(rand_val+1)
                        {
                            //case 0: music_id=ogg_music0_intro; break;//only in main menu
                            case 1: music_id=ogg_music1_intro; break;
                            case 2: music_id=ogg_music2_intro; break;
                            case 3: music_id=ogg_music3_intro; break;
                            case 4: music_id=ogg_music4_intro; break;
                            case 5: music_id=ogg_music5_intro; break;
                            case 6: music_id=ogg_music6_intro; break;
                            case 7: music_id=ogg_music7_intro; break;
                            case 8: music_id=ogg_music8_intro; break;
                            case 9: music_id=ogg_music9_intro; break;
                        }
                    }
                    while( music_id==m_music_id_playing );
                    m_pSound->playSimpleSound(music_id,1.0,m_music_source_id);
                    m_music_id_playing=music_id;
                    m_music_intro_playing=true;
                    cout<<"Starting new music id: "<<music_id<<endl;

                    //resume mship motor sound
                    m_pSound->set_volume(_sound_chan_motor_mship, 1.0);

                    //if on starmap tutorial
                    if(m_on_tutorial_level)
                    {
                        m_on_tutorial_level=false;
                        //report to hud for tutorial text
                        m_hud.set_draw_tutorial_text(-1,false);
                    }

                    if(m_show_gameover)
                    {
                        m_show_gameover=true;
                        m_game_state=gs_level_select;//stay in level select
                    }
                    else//load selected level
                    {
                        if(!load_selected_level())
                        {
                            //could not load level, back to menu...
                            m_game_state=gs_menu;
                        }
                        else m_game_state=gs_in_level;
                    }
                }
            }

        }break;

        case gs_in_level:
        {
            //cout<<"main update begin\n";
            float view_pos[4]={m_cam_pos[0],m_cam_pos[1],
                               m_cam_pos[0]+m_screen_width,m_cam_pos[1]+m_screen_height};//top left, down right

            //test if time should be simulated
            bool time_paused=false;
            if(m_convoy_phase==cs_fade_in_text||m_convoy_phase==cs_dark_screen_delay||m_show_manual||
               m_convoy_phase==cs_show_text_delay||m_convoy_phase==cs_fade_out_text||m_show_info||
               m_lost_screen_phase==is_wait)
             time_paused=true;

            //pause if showing manual
            if(m_show_manual)
            {
                //test only if unshow manual button is pressed, and if skipping tutorial
                for(int player_i=0;player_i<4;player_i++)
                {
                    //if(!m_player_active[player_i]) continue;

                    //unshow manual
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                       gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                       gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                       gamepad_data[player_i].dpad_up || gamepad_data[player_i].dpad_right ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB )
                    {
                        if(!m_vec_players[player_i].m_key_trigger_back)
                        {
                            m_show_manual=false;
                            m_vec_players[player_i].m_key_trigger_back=true;

                            //tutorial test
                            if(m_on_tutorial_level)
                            {
                                if( m_hud.get_draw_tutorial_text(tut_manual) )
                                {
                                    m_hud.set_draw_tutorial_text(tut_manual,false);
                                    m_hud.set_draw_tutorial_text(tut_fuel,true);
                                }
                            }
                        }
                    }
                    else//skip tutorial/manual with back button
                    {
                        if(m_on_tutorial_level)//handle both skip tutorial and unshow manual
                        {
                            if(gamepad_data[player_i].button_back)
                            {
                                if(m_vec_players[player_i].m_skip_tutorial_flag)
                                {
                                    //update timer
                                    m_vec_players[player_i].m_key_hold_time_back-=time_dif;
                                    //done test
                                    if(m_vec_players[player_i].m_key_hold_time_back<=0.0)
                                    {
                                        m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                                        m_vec_players[player_i].m_key_hold_time_start=_player_mship_control_key_timer;
                                        cout<<"Skipping the tutorial\n";
                                        m_on_tutorial_level=false;
                                        m_show_manual=false;
                                        m_fade_off=true;
                                        m_vec_players[player_i].m_skip_tutorial_flag=false;

                                        //turn off tutorial text in hud
                                        m_hud.set_draw_tutorial_text(-1,false);
                                    }
                                }
                                else if(!m_vec_players[player_i].m_key_trigger_back)
                                {
                                    m_vec_players[player_i].m_skip_tutorial_flag=true;
                                    m_vec_players[player_i].m_key_trigger_back=true;
                                    m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                                }
                            }
                            else
                            {
                                m_vec_players[player_i].m_key_trigger_back=false;

                                //if released when skip_tut_flag was true, unshow manual
                                if(m_vec_players[player_i].m_skip_tutorial_flag)
                                {
                                    m_show_manual=false;
                                    m_vec_players[player_i].m_skip_tutorial_flag=false;
                                    m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                                    m_vec_players[player_i].m_key_hold_time_start=_player_mship_control_key_timer;

                                    //tutorial test, update missions
                                    if( m_hud.get_draw_tutorial_text(tut_manual) )
                                    {
                                        m_hud.set_draw_tutorial_text(tut_manual,false);
                                        m_hud.set_draw_tutorial_text(tut_fuel,true);
                                    }
                                }
                            }
                        }
                        else//not on tutorial level, only handle unshowing of the manual
                        {
                            if(gamepad_data[player_i].button_back)
                            {
                                if(!m_vec_players[player_i].m_key_trigger_back)
                                {
                                    m_show_manual=false;
                                    m_vec_players[player_i].m_key_trigger_back=true;
                                    m_vec_players[player_i].m_key_hold_time_start=_player_mship_control_key_timer;
                                }
                            }
                            else m_vec_players[player_i].m_key_trigger_back=false;
                        }
                    }

                    //quit progress
                    if(gamepad_data[player_i].button_start)
                    {
                        //reduce countdown timer
                        m_vec_players[player_i].m_key_hold_time_start-=time_dif*2.0;
                        if(m_vec_players[player_i].m_key_hold_time_start<0.0)
                        {
                            m_vec_players[player_i].m_key_hold_time_start=_player_mship_control_key_timer;
                            cout<<"Quit game, return to menu\n";
                            m_show_lost=true;
                            m_show_manual=false;
                        }
                    }
                    //reduce timer
                    else if(m_vec_players[player_i].m_key_hold_time_start<_player_mship_control_key_timer)
                    {
                        m_vec_players[player_i].m_key_hold_time_start+=time_dif;
                        if(m_vec_players[player_i].m_key_hold_time_start>_player_mship_control_key_timer)
                         m_vec_players[player_i].m_key_hold_time_start=_player_mship_control_key_timer;
                    }
                }
                break;
            }


            //skip lost screen test
            if(m_show_lost && m_lost_screen_phase==is_wait)
            {
                //test only if unshow manual button is pressed
                for(int player_i=0;player_i<4;player_i++)
                {
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                       gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                       gamepad_data[player_i].button_start || gamepad_data[player_i].button_back ||
                       gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                       gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up )
                    {
                        m_lost_screen_phase=is_fade_out;
                    }
                }
            }



            //lost screen progress
            if(m_show_lost && !m_fade_off)
            {
                switch(m_lost_screen_phase)
                {
                    case is_off:
                    {
                        m_lost_screen_phase=is_fade_in;
                    }break;

                    case is_fade_in:
                    {
                        float fade_speed=0.5;
                        if(m_lost_screen_fade_level<1.0) m_lost_screen_fade_level+=time_dif*fade_speed;
                        if(m_lost_screen_fade_level>=1.0)
                        {
                            m_lost_screen_fade_level=1.0;
                            m_lost_screen_phase=is_wait;
                        }

                        //fade music
                        float volume=1.0-m_lost_screen_fade_level;
                        if(volume<0) volume=0;
                        if(volume>1) volume=1;
                        m_pSound->set_volume(m_music_source_id, volume);

                    }break;

                    case is_wait:
                    {
                        ;//wait for player input
                    }break;

                    case is_fade_out:
                    {
                        float fade_speed=0.5;
                        if(m_lost_screen_fade_level>0.0) m_lost_screen_fade_level-=time_dif*fade_speed;
                        if(m_lost_screen_fade_level<=0.0)
                        {
                            m_lost_screen_fade_level=0.0;
                            m_lost_screen_phase=is_off;
                            m_show_lost=false;

                            //reset game
                            reset();

                            //init game
                            cout<<"Restarting game\n";

                            return false;
                        }
                    }break;
                }
            }

            //if waiting for convoy, test if all players have returned
            if(m_waiting_for_convoy)
            {
                switch(m_convoy_phase)
                {
                    case cs_off://init
                    {
                        m_convoy_phase=cs_return_players;
                        //cout<<"Convoy phase: return players\n";
                    }break;

                    case cs_return_players:
                    {
                        //test if all players have returned
                        bool all_players_docked=true;
                        for(int i=0;i<4;i++)
                        {
                            if(m_vec_players[i].get_mship_dock_status()==0 && !m_vec_players[i].is_spawning())
                            {
                                all_players_docked=false;
                                break;
                            }
                        }
                        if(all_players_docked)
                        {
                            m_convoy_phase=cs_fade_out_screen;
                            m_waiting_for_convoy_screen_fade_level=0.0;
                            //cout<<"Convoy phase: fade out screen\n";
                        }

                    }break;

                    case cs_fade_out_screen:
                    {
                        float fade_speed=0.2;
                        if(m_waiting_for_convoy_screen_fade_level<1.0) m_waiting_for_convoy_screen_fade_level+=fade_speed*time_dif;
                        if(m_waiting_for_convoy_screen_fade_level>=1.0)
                        {
                            m_waiting_for_convoy_screen_fade_level=1.0;
                            m_convoy_phase=cs_dark_screen_delay;
                            m_waiting_for_convoy_screen_timer=float(3+rand()%10);//random time
                            //turn off convoy text in hud
                            m_hud.set_draw_tutorial_text(-2,false);
                            //cout<<"Convoy phase: screen delay\n";

                        }
                    }break;

                    case cs_dark_screen_delay:
                    {
                        if(m_waiting_for_convoy_screen_timer>0.0) m_waiting_for_convoy_screen_timer-=time_dif;
                        if(m_waiting_for_convoy_screen_timer<=0.0)
                        {
                            m_waiting_for_convoy_screen_timer=0.0;
                            m_convoy_phase=cs_fade_in_text;
                            //cout<<"Convoy phase: fade out text\n";
                        }
                    }break;

                    case cs_fade_in_text:
                    {
                        float fade_speed=0.2;
                        if(m_waiting_for_convoy_text_fade_level<1.0) m_waiting_for_convoy_text_fade_level+=fade_speed*time_dif;
                        if(m_waiting_for_convoy_text_fade_level>=1.0)
                        {
                            m_waiting_for_convoy_text_fade_level=1.0;
                            m_waiting_for_convoy_text_timer=m_waiting_for_convoy_text_time;
                            m_convoy_phase=cs_show_text_delay;
                            //cout<<"Convoy phase: delay\n";
                        }
                    }break;

                    case cs_show_text_delay:
                    {
                        if(m_waiting_for_convoy_text_timer>0.0) m_waiting_for_convoy_text_timer-=time_dif;
                        if(m_waiting_for_convoy_text_timer<=0.0)
                        {
                            m_waiting_for_convoy_text_timer=0.0;
                            m_convoy_phase=cs_fade_out_text;
                            //cout<<"Convoy phase: fade out text\n";
                        }
                    }break;

                    case cs_fade_out_text:
                    {
                        float fade_speed=0.5;
                        if(m_waiting_for_convoy_text_fade_level>0.0) m_waiting_for_convoy_text_fade_level-=fade_speed*time_dif;
                        if(m_waiting_for_convoy_text_fade_level<=0.0)
                        {
                            m_waiting_for_convoy_text_fade_level=0.0;
                            m_convoy_phase=cs_fade_in_screen;
                            //cout<<"Convoy phase: fade in screen\n";

                            //create convoy
                            create_convoy();
                        }
                    }break;

                    case cs_fade_in_screen:
                    {
                        float fade_speed=0.2;
                        if(m_waiting_for_convoy_screen_fade_level>0.0) m_waiting_for_convoy_screen_fade_level-=fade_speed*time_dif;
                        if(m_waiting_for_convoy_screen_fade_level<=0.0)
                        {
                            //complete
                            m_waiting_for_convoy_screen_fade_level=0.0;
                            m_convoy_phase=cs_off;
                            m_waiting_for_convoy=false;
                            //cout<<"Convoy phase: complete\n";
                        }
                    }break;
                }
            }

            if(time_paused) break;

            //lost test, outside of map
            if(!m_show_lost)
            {
                for(int player_i=0;player_i<4;player_i++)
                {
                    b2Vec2 player_pos=m_vec_players[player_i].get_body_ptr()->GetPosition();
                    if( player_pos.x*_Met2Pix<m_level_soft_borders[0]-m_level_static_fade_distance ||
                        player_pos.x*_Met2Pix>m_level_soft_borders[1]+m_level_static_fade_distance )
                    {
                        cout<<"Player outside map warning\n";
                        //add to timer
                        m_vec_players[player_i].m_outside_map_timer-=time_dif;
                        if(m_vec_players[player_i].m_outside_map_timer<=0.0)
                        {
                            //game lost
                            m_show_lost=true;
                            m_fade_off=true;
                            break;
                        }
                    }
                    else //reset timer
                     m_vec_players[player_i].m_outside_map_timer=_player_outside_map_time;
                }
            }

            //update particle engine
            m_pParticle_engine->update(time_dif);

            //update fuel content
            m_fuel_curr=m_pMain_ship->get_fuel();

            //alarm sound test
            for(int player_i=0;player_i<4;player_i++)
            {
                //sound alarm if low on HP or fuel
                if( m_vec_players[player_i].get_rel_hp()<_sound_alarm_fuel_hp_level ||
                    m_vec_players[player_i].get_rel_fuel()<_sound_alarm_fuel_hp_level )
                {
                    //show hud
                    m_hud.show_hud(player_i);

                    //play if player tries to move (triggers)
                    if( (gamepad_data[player_i].trigger_left>0  || gamepad_data[player_i].trigger_right>0) &&
                       m_vec_players[player_i].get_mship_dock_status()==0 &&
                       !m_vec_players[player_i].is_spawning() )
                     if(m_sound_alarm_delay_timer<=0)
                    {
                        m_sound_alarm_delay_timer=m_sound_alarm_delay;
                        //play sound
                        m_pSound->playSimpleSound(wav_alarm,1.0);
                    }
                }
            }

            //test if all players are docked
            {
                bool all_players_docked=true;
                for(int i=0;i<4;i++)
                {
                    if(m_vec_players[i].get_mship_dock_status()==0 && !m_vec_players[i].is_spawning())
                    {
                        all_players_docked=false;
                        break;
                    }
                }
                //enable takeoff possibilities, if all players are docked
                if(all_players_docked) m_pMain_ship->set_all_players_on_ship(true);
            }

            //update GAMEPAD INPUT
            int player_controlling_mship=m_pMain_ship->get_auto_pilot();
            //float thrust_highest[2]={0,0};
            //float gear_speed_highest[2]={0,0};
            bool fuel_transfer_active=false;
            for(int player_i=0;player_i<4;player_i++)
            {
                /*if(gamepad_data[player_i].button_A)
                 cout<<"real pressed\n";
                if(gamepad_data_copy[player_i].button_A)
                 cout<<"copy pressed\n";*/

                //if(!m_gamepad_connected[player_i] && m_master_player_controller==0)
                // continue;//dont read from disconnected gamepad, unless master control

                //read for average thrusters input (common input)
                /*if(m_pMain_ship->is_landing())
                {
                    if(gamepad_data[player_i].button_left_trigger)
                    {
                        thrust_highest[0]+=(float)gamepad_data[player_i].trigger_left/255.0;
                    }
                    if(gamepad_data[player_i].trigger_right)
                    {
                        thrust_highest[1]+=(float)gamepad_data[player_i].trigger_right/255.0;
                    }
                    int trigger_val=20000;
                    //read average gear speed input
                    if(gamepad_data[player_i].thumbstick_right_y>trigger_val ||
                       gamepad_data[player_i].thumbstick_right_y<-trigger_val)
                    {
                        gear_speed_highest[1]+=(float)gamepad_data[player_i].thumbstick_right_y/32768.0;
                    }
                    if(gamepad_data[player_i].thumbstick_left_y>trigger_val ||
                       gamepad_data[player_i].thumbstick_left_y<-trigger_val)
                    {
                        gear_speed_highest[0]+=(float)gamepad_data[player_i].thumbstick_left_y/32768.0;
                    }

                    continue;
                }*/

                //reset key back
                if(m_vec_players[player_i].m_key_trigger_back && !gamepad_data[player_i].button_back)
                 m_vec_players[player_i].m_key_trigger_back=false;

                //reset key start
                if(m_vec_players[player_i].m_key_trigger_start && !gamepad_data[player_i].button_start)
                 m_vec_players[player_i].m_key_trigger_start=false;

                //can player control mship
                if(player_i==player_controlling_mship)
                {
                    //update owner of the mship led pointer
                    m_mship_led_prog=m_vec_players[player_i].m_key_hold_time_back;

                    //if not docked, start auto pilot
                    if(!m_vec_players[player_i].connected_to_mship())
                    {
                        player_controlling_mship=-1;
                        m_pMain_ship->set_auto_pilot(-1);
                        m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                        m_vec_players[player_i].m_key_trigger_back=true;//have to be reset by releasing button
                        cout<<"Main ship auto pilot enabled"<<endl;
                    }

                    //test if player wants end mship control
                    if(gamepad_data[player_i].button_back && gamepad_data[player_i].button_start &&
                       !m_vec_players[player_i].m_key_trigger_back && !m_vec_players[player_i].m_key_trigger_start)
                    {
                        m_vec_players[player_i].m_takeoff_flag=false;//cancel takeoff prog
                        m_show_manual=false;
                        //reduce countdown timer
                        m_vec_players[player_i].m_key_hold_time_back-=time_dif;
                        if(m_vec_players[player_i].m_key_hold_time_back<0.0)
                        {
                            m_pMain_ship->set_auto_pilot(-1);
                            m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                            m_vec_players[player_i].m_key_trigger_back=true;//have to be reset by releasing button
                            m_vec_players[player_i].m_key_trigger_start=true;//have to be reset by releasing button
                            cout<<"Main ship auto pilot enabled"<<endl;

                            /*//if on tutorial mission and done, force takeoff
                            if(m_on_tutorial_level)
                            {
                                if(m_hud.get_draw_tutorial_text(tut_takeoff))
                                {
                                    m_pMain_ship->begin_takeoff();
                                }
                            }*/
                        }
                    }
                    else//restore key back timer
                    {
                        m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                    }

                    //control mship
                    //thruster left
                    if(gamepad_data[player_i].button_left_trigger>0)
                    {
                        float sens=30000.0*time_dif;
                        b2Vec2 force = b2Vec2( cos(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.5) * sens );
                        b2Vec2 force_left =((float)gamepad_data[player_i].trigger_left/255.0)*force;

                        //add thrust to body
                        m_pMain_ship->get_body_ptr()->ApplyForce( force_left, m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );

                        //add particle
                        m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-4.2,2.3) ), 0.05*(force_left) );
                        m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-4.2,2.3) ), 0.1*(force_left) );
                    }
                    //thruster right
                    if(gamepad_data[player_i].button_right_trigger>0)
                    {
                        float sens=30000.0*time_dif;
                        b2Vec2 force = b2Vec2( cos(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.5) * sens );
                        b2Vec2 force_right=((float)gamepad_data[player_i].trigger_right/255.0)*force;

                        //add thrust to body
                        m_pMain_ship->get_body_ptr()->ApplyForce( force_right, m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                        //add particle
                        m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2( 4.2,2.3) ), 0.05*(force_right) );
                        m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2( 4.2,2.3) ), 0.1*(force_right) );
                    }
                    //thruster side left
                    if(gamepad_data[player_i].button_LB)
                    {
                        float sens=30000.0*time_dif;
                        b2Vec2 force = b2Vec2( cos(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.0) * sens ,
                                               sin(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.0) * sens );

                        //add thrust to body
                        m_pMain_ship->get_body_ptr()->ApplyForce( force, m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-5.0,0.0) ), true );

                        //add particle
                        m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2( -5.0,0.0) ), 0.02*(force) );
                    }
                    //thruster side right
                    if(gamepad_data[player_i].button_RB)
                    {
                        float sens=30000.0*time_dif;
                        b2Vec2 force = b2Vec2( cos(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*1.0) * sens ,
                                               sin(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*1.0) * sens );

                        //add thrust to body
                        m_pMain_ship->get_body_ptr()->ApplyForce( force, m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(5.0,0.0) ), true );

                        //add particle
                        m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2( 5.0,0.0) ), 0.02*(force) );
                    }
                    //mship motor sound
                    if(gamepad_data[player_i].button_left_trigger || gamepad_data[player_i].button_right_trigger ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB )
                    {
                        //play sound

                        //calc sound area
                        b2Vec2 pos=m_pMain_ship->get_body_ptr()->GetPosition();
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(pos.x*_Met2Pix>view_pos[0] &&
                           pos.x*_Met2Pix<view_pos[2] &&
                           pos.y*_Met2Pix>view_pos[1] &&
                           pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                pos.x*_Met2Pix<view_pos[0] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(pos.x*_Met2Pix>view_pos[2] &&
                                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[3] &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }

                            float sound_volume=((float)gamepad_data[player_i].trigger_left+
                                                (float)gamepad_data[player_i].trigger_right)/510.0;
                            if(gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB)
                            {
                                if(sound_volume<0.4) sound_volume=0.4;
                            }
                            //if(sound_volume>0.5) sound_volume=0.5;//cap volume
                            //if( gamepad_data[player_i].trigger_left<gamepad_data[player_i].trigger_right )
                            // sound_volume=(float)gamepad_data[player_i].trigger_right/255.0;
                            float sound_pitch=0.5+sound_volume*0.5;
                            sound_data[18]=sound_pitch;
                            sound_data[19]*=sound_volume;

                            m_pSound->updateSound(_sound_chan_motor_mship,sound_data);
                        }
                    }
                    else//stop sound
                    {
                        float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                              0,1,0, 0,0,0, 0,0,0,
                                              1,  0,  0};

                        m_pSound->updateSound(_sound_chan_motor_mship,sound_data);
                    }

                    //landing gear
                    float gears_active=0;
                    int trigger_val=20000;
                    //left
                    if(gamepad_data[player_i].thumbstick_left_y>trigger_val ||
                       gamepad_data[player_i].thumbstick_left_y<-trigger_val)
                    {
                        if( m_pMain_ship->set_landing_gear_motor_speed_left((float)gamepad_data[player_i].thumbstick_left_y/32768.0) )
                         gears_active+=0.5;
                    }
                    else m_pMain_ship->set_landing_gear_motor_speed_left(0.0);//stop
                    //right
                    if(gamepad_data[player_i].thumbstick_right_y>trigger_val ||
                       gamepad_data[player_i].thumbstick_right_y<-trigger_val)
                    {
                        if( m_pMain_ship->set_landing_gear_motor_speed_right((float)gamepad_data[player_i].thumbstick_right_y/32768.0) )
                         gears_active+=0.5;
                    }
                    else m_pMain_ship->set_landing_gear_motor_speed_right(0.0);//stop

                    //landing gear sound
                    if(gears_active>0.0)
                    {
                        //calc sound area
                        b2Vec2 pos=m_pMain_ship->get_body_ptr()->GetPosition();
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(pos.x*_Met2Pix>view_pos[0] &&
                           pos.x*_Met2Pix<view_pos[2] &&
                           pos.y*_Met2Pix>view_pos[1] &&
                           pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                pos.x*_Met2Pix<view_pos[0] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(pos.x*_Met2Pix>view_pos[2] &&
                                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[3] &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            sound_data[19]*=gears_active;

                            m_pSound->updateSound(_sound_chan_gear_motor_mship,sound_data);
                        }
                    }

                    continue;//cannot control own ship
                }
                //test if player wants to control mship (HOLD BACK and START)
                else if(gamepad_data[player_i].button_back && gamepad_data[player_i].button_start && !m_on_tutorial_level &&
                        !m_vec_players[player_i].m_key_trigger_back && !m_vec_players[player_i].m_key_trigger_start)
                {
                    m_vec_players[player_i].m_takeoff_flag=false;//cancel takeoff prog
                    m_show_manual=false;
                    //only if no other controller
                    if(player_controlling_mship==-1)
                    {
                        //reduce countdown timer
                        m_vec_players[player_i].m_key_hold_time_back-=time_dif;
                        if(m_vec_players[player_i].m_key_hold_time_back<0.0)
                        {
                            m_pMain_ship->set_auto_pilot(player_i);
                            m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;
                            m_vec_players[player_i].m_key_trigger_back=true;//have to be reset by releasing button
                            m_vec_players[player_i].m_key_trigger_start=true;//have to be reset by releasing button
                            cout<<"Player in control of Main ship: "<<player_i+1<<endl;
                        }
                    }
                }
                //reset timer
                else m_vec_players[player_i].m_key_hold_time_back=_player_mship_control_key_timer;

                //show manual
                if(!gamepad_data[player_i].button_back && !gamepad_data[player_i].button_start && m_vec_players[player_i].m_manual_flag)
                {
                    m_show_manual=true;
                    m_vec_players[player_i].m_manual_flag=false;

                    //stop loop sounds
                    for(int sound_i=10;sound_i<30;sound_i++) m_pSound->set_volume(sound_i,0.0);
                    //disable sounds
                    m_pSound->enable_sound(false);
                }
                if(gamepad_data[player_i].button_back && !gamepad_data[player_i].button_start &&
                   !m_vec_players[player_i].m_key_trigger_back && m_vec_players[player_i].m_key_hold_time_back==_player_mship_control_key_timer)
                {
                    m_vec_players[player_i].m_manual_flag=true;
                }
                //reset manual test if start is pressed, to avoid shoing manual after holding start and back
                if(gamepad_data[player_i].button_start)
                {
                    m_vec_players[player_i].m_manual_flag=false;
                }

                //reset pos test
                if(m_on_tutorial_level && m_player_stuck[player_i])
                {
                    if(gamepad_data[player_i].button_A)
                    {
                        gamepad_data[player_i].button_A=false;//avoid direct fire after respawn
                        //reset position
                        m_player_stuck[player_i]=false;
                        m_vec_players[player_i].set_pos( b2Vec2(m_pMain_ship->get_body_ptr()->GetPosition().x-2.5,
                                                                m_pMain_ship->get_body_ptr()->GetPosition().y-7.0) );
                    }
                }

                //forced eject in tutorial
                if(m_on_tutorial_level && m_vec_players[player_i].connected_to_mship())
                {
                    bool force_eject=false;

                    //if wants to activate gear with X
                    if(gamepad_data[player_i].button_X && m_hud.get_draw_tutorial_helptext(ht_use_gear) )
                    {
                        force_eject=true;
                    }

                    //if wants to eject control unit
                    if(gamepad_data[player_i].button_right_thumbstick && m_hud.get_draw_tutorial_helptext(ht_drone_eject) )
                    {
                        force_eject=true;
                    }

                    if(force_eject)
                    {
                        m_pMain_ship->player_inside(player_i,false);
                        m_vec_players[player_i].disconnect_from_mship();
                        //give player ship a boost
                        float sens=30000.0*time_dif;
                        b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                        b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                        player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.0,0.3) ), true );
                        //add particle to engine
                        m_pParticle_engine->add_explosion( player_body->GetWorldPoint( b2Vec2(0.0,0.3) ),50,100,1.0 );

                        //abort takeoff possibilities
                        m_pMain_ship->set_all_players_on_ship(false);

                        //play sound
                        m_pSound->playSimpleSound(wav_mship_disconnect,0.3);
                    }
                }

                //drone spawn
                if(gamepad_data[player_i].button_right_thumbstick)
                {
                    //have to be disconnected from mship, and drone not spawned
                    if( !m_vec_players[player_i].connected_to_mship() && m_vec_players[player_i].get_drone_mode()==dm_off )
                    {
                        m_vec_players[player_i].m_drone_spawn_timer-=time_dif;
                        if(m_vec_players[player_i].m_drone_spawn_timer<=0.0)
                        {
                            cout<<"Spawning player drone\n";
                            m_vec_players[player_i].m_drone_spawn_timer=m_vec_players[player_i].m_drone_spawn_time;
                            //timer done, spawn drone
                            m_vec_players[player_i].set_drone_mode(dm_on);

                            //disconnect hook, if connected
                            m_vec_players[player_i].hook_disconnect();

                            //play sound
                            m_pSound->playSimpleSound(wav_drone_eject,1.0);

                            //activate drone motor channel
                            switch(player_i)
                            {
                                case 0: m_pSound->playSimpleSound(wav_drone_motor,0.0,_sound_chan_motor_p1); break;
                                case 1: m_pSound->playSimpleSound(wav_drone_motor,0.0,_sound_chan_motor_p2); break;
                                case 2: m_pSound->playSimpleSound(wav_drone_motor,0.0,_sound_chan_motor_p3); break;
                                case 3: m_pSound->playSimpleSound(wav_drone_motor,0.0,_sound_chan_motor_p4); break;
                            }

                            if(m_on_tutorial_level)
                            {
                                //update hud
                                m_hud.set_player_ptr( m_vec_players[player_i].get_player_drone_body_ptr() );

                                //complete mission
                                if( m_hud.get_draw_tutorial_helptext(ht_drone_eject) )
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_drone_eject,true);
                                }
                            }
                        }
                    }
                }
                else//reset counter
                {
                    m_vec_players[player_i].m_drone_spawn_timer=m_vec_players[player_i].m_drone_spawn_time;
                }

                //drone control
                if( m_vec_players[player_i].get_drone_mode()==dm_on )
                {
                    b2Body* drone_body=m_vec_players[player_i].get_player_drone_body_ptr();

                    //thruster (left thumbstick angle)
                    float thrust_x=(float)gamepad_data[player_i].thumbstick_left_x/32768.0;
                    float thrust_y=(float)gamepad_data[player_i].thumbstick_left_y/-32768.0;
                    float dead_zone=0.15;
                    if(thrust_x>dead_zone || thrust_y>dead_zone || thrust_x<-dead_zone || thrust_y<-dead_zone)
                    {
                        float sens=0.2;
                        b2Vec2 force = b2Vec2( thrust_x*sens , thrust_y*sens );
                        drone_body->ApplyForce(force, drone_body->GetPosition(), true );
                        //add particle to engine
                        if(rand()%3==0)
                         m_pParticle_engine->add_particle(drone_body->GetPosition(),force,0.2);

                        //play sound
                        //calc sound area
                        b2Vec2 pos=drone_body->GetPosition();
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(pos.x*_Met2Pix>view_pos[0] &&
                           pos.x*_Met2Pix<view_pos[2] &&
                           pos.y*_Met2Pix>view_pos[1] &&
                           pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                pos.x*_Met2Pix<view_pos[0] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(pos.x*_Met2Pix>view_pos[2] &&
                                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[3] &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }

                            int channel_id=0;
                            switch(player_i)
                            {
                                case 0: channel_id=_sound_chan_motor_p1; break;
                                case 1: channel_id=_sound_chan_motor_p2; break;
                                case 2: channel_id=_sound_chan_motor_p3; break;
                                case 3: channel_id=_sound_chan_motor_p4; break;
                            }
                            float sound_volume=(fabs(gamepad_data[player_i].thumbstick_left_x)+
                                                fabs(gamepad_data[player_i].thumbstick_left_y))/65534.0;
                            //if( gamepad_data[player_i].trigger_left<gamepad_data[player_i].trigger_right )
                            // sound_volume=(float)gamepad_data[player_i].trigger_right/255.0;
                            float sound_pitch=0.5+sound_volume*0.5;
                            sound_data[18]=sound_pitch;
                            sound_data[19]*=sound_volume;

                            m_pSound->updateSound(channel_id,sound_data);
                        }
                    }
                    else
                    {
                        //stop sound
                        int channel_id=0;
                        switch(player_i)
                        {
                            case 0: channel_id=_sound_chan_motor_p1; break;
                            case 1: channel_id=_sound_chan_motor_p2; break;
                            case 2: channel_id=_sound_chan_motor_p3; break;
                            case 3: channel_id=_sound_chan_motor_p4; break;
                        }
                        float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                              0,1,0, 0,0,0, 0,0,0,
                                              1,  0,  0};
                        m_pSound->updateSound(channel_id,sound_data);
                    }

                    continue;//no player more input
                }
                //turn off control if drone is destroyed
                if( m_vec_players[player_i].get_drone_mode()==dm_destroyed ) continue;

                //player input not read
                if(!m_player_input_enabled) break;
                //no input if no HP
                if(m_vec_players[player_i].get_rel_hp()<=0.0)
                {
                    //out of hp message
                    if(m_on_tutorial_level && !m_hud.m_text_out_of_hp_shown)
                    {
                        //display message
                        b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                        float pos[2]={ player_body->GetPosition().x*_Met2Pix-512.0 ,
                                       player_body->GetPosition().y*_Met2Pix-100.0 };
                        m_hud.show_out_of_hp(pos);
                    }

                    continue;
                }

                //if player not active, activation test (when the screen is clear to avoid direct unintentional input)
                if(!m_player_active[player_i] && m_screen_fade_prog==0.0)
                {
                    //activate by pushing A,Y,X,B,LB,RB,START,BACK,DPAD
                    if(gamepad_data[player_i].button_A || gamepad_data[player_i].button_B ||
                       gamepad_data[player_i].button_X || gamepad_data[player_i].button_Y ||
                       gamepad_data[player_i].button_LB || gamepad_data[player_i].button_RB ||
                       /*gamepad_data[player_i].button_start ||*/ gamepad_data[player_i].button_back ||
                       gamepad_data[player_i].dpad_down || gamepad_data[player_i].dpad_left ||
                       gamepad_data[player_i].dpad_right || gamepad_data[player_i].dpad_up )
                    {
                        if(m_on_tutorial_level)
                        {
                            m_hud.set_draw_tutorial_helptext(ht_raise,true);
                        }

                        //start lifting ship from mship
                        m_vec_players[player_i].raise_from_mship();
                        m_player_active[player_i]=true;

                        //play sound
                        m_pSound->playSimpleSound(wav_player_ship_raising,1.0);

                        //test if any players in the way
                        b2Vec2 land_pos_local(0,0);
                        switch(player_i)
                        {
                            case 0: land_pos_local.Set(0.00,-2.5); break;
                            case 1: land_pos_local.Set(1.25,-2.5); break;
                            case 2: land_pos_local.Set(2.75,-2.5); break;
                            case 3: land_pos_local.Set(4.00,-2.5); break;
                        }
                        b2Vec2 land_pos_world=m_pMain_ship->get_body_ptr()->GetWorldPoint( land_pos_local );
                        for(int player_i2=0;player_i2<(int)m_vec_players.size();player_i2++)
                        {
                            if(player_i==player_i2) continue;
                            //measure dist
                            b2Vec2 player_pos=m_vec_players[player_i2].get_body_ptr()->GetPosition();
                            float min_allowed_dist=1.0;
                            float dist=sqrt( (player_pos.x-land_pos_world.x)*(player_pos.x-land_pos_world.x)+
                                             (player_pos.y-land_pos_world.y)*(player_pos.y-land_pos_world.y) );
                            if(dist<min_allowed_dist)
                            {
                                cout<<"Reinit player: Other player in the way\n";
                                //eject player in the way
                                m_pMain_ship->player_inside(player_i2,false);
                                m_vec_players[player_i2].disconnect_from_mship();
                                b2Body* player_body=m_vec_players[player_i2].get_body_ptr();
                                //give player ship a boost
                                float sens=30000.0*time_dif;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.0,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_explosion( player_body->GetWorldPoint( b2Vec2(0.0,0.3) ),50,100,1.0 );
                            }
                            else cout<<"Reinit player: Dist to other player: "<<dist<<endl;
                        }
                    }
                    else continue;//player not active
                }

                //toogle cam follow
                if(gamepad_data[player_i].button_left_thumbstick)
                {
                    if(!m_vec_players[player_i].m_key_trigger_thumbstick_left)
                    {
                        m_vec_players[player_i].m_key_trigger_thumbstick_left=true;
                        if(m_came_mode==cm_follow_one)
                        {
                            m_came_mode=cm_follow_all;
                            cout<<"Cam: Cam follows all players\n";
                        }
                        else
                        {
                            m_came_mode=cm_follow_one;
                            m_cam_player_to_follow=player_i;
                            cout<<"Cam: Cam follows player: "<<player_i+1<<endl;
                        }
                    }
                }
                else m_vec_players[player_i].m_key_trigger_thumbstick_left=false;

                //move focus point, with left thumbstick
                m_vec_players[player_i].set_focus_point( (float)gamepad_data[player_i].thumbstick_left_x/32768.0,
                                                         (float)gamepad_data[player_i].thumbstick_left_y/-32768.0 );

                //get body
                b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                st_body_user_data* player_data=(st_body_user_data*)player_body->GetUserData();
        //cout<<"update 4\n";
                /*//get event handle from contact listener (sensor with other body)
                if(m_pEvent_flag_hook[player_i]==ev_connect)
                {
                    cout<<"Begin Weld "<<player_i<<endl;
                    b2Vec2 pos=m_ppBody_to_connect[player_i]->GetPosition();
                    cout<<"sPos: "<<pos.x<<", "<<pos.y<<endl;

                    m_vec_players[player_i].hook_connect(m_ppBody_to_connect[player_i]);
                    m_pEvent_flag_hook[player_i]=ev_idle;//reset
                }*/

                /*//force takeoff (TEMP)
                if(gamepad_data[player_i].button_X && gamepad_data[player_i].button_A )
                {
                    m_pMain_ship->begin_takeoff();
                }*/

                //takeoff
                if(!gamepad_data[player_i].button_start && !gamepad_data[player_i].button_back &&
                   m_vec_players[player_i].m_takeoff_flag && !m_pMain_ship->is_landing() )
                {
                    //cout<<"takeoff button released\n";
                    /*if(m_on_tutorial_level)
                    {
                        if( m_hud.get_draw_tutorial_helptext(ht_takeoff) )
                        {
                            //cout<<"tagoff flag true";
                            //show text
                            m_hud.set_draw_tutorial_text(tut_takeoff,true);
                            m_hud.set_draw_tutorial_text(tut_return,false);
                            //remove extra text
                            m_hud.set_draw_tutorial_helptext(ht_takeoff,true);
                        }
                    }*/

                    m_vec_players[player_i].m_takeoff_flag=false;
                    //if in takeoff, turn off, else start takeoff
                    if(m_pMain_ship->is_takeoff())
                    {
                        if(!m_on_tutorial_level)//not if on tutorial
                         m_pMain_ship->set_all_players_on_ship(false);
                    }
                    else m_pMain_ship->begin_takeoff();
                }
                if(gamepad_data[player_i].button_start && !gamepad_data[player_i].button_back && !m_pMain_ship->is_landing() &&
                   !m_vec_players[player_i].m_key_trigger_start && m_vec_players[player_i].m_key_hold_time_back==_player_mship_control_key_timer )
                {
                    //cout<<"Takeoff try...\n";
                    //test if all players are docked
                    bool all_docked=true;
                    for(int player_i2=0;player_i2<4;player_i2++)
                    {
                        if( !m_vec_players[player_i2].connected_to_mship() )
                        {
                            all_docked=false;
                            break;
                        }
                    }
                    if(all_docked)
                    {
                        //cout<<"all docked...\n";
                        if(m_on_tutorial_level)//not allowed on tutorial at any time
                        {
                            if( m_hud.get_draw_tutorial_helptext(ht_takeoff) )
                            {
                                //cout<<"takeoff flag true\n";
                                m_vec_players[player_i].m_takeoff_flag=true;
                            }
                        }
                        else
                        {
                            //m_player_input_enabled=false;//inactivate player controls

                            //raise takeoff flag, takeof started at button release, if not canceled
                            m_vec_players[player_i].m_takeoff_flag=true;
                            //cout<<" all players docked\n";
                        }
                    }
                }

                /*//TEMP
                if(gamepad_data[player_i].button_X)
                {
                    //explosion test
                    static int counter_delay=0;
                    if(counter_delay<=0)
                    {
                        counter_delay=30;
                        b2Vec2 pos=player_body->GetPosition();
                        float angle=player_body->GetAngle()*_Rad2Deg;
                        m_pParticle_engine->add_explosion( pos,100,100,1.0,angle );

                        //cout<<"HP: "<<m_vec_players[player_i].m_damage<<endl;//TEMP
                        b2Vec2 player_pos=player_body->GetPosition();
                        cout<<"Pos: "<<player_pos.x<<", "<<player_pos.y<<" : Angle: "<<angle<<endl;//TEMP

                    }else counter_delay--;
                }*/

                //force hook led on
                if(gamepad_data[player_i].button_Y && !m_vec_players[player_i].is_hook_off())
                 m_vec_players[player_i].m_force_led_on=true;

                //rope control and docking
                //no dock or rope control if cloaked
                if(m_vec_players[player_i].m_cloak_timer==m_vec_players[player_i].m_cloak_delay &&
                   m_vec_players[player_i].m_cloak_target_off)
                {
                    //rope control, not if connected to mship
                    if(gamepad_data[player_i].dpad_up ^ gamepad_data[player_i].dpad_down &&
                       !m_vec_players[player_i].connected_to_mship() )
                    {
                        if(gamepad_data[player_i].dpad_down)
                        {
                            if(m_on_tutorial_level)
                            {
                                m_hud.set_draw_tutorial_helptext(ht_towline,true);
                            }

                            m_vec_players[player_i].set_rope_motor(_rope_motor_strength*time_dif);
                        }
                        else
                        {
                            m_vec_players[player_i].set_rope_motor(-_rope_motor_strength*time_dif);
                        }

                        //play sound
                        int rope_length=m_vec_players[player_i].get_rope_length();
                        //cout<<rope_length<<endl;
                        if( (m_vec_players[player_i].get_rope_lock_status() && gamepad_data[player_i].dpad_up) ||
                            (rope_length>=_player_rope_length_max && gamepad_data[player_i].dpad_down) )
                        {
                            ;//dont play sound
                        }
                        else//play
                        {
                            //calc sound area
                            b2Vec2 pos=player_body->GetPosition();
                            int sound_box=0;//sound off
                            float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                            if(pos.x*_Met2Pix>view_pos[0] &&
                               pos.x*_Met2Pix<view_pos[2] &&
                               pos.y*_Met2Pix>view_pos[1] &&
                               pos.y*_Met2Pix<view_pos[3] )
                            {
                                sound_box=1;//on screen
                            }
                            else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                    pos.x*_Met2Pix<view_pos[0] &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=2;//left side
                            }
                            else if(pos.x*_Met2Pix>view_pos[2] &&
                                    pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=3;//right side
                            }
                            else if(pos.x*_Met2Pix>view_pos[0] &&
                                    pos.x*_Met2Pix<view_pos[2] &&
                                    pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                    pos.y*_Met2Pix<view_pos[1] )
                            {
                                sound_box=4;//top side
                            }
                            else if(pos.x*_Met2Pix>view_pos[0] &&
                                    pos.x*_Met2Pix<view_pos[2] &&
                                    pos.y*_Met2Pix>view_pos[3] &&
                                    pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                            {
                                sound_box=5;//top side
                            }

                            if(sound_box!=0)
                            {
                                float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                    0,1,0, 0,0,-1, 0,0,0,
                                                    1,  1,  0};
                                switch(sound_box)
                                {
                                    case 0: break;//no sound
                                    case 1:
                                    {
                                        sound_data[14]=0;
                                    }break;
                                    case 2://left
                                    {
                                        sound_data[12]=-_sound_box_side_shift;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 3://right
                                    {
                                        sound_data[12]=_sound_box_side_shift;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 4:
                                    {
                                        sound_data[12]=0;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                    case 5:
                                    {
                                        sound_data[12]=0;
                                        sound_data[19]=_sound_box_level_outside;
                                    }break;
                                }

                                //test if already playing
                                if( m_pSound->get_volume(_sound_chan_motor_rope) < sound_data[19] )
                                m_pSound->updateSound(_sound_chan_motor_rope,sound_data);
                            }
                        }
                    }
                    else m_vec_players[player_i].set_rope_motor(0.0);//turn rope motor off

                    //hook/land control
                    if(gamepad_data[player_i].button_Y)//activate hook sensor (hold)
                    {
                        if(!m_vec_players[player_i].player_hook_connected())//if not already connected to any object
                        {
                            //cout<<"Sensor flag: "<<m_pMship_landing_sensor_flag[player_i]<<"\tDock status: "<<m_vec_players[player_i].get_mship_dock_status()<<endl;
                            //test if inside main ship landing zone
                            if(m_pMship_landing_sensor_flag[player_i] &&
                               m_vec_players[player_i].get_mship_dock_status()==0 &&
                               m_vec_players[player_i].get_rope_lock_status() )
                            {
                                //possible to dock if faceing up
                                float body_angle=player_body->GetAngle()*_Rad2Deg+180.0;
                                float mship_angle=m_pMain_ship->get_body_ptr()->GetAngle()*_Rad2Deg+180.0;
                                while(body_angle<0)    body_angle+=360.0;
                                while(body_angle>=360) body_angle-=360.0;
                                while(mship_angle<0)    mship_angle+=360.0;
                                while(mship_angle>=360) mship_angle-=360.0;
                                float angle_dif=body_angle-mship_angle;
                                float variation_tolerance=15.0;
                                if(angle_dif+variation_tolerance>0.0 &&
                                   angle_dif-variation_tolerance<0.0)//within limit
                                {
                                    if(m_on_tutorial_level)
                                    {
                                        if(m_hud.get_draw_tutorial_helptext(ht_dock))
                                        {
                                            m_hud.set_draw_tutorial_helptext(ht_dock,true);
                                        }

                                        if(m_hud.get_draw_tutorial_helptext(ht_dock_again))
                                        {
                                            m_hud.set_draw_tutorial_helptext(ht_dock_again,true);
                                        }
                                    }

                                    //cout<<"Docking: start pos adjustment\n";
                                    //dock to main ship
                                    m_pMain_ship->player_inside(player_i,true);
                                    int ret_val=m_vec_players[player_i].dock_player_to_mship();
                                    switch(ret_val)
                                    {
                                        case 2://hook restored
                                        {
                                            //send new sensor to contact listener
                                            m_ppRope_hook_sensors[player_i]=m_vec_players[player_i].get_rope_hook_sensor();
                                        }break;
                                    }
                                    //old sensor was set to connectable, and have to be reset when hook have new sensor
                                    m_pEvent_flag_hook[player_i]=ev_idle;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_mship_connect,0.3);

                                    //test if all players are docked
                                    bool all_players_docked=true;
                                    for(int i=0;i<4;i++)
                                    {
                                        if(m_vec_players[i].get_mship_dock_status()==0 && !m_vec_players[i].is_spawning())
                                        {
                                            all_players_docked=false;
                                            break;
                                        }
                                    }
                                    //enable takeoff possibilities, if all players are docked
                                    if(all_players_docked) m_pMain_ship->set_all_players_on_ship(true);
                                }
                                //else cout<<"Docking: Invalid angle: "<<angle_dif<<endl;;
                            }
                            //test if hook is connectable to objects
                            else if(m_pEvent_flag_hook[player_i]==ev_connectable)
                            {
                                //cout<<"Hook connectable\n";
                                //make connection
                                if(m_vec_players[player_i].hook_connect(m_ppBody_to_connect[player_i]))//connection made if true
                                {
                                    if(m_on_tutorial_level)
                                    {
                                        m_hud.set_draw_tutorial_helptext(ht_connect,true);
                                    }

                                    //cout<<"Hook connection made\n";
                                    //notify carried body
                                    st_body_user_data* data=(st_body_user_data*)m_ppBody_to_connect[player_i]->GetUserData();
                                    data->b_is_carried=true;
                                    data->bp_carrier_body=player_body;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_hook_connect,1.0);
                                }
                                m_pEvent_flag_hook[player_i]=ev_idle;//reset
                            }
                            //else cout<<"Hook not connectable\n";
                        }
                    }

                    //disconnect hook/disconnect from main ship
                    if(gamepad_data[player_i].button_B && !gamepad_data[player_i].button_Y)
                    {
                        //if connected to ship, disconnect
                        if(m_vec_players[player_i].connected_to_mship() && !m_vec_players[player_i].is_spawning())
                        {
                            bool ignore_eject=false;
                            if(m_on_tutorial_level)
                            {
                                //blocked until told to eject
                                if( !m_hud.is_helptext_completed(ht_hud3) )
                                {
                                    ignore_eject=true;
                                }
                                else m_hud.set_draw_tutorial_helptext(ht_eject,true);

                                //not possible in tutorial
                                //if(m_pMain_ship->is_takeoff()) ignore_eject=true;
                            }

                            if(!ignore_eject)
                            {
                                m_pMain_ship->player_inside(player_i,false);
                                m_vec_players[player_i].disconnect_from_mship();
                                //give player ship a boost
                                float sens=30000.0*time_dif;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.0,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_explosion( player_body->GetWorldPoint( b2Vec2(0.0,0.3) ),50,100,1.0 );
                                //for(int i=0;i<40;i++)//add several particles
                                // m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ),force);

                                //abort takeoff possibilities
                                m_pMain_ship->set_all_players_on_ship(false);

                                //play sound
                                m_pSound->playSimpleSound(wav_mship_disconnect,0.3);
                            }
                        }
                        //cout<<m_vec_players[player_i].connected_to_mship()<<", "<<m_vec_players[player_i].is_spawning()<<endl;

                        //if hook connected, disconnect
                        if(m_vec_players[player_i].player_hook_connected())//only if connected
                        {
                            //destroy hook joint
                            m_vec_players[player_i].hook_disconnect();

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,1.0);
                        }
                    }
                }

                //thrusters
                if(gamepad_data[player_i].button_left_trigger && !m_vec_players[player_i].connected_to_mship())
                {
                    //test if player have fuel
                    if(m_vec_players[player_i].get_rel_fuel()>0.0 && m_vec_players[player_i].get_mship_dock_status()==0)
                    {
                        /*//exp sens
                        float trigger_sens=(float)gamepad_data[player_i].trigger_left/255.0*
                                           (float)gamepad_data[player_i].trigger_left/255.0;
                        float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                                   trigger_sens*time_dif;*/

                        //linear sens
                        float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                                   (float)gamepad_data[player_i].trigger_left/255.0*time_dif;

                        //extra fuel consumption if cloaked
                        if(m_vec_players[player_i].m_cloak_timer<m_vec_players[player_i].m_cloak_delay)
                        {
                            sens*=0.7;//weaker engine if cloaked
                            b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                            player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                            //no particle
                            //consume fuel
                            m_vec_players[player_i].change_fuel(-sens*_player_fuel_consumption_factor*10.0);
                        }
                        else//normal consumption
                        {
                            float extra_force=1.0;
                            if(m_vec_players[player_i].lifting_a_player_ship()) extra_force=_player_extra_friendly_lift_force;
                            b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                            player_body->ApplyForce(extra_force*force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                            //add particle to engine
                            m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.3,0.3) ),force);
                            //consume fuel
                            m_vec_players[player_i].change_fuel(-sens*_player_fuel_consumption_factor);
                        }
                    }

                    //out of fuel message
                    if(m_on_tutorial_level)
                    {
                        //out of fuel test
                        if(m_vec_players[player_i].get_rel_fuel()<=0.0 && !m_hud.m_text_out_of_fuel_shown)
                        {
                            //display message
                            float pos[2]={ player_body->GetPosition().x*_Met2Pix-512.0 ,
                                           player_body->GetPosition().y*_Met2Pix };
                            m_hud.show_out_of_fuel(pos);
                        }
                    }
                }
                if(gamepad_data[player_i].button_right_trigger && !m_vec_players[player_i].connected_to_mship())
                {
                    //test if player have fuel
                    if(m_vec_players[player_i].get_rel_fuel()>0.0 && m_vec_players[player_i].get_mship_dock_status()==0)
                    {
                        /*//exp sens
                        float trigger_sens=(float)gamepad_data[player_i].trigger_right/255.0*
                                           (float)gamepad_data[player_i].trigger_right/255.0;
                        float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                                   trigger_sens*time_dif;*/

                        //linear sens
                        float sens=m_vec_players[player_i].m_motor_thrust_power_max*
                                   (float)gamepad_data[player_i].trigger_right/255.0*time_dif;

                        //extra fuel consumption if cloaked
                        if(m_vec_players[player_i].m_cloak_timer<m_vec_players[player_i].m_cloak_delay)
                        {
                            sens*=0.7;//weaker engine if cloaked
                            b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                            player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                            //no particle
                            //consume fuel
                            m_vec_players[player_i].change_fuel(-sens*_player_fuel_consumption_factor*10.0);
                        }
                        else//normal consumption
                        {
                            float extra_force=1.0;
                            if(m_vec_players[player_i].lifting_a_player_ship()) extra_force=_player_extra_friendly_lift_force;
                            b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                            player_body->ApplyForce(extra_force*force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                            //add particle to engine
                            m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.3,0.3) ),force);
                            //consume fuel
                            m_vec_players[player_i].change_fuel(-sens*_player_fuel_consumption_factor);
                        }
                    }
                }
                //report trigger value to HUD
                m_hud.set_player_trigger_value_left (player_i,(float)gamepad_data[player_i].trigger_left/255.0);
                m_hud.set_player_trigger_value_right(player_i,(float)gamepad_data[player_i].trigger_right/255.0);
                //set motor sound
                if( (gamepad_data[player_i].button_left_trigger || gamepad_data[player_i].button_right_trigger) &&
                     m_vec_players[player_i].get_rel_fuel()>0.0 )
                {
                    //play sound

                    //calc sound area
                    b2Vec2 pos=player_body->GetPosition();
                    int sound_box=0;//sound off
                    float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                    if(pos.x*_Met2Pix>view_pos[0] &&
                       pos.x*_Met2Pix<view_pos[2] &&
                       pos.y*_Met2Pix>view_pos[1] &&
                       pos.y*_Met2Pix<view_pos[3] )
                    {
                        sound_box=1;//on screen
                    }
                    else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                            pos.x*_Met2Pix<view_pos[0] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=2;//left side
                    }
                    else if(pos.x*_Met2Pix>view_pos[2] &&
                            pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=3;//right side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[1] )
                    {
                        sound_box=4;//top side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[3] &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=5;//top side
                    }

                    if(sound_box!=0)
                    {
                        float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                            0,1,0, 0,0,-1, 0,0,0,
                                            1,  1,  0};
                        switch(sound_box)
                        {
                            case 0: break;//no sound
                            case 1:
                            {
                                sound_data[14]=0;
                            }break;
                            case 2://left
                            {
                                sound_data[12]=-_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 3://right
                            {
                                sound_data[12]=_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 4:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 5:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                        }

                        int channel_id=0;
                        switch(player_i)
                        {
                            case 0: channel_id=_sound_chan_motor_p1; break;
                            case 1: channel_id=_sound_chan_motor_p2; break;
                            case 2: channel_id=_sound_chan_motor_p3; break;
                            case 3: channel_id=_sound_chan_motor_p4; break;
                        }
                        float sound_volume=((float)gamepad_data[player_i].trigger_left+
                                            (float)gamepad_data[player_i].trigger_right)/510.0;
                        //if( gamepad_data[player_i].trigger_left<gamepad_data[player_i].trigger_right )
                        // sound_volume=(float)gamepad_data[player_i].trigger_right/255.0;
                        float sound_pitch=0.5+sound_volume*0.5;
                        sound_data[18]=sound_pitch;
                        sound_data[19]*=sound_volume;

                        //if stealth is on, lower volume
                        if(m_vec_players[player_i].m_cloak_timer<m_vec_players[player_i].m_cloak_delay)
                        {
                            //sound_data[18]*=0.1;
                            sound_data[19]=0.1+0.2*(m_vec_players[player_i].m_cloak_timer/m_vec_players[player_i].m_cloak_delay);
                        }

                        m_pSound->updateSound(channel_id,sound_data);
                    }
                }
                else//stop sound
                {
                    int channel_id=0;
                    switch(player_i)
                    {
                        case 0: channel_id=_sound_chan_motor_p1; break;
                        case 1: channel_id=_sound_chan_motor_p2; break;
                        case 2: channel_id=_sound_chan_motor_p3; break;
                        case 3: channel_id=_sound_chan_motor_p4; break;
                    }
                    float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                          0,1,0, 0,0,0, 0,0,0,
                                          1,  0,  0};

                    //unless gyro is on
                    if(m_vec_players[player_i].m_gyro_on)
                    {
                        //play sound
                        sound_data[19]=0.2;
                    }

                    m_pSound->updateSound(channel_id,sound_data);
                }

                //fire
                if(gamepad_data[player_i].button_A && !m_vec_players[player_i].connected_to_mship())
                {
                    if(m_on_tutorial_level)
                    {
                        m_hud.set_draw_tutorial_helptext(ht_fire,true);
                    }

                    if( m_vec_players[player_i].fire_turret(time_dif) )
                    {
                        float pos[2]={m_vec_players[player_i].get_body_ptr()->GetPosition().x*_Met2Pix,
                                      m_vec_players[player_i].get_body_ptr()->GetPosition().y*_Met2Pix};
                        //play weapon specific sound
                        switch(m_vec_players[player_i].get_weapon_ptr()->get_type())
                        {
                            case wt_pea:     m_pSound->play_sound_w_test(wav_weapon_pea,pos); break;
                            case wt_spread:  m_pSound->play_sound_w_test(wav_weapon_spread,pos); break;
                            case wt_burst:   m_pSound->play_sound_w_test(wav_weapon_pea,pos); break;
                            case wt_rapid:   m_pSound->play_sound_w_test(wav_weapon_pea,pos); break;
                            case wt_rocket:  m_pSound->play_sound_w_test(wav_weapon_rocket,pos); break;
                            case wt_grenade: m_pSound->play_sound_w_test(wav_weapon_grenade,pos); break;
                            case wt_cannon:  m_pSound->play_sound_w_test(wav_weapon_cannon,pos); break;
                            case wt_beam:    m_pSound->play_sound_w_test(wav_weapon_laser,pos); break;
                            case wt_mine:    m_pSound->play_sound_w_test(wav_weapon_mine,pos); break;
                        }
                    }

                    //if cloaked, turn off
                    if(m_vec_players[player_i].m_cloak_timer!=m_vec_players[player_i].m_cloak_delay)
                    {
                        m_vec_players[player_i].m_cloak_timer=m_vec_players[player_i].m_cloak_delay;
                        player_data->b_cloaked=false;
                    }
                }

                //show hud
                int show_hud_trigger_val=10000;
                if(gamepad_data[player_i].thumbstick_left_x>show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_left_x<-show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_left_y>show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_left_y<-show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_right_x>show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_right_x<-show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_right_y>show_hud_trigger_val ||
                   gamepad_data[player_i].thumbstick_right_y<-show_hud_trigger_val)
                {
                    m_hud.show_hud(player_i);
                }

                //activate/deactivate gear
                bool boosted=false;
                if(gamepad_data[player_i].button_X && !m_vec_players[player_i].m_key_trigger_use_gear)
                {
                    m_vec_players[player_i].m_key_trigger_use_gear=true;

                    int gear_now_on=-1;//sound disabled

                    gear* pGear_to_use=m_vec_players[player_i].get_gear_ptr();
                    switch(pGear_to_use->get_type())
                    {
                        case gt_unarmed:
                        {
                            ;//do nothing
                        }break;

                        case gt_cloak:
                        {
                            if(!m_vec_players[player_i].connected_to_mship())//not if docked
                            {
                                //toggle cloak on/off
                                m_vec_players[player_i].m_cloak_target_off=!m_vec_players[player_i].m_cloak_target_off;
                                cout<<"Cloak is ";
                                if(m_vec_players[player_i].m_cloak_target_off) cout<<"OFF\n";
                                else cout<<"ON\n";

                                if(m_vec_players[player_i].m_cloak_target_off) gear_now_on=0;
                                else gear_now_on=1;
                            }

                        }break;

                        case gt_gyro:
                        {
                            if(!m_vec_players[player_i].connected_to_mship())//not if docked
                            {
                                if(m_on_tutorial_level)
                                 m_hud.set_draw_tutorial_helptext(ht_use_gear,true);

                                m_vec_players[player_i].m_gyro_on=!m_vec_players[player_i].m_gyro_on;

                                if(m_vec_players[player_i].m_gyro_on) gear_now_on=1;
                                else gear_now_on=0;
                            }
                        }break;

                        case gt_boost:
                        {
                            //activate extra thrust
                            if(m_vec_players[player_i].get_rel_fuel()>0.0 && m_vec_players[player_i].get_mship_dock_status()==0)
                            {
                                boosted=true;

                                if(m_vec_players[player_i].m_boost_timer<m_vec_players[player_i].m_boost_delay)
                                 m_vec_players[player_i].m_boost_timer+=time_dif;
                                if(m_vec_players[player_i].m_boost_timer>m_vec_players[player_i].m_boost_delay)
                                 m_vec_players[player_i].m_boost_timer=m_vec_players[player_i].m_boost_delay;
                                float extra_thrust=m_vec_players[player_i].m_boost_timer/m_vec_players[player_i].m_boost_delay*
                                                   m_vec_players[player_i].m_boost_multiplyer;
                                float sens=300.0*time_dif*extra_thrust;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.4,0.3) ),force);
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ),force);
                                //consume fuel
                                m_vec_players[player_i].change_fuel(-sens*_player_fuel_consumption_factor*10.0);

                                //play boost sound
                                //calc sound area
                                b2Vec2 pos=player_body->GetPosition();
                                int sound_box=0;//sound off
                                float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                                if(pos.x*_Met2Pix>view_pos[0] &&
                                   pos.x*_Met2Pix<view_pos[2] &&
                                   pos.y*_Met2Pix>view_pos[1] &&
                                   pos.y*_Met2Pix<view_pos[3] )
                                {
                                    sound_box=1;//on screen
                                }
                                else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                        pos.x*_Met2Pix<view_pos[0] &&
                                        pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                        pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                {
                                    sound_box=2;//left side
                                }
                                else if(pos.x*_Met2Pix>view_pos[2] &&
                                        pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                        pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                        pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                {
                                    sound_box=3;//right side
                                }
                                else if(pos.x*_Met2Pix>view_pos[0] &&
                                        pos.x*_Met2Pix<view_pos[2] &&
                                        pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                        pos.y*_Met2Pix<view_pos[1] )
                                {
                                    sound_box=4;//top side
                                }
                                else if(pos.x*_Met2Pix>view_pos[0] &&
                                        pos.x*_Met2Pix<view_pos[2] &&
                                        pos.y*_Met2Pix>view_pos[3] &&
                                        pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                {
                                    sound_box=5;//top side
                                }

                                if(sound_box!=0)
                                {
                                    float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                        0,1,0, 0,0,-1, 0,0,0,
                                                        1,  1,  0};
                                    switch(sound_box)
                                    {
                                        case 0: break;//no sound
                                        case 1:
                                        {
                                            sound_data[14]=0;
                                        }break;
                                        case 2://left
                                        {
                                            sound_data[12]=-_sound_box_side_shift;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                        case 3://right
                                        {
                                            sound_data[12]=_sound_box_side_shift;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                        case 4:
                                        {
                                            sound_data[12]=0;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                        case 5:
                                        {
                                            sound_data[12]=0;
                                            sound_data[19]=_sound_box_level_outside;
                                        }break;
                                    }

                                    int channel_id=0;
                                    switch(player_i)
                                    {
                                        case 0: channel_id=_sound_chan_boost_p1; break;
                                        case 1: channel_id=_sound_chan_boost_p2; break;
                                        case 2: channel_id=_sound_chan_boost_p3; break;
                                        case 3: channel_id=_sound_chan_boost_p4; break;
                                    }

                                    m_pSound->updateSound(channel_id,sound_data);
                                }
                            }
                            else
                            {
                                //stop boost sound
                                int channel_id=0;
                                switch(player_i)
                                {
                                    case 0: channel_id=_sound_chan_boost_p1; break;
                                    case 1: channel_id=_sound_chan_boost_p2; break;
                                    case 2: channel_id=_sound_chan_boost_p3; break;
                                    case 3: channel_id=_sound_chan_boost_p4; break;
                                }
                                float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                                      0,1,0, 0,0,0, 0,0,0,
                                                      1,  0,  0};
                                m_pSound->updateSound(channel_id,sound_data);
                            }
                            //no toggle function
                            m_vec_players[player_i].m_key_trigger_use_gear=false;
                        }break;

                        case gt_turret_rotation:
                        {
                            m_vec_players[player_i].m_turret_aim_on=!m_vec_players[player_i].m_turret_aim_on;

                            if(m_vec_players[player_i].m_turret_aim_on) gear_now_on=1;
                            else gear_now_on=0;
                        }break;

                        case gt_turret_aim:
                        {
                            m_vec_players[player_i].m_turret_aim_on=!m_vec_players[player_i].m_turret_aim_on;

                            if(m_vec_players[player_i].m_turret_aim_on) gear_now_on=1;
                            else gear_now_on=0;
                        }break;

                        case gt_turret_auto_aim:
                        {
                            m_vec_players[player_i].m_turret_aim_on=!m_vec_players[player_i].m_turret_aim_on;

                            if(m_vec_players[player_i].m_turret_aim_on) gear_now_on=1;
                            else gear_now_on=0;
                        }break;

                        case gt_shield:
                        {
                            //shield always active
                        }break;

                        case gt_cam_control:
                        {
                            //eject cam satellite...

                        }break;
                    }

                    //play sound
                    if(gear_now_on!=-1)
                    {
                        b2Vec2 player_pos=player_body->GetPosition();
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(player_pos.x*_Met2Pix>view_pos[0] &&
                           player_pos.x*_Met2Pix<view_pos[2] &&
                           player_pos.y*_Met2Pix>view_pos[1] &&
                           player_pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                player_pos.x*_Met2Pix<view_pos[0] &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[2] &&
                                player_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0] &&
                                player_pos.x*_Met2Pix<view_pos[2] &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0] &&
                                player_pos.x*_Met2Pix<view_pos[2] &&
                                player_pos.y*_Met2Pix>view_pos[3] &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }

                            if(gear_now_on==1) m_pSound->playWAVE(wav_gear_enable,sound_data);
                            else m_pSound->playWAVE(wav_gear_disable,sound_data);
                        }
                    }
                }
                else if(!gamepad_data[player_i].button_X) m_vec_players[player_i].m_key_trigger_use_gear=false;

                //weapon/gear swap, if docked
                if(m_vec_players[player_i].connected_to_mship())
                {
                    //show hud if connected to mship
                    m_hud.show_hud(player_i);

                    int trigger_val=30000;
                    int reset_val=20000;

                    //swap weapon, left/right
                    if(!m_vec_pWeapon_stored.empty())//storage have to contain something
                    {
                        //next
                        if( !m_vec_players[player_i].m_key_trigger_weapon_swap &&
                            gamepad_data[player_i].thumbstick_left_x>trigger_val)
                        {
                            m_vec_players[player_i].m_key_trigger_weapon_swap=true;

                            //get current index
                            bool use_default_weapon=false;
                            int weapon_index=m_vec_players[player_i].get_weapon_index()+1;
                            //if index is outside limit AND not using default weapon, use default weapon
                            //if index is outside limit AND using default weapon, reset index
                            if( weapon_index<0 || weapon_index>=(int)m_vec_pWeapon_stored.size()  )
                            {
                                if( m_vec_players[player_i].m_using_default_weapon )//reset index
                                {
                                    weapon_index=0;
                                }
                                else//use default weapon
                                {
                                    //fake element outside vector is the default weapon
                                    use_default_weapon=true;
                                    weapon_index++;
                                }
                            }
                            m_vec_players[player_i].set_weapon_index(weapon_index);
                            //cout<<"index: "<<weapon_index<<endl;

                            if(use_default_weapon)
                            {
                                //put old weapon in vector
                                weapon* old_weapon=m_vec_players[player_i].get_weapon_ptr();
                                m_vec_pWeapon_stored.push_back(old_weapon);
                                //swap to default
                                m_vec_players[player_i].use_default_weapon();
                            }
                            else//pick weapon from vector
                            {
                                //get and swap weapon
                                weapon* new_weapon=m_vec_pWeapon_stored[weapon_index];
                                //if player's current weapon is the default, do not put in vector
                                if(m_vec_players[player_i].m_using_default_weapon)
                                {
                                    //take step back in vector due to shortening of vector
                                    m_vec_players[player_i].set_weapon_index(weapon_index-1);
                                    //remove the taken weapon from vector
                                    m_vec_pWeapon_stored.erase( m_vec_pWeapon_stored.begin()+weapon_index );
                                }
                                else//store old weapon in storage
                                {
                                    m_vec_pWeapon_stored[weapon_index]=m_vec_players[player_i].get_weapon_ptr();
                                }
                                //give new weapon to the player
                                m_vec_players[player_i].set_current_weapon(new_weapon);
                            }

                            //update hud color TEMP
                            m_hud.set_weapon_color(player_i,m_vec_players[player_i].get_weapon_ptr()->m_weapon_color);

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);

                            //show on hud
                            m_hud.draw_selected_item(it_weapon,player_i);
                        }
                        //prev
                        if( !m_vec_players[player_i].m_key_trigger_weapon_swap &&
                            gamepad_data[player_i].thumbstick_left_x<-trigger_val)
                        {
                            m_vec_players[player_i].m_key_trigger_weapon_swap=true;
                            //get index
                            bool use_default_weapon=false;
                            int weapon_index=m_vec_players[player_i].get_weapon_index();
                            //if index is outside limit AND not using default weapon, use default weapon
                            //if index is outside limit AND using default weapon, reset index
                            if( weapon_index<0 || weapon_index>=(int)m_vec_pWeapon_stored.size()  )
                            {
                                if( m_vec_players[player_i].m_using_default_weapon )//reset index
                                {
                                    weapon_index=(int)m_vec_pWeapon_stored.size()-1;
                                }
                                else//use default weapon
                                {
                                    //fake element outside vector is the default weapon
                                    use_default_weapon=true;
                                }
                            }
                            m_vec_players[player_i].set_weapon_index(weapon_index-1);
                            //cout<<"index: "<<weapon_index<<endl;

                            if(use_default_weapon)
                            {
                                //put old weapon in vector (push front)
                                weapon* old_weapon=m_vec_players[player_i].get_weapon_ptr();
                                vector<weapon*> vec_weapon_copy=m_vec_pWeapon_stored;
                                m_vec_pWeapon_stored.clear();
                                m_vec_pWeapon_stored.push_back(old_weapon);
                                m_vec_pWeapon_stored.insert( m_vec_pWeapon_stored.end(), vec_weapon_copy.begin(), vec_weapon_copy.end() );
                                //swap to default
                                m_vec_players[player_i].use_default_weapon();
                            }
                            else//pick weapon from vector
                            {
                                //get and swap weapon
                                weapon* new_weapon=m_vec_pWeapon_stored[weapon_index];
                                //if player's current weapon is the default, do not put in vector
                                if(m_vec_players[player_i].m_using_default_weapon)
                                {
                                    //take step back in vector due to shortening of vector
                                    //m_vec_players[player_i].set_weapon_index(weapon_index-1);
                                    //remove the taken weapon from vector
                                    m_vec_pWeapon_stored.erase( m_vec_pWeapon_stored.begin()+weapon_index );
                                }
                                else//store old weapon in storage
                                {
                                    m_vec_pWeapon_stored[weapon_index]=m_vec_players[player_i].get_weapon_ptr();
                                }
                                //give new weapon to the player
                                m_vec_players[player_i].set_current_weapon(new_weapon);
                            }

                            //update hud color TEMP
                            m_hud.set_weapon_color(player_i,m_vec_players[player_i].get_weapon_ptr()->m_weapon_color);

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);

                            //show on hud
                            m_hud.draw_selected_item(it_weapon,player_i);
                        }
                        //reset
                        if( m_vec_players[player_i].m_key_trigger_weapon_swap &&
                            fabs(gamepad_data[player_i].thumbstick_left_x)<reset_val)
                        {
                            m_vec_players[player_i].m_key_trigger_weapon_swap=false;
                        }
                    }
                    //if no weapons in inventory, but is not using default weapon, could swap to default and store current weapon
                    else if(!m_vec_players[player_i].m_using_default_weapon)
                    {
                        //next or prev
                        if( !m_vec_players[player_i].m_key_trigger_weapon_swap &&
                            (gamepad_data[player_i].thumbstick_left_x>trigger_val ||
                             gamepad_data[player_i].thumbstick_left_x<-trigger_val) )
                        {
                            m_vec_players[player_i].m_key_trigger_weapon_swap=true;

                            //put old weapon in vector
                            weapon* old_weapon=m_vec_players[player_i].get_weapon_ptr();
                            m_vec_pWeapon_stored.push_back(old_weapon);
                            //swap to default
                            m_vec_players[player_i].use_default_weapon();

                            //update hud color TEMP
                            m_hud.set_weapon_color(player_i,m_vec_players[player_i].get_weapon_ptr()->m_weapon_color);

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);

                            //show on hud
                            m_hud.draw_selected_item(it_weapon,player_i);
                        }
                        //reset
                        if( m_vec_players[player_i].m_key_trigger_weapon_swap &&
                            fabs(gamepad_data[player_i].thumbstick_left_x)<reset_val)
                        {
                            m_vec_players[player_i].m_key_trigger_weapon_swap=false;
                        }
                    }

                    //swap gear, up/down
                    if(!m_vec_pGear_stored.empty())//storage have to contain something
                    {
                        //next
                        if( !m_vec_players[player_i].m_key_trigger_gear_swap &&
                            gamepad_data[player_i].thumbstick_right_x>trigger_val)
                        {
                            m_vec_players[player_i].m_key_trigger_gear_swap=true;

                            //get current index
                            bool use_default_gear=false;
                            int gear_index=m_vec_players[player_i].get_gear_index()+1;
                            //if index is outside limit AND not using default gear, use default gear
                            //if index is outside limit AND using default gear, reset index
                            if( gear_index<0 || gear_index>=(int)m_vec_pGear_stored.size()  )
                            {
                                if( m_vec_players[player_i].m_using_default_gear )//reset index
                                {
                                    gear_index=0;
                                }
                                else//use default gear
                                {
                                    //fake element outside vector is the default gear
                                    use_default_gear=true;
                                    gear_index++;
                                }
                            }
                            m_vec_players[player_i].set_gear_index(gear_index);
                            //cout<<"index: "<<gear_index<<endl;

                            if(use_default_gear)
                            {
                                //put old gear in vector
                                gear* old_gear=m_vec_players[player_i].get_gear_ptr();
                                m_vec_pGear_stored.push_back(old_gear);
                                //swap to default
                                m_vec_players[player_i].use_default_gear();
                            }
                            else//pick gear from vector
                            {
                                //get and swap gear
                                gear* new_gear=m_vec_pGear_stored[gear_index];
                                //if player's current gear is the default, do not put in vector
                                if(m_vec_players[player_i].m_using_default_gear)
                                {
                                    //take step back in vector due to shortening of vector
                                    m_vec_players[player_i].set_gear_index(gear_index-1);
                                    //remove the taken gear from vector
                                    m_vec_pGear_stored.erase( m_vec_pGear_stored.begin()+gear_index );
                                }
                                else//store old gear in storage
                                {
                                    m_vec_pGear_stored[gear_index]=m_vec_players[player_i].get_gear_ptr();
                                }
                                //give new gear to the player
                                m_vec_players[player_i].set_current_gear(new_gear);
                            }

                            //if selected gear is the shield, get shield data
                            if(m_vec_players[player_i].get_gear_ptr()->get_type()==gt_shield)
                            {
                                float shield_data[3]={m_vec_players[player_i].m_shield_regen_delay,
                                                      m_vec_players[player_i].m_shield_hp_max,
                                                      m_vec_players[player_i].m_shield_regen_speed};
                                //update current values
                                m_vec_players[player_i].get_gear_ptr()->get_shield_data(shield_data);
                            }

                            //update hud color TEMP
                            m_hud.set_gear_color(player_i,m_vec_players[player_i].get_gear_ptr()->m_gear_color);

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);

                            //show on hud
                            m_hud.draw_selected_item(it_gear,player_i);
                        }
                        //prev
                        if( !m_vec_players[player_i].m_key_trigger_gear_swap &&
                            gamepad_data[player_i].thumbstick_right_x<-trigger_val)
                        {
                            m_vec_players[player_i].m_key_trigger_gear_swap=true;
                            //get index
                            bool use_default_gear=false;
                            int gear_index=m_vec_players[player_i].get_gear_index();
                            //if index is outside limit AND not using default gear, use default gear
                            //if index is outside limit AND using default gear, reset index
                            if( gear_index<0 || gear_index>=(int)m_vec_pGear_stored.size()  )
                            {
                                if( m_vec_players[player_i].m_using_default_gear )//reset index
                                {
                                    gear_index=(int)m_vec_pGear_stored.size()-1;
                                }
                                else//use default gear
                                {
                                    //fake element outside vector is the default gear
                                    use_default_gear=true;
                                }
                            }
                            m_vec_players[player_i].set_gear_index(gear_index-1);
                            //cout<<"index: "<<gear_index<<endl;

                            if(use_default_gear)
                            {
                                //put old gear in vector (push front)
                                gear* old_gear=m_vec_players[player_i].get_gear_ptr();
                                vector<gear*> vec_gear_copy=m_vec_pGear_stored;
                                m_vec_pGear_stored.clear();
                                m_vec_pGear_stored.push_back(old_gear);
                                m_vec_pGear_stored.insert( m_vec_pGear_stored.end(), vec_gear_copy.begin(), vec_gear_copy.end() );
                                //swap to default
                                m_vec_players[player_i].use_default_gear();
                            }
                            else//pick gear from vector
                            {
                                //get and swap gear
                                gear* new_gear=m_vec_pGear_stored[gear_index];
                                //if player's current gear is the default, do not put in vector
                                if(m_vec_players[player_i].m_using_default_gear)
                                {
                                    //take step back in vector due to shortening of vector
                                    //m_vec_players[player_i].set_gear_index(gear_index-1);
                                    //remove the taken gear from vector
                                    m_vec_pGear_stored.erase( m_vec_pGear_stored.begin()+gear_index );
                                }
                                else//store old gear in storage
                                {
                                    m_vec_pGear_stored[gear_index]=m_vec_players[player_i].get_gear_ptr();
                                }
                                //give new gear to the player
                                m_vec_players[player_i].set_current_gear(new_gear);
                            }

                            //if selected gear is the shield, get shield data
                            if(m_vec_players[player_i].get_gear_ptr()->get_type()==gt_shield)
                            {
                                float shield_data[3]={m_vec_players[player_i].m_shield_regen_delay,
                                                      m_vec_players[player_i].m_shield_hp_max,
                                                      m_vec_players[player_i].m_shield_regen_speed};
                                //update current values
                                m_vec_players[player_i].get_gear_ptr()->get_shield_data(shield_data);
                            }

                            //update hud color TEMP
                            m_hud.set_gear_color(player_i,m_vec_players[player_i].get_gear_ptr()->m_gear_color);

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);

                            //show on hud
                            m_hud.draw_selected_item(it_gear,player_i);
                        }
                        //reset
                        if( m_vec_players[player_i].m_key_trigger_gear_swap &&
                            fabs(gamepad_data[player_i].thumbstick_right_x)<reset_val)
                        {
                            m_vec_players[player_i].m_key_trigger_gear_swap=false;
                        }

                        if(m_on_tutorial_level)
                        {
                            //test of non default gear is selected
                            if( m_vec_players[player_i].get_gear_ptr()!=m_vec_players[player_i].get_default_gear_ptr() )
                             m_hud.set_draw_tutorial_helptext(ht_select_gear,true);
                        }
                    }
                    //if no weapons in inventory, but is not using default gear, could swap to default and store current gear
                    else if(!m_vec_players[player_i].m_using_default_gear)
                    {
                        //next or prev
                        if( !m_vec_players[player_i].m_key_trigger_gear_swap &&
                            (gamepad_data[player_i].thumbstick_right_x>trigger_val ||
                             gamepad_data[player_i].thumbstick_right_x<-trigger_val) )
                        {
                            m_vec_players[player_i].m_key_trigger_gear_swap=true;

                            //put old gear in vector
                            gear* old_gear=m_vec_players[player_i].get_gear_ptr();
                            m_vec_pGear_stored.push_back(old_gear);
                            //swap to default
                            m_vec_players[player_i].use_default_gear();

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);

                            //show on hud
                            m_hud.draw_selected_item(it_gear,player_i);
                        }
                        //reset
                        if( m_vec_players[player_i].m_key_trigger_gear_swap &&
                            fabs(gamepad_data[player_i].thumbstick_right_x)<reset_val)
                        {
                            m_vec_players[player_i].m_key_trigger_gear_swap=false;
                        }

                        //update hud color TEMP
                        m_hud.set_gear_color(player_i,m_vec_players[player_i].get_gear_ptr()->m_gear_color);
                    }
                }

                //swap tow line/fuel line (not on tutorial)
                if(m_vec_players[player_i].connected_to_mship() && !m_on_tutorial_level)
                {
                    int trigger_val=30000;
                    int reset_val=20000;

                    if( gamepad_data[player_i].thumbstick_right_y>trigger_val ||
                        gamepad_data[player_i].thumbstick_right_y<-trigger_val )
                    {
                        if(!m_vec_players[player_i].m_key_trigger_line_swap)
                        {
                            //swap line
                            m_vec_players[player_i].m_key_trigger_line_swap=true;

                            m_vec_players[player_i].m_line_type_tow=!m_vec_players[player_i].m_line_type_tow;

                            //show on hud
                            m_hud.draw_selected_item(it_line,player_i);

                            //play sound
                            m_pSound->playSimpleSound(wav_hook_disconnect,0.5);
                        }
                    }

                    //reset test
                    if( fabs(gamepad_data[player_i].thumbstick_right_y)<reset_val )
                    {
                        m_vec_players[player_i].m_key_trigger_line_swap=false;
                    }
                }

                //test fuel transfer
                if(!m_vec_players[player_i].m_line_type_tow && m_vec_players[player_i].player_hook_connected())
                {
                    int player_acceptor=m_vec_players[player_i].m_fuel_line_to_player_ind;
                    //test if needs fuel
                    if(m_vec_players[player_acceptor].get_rel_fuel()<1.0)
                    {
                        //test if doner have any fuel
                        if(m_vec_players[player_i].get_rel_fuel()>0.0)
                        {
                            m_vec_players[player_i].change_fuel(-_fuel_transfer_speed);
                            m_vec_players[player_acceptor].change_fuel(_fuel_transfer_speed);

                            //for sound later
                            fuel_transfer_active=true;
                        }
                    }
                }

                //upgrade ship, if docked
                if( m_vec_players[player_i].connected_to_mship() && !m_vec_players[player_i].is_spawning() )
                {
                    bool button_pressed=false;
                    bool upgrade_made=false;
                    bool motor_upgrade_made=false;
                    //hp, up
                    if(gamepad_data[player_i].dpad_up)
                    {
                        button_pressed=true;
                        //make upgrade
                        if(!m_vec_players[player_i].m_key_trigger_dpad)
                        {
                            //if mship have enough resources
                            if(m_pMain_ship->change_resources(-_player_upgrade_cost))
                            {
                                if(m_on_tutorial_level)
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_ship_upgrade,true);
                                }

                                upgrade_made=true;
                                m_vec_players[player_i].m_hp_max+=10.0;
                            }
                        }
                    }
                    //ammo, left
                    if(gamepad_data[player_i].dpad_left)
                    {
                        button_pressed=true;
                        //make upgrade
                        if(!m_vec_players[player_i].m_key_trigger_dpad)
                        {
                            //if mship have enough resources
                            if(m_pMain_ship->change_resources(-_player_upgrade_cost))
                            {
                                if(m_on_tutorial_level)
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_ship_upgrade,true);
                                }

                                upgrade_made=true;
                                m_vec_players[player_i].m_ammo_max+=10.0;
                            }
                        }
                    }
                    //tank, right
                    if(gamepad_data[player_i].dpad_right)
                    {
                        button_pressed=true;
                        //make upgrade
                        if(!m_vec_players[player_i].m_key_trigger_dpad)
                        {
                            //if mship have enough resources
                            if(m_pMain_ship->change_resources(-_player_upgrade_cost))
                            {
                                if(m_on_tutorial_level)
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_ship_upgrade,true);
                                }

                                upgrade_made=true;
                                m_vec_players[player_i].m_fuel_max+=10.0;
                            }
                        }
                    }
                    //motor, down
                    if(gamepad_data[player_i].dpad_down)
                    {
                        button_pressed=true;
                        //make upgrade
                        if(!m_vec_players[player_i].m_key_trigger_dpad)
                        {
                            //if mship have enough resources
                            if(m_pMain_ship->change_resources(-_player_upgrade_cost))
                            {
                                if(m_on_tutorial_level)
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_ship_upgrade,true);
                                }

                                upgrade_made=true;
                                motor_upgrade_made=true;//will not increase ship mass
                                m_vec_players[player_i].m_motor_thrust_power_max+=10.0;
                            }
                        }
                    }

                    //update ship mass
                    if(upgrade_made && !motor_upgrade_made)
                    {
                        m_vec_players[player_i].change_ship_mass(_player_ship_upgrade_mass_shift);
                        //update player body pointer vector and hook sensor vector for contact listener
                        m_ppPlayer_bodies[player_i]=m_vec_players[player_i].get_body_ptr();
                        m_ppRope_hook_sensors[player_i]=m_vec_players[player_i].get_rope_hook_sensor();

                        //update hud's player pointer
                        if(m_on_tutorial_level)
                        {
                            for(int player_search_i=0;player_search_i<4;player_search_i++)
                            {
                                if(m_player_active[player_search_i])
                                {
                                    m_hud.set_player_ptr(m_vec_players[player_search_i].get_body_ptr());
                                    break;
                                }
                            }
                        }
                    }

                    //set reset value
                    if(!button_pressed) m_vec_players[player_i].m_key_trigger_dpad=false;
                    else                m_vec_players[player_i].m_key_trigger_dpad=true;

                    //increment upgrade counter
                    if(upgrade_made) m_vec_players[player_i].m_upgrade_counter++;

                    //play sound
                    if(upgrade_made) m_pSound->playSimpleSound(wav_ship_upgrade,0.2);
                }

                //sell weapon (LB)
                bool item_sold=false;
                if(gamepad_data[player_i].button_LB && m_vec_players[player_i].connected_to_mship())
                {
                    if(!m_vec_players[player_i].m_key_trigger_LB)//press-release key
                    {
                        //only if not default is selected
                        if(!m_vec_players[player_i].m_using_default_weapon)
                        {
                            delete m_vec_players[player_i].get_weapon_ptr();

                            //set default
                            m_vec_players[player_i].use_default_weapon();
                            //update hud color TEMP
                            m_hud.set_weapon_color(player_i,m_vec_players[player_i].get_weapon_ptr()->m_weapon_color);

                            //get resource
                            m_pMain_ship->change_resources(_player_upgrade_cost);

                            item_sold=true;

                            cout<<"Weapon recycled\n";
                        }
                    }
                    m_vec_players[player_i].m_key_trigger_LB=true;
                }
                else m_vec_players[player_i].m_key_trigger_LB=false;

                //sell gear (RB)
                if(gamepad_data[player_i].button_RB && m_vec_players[player_i].connected_to_mship())
                {
                    if(!m_vec_players[player_i].m_key_trigger_RB)//press-release key
                    {
                        //only if not default is selected
                        if(!m_vec_players[player_i].m_using_default_gear &&
                           m_vec_players[player_i].get_gear_ptr()->get_type()!=gt_cam_control)//can not sell cam_control
                        {
                            if(m_on_tutorial_level)
                            {
                                m_hud.set_draw_tutorial_helptext(ht_recycle_now,true);
                            }

                            gear* selected_gear=m_vec_players[player_i].get_gear_ptr();
                            switch(selected_gear->get_type())
                            {
                                //reset turret angle
                                case gt_turret_rotation: m_vec_players[player_i].set_turret_angle(0.0); break;
                                case gt_turret_aim:      m_vec_players[player_i].set_turret_angle(0.0); break;
                                case gt_turret_auto_aim: m_vec_players[player_i].set_turret_angle(0.0); break;

                                //turn off gyro
                                case gt_gyro: m_vec_players[player_i].m_gyro_on=false; break;
                                //turn off cloak
                                case gt_cloak: m_vec_players[player_i].m_cloak_target_off=true;
                                               player_data->b_cloaked=false; break;
                                //turn off shield
                                case gt_shield: m_vec_players[player_i].m_shield_hp_curr=0.0; break;
                            }

                            delete m_vec_players[player_i].get_gear_ptr();

                            //set default
                            m_vec_players[player_i].use_default_gear();
                            //update hud color TEMP
                            m_hud.set_gear_color(player_i,m_vec_players[player_i].get_gear_ptr()->m_gear_color);

                            //get resource
                            m_pMain_ship->change_resources(_player_upgrade_cost);

                            item_sold=true;

                            cout<<"Gear recycled\n";
                        }
                    }
                    m_vec_players[player_i].m_key_trigger_RB=true;
                }
                else m_vec_players[player_i].m_key_trigger_RB=false;
                //play sound
                if(item_sold)
                {
                    //calc sound area
                    b2Vec2 pos=player_body->GetPosition();
                    int sound_box=0;//sound off
                    float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                    if(pos.x*_Met2Pix>view_pos[0] &&
                       pos.x*_Met2Pix<view_pos[2] &&
                       pos.y*_Met2Pix>view_pos[1] &&
                       pos.y*_Met2Pix<view_pos[3] )
                    {
                        sound_box=1;//on screen
                    }
                    else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                            pos.x*_Met2Pix<view_pos[0] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=2;//left side
                    }
                    else if(pos.x*_Met2Pix>view_pos[2] &&
                            pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=3;//right side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[1] )
                    {
                        sound_box=4;//top side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[3] &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=5;//top side
                    }

                    if(sound_box!=0)
                    {
                        float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                            0,1,0, 0,0,-1, 0,0,0,
                                            1,  1,  0};
                        switch(sound_box)
                        {
                            case 0: break;//no sound
                            case 1:
                            {
                                sound_data[14]=0;
                            }break;
                            case 2://left
                            {
                                sound_data[12]=-_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 3://right
                            {
                                sound_data[12]=_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 4:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 5:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                        }

                        m_pSound->playWAVE(wav_mship_recycle,sound_data);
                    }
                }

                //update gear
                bool turret_moved=false;
                gear* pGear_curr=m_vec_players[player_i].get_gear_ptr();
                switch(pGear_curr->get_type())
                {
                    case gt_unarmed:
                    {
                        ;//do nothing
                    }break;

                    case gt_cloak:
                    {
                        //update cloak timer (if timer is less than the delay, cloak ON, if equal OFF)
                        if(m_vec_players[player_i].m_cloak_target_off)
                        {
                            //increase timer
                            if(m_vec_players[player_i].m_cloak_timer<m_vec_players[player_i].m_cloak_delay)
                             m_vec_players[player_i].m_cloak_timer+=time_dif;
                            if(m_vec_players[player_i].m_cloak_timer>m_vec_players[player_i].m_cloak_delay)
                             m_vec_players[player_i].m_cloak_timer=m_vec_players[player_i].m_cloak_delay;

                            //set body data to uncloaked
                            player_data->b_cloaked=false;
                        }
                        else//decrease timer
                        {
                            if(m_vec_players[player_i].m_cloak_timer>0.0)
                             m_vec_players[player_i].m_cloak_timer-=time_dif;
                            if(m_vec_players[player_i].m_cloak_timer<0.0)
                            {
                                m_vec_players[player_i].m_cloak_timer=0.0;
                                //set body data to cloaked
                                player_data->b_cloaked=true;
                            }
                        }

                        //if out of fuel, tur off cloak
                        if(m_vec_players[player_i].get_rel_fuel()<=0.0)
                        {
                            m_vec_players[player_i].m_cloak_timer=m_vec_players[player_i].m_cloak_delay;
                            st_body_user_data* data=(st_body_user_data*)( m_vec_players[player_i].get_body_ptr()->GetUserData() );
                            data->b_cloaked=false;
                        }

                    }break;

                    case gt_gyro:
                    {
                        if(m_vec_players[player_i].m_gyro_on && m_vec_players[player_i].get_rel_fuel()>0.0)
                        {
                            //switch off if one thruster is on but not the other
                            if( (gamepad_data[player_i].trigger_left>100 && gamepad_data[player_i].trigger_right<50) ||
                                (gamepad_data[player_i].trigger_right>100 && gamepad_data[player_i].trigger_left<50) )
                             break;

                            //consume fuel
                            m_vec_players[player_i].change_fuel(-0.4*_player_fuel_consumption_factor);

                            //fix tilt
                            float body_angle=player_body->GetAngle()*_Rad2Deg;
                            //wrap
                            while(body_angle> 180.0) body_angle-=360.0;
                            while(body_angle<-180.0) body_angle+=360.0;
                            if(body_angle>m_vec_players[player_i].m_gyro_tilt_limit)
                            {
                                //blast right
                                float sens=0.2*_world_gravity;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.4,0.3) ),force);
                            }
                            else if(body_angle<-m_vec_players[player_i].m_gyro_tilt_limit)
                            {
                                //blast left
                                float sens=0.2*_world_gravity;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ),force);
                            }
                            //adjust tilt
                            if(body_angle>0.0)
                            {
                                //blast right
                                float sens=0.05*_world_gravity;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                                //add particle to engine
                                //m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.4,0.3) ),force);
                            }
                            else if(body_angle<0.0)
                            {
                                //blast left
                                float sens=0.05*_world_gravity;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                                //add particle to engine
                                //m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ),force);
                            }

                            b2Vec2 lin_speed=player_body->GetLinearVelocity();
                            //cout<<"fall: "<<lin_speed.y<<endl;
                            //prevent fall
                            if(lin_speed.y>m_vec_players[player_i].m_gyro_fall_speed_limit)
                            {
                                //blast both
                                float sens=0.80*_world_gravity;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.4,0.3) ),0.5*force);
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ),0.5*force);
                            }
                            //keep height
                            else if(lin_speed.y>0.0)
                            {
                                //blast both
                                float sens=0.20*_world_gravity;
                                b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.4,0.3) ), true );
                                player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ), true );
                                //add particle to engine
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(0.4,0.3) ),0.5*force);
                                m_pParticle_engine->add_particle(player_body->GetWorldPoint( b2Vec2(-0.4,0.3) ),0.5*force);
                            }
                            //else fall
                        }
                    }break;

                    case gt_boost:
                    {
                        if(!gamepad_data[player_i].button_X)
                        {
                            //turn off booster force
                            if(m_vec_players[player_i].m_boost_timer>0.0)
                             m_vec_players[player_i].m_boost_timer-=time_dif;
                            if(m_vec_players[player_i].m_boost_timer<0.0)
                             m_vec_players[player_i].m_boost_timer=0.0;
                        }
                    }break;

                    case gt_turret_rotation:
                    {
                        if(m_vec_players[player_i].m_turret_aim_on)//allow rotaion
                        {
                            //turret control, rotate turret
                            if( fabs(gamepad_data[player_i].thumbstick_left_x)>_thumbstick_deadzone)
                            {
                                float rotation_force=gamepad_data[player_i].thumbstick_left_x/32768.0*
                                                     m_vec_players[player_i].m_turret_rotation_speed_slow*time_dif;
                                m_vec_players[player_i].change_turret_rotation(rotation_force);

                                turret_moved=true;
                            }
                        }
                        else
                        {
                            //reset turret angle
                            float turret_angle=m_vec_players[player_i].get_turret_angle();
                            if(turret_angle>0.0)
                            {
                                float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                m_vec_players[player_i].change_turret_rotation( -rotation_force );

                                turret_moved=true;
                            }
                            else if(turret_angle<0.0)
                            {
                                float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                m_vec_players[player_i].change_turret_rotation( rotation_force );

                                turret_moved=true;
                            }
                            //test if centered
                            float turret_angle_new=m_vec_players[player_i].get_turret_angle();
                            if( (turret_angle<0.0 && turret_angle_new>0.0) ||
                                (turret_angle>0.0 && turret_angle_new<0.0) )
                            {
                                m_vec_players[player_i].set_turret_angle(0.0);
                            }
                        }
                    }break;

                    case gt_turret_aim:
                    {
                        if(m_vec_players[player_i].m_turret_aim_on)//allow rotaion
                        {
                            //turret control, aim with thumbstick
                            if( fabs(gamepad_data[player_i].thumbstick_left_x)>_thumbstick_deadzone ||
                                fabs(gamepad_data[player_i].thumbstick_left_y)>_thumbstick_deadzone )
                            {
                                float target_angle=atan2f(gamepad_data[player_i].thumbstick_left_x,
                                                          gamepad_data[player_i].thumbstick_left_y)*_Rad2Deg;

                                float turret_angle_curr=m_vec_players[player_i].get_turret_angle();
                                if(turret_angle_curr>target_angle)
                                {
                                    float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                    m_vec_players[player_i].change_turret_rotation( -rotation_force );

                                    turret_moved=true;
                                }
                                else if(turret_angle_curr<target_angle)
                                {
                                    float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                    m_vec_players[player_i].change_turret_rotation( rotation_force );

                                    turret_moved=true;
                                }
                                //test if centered
                                float turret_angle_new=m_vec_players[player_i].get_turret_angle();
                                if( (turret_angle_curr<target_angle && turret_angle_new>target_angle) ||
                                    (turret_angle_curr>target_angle && turret_angle_new<target_angle) )
                                {
                                    m_vec_players[player_i].set_turret_angle(target_angle);
                                }
                            }
                        }
                        else
                        {
                            //reset turret angle
                            float turret_angle=m_vec_players[player_i].get_turret_angle();
                            if(turret_angle>0.0)
                            {
                                float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                m_vec_players[player_i].change_turret_rotation( -rotation_force );

                                turret_moved=true;
                            }
                            else if(turret_angle<0.0)
                            {
                                float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                m_vec_players[player_i].change_turret_rotation( rotation_force );

                                turret_moved=true;
                            }
                            //test if centered
                            float turret_angle_new=m_vec_players[player_i].get_turret_angle();
                            if( (turret_angle<0.0 && turret_angle_new>0.0) ||
                                (turret_angle>0.0 && turret_angle_new<0.0) )
                            {
                                m_vec_players[player_i].set_turret_angle(0.0);
                            }
                        }
                    }break;

                    case gt_turret_auto_aim:
                    {
                        if(m_vec_players[player_i].m_turret_aim_on)
                        {
                            //get targets within limit
                            float detection_radius=m_vec_players[player_i].m_turret_range;
                            b2Vec2 center_pos=player_body->GetPosition();
                            b2AABB aabb_box;
                            aabb_box.lowerBound=center_pos-b2Vec2(detection_radius,detection_radius);
                            aabb_box.upperBound=center_pos+b2Vec2(detection_radius,detection_radius);
                            MyQueryCallback aabb_callback;
                            m_pWorld->QueryAABB(&aabb_callback,aabb_box);

                            //translate to bodies
                            vector<b2Body*> vec_bodies_involved;
                            vector<float>   vec_distance;
                            for(int fixture_i=0;fixture_i<(int)aabb_callback.m_vec_fixtures.size();fixture_i++)
                            {
                                b2Body* body_ptr=aabb_callback.m_vec_fixtures[fixture_i]->GetBody();
                                st_body_user_data* data=(st_body_user_data*)body_ptr->GetUserData();
                                //only act on enemies
                                if(data->s_info!="enemy") continue;
                                //ignore dead enemies
                                if(!data->b_alive) continue;

                                //test if new
                                bool is_new=true;
                                for(int body_i=0;body_i<(int)vec_bodies_involved.size();body_i++)
                                {
                                    if( vec_bodies_involved[body_i]==body_ptr )
                                    {
                                        is_new=false;
                                        break;
                                    }
                                }
                                if(!is_new) continue;//body already in vector

                                //test if anything in the way (ON or OFF)
                                MyRayCastCallback raycast;
                                raycast.set_ignore_body(body_ptr);
                                raycast.set_ignore_body_type("rope");
                                raycast.set_ignore_body_type("hook");
                                b2Vec2 target_pos=body_ptr->GetPosition();
                                m_pWorld->RayCast(&raycast,center_pos,target_pos);
                                if(!raycast.m_any_hit)//nothing in the way
                                {
                                    vec_bodies_involved.push_back(body_ptr);
                                    vec_distance.push_back( (center_pos.x-target_pos.x)*(center_pos.x-target_pos.x)+
                                                            (center_pos.y-target_pos.y)*(center_pos.y-target_pos.y) );
                                }
                            }

                            //found something
                            if(!vec_bodies_involved.empty())
                            {
                                //find closest
                                float closest_dist=vec_distance[0];
                                int closest_ind=0;
                                for(int target_i=1;target_i<(int)vec_distance.size();target_i++)
                                {
                                    if(vec_distance[target_i]<closest_dist)
                                    {
                                        closest_dist=vec_distance[target_i];
                                        closest_ind=target_i;
                                    }
                                }

                                //adjust aim
                                b2Vec2 target_direction=vec_bodies_involved[closest_ind]->GetPosition() - center_pos;
                                float target_angle=atan2f( target_direction.x , -target_direction.y )*_Rad2Deg -
                                                   player_body->GetAngle()*_Rad2Deg;

                                float turret_angle_curr=m_vec_players[player_i].get_turret_angle();
                                if(turret_angle_curr>target_angle)
                                {
                                    float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                    m_vec_players[player_i].change_turret_rotation( -rotation_force );

                                    turret_moved=true;
                                }
                                else if(turret_angle_curr<target_angle)
                                {
                                    float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                    m_vec_players[player_i].change_turret_rotation( rotation_force );

                                    turret_moved=true;
                                }
                                //test if centered
                                float turret_angle_new=m_vec_players[player_i].get_turret_angle();
                                if( (turret_angle_curr<target_angle && turret_angle_new>target_angle) ||
                                    (turret_angle_curr>target_angle && turret_angle_new<target_angle) )
                                {
                                    m_vec_players[player_i].set_turret_angle(target_angle);
                                }
                            }
                            else//reset aim
                            {
                                //reset turret angle to 0.0
                                float turret_angle=m_vec_players[player_i].get_turret_angle();
                                if(turret_angle>0.0)
                                {
                                    float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                    m_vec_players[player_i].change_turret_rotation( -rotation_force );

                                    turret_moved=true;
                                }
                                else if(turret_angle<0.0)
                                {
                                    float rotation_force=m_vec_players[player_i].m_turret_rotation_speed_fast*time_dif;
                                    m_vec_players[player_i].change_turret_rotation( rotation_force );

                                    turret_moved=true;
                                }
                                //test if centered
                                float turret_angle_new=m_vec_players[player_i].get_turret_angle();
                                if( (turret_angle<0.0 && turret_angle_new>0.0) ||
                                    (turret_angle>0.0 && turret_angle_new<0.0) )
                                {
                                    m_vec_players[player_i].set_turret_angle(0.0);
                                }
                            }
                        }

                    }break;

                    case gt_shield:
                    {
                        //regen hp
                        if(!m_vec_players[player_i].m_shield_broken)
                        {
                            if(m_vec_players[player_i].m_shield_regen_timer>0.0)
                             m_vec_players[player_i].m_shield_regen_timer-=time_dif;
                            else//regen delay complete
                            {
                                if(m_vec_players[player_i].m_shield_hp_curr<m_vec_players[player_i].m_shield_hp_max)
                                 m_vec_players[player_i].m_shield_hp_curr+=m_vec_players[player_i].m_shield_regen_speed;
                                if(m_vec_players[player_i].m_shield_hp_curr>m_vec_players[player_i].m_shield_hp_max)
                                 m_vec_players[player_i].m_shield_hp_curr=m_vec_players[player_i].m_shield_hp_max;
                            }
                        }
                    }break;

                    case gt_cam_control:
                    {
                        /*//move camera focus, if not docked to mship (done in update)
                        if(!m_vec_players[player_i].connected_to_mship())
                        {
                            //left thumbstick moves cam
                            m_vec_players[player_i].set_focus_point( (float)gamepad_data[player_i].thumbstick_left_x/32768.0,
                                                                     (float)gamepad_data[player_i].thumbstick_left_y/-32768.0 );
                        }
                        else//center focus point
                        {
                            m_vec_players[player_i].set_focus_point( 0.0, 0.0 );
                        }*/
                    }break;
                }

                //play sound
                if(turret_moved)
                {
                    //calc sound area
                    b2Vec2 pos=player_body->GetPosition();
                    int sound_box=0;//sound off
                    float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                    if(pos.x*_Met2Pix>view_pos[0] &&
                       pos.x*_Met2Pix<view_pos[2] &&
                       pos.y*_Met2Pix>view_pos[1] &&
                       pos.y*_Met2Pix<view_pos[3] )
                    {
                        sound_box=1;//on screen
                    }
                    else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                            pos.x*_Met2Pix<view_pos[0] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=2;//left side
                    }
                    else if(pos.x*_Met2Pix>view_pos[2] &&
                            pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=3;//right side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[1] )
                    {
                        sound_box=4;//top side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[3] &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=5;//top side
                    }

                    if(sound_box!=0)
                    {
                        float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                            0,1,0, 0,0,-1, 0,0,0,
                                            1,  1,  0};
                        switch(sound_box)
                        {
                            case 0: break;//no sound
                            case 1:
                            {
                                sound_data[14]=0;
                            }break;
                            case 2://left
                            {
                                sound_data[12]=-_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 3://right
                            {
                                sound_data[12]=_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 4:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 5:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                        }

                        //test if already playing
                        if( m_pSound->get_volume(_sound_chan_motor_turret) < sound_data[19] )
                        m_pSound->updateSound(_sound_chan_motor_turret,sound_data);
                    }
                }

                //stop boost sound
                if(!boosted)
                {
                    int channel_id=0;
                    switch(player_i)
                    {
                        case 0: channel_id=_sound_chan_boost_p1; break;
                        case 1: channel_id=_sound_chan_boost_p2; break;
                        case 2: channel_id=_sound_chan_boost_p3; break;
                        case 3: channel_id=_sound_chan_boost_p4; break;
                    }
                    float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                          0,1,0, 0,0,0, 0,0,0,
                                          1,  0,  0};
                    m_pSound->updateSound(channel_id,sound_data);
                }
            }

            //fuel transfer sound
            if(fuel_transfer_active)
            {
                m_pSound->set_volume(_sound_chan_fuel_transfer,0.5);
            }
            else m_pSound->set_volume(_sound_chan_fuel_transfer,0.0);

            //update value of mship led for player takeover (if not already owned)
            if(player_controlling_mship==-1)
            {
                //find player with most progress
                float highest_progress=m_vec_players[0].m_key_hold_time_back;
                for(int player_i=1;player_i<4;player_i++)
                {
                    if(m_vec_players[player_i].m_key_hold_time_back<highest_progress)
                    {
                        highest_progress=m_vec_players[player_i].m_key_hold_time_back;
                    }
                }
                m_mship_led_prog=highest_progress;
            }

            /*//control mship (common)
            if(m_pMain_ship->is_landing())
            {
                //thrusters
                if(thrust_highest[0]>0.0 || thrust_highest[1]>0.0)
                {
                    //cull value
                    if(thrust_highest[0]>1.0) thrust_highest[0]=1.0;
                    if(thrust_highest[1]>1.0) thrust_highest[1]=1.0;

                    float sens=30000.0*time_dif;
                    b2Vec2 force = b2Vec2( cos(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pMain_ship->get_body_ptr()->GetAngle()-_pi*0.5) * sens );
                    b2Vec2 force_left =thrust_highest[0]*force;
                    b2Vec2 force_right=thrust_highest[1]*force;

                    //add thrust to body
                    m_pMain_ship->get_body_ptr()->ApplyForce( force_left, m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pMain_ship->get_body_ptr()->ApplyForce( force_right, m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-4.0,2.0) ), 0.05*(force_left) );
                    m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2( 4.0,2.0) ), 0.05*(force_right) );
                    m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2(-4.0,2.0) ), 0.1*(force_left) );
                    m_pParticle_engine->add_particle( m_pMain_ship->get_body_ptr()->GetWorldPoint( b2Vec2( 4.0,2.0) ), 0.1*(force_right) );
                }

                //landing gear
                if(gear_speed_highest[0]!=0.0 || gear_speed_highest[1]!=0.0)
                {
                    //cull value
                    if(gear_speed_highest[0]>1.0) gear_speed_highest[0]=1.0;
                    if(gear_speed_highest[0]<-1.0) gear_speed_highest[0]=-1.0;

                    m_pMain_ship->set_landing_gear_motor_speed_left(gear_speed_highest[0]);
                }
                else m_pMain_ship->set_landing_gear_motor_speed_left(0.0);//stop
                if(gear_speed_highest[0]!=0.0 || gear_speed_highest[1]!=0.0)
                {
                    //cull value
                    if(gear_speed_highest[1]>1.0) gear_speed_highest[1]=1.0;
                    if(gear_speed_highest[1]<-1.0) gear_speed_highest[1]=-1.0;

                    m_pMain_ship->set_landing_gear_motor_speed_right(gear_speed_highest[1]);
                }
                else m_pMain_ship->set_landing_gear_motor_speed_right(0.0);//stop
            }*/

            //update player damage from collision
            for(int player_i=0;player_i<4;player_i++)
            {
                //get body data
                b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                st_body_user_data* data=(st_body_user_data*)player_body->GetUserData();
                if(data->f_collision_damage_update!=0)
                {
                    cout<<"Player collision damage: "<<data->f_collision_damage_update<<endl;
                    if(!m_vec_players[player_i].change_hp(data->f_collision_damage_update))
                    {
                        switch(player_i)
                        {
                            case 0: m_pSound->stopSound(_sound_chan_motor_p1); break;
                            case 1: m_pSound->stopSound(_sound_chan_motor_p2); break;
                            case 2: m_pSound->stopSound(_sound_chan_motor_p3); break;
                            case 3: m_pSound->stopSound(_sound_chan_motor_p4); break;
                        }
                    }

                    //show hud
                    m_hud.show_hud(player_i);

                    //if cloaked, turn off
                    if(m_vec_players[player_i].m_cloak_timer!=m_vec_players[player_i].m_cloak_delay)
                    {
                        m_vec_players[player_i].m_cloak_timer=m_vec_players[player_i].m_cloak_delay;
                        data->b_cloaked=false;
                    }

                    //play sound
                    if(data->f_collision_damage_update<-_sound_col_min_level &&
                       m_vec_players[player_i].m_sound_col_timer<=0)
                    {
                        m_vec_players[player_i].m_sound_col_timer=_sound_player_col_max_time;

                        b2Vec2 player_pos=player_body->GetPosition();
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(player_pos.x*_Met2Pix>view_pos[0] &&
                           player_pos.x*_Met2Pix<view_pos[2] &&
                           player_pos.y*_Met2Pix>view_pos[1] &&
                           player_pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                player_pos.x*_Met2Pix<view_pos[0] &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[2] &&
                                player_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0] &&
                                player_pos.x*_Met2Pix<view_pos[2] &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0] &&
                                player_pos.x*_Met2Pix<view_pos[2] &&
                                player_pos.y*_Met2Pix>view_pos[3] &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            sound_data[19]*=-data->f_collision_damage_update/30.0;
                            if(sound_data[19]>1.0) sound_data[19]=1;
                            m_pSound->playWAVE(wav_ship_col,sound_data);
                        }
                    }

                    data->f_collision_damage_update=0.0;//reset
                }
            }
            //update enemy damage from collision
            for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
            {
                //get body data
                b2Body* enemy_body=m_vec_pEnemies[enemy_i]->get_body_ptr();
                st_body_user_data* data=(st_body_user_data*)enemy_body->GetUserData();
                if(data->f_collision_damage_update!=0)
                {
                    m_vec_pEnemies[enemy_i]->change_hp(data->f_collision_damage_update);

                    //play sound
                    if(data->f_collision_damage_update<-_sound_col_min_level &&
                       m_vec_pEnemies[enemy_i]->m_sound_col_timer<=0)
                    {
                        m_vec_pEnemies[enemy_i]->m_sound_col_timer=_sound_player_col_max_time;

                        b2Vec2 enemy_pos=enemy_body->GetPosition();
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(enemy_pos.x*_Met2Pix>view_pos[0] &&
                           enemy_pos.x*_Met2Pix<view_pos[2] &&
                           enemy_pos.y*_Met2Pix>view_pos[1] &&
                           enemy_pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                enemy_pos.x*_Met2Pix<view_pos[0] &&
                                enemy_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                enemy_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[2] &&
                                enemy_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                enemy_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                enemy_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[0] &&
                                enemy_pos.x*_Met2Pix<view_pos[2] &&
                                enemy_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                enemy_pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[0] &&
                                enemy_pos.x*_Met2Pix<view_pos[2] &&
                                enemy_pos.y*_Met2Pix>view_pos[3] &&
                                enemy_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            sound_data[19]*=-data->f_collision_damage_update/30.0;
                            if(sound_data[19]>1.0) sound_data[19]=1;
                            m_pSound->playWAVE(wav_ship_col,sound_data);
                        }
                    }

                    data->f_collision_damage_update=0.0;
                }
            }
            //update player damage from projectiles
            for(int player_i=0;player_i<4;player_i++)
            {
                //get body data
                b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                st_body_user_data* data=(st_body_user_data*)player_body->GetUserData();
                if(data->f_projectile_damage_update!=0)
                {
                    //cout<<"Damage from projectile to player\n";
                    if(!m_vec_players[player_i].change_hp(data->f_projectile_damage_update))
                    {
                        switch(player_i)
                        {
                            case 0: m_pSound->stopSound(_sound_chan_motor_p1); break;
                            case 1: m_pSound->stopSound(_sound_chan_motor_p2); break;
                            case 2: m_pSound->stopSound(_sound_chan_motor_p3); break;
                            case 3: m_pSound->stopSound(_sound_chan_motor_p4); break;
                        }
                    }

                    //show hud
                    m_hud.show_hud(player_i);

                    //if cloaked, turn off
                    if(m_vec_players[player_i].m_cloak_timer!=m_vec_players[player_i].m_cloak_delay)
                    {
                        m_vec_players[player_i].m_cloak_timer=m_vec_players[player_i].m_cloak_delay;
                        data->b_cloaked=false;
                    }

                    //play sound
                    if(data->f_projectile_damage_update<-_sound_col_min_level &&
                       m_vec_players[player_i].m_sound_col_timer<=0)
                    {
                        m_vec_players[player_i].m_sound_col_timer=_sound_player_col_max_time;

                        b2Vec2 player_pos=player_body->GetPosition();
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(player_pos.x*_Met2Pix>view_pos[0] &&
                           player_pos.x*_Met2Pix<view_pos[2] &&
                           player_pos.y*_Met2Pix>view_pos[1] &&
                           player_pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                player_pos.x*_Met2Pix<view_pos[0] &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[2] &&
                                player_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0] &&
                                player_pos.x*_Met2Pix<view_pos[2] &&
                                player_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                player_pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(player_pos.x*_Met2Pix>view_pos[0] &&
                                player_pos.x*_Met2Pix<view_pos[2] &&
                                player_pos.y*_Met2Pix>view_pos[3] &&
                                player_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            sound_data[19]*=-data->f_projectile_damage_update/30.0;
                            if(sound_data[19]>1.0) sound_data[19]=1;
                            m_pSound->playWAVE(wav_ship_col,sound_data);
                        }
                    }

                    data->f_projectile_damage_update=0.0;
                }
            }
            //update enemy damage from projectiles
            for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
            {
                //get body data
                b2Body* enemy_body=m_vec_pEnemies[enemy_i]->get_body_ptr();
                st_body_user_data* data=(st_body_user_data*)enemy_body->GetUserData();
                if(data->f_projectile_damage_update!=0)
                {
                    //cout<<"Damage from projectile to enemy\n";
                    m_vec_pEnemies[enemy_i]->change_hp(data->f_projectile_damage_update);

                    //play sound
                    if(data->f_projectile_damage_update<-_sound_col_min_level &&
                       m_vec_pEnemies[enemy_i]->m_sound_col_timer<=0)
                    {
                        m_vec_pEnemies[enemy_i]->m_sound_col_timer=_sound_player_col_max_time;

                        b2Vec2 enemy_pos=enemy_body->GetPosition();
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(enemy_pos.x*_Met2Pix>view_pos[0] &&
                           enemy_pos.x*_Met2Pix<view_pos[2] &&
                           enemy_pos.y*_Met2Pix>view_pos[1] &&
                           enemy_pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                enemy_pos.x*_Met2Pix<view_pos[0] &&
                                enemy_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                enemy_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[2] &&
                                enemy_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                enemy_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                enemy_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[0] &&
                                enemy_pos.x*_Met2Pix<view_pos[2] &&
                                enemy_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                enemy_pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(enemy_pos.x*_Met2Pix>view_pos[0] &&
                                enemy_pos.x*_Met2Pix<view_pos[2] &&
                                enemy_pos.y*_Met2Pix>view_pos[3] &&
                                enemy_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            sound_data[19]*=-data->f_projectile_damage_update/30.0;
                            if(sound_data[19]>1.0) sound_data[19]=1;
                            m_pSound->playWAVE(wav_ship_col,sound_data);
                        }
                    }

                    data->f_projectile_damage_update=0.0;
                }
            }
            //update mship damage from projectiles
            {
                //get body data
                b2Body* mship_body=m_pMain_ship->get_body_ptr();
                st_body_user_data* data=(st_body_user_data*)mship_body->GetUserData();
                if(data->f_projectile_damage_update!=0)
                {
                    //cout<<"Damage from projectile to mship\n";


                    //eject random docked player
                    int player_to_eject=rand()%4;
                    if( m_vec_players[player_to_eject].connected_to_mship() )
                    {
                        m_pMain_ship->player_inside(player_to_eject,false);
                        m_vec_players[player_to_eject].disconnect_from_mship();
                        b2Body* player_body=m_vec_players[player_to_eject].get_body_ptr();
                        //give player ship a boost
                        float sens=30000.0*time_dif;
                        b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                        player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.0,0.3) ), true );
                        //add particle to engine
                        m_pParticle_engine->add_explosion( player_body->GetWorldPoint( b2Vec2(0.0,0.3) ),50,100,1.0 );
                    }

                    //abort takeoff, if in takeoff
                    m_pMain_ship->set_all_players_on_ship(false);
                    m_player_input_enabled=true;//reenable player input

                    //play sound
                    if(data->f_projectile_damage_update<-_sound_col_min_level &&
                       m_pMain_ship->m_sound_col_timer<=0)
                    {
                        m_pMain_ship->m_sound_col_timer=_sound_player_col_max_time;

                        b2Vec2 mship_pos=mship_body->GetPosition();
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(mship_pos.x*_Met2Pix>view_pos[0] &&
                           mship_pos.x*_Met2Pix<view_pos[2] &&
                           mship_pos.y*_Met2Pix>view_pos[1] &&
                           mship_pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(mship_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                mship_pos.x*_Met2Pix<view_pos[0] &&
                                mship_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                mship_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(mship_pos.x*_Met2Pix>view_pos[2] &&
                                mship_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                mship_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                mship_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(mship_pos.x*_Met2Pix>view_pos[0] &&
                                mship_pos.x*_Met2Pix<view_pos[2] &&
                                mship_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                mship_pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(mship_pos.x*_Met2Pix>view_pos[0] &&
                                mship_pos.x*_Met2Pix<view_pos[2] &&
                                mship_pos.y*_Met2Pix>view_pos[3] &&
                                mship_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            sound_data[19]*=-data->f_projectile_damage_update/30.0;
                            if(sound_data[19]>1.0) sound_data[19]=1;
                            m_pSound->playWAVE(wav_ship_col,sound_data);
                        }
                    }

                    data->f_projectile_damage_update=0.0;
                }
            }
            //update drone damage from projectiles
            {
                b2Body* tmp=m_pWorld->GetBodyList();
                while(tmp)
                {
                    st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                    if(data->s_info=="drone")
                    {
                        //test if drone have taken damage
                        if(data->f_collision_damage_update!=0.0 || data->f_projectile_damage_update!=0.0)
                        {
                            //mark for removal
                            m_vec_collision_events.push_back( st_collision_event(tmp,ct_unknown) );
                        }
                    }

                    tmp=tmp->GetNext();
                }
            }

            //handle collision events (remove drones)
            for(int col_i=0;col_i<(int)m_vec_collision_events.size();col_i++)
            {
                cout<<"Removing drone: "<<col_i<<endl;
                //test if already processed
                st_body_user_data* dataA=(st_body_user_data*)m_vec_collision_events[col_i].bodyA->GetUserData();
                if(dataA->b_to_be_deleted)
                {
                    cout<<" already processed\n";
                    //remove current event from vector
                    m_vec_collision_events.erase( m_vec_collision_events.begin()+col_i );
                    col_i--;
                    continue;
                }

                switch(m_vec_collision_events[col_i].collision_type)
                {
                    case ct_unknown://drone took damage
                    {
                        cout<<" unknown collision\n";
                        //report to player
                        if(dataA->i_id>3 || dataA->i_id<0)
                        {
                            cout<<"ERROR: Could not remove player drone\n";
                            break;
                        }
                        m_vec_players[dataA->i_id].set_drone_mode(dm_destroyed);

                        //make ship active
                        m_player_active[dataA->i_id]=false;

                        //draw explosion
                        m_pParticle_engine->add_explosion(m_vec_collision_events[col_i].bodyA->GetPosition(),20,300,0.2);

                        //mark for removal
                        dataA->b_to_be_deleted=true;

                        //play sound
                        m_pSound->playSimpleSound(wav_drone_crash,1.0);

                        //test if lost
                        lost_test();

                        //deactivate drone motor channel
                        switch(dataA->i_id)
                        {
                            case 0: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1); break;
                            case 1: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2); break;
                            case 2: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3); break;
                            case 3: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4); break;
                        }

                        if(m_on_tutorial_level)
                        {
                            //update hud
                            m_hud.set_player_ptr( m_vec_players[dataA->i_id].get_body_ptr() );
                        }

                    }break;

                    case ct_drone_player:
                    {
                        cout<<" player collision\n";
                        st_body_user_data* dataB=(st_body_user_data*)m_vec_collision_events[col_i].bodyB->GetUserData();
                        if(dataB->i_id>3 || dataB->i_id<0 || dataA->i_id>3 || dataA->i_id<0)
                        {
                            cout<<"ERROR: Could not find player index for drone collision\n";
                            break;
                        }
                        //test if that ship is under control
                        if(!m_player_active[dataB->i_id])
                        {
                            //take control over inactive player
                            //m_input_reroute[ dataB->i_id ]=m_input_reroute[ dataA->i_id ];
                            //cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                            int backup_val=m_input_reroute[ dataA->i_id ];
                            m_input_reroute[ dataA->i_id ]=m_input_reroute[ dataB->i_id ];
                            cout<<"Input reroute: Control "<<m_input_reroute[dataA->i_id]+1<<" is now controlling ship "<<dataA->i_id+1<<endl;
                            m_input_reroute[ dataB->i_id ]=backup_val;
                            cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                            //report to player's old ship
                            m_vec_players[dataA->i_id].set_drone_mode(dm_destroyed);

                            //report to new ship (reset drone mode to off if was changed earlier)
                            m_vec_players[dataB->i_id].set_drone_mode(dm_off);

                            //set cam focus on the new ship
                            if(m_came_mode==cm_follow_one)
                            {
                                m_cam_player_to_follow=dataB->i_id;
                            }

                            //make new ship active
                            m_player_active[dataB->i_id]=true;
                            //make old ship inactive
                            m_player_active[dataA->i_id]=false;

                            //mark for removal
                            dataA->b_to_be_deleted=true;

                            //play sound
                            m_pSound->playSimpleSound(wav_drone_join_ship,1.0);

                            //deactivate drone motor channel
                            switch(dataA->i_id)
                            {
                                case 0: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1); break;
                                case 1: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2); break;
                                case 2: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3); break;
                                case 3: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4); break;
                            }

                            if(m_on_tutorial_level)
                            {
                                //update hud
                                m_hud.set_player_ptr( m_vec_players[dataB->i_id].get_body_ptr() );

                                //uncomplete mission, will not work if done quick (tested later)
                                if( m_hud.get_draw_tutorial_helptext(ht_drone_takeover) )
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_drone_eject,false);//show
                                    m_hud.set_draw_tutorial_helptext(ht_drone_takeover,false);//unshow
                                }
                            }

                            break;
                        }

                        //ship is active but is it empty (no drone)
                        if( m_vec_players[dataB->i_id].get_drone_mode()==dm_on ||
                            m_vec_players[dataB->i_id].get_drone_mode()==dm_destroyed )
                        {
                            //take control over empty ship

                            if(dataA->i_id==dataB->i_id)//same ship (but maybe not original owner)
                            {
                                //no need to swap control input
                                cout<<"Drone joined its ship\n";
                                //report to player's ship
                                m_vec_players[dataA->i_id].set_drone_mode(dm_off);
                            }
                            else//different ships
                            {
                                cout<<"Drone joined other ship\n";
                                //if other ship's drone is active, swap ownership of that drone
                                if(m_vec_players[dataB->i_id].get_drone_mode()==dm_on)
                                {
                                    cout<<" with its drone active\n";

                                    //report to new ship
                                    m_vec_players[dataB->i_id].set_drone_mode(dm_off);

                                    //swap drone ownership, other drone is now owned by the other ship
                                    int backup_val=m_input_reroute[ dataA->i_id ];
                                    m_input_reroute[ dataA->i_id ]=m_input_reroute[ dataB->i_id ];
                                    cout<<"Input reroute: Control "<<m_input_reroute[dataA->i_id]+1<<" is now controlling ship "<<dataA->i_id+1<<endl;
                                    m_input_reroute[ dataB->i_id ]=backup_val;
                                    cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                                    b2Body* drone_ptr_other_ship=m_vec_players[dataB->i_id].get_player_drone_body_ptr();
                                    st_body_user_data* data=(st_body_user_data*)drone_ptr_other_ship->GetUserData();
                                    //set new owner
                                    data->i_id=dataA->i_id;
                                    m_vec_players[dataA->i_id].set_player_drone_body_ptr( drone_ptr_other_ship );

                                    //report to player's old ship, with new drone
                                    m_vec_players[dataA->i_id].set_drone_mode(dm_on);
                                }
                                else//drone was destroyed for that ship
                                {
                                    cout<<" with its drone destroyed\n";

                                    //swap control input
                                    //m_input_reroute[ dataB->i_id ]=m_input_reroute[ dataA->i_id ];
                                    //cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                                    //swap drone ownership, other drone is now owned by the other ship
                                    int backup_val=m_input_reroute[ dataA->i_id ];
                                    m_input_reroute[ dataA->i_id ]=m_input_reroute[ dataB->i_id ];
                                    cout<<"Input reroute: Control "<<m_input_reroute[dataA->i_id]+1<<" is now controlling ship "<<dataA->i_id+1<<endl;
                                    m_input_reroute[ dataB->i_id ]=backup_val;
                                    cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                                    //report to new ship
                                    m_vec_players[dataB->i_id].set_drone_mode(dm_off);

                                    //report to player's old ship
                                    m_vec_players[dataA->i_id].set_drone_mode(dm_destroyed);

                                    //make old ship inactive
                                    m_player_active[dataA->i_id]=false;
                                }
                            }

                            //mark drone for removal
                            dataA->b_to_be_deleted=true;

                            //play sound
                            m_pSound->playSimpleSound(wav_drone_join_ship,1.0);

                            //deactivate drone motor channel
                            switch(dataA->i_id)
                            {
                                case 0: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1); break;
                                case 1: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2); break;
                                case 2: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3); break;
                                case 3: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4); break;
                            }

                            if(m_on_tutorial_level)
                            {
                                //update hud
                                m_hud.set_player_ptr( m_vec_players[dataB->i_id].get_body_ptr() );

                                //uncomplete mission, will not work if done quick (tested later)
                                if( m_hud.get_draw_tutorial_helptext(ht_drone_takeover) )
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_drone_eject,false);//show
                                    m_hud.set_draw_tutorial_helptext(ht_drone_takeover,false);//unshow
                                }
                            }

                            break;
                        }

                        //no takeover possible, drone crash
                        cout<<" drone crashed with active player ship\n";
                        if(dataA->i_id>3 || dataA->i_id<0)
                        {
                            cout<<"ERROR: Could not remove that player drone\n";
                            break;
                        }
                        m_vec_players[dataA->i_id].set_drone_mode(dm_destroyed);

                        //make ship active
                        m_player_active[dataA->i_id]=false;

                        //draw explosion
                        m_pParticle_engine->add_explosion(m_vec_collision_events[col_i].bodyA->GetPosition(),20,300,0.2);

                        //mark for removal
                        dataA->b_to_be_deleted=true;

                        //test if lost
                        lost_test();

                        //play sound
                        m_pSound->playSimpleSound(wav_drone_crash,1.0);

                        //deactivate drone motor channel
                        switch(dataA->i_id)
                        {
                            case 0: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1); break;
                            case 1: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2); break;
                            case 2: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3); break;
                            case 3: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4); break;
                        }

                        if(m_on_tutorial_level)
                        {
                            //update hud
                            m_hud.set_player_ptr( m_vec_players[dataA->i_id].get_body_ptr() );
                        }

                    }break;

                    case ct_drone_inputbox:
                    {
                        //test if any unused ships in mship
                        bool found_empty_ship=false;
                        for(int player_i=0;player_i<4;player_i++)
                        {
                            //test if docked and inactive
                            if( m_vec_players[player_i].connected_to_mship() && !m_player_active[player_i])
                            {
                                //get ship data
                                st_body_user_data* dataB=(st_body_user_data*)m_vec_players[player_i].get_body_ptr()->GetUserData();
                                if(dataB->i_id>3 || dataB->i_id<0 || dataA->i_id>3 || dataA->i_id<0)
                                {
                                    cout<<"ERROR: Could not find player index for drone collision\n";
                                    break;
                                }

                                //takeover that ship
                                //m_input_reroute[ dataB->i_id ]=m_input_reroute[ dataA->i_id ];
                                //cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                                int backup_val=m_input_reroute[ dataA->i_id ];
                                m_input_reroute[ dataA->i_id ]=m_input_reroute[ dataB->i_id ];
                                cout<<"Input reroute: Control "<<m_input_reroute[dataA->i_id]+1<<" is now controlling ship "<<dataA->i_id+1<<endl;
                                m_input_reroute[ dataB->i_id ]=backup_val;
                                cout<<"Input reroute: Control "<<m_input_reroute[dataB->i_id]+1<<" is now controlling ship "<<dataB->i_id+1<<endl;

                                //report to player's old ship
                                m_vec_players[dataA->i_id].set_drone_mode(dm_destroyed);

                                //report to new ship (reset drone mode to off if was changed earlier)
                                m_vec_players[dataB->i_id].set_drone_mode(dm_off);

                                //set cam focus on the new ship
                                if(m_came_mode==cm_follow_one)
                                {
                                    m_cam_player_to_follow=dataB->i_id;
                                }

                                //make new ship active
                                m_player_active[dataB->i_id]=true;
                                //make old ship inactive
                                m_player_active[dataA->i_id]=false;

                                //raise ship if not raised
                                if(m_vec_players[player_i].is_spawning())
                                {
                                    m_vec_players[player_i].raise_from_mship();

                                    //play sound
                                    m_pSound->playSimpleSound(wav_player_ship_raising,1.0);
                                }

                                //mark for removal
                                dataA->b_to_be_deleted=true;

                                //play sound
                                m_pSound->playSimpleSound(wav_drone_join_ship,1.0);

                                //deactivate drone motor channel
                                switch(dataA->i_id)
                                {
                                    case 0: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1); break;
                                    case 1: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2); break;
                                    case 2: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3); break;
                                    case 3: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4); break;
                                }

                                found_empty_ship=true;

                                if(m_on_tutorial_level)
                                {
                                    //update hud
                                    m_hud.set_player_ptr( m_vec_players[dataB->i_id].get_body_ptr() );

                                    //complete mission
                                    if( m_hud.get_draw_tutorial_helptext(ht_drone_takeover) )
                                    {
                                        m_hud.set_draw_tutorial_helptext(ht_drone_takeover,true);
                                    }
                                }

                                break;
                            }
                        }

                        if(!found_empty_ship)
                        {
                            //no empty ships, destroy drone
                            m_vec_players[dataA->i_id].set_drone_mode(dm_destroyed);

                            //make ship active
                            m_player_active[dataA->i_id]=false;

                            //draw explosion
                            m_pParticle_engine->add_explosion(m_vec_collision_events[col_i].bodyA->GetPosition(),20,300,0.2);

                            //mark for removal
                            dataA->b_to_be_deleted=true;

                            //test if lost
                            lost_test();

                            //play sound
                            m_pSound->playSimpleSound(wav_drone_crash,1.0);

                            //deactivate drone motor channel
                            switch(dataA->i_id)
                            {
                                case 0: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1); break;
                                case 1: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2); break;
                                case 2: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3); break;
                                case 3: m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4); break;
                            }
                        }

                    }break;
                }
            }
            //remove bodies marked for removal
            for(int col_i=0;col_i<(int)m_vec_collision_events.size();col_i++)
            {
                //remove data and body
                st_body_user_data* data=(st_body_user_data*)m_vec_collision_events[col_i].bodyA->GetUserData();

                //delete data;
                //m_pWorld->DestroyBody(m_vec_collision_events[col_i].bodyA);

                //mark for removal
                data->b_to_be_deleted=true;
            }
            m_vec_collision_events.clear();

            //remove collided projectiles
            //cout<<"main update: collided projectile\n";
            if(!m_vec_projectiles_to_remove.empty())
            {
                //cout<<"Removing projectiles... ";
                for(int proj_i=0;proj_i<(int)m_vec_projectiles_to_remove.size();proj_i++)
                {
                    st_body_user_data* data=(st_body_user_data*)m_vec_projectiles_to_remove[proj_i]->GetUserData();
                    b2Vec2 pos=m_vec_projectiles_to_remove[proj_i]->GetPosition();
                    //cout<<data->i_id<<" ";

                    //if rocket, grenade or mine
                    if(data->i_id==wt_rocket || data->i_id==wt_grenade || data->i_id==wt_mine)
                    {
                        //test if grenade should explode (not if timed or first bounce)
                        if(data->i_id==wt_grenade && data->i_subtype!=wst_impact && data->i_subtype!=wst_timed)
                        {
                            cout<<data->i_subtype<<endl;

                            switch(data->i_subtype)
                            {
                                case wst_second_impact:
                                {
                                    //change to impact, will explode on next impact
                                    data->i_subtype=wst_impact;
                                }break;

                                case wst_second_timed:
                                {
                                    //change to timed
                                    data->i_subtype=wst_timed;
                                }break;
                            }

                            //will not explode this time
                            continue;
                        }

                        //add explosion
                        add_explotion(m_vec_projectiles_to_remove[proj_i]);
                    }

                    //add visual explosion
                    m_pParticle_engine->add_explosion(pos,20,300,0.2);

                    //play sound
                    if(true)
                    {
                        //calc sound area
                        int sound_box=0;//sound off
                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                        if(pos.x*_Met2Pix>view_pos[0] &&
                           pos.x*_Met2Pix<view_pos[2] &&
                           pos.y*_Met2Pix>view_pos[1] &&
                           pos.y*_Met2Pix<view_pos[3] )
                        {
                            sound_box=1;//on screen
                        }
                        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                pos.x*_Met2Pix<view_pos[0] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=2;//left side
                        }
                        else if(pos.x*_Met2Pix>view_pos[2] &&
                                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=3;//right side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                pos.y*_Met2Pix<view_pos[1] )
                        {
                            sound_box=4;//top side
                        }
                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                pos.x*_Met2Pix<view_pos[2] &&
                                pos.y*_Met2Pix>view_pos[3] &&
                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                        {
                            sound_box=5;//top side
                        }

                        if(sound_box!=0)
                        {
                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                0,1,0, 0,0,-1, 0,0,0,
                                                1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    sound_data[14]=0;
                                }break;
                                case 2://left
                                {
                                    sound_data[12]=-_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    sound_data[12]=_sound_box_side_shift;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    sound_data[12]=0;
                                    sound_data[19]=_sound_box_level_outside;
                                }break;
                            }
                            //select sound
                            if(data->i_id==wt_rocket || data->i_id==wt_grenade || data->i_id==wt_mine)
                            {
                                //rand sound feature
                                sound_data[18]=(float)(rand()%200)/500+0.6;
                                m_pSound->playWAVE(wav_bullet_explosion,sound_data);
                            }
                            else m_pSound->playWAVE(wav_bullet_hit,sound_data);
                        }
                    }

                    //delete data
                    //delete data;
                    //remove body
                    //m_pWorld->DestroyBody(m_vec_projectiles_to_remove[proj_i]);

                    //mark for removal
                    data->b_to_be_deleted=true;
                }
                //clear vector
                m_vec_projectiles_to_remove.clear();
                //cout<<"done\n";
            }

            //update main ships input box
            //cout<<"main update: input box\n";
            //bool temp_flag=false;
            //bool temp_joint_flag=false;
            //bool temp_multiple_flag=false;
            if(*m_pEvent_flag_input_box==true)
            {
                int body_index=0;
                b2Body* pBody_in_box=m_ppBody_in_mship_input[0];
                while(true)
                {
                    //test if that body can be recycled
                    st_body_user_data* data=(st_body_user_data*)pBody_in_box->GetUserData();
                    if(data->s_info=="object"||data->s_info=="enemy"||data->s_info=="player")//or enemy or destroyed player(treated differently)
                    {
                        cout<<"MShip input box: Removing body from input box\n";
                        //go through all joints and see if this body is connected
                        vector<b2Joint*> vec_joints_to_destroy;
                        vector<b2Body*> vec_owner_body;
                        b2Joint* tmp_joint=m_pWorld->GetJointList();
                        while(tmp_joint)
                        {
                            if( pBody_in_box==tmp_joint->GetBodyA() )
                            {
                                //ignore if player to own rope/hook connection
                                st_body_user_data* joint_owner_data=(st_body_user_data*)tmp_joint->GetBodyB()->GetUserData();
                                if( (joint_owner_data->s_info=="rope" || joint_owner_data->s_info=="hook") &&
                                     joint_owner_data->i_id==data->i_id )
                                {
                                    //skip joint
                                    tmp_joint=tmp_joint->GetNext();
                                    continue;
                                }

                                //mark joint for destruction
                                vec_joints_to_destroy.push_back(tmp_joint);
                                //store owner (other body)
                                vec_owner_body.push_back(tmp_joint->GetBodyB());
                            }
                            else if( pBody_in_box==tmp_joint->GetBodyB() )
                            {
                                //ignore if player to own rope/hook connection
                                st_body_user_data* joint_owner_data=(st_body_user_data*)tmp_joint->GetBodyA()->GetUserData();
                                if( (joint_owner_data->s_info=="rope" || joint_owner_data->s_info=="hook") &&
                                     joint_owner_data->i_id==data->i_id )
                                {
                                    //skip joint
                                    tmp_joint=tmp_joint->GetNext();
                                    continue;
                                }

                                //mark joint for destruction
                                vec_joints_to_destroy.push_back(tmp_joint);
                                //store owner (other body)
                                vec_owner_body.push_back(tmp_joint->GetBodyA());
                            }

                            tmp_joint=tmp_joint->GetNext();
                        }
                        if(!vec_joints_to_destroy.empty())
                        {
                            //cout<<"Input box: joint removal\n";

                            for(int joint_i=0;joint_i<(int)vec_joints_to_destroy.size();joint_i++)
                            {
                                st_body_user_data* joint_owner_data=(st_body_user_data*)vec_owner_body[joint_i]->GetUserData();
                                //report to player
                                if(joint_owner_data->s_info=="hook")
                                {
                                    cout<<"MShip input box: Removing joint connected to a player\n";
                                    int player_ind=joint_owner_data->i_id;
                                    if( !m_vec_players[player_ind].hook_disconnect() )
                                    {
                                        cout<<"MShip input box: Player was not connected with its hook\n";
                                    }
                                    continue;
                                }
                                //report to enemy (lifter)
                                if(joint_owner_data->s_info=="enemy")
                                {
                                    //find enemy
                                    cout<<"MShip input box: Removing joint connected to an enemy\n";
                                    bool enemy_found=false;
                                    for(int enemy_i=0;enemy_i<4;enemy_i++)
                                    {
                                        if( vec_owner_body[joint_i]==m_vec_pEnemies[enemy_i]->get_body_ptr() )
                                        {
                                            if( !m_vec_pEnemies[enemy_i]->hook_disconnect() )
                                            {
                                                cout<<"MShip input box: Enemy lifter was not connected with its hook\n";
                                            }
                                            enemy_found=true;
                                            break;
                                        }
                                    }
                                    if(enemy_found) continue;
                                }

                                //if not delete joint without report
                                cout<<"ERROR: MShip input box: Destroying unknown joint\n";
                                m_pWorld->DestroyJoint(vec_joints_to_destroy[joint_i]);
                            }

                            //cout<<"Input box: joint removal done\n";
                        }

                        //OLD if(!data->b_is_carried) //destroy object if not connected, not tested anymore

                        //destroy body
                        bool element_found=false;
                        if(data->s_info=="object")
                        {
                            //find object in vector
                            for(int obj_i=0;obj_i<(int)m_vec_objects.size();obj_i++)//find same id
                            {
                                if(m_vec_objects[obj_i].get_object_ptr()==pBody_in_box)
                                {//remove from vector
                                    element_found=true;

                                    //get content of the object
                                    switch(m_vec_objects[obj_i].m_object_type)
                                    {
                                        case ot_fuel:
                                        {
                                            //fill mship fuel tank
                                            float fuel_content=m_vec_objects[obj_i].get_content();
                                            m_pMain_ship->change_fuel(fuel_content);
                                        }break;

                                        case ot_resource:
                                        {
                                            //fill mship with resources
                                            float resource_content=m_vec_objects[obj_i].get_content();
                                            m_pMain_ship->change_resources(resource_content);
                                        }break;
                                    }

                                    //mark for removal
                                    data->b_to_be_deleted=true;

                                    //delete data(OLD)
                                    //delete data;
                                    //m_vec_objects.erase( m_vec_objects.begin()+obj_i );
                                    m_vec_objects[obj_i]=m_vec_objects.back();
                                    m_vec_objects.pop_back();
                                    //m_pWorld->DestroyBody(pBody_in_box);
                                    *m_pEvent_flag_input_box=false;

                                    //play sound
                                    m_pSound->playSimpleSound(wav_mship_input_fuel,0.3);

                                    //if tutorial, update mission
                                    if(m_on_tutorial_level)
                                    {
                                        m_hud.set_draw_tutorial_text(tut_fuel,false);
                                    }

                                    //report to starmap
                                    m_Starmap.change_curr_planet_level_fuel(-1);

                                    //test if a convoy is required (if this was the last fuel box)
                                    if(require_convoy_test())
                                    {
                                        cout<<"Input box: This was the last fuel box, convoy required\n";
                                        m_waiting_for_convoy=true;
                                        m_hud.set_draw_tutorial_text(-2,true);
                                    }

                                    break;
                                }
                            }
                        }
                        else if(data->s_info=="enemy")
                        {
                            for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
                            {
                                if( m_vec_pEnemies[enemy_i]==data->vp_this )
                                {
                                    //get weapon and add to inventory
                                    if(data->i_id==et_burst_bot ||
                                       data->i_id==et_auto_flat ||
                                       data->i_id==et_flipper ||
                                       data->i_id==et_rocket_tank ||
                                       data->i_id==et_grenade_ship ||
                                       data->i_id==et_cannon_tank ||
                                       data->i_id==et_miner ||
                                       data->i_id==et_beamer )
                                    {
                                        //get weapon
                                        weapon* pWeapon=m_vec_pEnemies[enemy_i]->get_weapon_ptr();
                                        m_vec_pWeapon_stored.push_back(pWeapon);
                                        //delete gear (no, allocated memory required for pointer in vector)
                                        //delete m_vec_pEnemies[enemy_i]->get_gear_ptr();
                                    }
                                    //get tool and add to inventory....
                                    else if(data->i_id==et_cloaker ||
                                            data->i_id==et_scanner ||
                                            data->i_id==et_lifter ||
                                            data->i_id==et_stand_turret ||
                                            data->i_id==et_flying_turret ||
                                            data->i_id==et_aim_bot ||
                                            data->i_id==et_rammer )
                                    {
                                        //get gear
                                        gear* pGear=m_vec_pEnemies[enemy_i]->get_gear_ptr();
                                        m_vec_pGear_stored.push_back(pGear);
                                        //delete weapon (no, allocated memory required for pointer in vector)
                                        //delete m_vec_pEnemies[enemy_i]->get_weapon_ptr();
                                    }


                                    //remove from vector
                                    element_found=true;

                                    //mark for removal
                                    data->b_to_be_deleted=true;

                                    //delete data (OLD)
                                    //delete data;
                                    delete m_vec_pEnemies[enemy_i];
                                    //m_vec_pEnemies.erase( m_vec_pEnemies.begin()+enemy_i );
                                    m_vec_pEnemies[enemy_i]=m_vec_pEnemies.back();
                                    m_vec_pEnemies.pop_back();
                                    //m_pWorld->DestroyBody(pBody_in_box);
                                    *m_pEvent_flag_input_box=false;

                                    //if tutorial, update mission
                                    if(m_on_tutorial_level)
                                    {
                                        m_hud.set_draw_tutorial_helptext(ht_salvage,true);
                                        m_hud.set_draw_tutorial_text(tut_enemy,false);
                                    }

                                    //report to starmap
                                    m_Starmap.change_curr_planet_level_enemy(-1);

                                    //play sound
                                    m_pSound->playSimpleSound(wav_mship_input_ship,0.5);

                                    break;
                                }
                            }
                        }
                        else if(data->s_info=="player")
                        {
                            element_found=true;//always found
                            //if player is out of HP (or not connected), reinit/move at/to startpos
                            int player_ind=data->i_id;
                            if(true/*m_vec_players[player_ind].get_rel_hp()<=0.0 ||
                               m_vec_players[player_ind].get_rel_fuel()<=0.0 ||
                               !m_gamepad_connected[player_ind]*/)
                            {
                                //cout<<"Reinit player: "<<player_ind+1<<endl;

                                //on tutorial and lost the gear
                                if(m_on_tutorial_level)
                                {
                                    if(m_vec_players[player_ind].get_gear_ptr()->get_type()==gt_gyro )
                                    {
                                        //spawn a new in the main ship, otherwise the tutorial progress could be locked
                                        m_vec_pGear_stored.push_back( new gear(m_pWorld,gt_gyro,1.0) );
                                    }
                                }

                                //destroy user data(old)
                                //delete data;

                                //destroy joints and bodies, and reset weapon and gear
                                m_vec_players[player_ind].destroy_body_and_joints();

                                /*//test if any players in the way (tested at ship lift)
                                b2Vec2 land_pos_local;
                                switch(player_ind)
                                {
                                    case 0: land_pos_local.Set(0.00,-2.5); break;
                                    case 1: land_pos_local.Set(1.25,-2.5); break;
                                    case 2: land_pos_local.Set(2.75,-2.5); break;
                                    case 3: land_pos_local.Set(4.00,-2.5); break;
                                }
                                b2Vec2 land_pos_world=m_pMain_ship->get_body_ptr()->GetWorldPoint( land_pos_local );
                                for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                                {
                                    if(player_ind==player_i) continue;
                                    //measure dist
                                    b2Vec2 player_pos=m_vec_players[player_i].get_body_ptr()->GetPosition();
                                    float min_allowed_dist=1.0;
                                    float dist=sqrt( (player_pos.x-land_pos_world.x)*(player_pos.x-land_pos_world.x)+
                                                     (player_pos.y-land_pos_world.y)*(player_pos.y-land_pos_world.y) );
                                    if(dist<min_allowed_dist)
                                    {
                                        cout<<"Reinit player: Other player in the way\n";
                                        //eject player in the way
                                        m_vec_players[player_i].disconnect_from_mship();
                                        b2Body* player_body=m_vec_players[player_i].get_body_ptr();
                                        //give player ship a boost
                                        float sens=30000.0*time_dif;
                                        b2Vec2 force = b2Vec2( cos(player_body->GetAngle()-_pi*0.5) * sens , sin(player_body->GetAngle()-_pi*0.5) * sens );
                                        player_body->ApplyForce(force, player_body->GetWorldPoint( b2Vec2(0.0,0.3) ), true );
                                        //add particle to engine
                                        m_pParticle_engine->add_explosion( player_body->GetWorldPoint( b2Vec2(0.0,0.3) ),50,100,1.0 );
                                    }
                                    else cout<<"Reinit player: Dist to other player: "<<dist<<endl;
                                }*/

                                //remove weapon, default and current ( done in destroy_body_and_joints() )
                                /*if(m_vec_players[player_ind].get_weapon_ptr()==m_vec_players[player_ind].get_default_weapon_ptr())
                                {
                                    //same
                                    delete m_vec_players[player_ind].get_weapon_ptr();
                                }
                                else//remove both
                                {
                                    delete m_vec_players[player_ind].get_weapon_ptr();
                                    delete m_vec_players[player_ind].get_default_weapon_ptr();
                                }*/

                                /*//remove gear, default and current ( done in destroy_body_and_joints() )
                                if(m_vec_players[player_ind].get_gear_ptr()==m_vec_players[player_ind].get_default_gear_ptr())
                                {
                                    //same
                                    delete m_vec_players[player_ind].get_gear_ptr();
                                }
                                else//remove both
                                {
                                    //if cam controller was in use, create new and give to mship
                                    if(m_vec_players[player_ind].get_gear_ptr()->get_type()==gt_cam_control)
                                     m_vec_pGear_stored.push_back( new gear(m_pWorld,gt_cam_control,1.0) );

                                    delete m_vec_players[player_ind].get_gear_ptr();
                                    delete m_vec_players[player_ind].get_default_gear_ptr();
                                }*/

                                //calc number of ship upgrades to restore to mship (half is lost)
                                float resource_restored=m_vec_players[player_ind].m_upgrade_counter*_player_upgrade_cost*0.5;
                                m_pMain_ship->change_resources(resource_restored);

                                //reinit (real init to reset all values)
                                m_pMain_ship->player_inside(player_ind,true);
                                bool drone_active=false;
                                if(m_vec_players[player_ind].get_drone_mode()==dm_on) drone_active=true;
                                m_vec_players[player_ind].init( m_pWorld,m_pMain_ship->get_body_ptr(),
                                                                m_pParticle_engine,m_pSound,m_tex_decal,player_ind,false,
                                                                b2Vec2(m_player_start_pos[0],m_player_start_pos[1]) );

                                /*//update hud's player pointer
                                if(m_on_tutorial_level)
                                {
                                    for(int player_search_i=0;player_search_i<4;player_search_i++)
                                    {
                                        if(m_player_active[player_search_i])
                                        {
                                            m_hud.set_player_ptr(m_vec_players[player_search_i].get_body_ptr());
                                            break;
                                        }
                                    }
                                }*/

                                //restore drone state
                                if(drone_active) m_vec_players[player_ind].set_drone_mode(dm_on,true);

                                //inactivate player (will stay inside mship)
                                m_player_active[player_ind]=false;

                                //update contact listener
                                m_ppPlayer_bodies[player_ind]=m_vec_players[player_ind].get_body_ptr();
                                m_ppRope_hook_sensors[player_ind]=m_vec_players[player_ind].get_rope_hook_sensor();

                                //update enemies list of player bodies
                                for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
                                {
                                    m_vec_pEnemies[enemy_i]->update_player_bodies_ptr();
                                }

                                //update player ptr in tutorial
                                if(m_on_tutorial_level)
                                {
                                    m_hud.set_player_ptr(m_vec_players[player_ind].get_body_ptr());
                                }

                                //test if all players are docked to allow takeoff
                                bool all_players_docked=true;
                                for(int i=0;i<4;i++)
                                {
                                    if(m_vec_players[i].get_mship_dock_status()==0 && !m_vec_players[i].is_spawning())
                                    {
                                        all_players_docked=false;
                                        break;
                                    }
                                }
                                //enable takeoff possibilities, if all players are docked
                                if(all_players_docked) m_pMain_ship->set_all_players_on_ship(true);

                                *m_pEvent_flag_input_box=false;

                                //play sound
                                m_pSound->playSimpleSound(wav_mship_input_ship,0.5);

                                //cout<<"Reinit player: Done\n";
                            }
                        }

                        if(!element_found)
                        {
                            cout<<"ERROR: Could not find current element in vector for removal in input box\n";
                        }
                    }

                    //handle next body, if more to handle
                    if( body_index<(int)m_myContactListenerInstance.m_vec_pBodies_in_input_box.size() )
                    {
                        //cout<<"Input box: multiple, now next\n";
                        pBody_in_box=m_myContactListenerInstance.m_vec_pBodies_in_input_box[body_index];
                        body_index++;
                    }
                    else//done
                    {
                        m_myContactListenerInstance.m_vec_pBodies_in_input_box.clear();

                        break;
                    }
                }
            }

            //if(m_pEvent_flag_hook[0]) cout<<"Hook sensor active\n";

            //main ship update
            //cout<<"main update: mship\n";
            int ret_val=m_pMain_ship->update(time_dif);
            switch(ret_val)//0 is default
            {
                case 1://in landing gear extension phase
                {
                    /*//test if landing gear sensors have reached ground (now tested locally)
                    if(m_pEvent_flag_landing_gear[0])
                    {
                        //stop left motor
                        m_pMain_ship->landing_gear_motor_left_lock(true);
                    }
                    if(m_pEvent_flag_landing_gear[1])
                    {
                        //stop right motor
                        m_pMain_ship->landing_gear_motor_right_lock(true);
                    }*/
                }break;

                case 2://landing complete
                {
                    cout<<"Landing Complete\n";
                    m_mship_landed=true;

                    //update tutorial
                    if(m_on_tutorial_level)
                    {
                        m_hud.set_draw_tutorial_text(tut_land,false);
                        m_hud.set_draw_tutorial_text(tut_manual,true);
                    }

                    //test if a convoy is required
                    if(require_convoy_test())
                    {
                        cout<<"Main ship landed: Convoy required\n";
                        m_waiting_for_convoy=true;
                        m_hud.set_draw_tutorial_text(-2,true);
                    }

                }break;

                case 3://successful takeoff
                {
                    //go to level selection menu
                    //m_game_state=gs_level_select;
                    //by triggering fade off
                    m_fade_off=true;
                }break;

                case 4://successful takeoff (manual)
                {
                    //go to level selection menu (if not on tutorial level)
                    //by triggering fade off
                    cout<<"Takeoff: Manual leave\n";
                    if(!m_on_tutorial_level) m_fade_off=true;
                }break;
            }
            //mship engine sound
            if(m_pMain_ship->get_auto_pilot()==-1)
            {
                //motor sound
                float mship_engine_thrust=-m_pMain_ship->get_motor_thrust();
                if(mship_engine_thrust>1.0 && (m_pMain_ship->is_landing() || m_pMain_ship->is_takeoff()) )
                {
                    //cout<<mship_engine_thrust<<endl;

                    //calc sound area
                    b2Vec2 pos=m_pMain_ship->get_body_ptr()->GetPosition();
                    int sound_box=0;//sound off
                    float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                    if(pos.x*_Met2Pix>view_pos[0] &&
                       pos.x*_Met2Pix<view_pos[2] &&
                       pos.y*_Met2Pix>view_pos[1] &&
                       pos.y*_Met2Pix<view_pos[3] )
                    {
                        sound_box=1;//on screen
                    }
                    else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                            pos.x*_Met2Pix<view_pos[0] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=2;//left side
                    }
                    else if(pos.x*_Met2Pix>view_pos[2] &&
                            pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=3;//right side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                            pos.y*_Met2Pix<view_pos[1] )
                    {
                        sound_box=4;//top side
                    }
                    else if(pos.x*_Met2Pix>view_pos[0] &&
                            pos.x*_Met2Pix<view_pos[2] &&
                            pos.y*_Met2Pix>view_pos[3] &&
                            pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                    {
                        sound_box=5;//top side
                    }

                    if(sound_box!=0)
                    {
                        float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                            0,1,0, 0,0,-1, 0,0,0,
                                            1,  1,  0};
                        switch(sound_box)
                        {
                            case 0: break;//no sound
                            case 1:
                            {
                                sound_data[14]=0;
                            }break;
                            case 2://left
                            {
                                sound_data[12]=-_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 3://right
                            {
                                sound_data[12]=_sound_box_side_shift;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 4:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                            case 5:
                            {
                                sound_data[12]=0;
                                sound_data[19]=_sound_box_level_outside;
                            }break;
                        }

                        float sound_volume=mship_engine_thrust/1000.0;
                        if(sound_volume>1.0) sound_volume=1.0;

                        float original_volume=sound_data[19];
                        float sound_pitch=0.5+sound_volume*0.5;
                        sound_data[18]=sound_pitch;
                        sound_data[19]*=sound_volume;

                        m_pSound->updateSound(_sound_chan_motor_mship,sound_data);

                        //gear sound
                        if(m_pMain_ship->get_gear_motor_speed()!=0.0)
                        {
                            sound_data[19]=original_volume*m_pMain_ship->get_gear_motor_speed();

                            m_pSound->updateSound(_sound_chan_gear_motor_mship,sound_data);
                        }
                    }
                }
                else//stop sound
                {
                    float sound_data[21]={0,0,0, 0,0,0, 0,0,-1,
                                          0,1,0, 0,0,0, 0,0,0,
                                          1,  0,  0};

                    m_pSound->updateSound(_sound_chan_motor_mship,sound_data);
                }
            }

            //update players
            //cout<<"main update: mship\n";
            for(int player_i=0;player_i<4;player_i++)
            {
                //if docked, resupply
                if(m_vec_players[player_i].get_mship_dock_status()==2)
                {
                    bool value_changed=false;

                    //repair shields
                    if(m_vec_players[player_i].get_gear_ptr()->get_type()==gt_shield &&
                       m_vec_players[player_i].m_shield_broken)
                    {
                        m_vec_players[player_i].m_shield_broken=false;
                        cout<<"Shield repaired\n";
                    }

                    //check hp
                    if(m_vec_players[player_i].get_rel_hp()<1.0)
                    {
                        m_vec_players[player_i].change_hp(_player_refill_factor_hp*time_dif);
                        value_changed=true;
                    }
                    //check fuel
                    if(m_vec_players[player_i].get_rel_fuel()<1.0)
                    {
                        m_vec_players[player_i].change_fuel(_player_refill_factor_fuel*time_dif);
                        value_changed=true;
                    }
                    //check ammo
                    if(m_vec_players[player_i].get_rel_ammo()<1.0)
                    {
                        m_vec_players[player_i].change_ammo(_player_refill_factor_ammo*time_dif);
                        value_changed=true;
                    }

                    //show hud
                    if(value_changed) m_hud.show_hud(player_i);
                }

                m_vec_players[player_i].update(time_dif,view_pos);
            }

            //update enemies
            //cout<<"main update: enemies\n";
            for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
            {
                int enemy_ret_val=m_vec_pEnemies[enemy_i]->update(time_dif,view_pos);

                //m_vec_pEnemies[enemy_i]->set_target_pos( m_vec_players.front().get_body_ptr()->GetPosition() );//temp set enemy target to follow

                //if tutorial, test if enemy within screen
                if(m_on_tutorial_level)
                {
                    b2Body* enemy_ptr=m_vec_pEnemies[enemy_i]->get_body_ptr();
                    b2Vec2 enemy_pos=enemy_ptr->GetPosition();
                    //screen test
                    if(enemy_pos.x*_Met2Pix>m_cam_pos[0] &&
                       enemy_pos.x*_Met2Pix<m_cam_pos[0]+m_screen_width &&
                       enemy_pos.y*_Met2Pix>m_cam_pos[1] &&
                       enemy_pos.y*_Met2Pix<m_cam_pos[1]+m_screen_height)
                    {
                        //enemy on screen, start tutorial mission
                        m_hud.set_draw_tutorial_text(tut_enemy,true);
                    }
                }
                //if returned 2, convoy ship in position
                else if(enemy_ret_val==2)
                {
                    //test if lifter with fuel object
                    st_body_user_data* data=(st_body_user_data*)m_vec_pEnemies[enemy_i]->get_body_ptr()->GetUserData();

                    //if enemy connected to player, abort
                    if(!data->b_is_carried)
                    {
                        cout<<"Enemy in convoy is ready to leave map\n";

                        if(data->i_id==et_lifter)
                        {
                            //test if lifting
                            if(m_vec_pEnemies[enemy_i]->hook_status())
                            {
                                b2Body* fuel_box_ptr=m_vec_pEnemies[enemy_i]->get_connected_body();

                                //delete joint
                                m_vec_pEnemies[enemy_i]->hook_disconnect();

                                //delete lifted fuel box (if not connected to others)
                                st_body_user_data* fuel_data=(st_body_user_data*)fuel_box_ptr->GetUserData();
                                if(!fuel_data->b_is_carried)
                                {
                                    //find fuel object in vector
                                    bool element_found=false;
                                    for(int obj_i=0;obj_i<(int)m_vec_objects.size();obj_i++)//find same id
                                    {
                                        if(m_vec_objects[obj_i].get_object_ptr()==fuel_box_ptr)
                                        {//remove from vector
                                            element_found=true;

                                            //mark for removal
                                            fuel_data->b_to_be_deleted=true;

                                            //delete data(OLD)
                                            //delete fuel_data;
                                            //m_vec_objects.erase( m_vec_objects.begin()+obj_i );
                                            m_vec_objects[obj_i]=m_vec_objects.back();
                                            m_vec_objects.pop_back();
                                            //m_pWorld->DestroyBody(fuel_box_ptr);

                                            //test if a convoy is required (if this was the last fuel box)
                                            if(require_convoy_test())
                                            {
                                                cout<<"Input box: This was the last fuel box, convoy required\n";
                                                m_waiting_for_convoy=true;
                                                m_hud.set_draw_tutorial_text(-2,true);
                                            }

                                            break;
                                        }
                                    }
                                    if(!element_found) cout<<"ERROR: Could not find object in vector (convoy)\n";
                                }
                            }
                        }

                        //mark for removal
                        data->b_to_be_deleted=true;

                        //delete enemy(OLD)
                        //delete data;
                        //m_pWorld->DestroyBody( m_vec_pEnemies[enemy_i]->get_body_ptr() );
                        delete m_vec_pEnemies[enemy_i];
                        //m_vec_pEnemies.erase( m_vec_pEnemies.begin()+enemy_i );
                        m_vec_pEnemies[enemy_i]=m_vec_pEnemies.back();
                        m_vec_pEnemies.pop_back();
                    }
                }
            }

            //update objects
            for(int object_i=0;object_i<(int)m_vec_objects.size();object_i++)
            {
                m_vec_objects[object_i].update();

                //if tutorial, test if a fuel box is within the screen
                if(m_on_tutorial_level && !m_hud.m_text_fuel_box_shown)
                {
                    b2Body* object_ptr=m_vec_objects[object_i].get_object_ptr();
                    b2Vec2 fuel_pos=object_ptr->GetPosition();
                    //screen test
                    if(fuel_pos.x*_Met2Pix>m_cam_pos[0] &&
                       fuel_pos.x*_Met2Pix<m_cam_pos[0]+m_screen_width &&
                       fuel_pos.y*_Met2Pix>m_cam_pos[1] &&
                       fuel_pos.y*_Met2Pix<m_cam_pos[1]+m_screen_height)
                    {
                        //fuel on screen, show text
                        m_hud.set_draw_tutorial_helptext(ht_fuel_container,false);
                        m_hud.m_fuel_box_pos=fuel_pos;
                    }
                }
            }

            //update projectiles
            //cout<<"main update: update projectile\n";
            b2Body* tmp=m_pWorld->GetBodyList();
            vector<b2Body*> vec_bodies_to_remove;
            while(tmp)
            {
                st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                if(data->s_info=="projectile" )
                {
                    switch(data->i_id)
                    {
                        case wt_rocket:
                        {
                            //add smoke
                            b2Vec2 lin_sped=tmp->GetLinearVelocity();
                            m_pParticle_engine->add_particle(tmp->GetWorldPoint( b2Vec2(0.0,0.0) ), 0.1*lin_sped, 0.2 );
                        }break;

                        case wt_grenade:
                        {
                            //update timer
                            if(data->i_subtype==wst_timed)
                            {
                                data->f_time_left-=time_dif;
                                if(data->f_time_left<=0.0)
                                {
                                    //explode
                                    add_explotion(tmp);

                                    //add visual explosion
                                    m_pParticle_engine->add_explosion(tmp->GetPosition(),20,300,0.2);

                                    //add to remove body vector
                                    vec_bodies_to_remove.push_back(tmp);

                                    //play sound
                                    {
                                        b2Vec2 pos=tmp->GetPosition();
                                        //calc sound area
                                        int sound_box=0;//sound off
                                        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
                                        if(pos.x*_Met2Pix>view_pos[0] &&
                                           pos.x*_Met2Pix<view_pos[2] &&
                                           pos.y*_Met2Pix>view_pos[1] &&
                                           pos.y*_Met2Pix<view_pos[3] )
                                        {
                                            sound_box=1;//on screen
                                        }
                                        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                                                pos.x*_Met2Pix<view_pos[0] &&
                                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                        {
                                            sound_box=2;//left side
                                        }
                                        else if(pos.x*_Met2Pix>view_pos[2] &&
                                                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                        {
                                            sound_box=3;//right side
                                        }
                                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                                pos.x*_Met2Pix<view_pos[2] &&
                                                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                                                pos.y*_Met2Pix<view_pos[1] )
                                        {
                                            sound_box=4;//top side
                                        }
                                        else if(pos.x*_Met2Pix>view_pos[0] &&
                                                pos.x*_Met2Pix<view_pos[2] &&
                                                pos.y*_Met2Pix>view_pos[3] &&
                                                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
                                        {
                                            sound_box=5;//top side
                                        }

                                        if(sound_box!=0)
                                        {
                                            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                                                0,1,0, 0,0,-1, 0,0,0,
                                                                1,  1,  0};
                                            switch(sound_box)
                                            {
                                                case 0: break;//no sound
                                                case 1:
                                                {
                                                    sound_data[14]=0;
                                                }break;
                                                case 2://left
                                                {
                                                    sound_data[12]=-_sound_box_side_shift;
                                                    sound_data[19]=_sound_box_level_outside;
                                                }break;
                                                case 3://right
                                                {
                                                    sound_data[12]=_sound_box_side_shift;
                                                    sound_data[19]=_sound_box_level_outside;
                                                }break;
                                                case 4:
                                                {
                                                    sound_data[12]=0;
                                                    sound_data[19]=_sound_box_level_outside;
                                                }break;
                                                case 5:
                                                {
                                                    sound_data[12]=0;
                                                    sound_data[19]=_sound_box_level_outside;
                                                }break;
                                            }
                                            //rand sound feature
                                            sound_data[18]=(float)(rand()%200)/500+0.6;
                                            m_pSound->playWAVE(wav_bullet_explosion,sound_data);
                                        }
                                    }
                                }
                            }
                        }break;

                    }
                }

                tmp=tmp->GetNext();
            }
            //remove exploded projectiles
            //cout<<"main update: collided projectile remove\n";
            for(int proj_i=0;proj_i<(int)vec_bodies_to_remove.size();proj_i++)
            {
                st_body_user_data* data=(st_body_user_data*)vec_bodies_to_remove[proj_i]->GetUserData();
                //delete data(OLD)
                //delete data;
                //remove body
                //m_pWorld->DestroyBody(vec_bodies_to_remove[proj_i]);

                //mark for removal
                data->b_to_be_deleted=true;
            }


            //update HUD
            //cout<<"main update: hud\n";
            m_hud.update(time_dif);

            //tutorial update, test if all players have landed
            //if(m_tutorial_test_timer>0.0) m_tutorial_test_timer-=time_dif;
            if(m_on_tutorial_level /*&& m_tutorial_test_timer<=0.0*/)
            {
                //m_tutorial_test_timer=_tutorial_test_time;

                //complete dock mission if already docked
                if(m_hud.get_draw_tutorial_helptext(ht_dock)||m_hud.get_draw_tutorial_helptext(ht_dock_again))
                {
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        //find active tutorial player
                        if( m_vec_players[player_i].get_body_ptr()==m_hud.get_player_ptr() )
                        {
                            if( m_vec_players[player_i].connected_to_mship() )
                            {
                                if(m_hud.get_draw_tutorial_helptext(ht_dock))
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_dock,true);
                                }

                                if(m_hud.get_draw_tutorial_helptext(ht_dock_again))
                                {
                                    m_hud.set_draw_tutorial_helptext(ht_dock_again,true);
                                }
                            }

                            break;
                        }
                    }
                }

                //complete drone eject mission if already ejected
                if(m_hud.get_draw_tutorial_helptext(ht_drone_eject))
                {
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        //find active tutorial player
                        if( m_vec_players[player_i].get_drone_mode()==dm_on )
                        {
                            if( m_hud.get_player_ptr()==m_vec_players[player_i].get_player_drone_body_ptr() )
                            {
                                m_hud.set_draw_tutorial_helptext(ht_drone_eject,true);

                                break;
                            }
                        }
                    }
                }

                //hook off test
                for(int player_i=0;player_i<4;player_i++)
                {
                    if(m_vec_players[player_i].is_hook_off() && !m_hud.m_text_hook_off_shown)
                    {
                        m_hud.set_draw_tutorial_helptext(ht_hook_off,false);
                        m_hud.set_player_ptr(m_vec_players[player_i].get_body_ptr());
                    }
                }

                //test if drone was returned wrong
                if( m_hud.get_draw_tutorial_helptext(ht_drone_takeover) )
                {
                    //have any players any active drone
                    bool all_player_drones_inactive=true;
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if( m_vec_players[player_i].get_drone_mode()==dm_destroyed ||
                            m_vec_players[player_i].get_drone_mode()==dm_on )
                        {
                            all_player_drones_inactive=false;
                            break;
                        }
                    }
                    if(all_player_drones_inactive)//no active drones
                    {
                        //uncomplete mission, show eject drone mission
                        m_hud.set_draw_tutorial_helptext(ht_drone_eject,false);//show
                        m_hud.set_draw_tutorial_helptext(ht_drone_takeover,false);//unshow
                    }
                }

                //test if abandoned ship have been returned
                if( m_hud.get_draw_tutorial_helptext(ht_bring_back_ship) )
                {
                    bool all_player_drones_inactive=true;
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if( m_vec_players[player_i].get_drone_mode()==dm_destroyed ||
                            m_vec_players[player_i].get_drone_mode()==dm_on )
                        {
                            all_player_drones_inactive=false;
                            break;
                        }
                    }
                    if(all_player_drones_inactive)//no active drones
                    {
                        //complete mission
                        m_hud.set_draw_tutorial_helptext(ht_bring_back_ship,true);
                    }
                }

                //test if players should return to mship
                if( m_hud.get_draw_tutorial_text(tut_return) )
                {
                    //cout<<"on return mission"<<endl;

                    //test if all players have landed
                    bool all_players_docked=true;
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if( m_vec_players[player_i].get_mship_dock_status()==0 && !m_vec_players[player_i].is_spawning() )
                        {
                            all_players_docked=false;
                            break;
                        }
                    }
                    if(all_players_docked)//all landed
                    {
                        //finish mission
                        m_hud.set_draw_tutorial_text(tut_return,false);

                        //show takeoff text
                        //m_hud.set_draw_tutorial_helptext(ht_takeoff,false);
                    }
                    /*//if not showing helptext about takeoff
                    else if( m_hud.get_draw_tutorial_helptext(ht_takeoff) )
                    {
                        //show takeoff text
                        m_hud.set_draw_tutorial_helptext(ht_takeoff,false);
                    }*/
                }
                //not on return mission, and not any previous missions active
                else if( !m_hud.get_draw_tutorial_text(tut_land) &&
                         !m_hud.get_draw_tutorial_text(tut_manual) &&
                         !m_hud.get_draw_tutorial_text(tut_fuel) &&
                         !m_hud.get_draw_tutorial_text(tut_enemy) &&
                         !m_hud.get_draw_tutorial_text(tut_takeoff) )
                {
                    //cout<<"no mission active\n";
                    //nor any help text, start return mission (but not if takeoff have started, to avoid reshow)
                    if( m_hud.is_helptext_completed() && !m_pMain_ship->is_takeoff() )
                    {
                        m_hud.set_draw_tutorial_text(tut_return,true);
                        m_hud.set_draw_tutorial_helptext(ht_takeoff,false);
                    }


                    /*bool help_text_shown=false;
                    for(int text_i=0;text_i<26;text_i++)
                    {
                        if( m_hud.get_draw_tutorial_helptext(text_i) )
                        {
                            //cout<<"text shown: "<<text_i<<endl;
                            help_text_shown=true;
                            break;
                        }
                    }

                    //show text, start return mission
                    if(!help_text_shown)
                     m_hud.set_draw_tutorial_text(tut_return,true);*/
                }

                //test if player is stuck
                bool player_struck=false;
                for(int player_i=0;player_i<4;player_i++)
                {
                    if(m_player_active[player_i] && !m_vec_players[player_i].connected_to_mship())
                    {
                        //test body angle
                        float angle_tol=30;//deg
                        if( fabs(m_vec_players[player_i].get_body_ptr()->GetAngle()*_Rad2Deg)<angle_tol )
                         continue;//skip test, pointed upwards

                        //ignore if drone is active
                        if( m_vec_players[player_i].get_drone_mode()==dm_on )
                        {
                            m_player_stuck[player_i]=false;
                            continue;
                        }

                        //test if pos have moved
                        float move_tol=0.1;
                        b2Vec2 body_pos=m_vec_players[player_i].get_body_ptr()->GetPosition();
                        if( fabs(body_pos.x-m_vec_players[player_i].m_body_pos_old.x)<move_tol &&
                            fabs(body_pos.y-m_vec_players[player_i].m_body_pos_old.y)<move_tol )
                        {
                            //increase stuck timer
                            m_vec_players[player_i].m_stuck_timer-=time_dif;
                            if(m_vec_players[player_i].m_stuck_timer<0.0)
                            {
                                m_vec_players[player_i].m_stuck_timer=0;
                                m_player_stuck[player_i]=true;

                                //increase draw timer
                                player_struck=true;
                                if(m_draw_stuck_timer<1.0)
                                {
                                    m_draw_stuck_timer+=time_dif;
                                    if(m_draw_stuck_timer>1.0) m_draw_stuck_timer=1.0;
                                }

                            }


                        }
                        else//update old pos
                        {
                            //not stuck
                            m_player_stuck[player_i]=false;

                            m_vec_players[player_i].m_body_pos_old=body_pos;
                            m_vec_players[player_i].m_stuck_timer=_player_stuck_time;
                        }
                    }
                }
                if(!player_struck)
                {
                    //decrease draw timer
                    if(m_draw_stuck_timer>0.0)
                     m_draw_stuck_timer-=time_dif;
                }
            }

            /*//test if lost (tested when drones are destroyed)
            if(!m_show_lost)
            {
                //test if any active player, without destroyed drone
                bool alive_player_found=false;
                bool have_active_players=false;
                for(int player_i=0;player_i<4;player_i++)
                {
                    if( m_player_active[player_i] )
                    {
                        have_active_players=true;

                        if( m_vec_players[player_i].get_drone_mode()!=dm_destroyed )
                        {
                            alive_player_found=true;
                            break;
                        }
                    }
                }
                //will not end if all players are inactive (at start)
                if(!alive_player_found && have_active_players)
                {
                    m_show_lost=true;
                    m_fade_off=true;
                }
            }*/

            //cam pos update
            b2Vec2 target_pos(0,0);
            //float rel_focus_point[2]={0.0,0.0};
            switch(m_came_mode)
            {
                case cm_follow_one:
                {
                    //find player with cam control gear (not active if in drone mode)
                    int cam_player_index=-1;
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if( m_vec_players[player_i].get_gear_ptr()->get_type()==gt_cam_control &&
                            m_vec_players[player_i].get_drone_mode()==dm_off )
                        {
                            cam_player_index=player_i;
                            break;
                        }
                    }

                    /*//follow player with cam controller
                    if(cam_player_index==-1)
                    {
                        //no player have cam control gear, focus mship
                        target_pos=m_pMain_ship->get_body_ptr()->GetPosition();
                        //or cam satellite...

                    }
                    else//player with cam control found
                    {
                        target_pos=m_vec_players[cam_player_index].get_body_ptr()->GetWorldCenter();
                        float focus_point[2]={0.0,0.0};
                        m_vec_players[cam_player_index].get_focus_point(focus_point);
                        target_pos.x+=focus_point[0]*m_screen_width*_Pix2Met*_screen_cam_player_focus;
                        target_pos.y+=focus_point[1]*m_screen_height*_Pix2Met*_screen_cam_player_focus;
                    }*/

                    //follow selected player
                    if(m_vec_players[m_cam_player_to_follow].get_drone_mode()==dm_on)
                    {
                        target_pos=m_vec_players[m_cam_player_to_follow].get_player_drone_body_ptr()->GetPosition();
                    }
                    else//follow ship
                    {
                        target_pos=m_vec_players[m_cam_player_to_follow].get_body_ptr()->GetPosition();
                    }


                    //added cam control
                    if(cam_player_index!=-1)//player with cam control found
                    {
                        float focus_point[2]={0.0,0.0};
                        m_vec_players[cam_player_index].get_focus_point(focus_point);
                        target_pos.x+=focus_point[0]*m_screen_width*_Pix2Met*_screen_cam_player_focus;
                        target_pos.y+=focus_point[1]*m_screen_height*_Pix2Met*_screen_cam_player_focus;
                        //cout<<focus_point[0]<<", "<<focus_point[1]<<endl;
                    }

                    //TEMP
                    //target_pos=m_vec_players.front().get_body_ptr()->GetPosition();

                }break;

                case cm_follow_all:
                {
                    int player_counter=0;
                    bool first_found_player_found=false;
                    float first_found_player_pos_x=0;//to measure relative dist to other players
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if(m_player_active[player_i])
                        {
                            player_counter++;

                            b2Vec2 player_pos_temp(0,0);
                            if(m_vec_players[player_i].get_drone_mode()==dm_on)
                             player_pos_temp=m_vec_players[player_i].get_player_drone_body_ptr()->GetWorldCenter();
                            else
                             player_pos_temp=m_vec_players[player_i].get_body_ptr()->GetWorldCenter();

                            //if first found player, store index
                            if(!first_found_player_found)
                            {
                                first_found_player_found=true;
                                first_found_player_pos_x=player_pos_temp.x;
                                target_pos.x+=player_pos_temp.x;
                                target_pos.y+=player_pos_temp.y;
                            }
                            else//find shortest dist rel to first found player
                            {
                                //find shortest dist alternative to first player (looped world)
                                float dist1=fabs(first_found_player_pos_x-player_pos_temp.x-m_world_max_x);
                                float dist2=fabs(first_found_player_pos_x-player_pos_temp.x);
                                float dist3=fabs(first_found_player_pos_x-player_pos_temp.x+m_world_max_x);
                                if( dist1<dist2 ) target_pos.x+=player_pos_temp.x-m_world_max_x;
                                if( dist3<dist2 ) target_pos.x+=player_pos_temp.x+m_world_max_x;
                                else              target_pos.x+=player_pos_temp.x;
                                target_pos.y+=player_pos_temp.y;
                            }
                        }
                    }
                    if(player_counter==0)
                    {//no players active
                        //zoom to mship
                        target_pos=m_pMain_ship->get_body_ptr()->GetPosition();
                        break;
                    }
                    //average pos
                    target_pos.x/=(float)player_counter;
                    target_pos.y/=(float)player_counter;
                }break;

                case cm_follow_other:
                {
                    //follow selected enemy
                    if( m_cam_enemy_to_follow<(int)m_vec_pEnemies.size() )
                     target_pos=m_vec_pEnemies[m_cam_enemy_to_follow]->get_body_ptr()->GetPosition();
                    //follow first player
                    //target_pos=m_vec_players.front().get_body_ptr()->GetPosition();
                    //cout<<target_pos.x<<", "<<target_pos.y<<endl;
                }break;

                case cm_follow_none:
                {
                    //move cam with numpad
                    target_pos.x=(m_cam_pos[0]+m_screen_width*0.5)*_Pix2Met;
                    target_pos.y=(m_cam_pos[1]+m_screen_height*0.5)*_Pix2Met;

                    m_cam_speed[0]=m_cam_speed[1]=0.0;
                }break;
            }

            //test if player is outside screen edge, update cam speed
            float rel_pos_to_center_x=((float)target_pos.x*_Met2Pix-m_cam_pos[0]-(float)m_screen_width*0.5)/(float)m_screen_width;
            float rel_pos_to_center_y=((float)target_pos.y*_Met2Pix-m_cam_pos[1]-(float)m_screen_height*0.5)/(float)m_screen_height;
            //update cam_speed x
            if( rel_pos_to_center_x>_screen_shift_thres_max||rel_pos_to_center_x<-_screen_shift_thres_max )
            {//fast shift
                /*if(rel_pos_to_center_x>0.0)
                 m_cam_speed[0]=_screen_cam_speed_up*rel_pos_to_center_x*rel_pos_to_center_x*10.0;
                else
                 m_cam_speed[0]=-_screen_cam_speed_up*rel_pos_to_center_x*rel_pos_to_center_x*10.0;*/

                //move cam directly
                if(m_pMain_ship->get_takeoff_phase()!=tp_blastoff_fadeout)//not if takeoff
                {
                    if(rel_pos_to_center_x>0.0)
                     m_cam_pos[0]=(float)target_pos.x*_Met2Pix-m_screen_width;
                    else
                     m_cam_pos[0]=(float)target_pos.x*_Met2Pix;
                    m_cam_speed[0]=_screen_cam_speed_up*rel_pos_to_center_x;
                }
            }
            else if( rel_pos_to_center_x>=_screen_shift_thres_min_x||rel_pos_to_center_x<=-_screen_shift_thres_min_x )
            {//slow shift, floating cam
                if(rel_pos_to_center_x>0.0)
                 m_cam_speed[0]=_screen_cam_speed_up*rel_pos_to_center_x*rel_pos_to_center_x;
                else
                 m_cam_speed[0]=-_screen_cam_speed_up*rel_pos_to_center_x*rel_pos_to_center_x;
            }
            else if(m_cam_speed[0]!=0.0)//slow down cam speed
            {
                m_cam_speed[0]*=0.9999;

                /*if(m_cam_speed[0]>0.0)
                {
                    m_cam_speed[0]-=_screen_cam_speed_down;
                    if(m_cam_speed[0]<0.0) m_cam_speed[0]=0.0;
                }
                else if(m_cam_speed[0]<0.0)
                {
                    m_cam_speed[0]+=_screen_cam_speed_down;
                    if(m_cam_speed[0]>0.0) m_cam_speed[0]=0.0;
                }*/
            }

            //update cam_speed y
            if( rel_pos_to_center_y>_screen_shift_thres_max||rel_pos_to_center_y<-_screen_shift_thres_max )
            {//fast shift
                /*if(rel_pos_to_center_y>0.0)
                 m_cam_speed[1]=_screen_cam_speed_up*rel_pos_to_center_y*rel_pos_to_center_y*10.0;
                else
                 m_cam_speed[1]=-_screen_cam_speed_up*rel_pos_to_center_y*rel_pos_to_center_y*10.0;*/

                if(m_pMain_ship->get_takeoff_phase()!=tp_blastoff_fadeout)//not if takeoff
                {
                    //move cam directly
                    if(rel_pos_to_center_y>0.0)
                     m_cam_pos[1]=(float)target_pos.y*_Met2Pix-m_screen_height;
                    else
                     m_cam_pos[1]=(float)target_pos.y*_Met2Pix;
                    m_cam_speed[1]=_screen_cam_speed_up*rel_pos_to_center_y;
                }
            }
            else if( rel_pos_to_center_y>=_screen_shift_thres_min_y||rel_pos_to_center_y<=-_screen_shift_thres_min_y )
            {//slow shift, floating cam
                if(rel_pos_to_center_y>0.0)
                 m_cam_speed[1]=_screen_cam_speed_up*rel_pos_to_center_y*rel_pos_to_center_y;
                else
                 m_cam_speed[1]=-_screen_cam_speed_up*rel_pos_to_center_y*rel_pos_to_center_y;
            }
            else if(m_cam_speed[1]!=0.0)//slow down cam speed
            {
                m_cam_speed[1]*=0.9999;

                /*if(m_cam_speed[1]>0.0)
                {
                    m_cam_speed[1]-=_screen_cam_speed_down;
                    if(m_cam_speed[1]<0.0) m_cam_speed[1]=0.0;
                }
                else if(m_cam_speed[1]<0.0)
                {
                    m_cam_speed[1]+=_screen_cam_speed_down;
                    if(m_cam_speed[1]>0.0) m_cam_speed[1]=0.0;
                }*/
            }

            //update cam pos
            m_cam_pos[0]+=m_cam_speed[0]*time_dif;
            m_cam_pos[1]+=m_cam_speed[1]*time_dif;

            //WRAPED WORLD OFF
            /*//shift player pos if outside world (not if inactive or carried by another player)
            for(int player_i=0;player_i<4;player_i++)
            {
                if(!m_player_active[player_i]) continue;
                //if(m_vec_players[player_i].is_carried_by() != -1) continue;

                b2Vec2 player_pos_temp=m_vec_players[player_i].get_body_ptr()->GetWorldCenter();

                while( player_pos_temp.x < (0.0-m_world_offside_limit) )
                {
                    if( m_vec_players[player_i].shift_player_pos(m_world_max_x,0.0) )//true if hook connected
                    {
                        //get body type of connected object
                        b2Body* connected_body=m_vec_players[player_i].get_connected_body_ptr();
                        if(connected_body==m_vec_players[player_i].get_body_ptr())
                        {
                            cout<<"ERROR: Hook should not be connected\n";
                            break;
                        }
                        st_body_user_data* data=(st_body_user_data*)connected_body->GetUserData();
                        string object_type=data->s_info;
                        if(object_type=="terrain")
                        {
                            //detatch hook
                            m_vec_players[player_i].hook_disconnect();
                        }
                        else if(object_type=="player")
                        {
                            //move player
                            int player_id=data->i_id;
                            if( m_vec_players[player_id].shift_player_pos(m_world_max_x,0.0) )
                            {
                                //disconnect hook
                                m_vec_players[player_id].hook_disconnect();
                            }

                        }
                        else if(object_type=="object")
                        {
                            //move object
                            float angle=connected_body->GetAngle();
                            b2Vec2 pos=connected_body->GetWorldCenter();
                            connected_body->SetTransform( b2Vec2( pos.x+m_world_max_x,pos.y ), angle );
                        }
                        else if(object_type=="rope" || object_type=="hook")
                        {
                            //move player
                            int player_id=data->i_id;
                            if( m_vec_players[player_id].shift_player_pos(m_world_max_x,0.0) )
                            {
                                //disconnect hook
                                m_vec_players[player_id].hook_disconnect();
                            }
                        }
                    }

                    player_pos_temp=m_vec_players[player_i].get_body_ptr()->GetWorldCenter();//update for loop test

                    //move cam
                    m_cam_pos[0]+=m_world_max_x*_Met2Pix;
                }
                while( player_pos_temp.x > m_world_max_x+m_world_offside_limit )
                {
                    if( m_vec_players[player_i].shift_player_pos(-m_world_max_x,0.0) )//true if hook connected
                    {
                        //get body type of connected object
                        b2Body* connected_body=m_vec_players[player_i].get_connected_body_ptr();
                        if(connected_body==m_vec_players[player_i].get_body_ptr())
                        {
                            cout<<"ERROR: Hook should not be connected\n";
                            break;
                        }
                        st_body_user_data* data=(st_body_user_data*)connected_body->GetUserData();
                        string object_type=data->s_info;
                        if(object_type=="terrain")
                        {
                            //detatch hook
                            m_vec_players[player_i].hook_disconnect();
                        }
                        else if(object_type=="player")
                        {
                            //move player
                            int player_id=data->i_id;
                            if( m_vec_players[player_id].shift_player_pos(-m_world_max_x,0.0) )
                            {
                                //disconnect hook
                                m_vec_players[player_id].hook_disconnect();
                            }

                        }
                        else if(object_type=="object")
                        {
                            //move object
                            float angle=connected_body->GetAngle();
                            b2Vec2 pos=connected_body->GetWorldCenter();
                            connected_body->SetTransform( b2Vec2( pos.x-m_world_max_x,pos.y ), angle );
                        }
                        else if(object_type=="rope" || object_type=="hook")
                        {
                            //move player
                            int player_id=data->i_id;
                            if( m_vec_players[player_id].shift_player_pos(-m_world_max_x,0.0) )
                            {
                                //disconnect hook
                                m_vec_players[player_id].hook_disconnect();
                            }
                        }
                    }

                    player_pos_temp=m_vec_players[player_i].get_body_ptr()->GetWorldCenter();//update for loop test

                    //move cam
                    m_cam_pos[0]-=m_world_max_x*_Met2Pix;
                }
            }
            //shift objects that are not connected*/

            //update screen fade
            if(m_screen_fade_prog>0.0 && !m_fade_off)
            {
                float fade_speed=1.0;
                m_screen_fade_prog-=time_dif*fade_speed;
                //done
                if(m_screen_fade_prog<0.0)
                {
                    m_screen_fade_prog=0.0;
                }
            }
            if(m_screen_fade_prog<1.0 && m_fade_off)
            {
                float fade_speed=0.4;
                m_screen_fade_prog+=time_dif*fade_speed;

                if(m_on_tutorial_level)
                {
                    if(m_hud.get_draw_tutorial_helptext(ht_takeoff))
                    {
                        m_hud.set_draw_tutorial_helptext(ht_takeoff,true);
                    }
                }

                //fade music
                float volume=1.0-m_screen_fade_prog;
                if(volume<0) volume=0;
                if(volume>1) volume=1;
                m_pSound->set_volume(m_music_source_id, volume);
                //fade mship motor sound
                m_pSound->set_volume_rel(_sound_chan_motor_mship, volume);

                //done
                if(m_screen_fade_prog>1.0)
                {
                    m_screen_fade_prog=1.0;
                    m_fade_off=false;

                    //leave level
                    m_game_state=gs_level_select;

                    //activate starmap noise sound
                    if(!m_music_on) m_music_was_off_at_level_start=true;//remember to start music if resumed later
                    m_pSound->playSimpleSound(ogg_starmap_noise,0.0,m_music_source_id,true);

                    if(m_on_tutorial_level && !m_show_lost)
                    {
                        //m_on_tutorial_level=false;
                        //report to hud for tutorial text (delayed to after starmap)
                        //m_hud.set_draw_tutorial_text(-1,false);

                        //show info screen
                        //m_show_info=true;
                        m_info_screen_phase=is_off;
                        m_game_state=gs_level_select;
                        //m_game_state=gs_in_level;//stay in this state to show info screen
                    }

                    //if lost
                    if(m_show_lost)
                    {
                        m_game_state=gs_in_level;//stay in this state to show lost screen
                        m_lost_screen_phase=is_off;
                    }
                    else//save game state
                     save_game_to_file();

                    //reset speed of players
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        m_vec_players[player_i].reset_motion();
                    }
                }
            }

            //if not in manual keep sound enabled
            if(!m_show_manual && !m_show_lost) m_pSound->enable_sound(true);

            //master destroy body
            b2Body* tmp_body=m_pWorld->GetBodyList();
            while(tmp_body)
            {
                st_body_user_data* data=(st_body_user_data*)tmp_body->GetUserData();
                if(data->b_to_be_deleted)
                {
                    b2Body* body_to_remove=tmp_body;
                    tmp_body=tmp_body->GetNext();
                    //joint checks
                    b2Joint* tmp_joint=m_pWorld->GetJointList();
                    while(tmp_joint)
                    {
                        if(body_to_remove==tmp_joint->GetBodyA() || body_to_remove==tmp_joint->GetBodyB())
                        {
                            b2Joint* tmp_joint_to_remove=tmp_joint;
                            tmp_joint=tmp_joint->GetNext();

                            m_pWorld->DestroyJoint(tmp_joint_to_remove);
                        }
                        else tmp_joint=tmp_joint->GetNext();
                    }

                    delete data;
                    m_pWorld->DestroyBody(body_to_remove);
                }
                else tmp_body=tmp_body->GetNext();
            }

            //cout<<"main update: after, before step\n";

            m_pWorld->Step( _world_step_time,_world_velosity_it,_world_position_it );//box2d dont like a variable time step

            //cout<<"main update: after step\n";

        }break;

        case gs_game_over:
        {
            //update particle engine
            m_pParticle_engine->update(time_dif);

            //update gamepad input
            for(int player_i=0;player_i<4;player_i++)
            {
                if(!m_gamepad_connected[player_i]) continue;//dont read from disconnected gamepad

                if(gamepad_data[player_i].button_start)
                {
                    if(!m_vec_players[player_i].m_key_trigger_start)
                    {
                        m_vec_players[player_i].m_key_trigger_start=true;
                        m_game_state=gs_menu;
                    }
                }
                else m_vec_players[player_i].m_key_trigger_start=false;
            }
        }break;
    }

    return true;
}

bool game::draw()
{
    bool debug_draw=false;
    bool debug_draw_joints=false;

    //draw center screen first, then small windows for every active player outside the screen
    vector<st_float_float_int> vec_taken_window;//with side value
    for(int multi_draw=-1;multi_draw<4;multi_draw++)
    {
        float cam_pos[2]={m_cam_pos[0],m_cam_pos[1]};
        bool draw_window=false;
        int window_size=int((float)m_screen_width*0.15);//square
        int window_gap=10;
        int window_pos_x=0;
        int window_pos_y=0;
        //test if player needs window
        if(multi_draw>=0)
        {
            if(!m_player_active[multi_draw]) continue;//not if inactive player
            if(m_game_state!=gs_in_level || m_show_gameover || m_show_info ||
               m_show_lost || m_show_manual || m_pMain_ship->is_takeoff() )
             break;//not if not in game level nor if text slide is shown, not during takeoff

            b2Vec2 player_pos=m_vec_players[multi_draw].get_body_ptr()->GetPosition();
            if( m_vec_players[multi_draw].get_drone_mode()==dm_on )
             player_pos=m_vec_players[multi_draw].get_player_drone_body_ptr()->GetPosition();//get drone instead
            if(player_pos.x*_Met2Pix<cam_pos[0] || player_pos.x*_Met2Pix>cam_pos[0]+m_screen_width ||
               player_pos.y*_Met2Pix<cam_pos[1] || player_pos.y*_Met2Pix>cam_pos[1]+m_screen_height)
            {
                draw_window=true;

                st_float_float_int wind_dim;

                //int window_width=int((float)m_screen_width*0.15);
                //int window_height=int((float)m_screen_height*0.15);
                //int window_size=int((float)m_screen_width*0.15);//square

                //set new cam pos
                cam_pos[0]=player_pos.x*_Met2Pix-window_size*0.5;
                cam_pos[1]=player_pos.y*_Met2Pix-window_size*0.5;

                //set window position
                window_pos_x=cam_pos[0]-m_cam_pos[0];
                window_pos_y=cam_pos[1]-m_cam_pos[1];
                //cap position
                if(window_pos_y<int((float)m_screen_height*0.05))
                {
                    window_pos_y=int((float)m_screen_height*0.05);
                    wind_dim.val_i=dir_up;
                }
                if(window_pos_y>m_screen_height-window_size-int((float)m_screen_height*0.05))
                {
                    window_pos_y=m_screen_height-window_size-int((float)m_screen_height*0.05);
                    wind_dim.val_i=dir_down;
                }
                if(window_pos_x<0)
                {
                    window_pos_x=0;
                    wind_dim.val_i=dir_left;
                }
                if(window_pos_x>m_screen_width-window_size)
                {
                    window_pos_x=m_screen_width-window_size;
                    wind_dim.val_i=dir_right;
                }
                wind_dim.val_f1=window_pos_x;
                wind_dim.val_f2=window_pos_y;

                //test if window space is taken
                while(true)
                {
                    bool space_taken=false;
                    for(unsigned int wind_i=0;wind_i<vec_taken_window.size();wind_i++)
                    {
                        if( wind_dim.val_f1>vec_taken_window[wind_i].val_f1+window_size ||
                            wind_dim.val_f1<vec_taken_window[wind_i].val_f1-window_size ||
                            wind_dim.val_f2>vec_taken_window[wind_i].val_f2+window_size ||
                            wind_dim.val_f2<vec_taken_window[wind_i].val_f2-window_size )
                        {
                            ;//pos ok
                        }
                        else
                        {
                            //space taken
                            space_taken=true;
                            break;
                        }
                    }
                    if(!space_taken) break;

                    //else shift pos of that window
                    bool pos_error=false;
                    switch(wind_dim.val_i)
                    {
                        case dir_up:    wind_dim.val_f2+=window_size+window_gap; break;
                        case dir_right: wind_dim.val_f1-=window_size+window_gap; break;
                        case dir_down:  wind_dim.val_f2-=window_size+window_gap; break;
                        case dir_left:  wind_dim.val_f1+=window_size+window_gap; break;

                        default : pos_error=true;
                    }
                    if(pos_error)
                    {
                        break;//should not happen
                    }
                }
                //store window pos
                vec_taken_window.push_back(wind_dim);
                window_pos_x=wind_dim.val_f1;
                window_pos_y=wind_dim.val_f2;

                //draw black canvas
                glColor3f(0,0,0);
                glBegin(GL_QUADS);
                glVertex2f(window_pos_x+window_size,window_pos_y);
                glVertex2f(window_pos_x+window_size,window_pos_y+window_size);
                glVertex2f(window_pos_x,window_pos_y+window_size);
                glVertex2f(window_pos_x,window_pos_y);
                glEnd();

                //draw window border
                b2Vec2 screen_center=b2Vec2((m_cam_pos[0]+m_screen_width*0.5)*_Pix2Met, (m_cam_pos[1]+m_screen_height*0.5)*_Pix2Met);
                b2Vec2 rel_pos(player_pos);
                rel_pos-=screen_center;
                float border_color=rel_pos.Length()/50.0;
                if(border_color<0.3) border_color=0.3;
                if(border_color>1.0) border_color=1.0;
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE,GL_ONE);
                glLineWidth(2);
                glColor4f(border_color,border_color,border_color,0.9);
                glBegin(GL_LINE_STRIP);
                glVertex2f(window_pos_x+window_size,window_pos_y);
                glVertex2f(window_pos_x+window_size,window_pos_y+window_size);
                glVertex2f(window_pos_x,window_pos_y+window_size);
                glVertex2f(window_pos_x,window_pos_y);
                glVertex2f(window_pos_x+window_size,window_pos_y);
                glEnd();
                glLineWidth(1);
                glDisable(GL_BLEND);

                //set viewport
                glViewport(window_pos_x,m_screen_height-window_pos_y-window_size,window_size,window_size);
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(0, window_size, window_size, 0,-1,1);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

                glClear(GL_STENCIL_BUFFER_BIT);
            }
        }
        else//set viewport to fullscreen
        {
            glViewport(0,0,m_screen_width,m_screen_height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, m_screen_width, m_screen_height, 0,-1,1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        }

        if(!draw_window && multi_draw!=-1) continue;//test next player

        switch(m_game_state)
        {
            case gs_menu:
            {
                //draw menu level

                //draw stars
                glPushMatrix();
                glTranslatef(((112.5*_Met2Pix-m_screen_width*0.5)-cam_pos[0])*0.7,-cam_pos[1]*0.7,0);
                glBegin(GL_POINTS);
                for(int i=0;i<(int)m_vec_stars.size();i++)
                {
                    glColor3f(m_vec_stars[i].val_f,m_vec_stars[i].val_f,m_vec_stars[i].val_f);
                    glVertex2f( m_vec_stars[i].val_i1,m_vec_stars[i].val_i2 );
                }
                glEnd();
                glPopMatrix();

                glPushMatrix();
                //move cam to cam_pos
                glTranslatef(-cam_pos[0],-cam_pos[1],0);

                //debug draw
                if(debug_draw)
                {
                    float curr_color[3]={1.0,1.0,1.0};
                    for(int rep=1;rep<2;rep++)//draw world again next to original, if cam is close to edge
                    {
                        switch(rep)
                        {
                            case 0://left rep
                            {
                                //test cam pos
                                if( cam_pos[0]*_Pix2Met<0.0 )//draw left copy
                                {
                                    curr_color[0]=1; curr_color[1]=0; curr_color[2]=0;
                                    glPushMatrix();
                                    glTranslatef(-m_world_max_x*_Met2Pix,0,0);
                                }
                                else continue;//skip drawing
                            }break;

                            case 1://center rep
                            {
                                curr_color[0]=1; curr_color[1]=1; curr_color[2]=1;
                                glPushMatrix();//draw always
                            }break;

                            case 2://right rep
                            {
                                //test cam pos
                                if( cam_pos[0]*_Pix2Met+(float)m_screen_width*_Pix2Met>m_world_max_x )//draw left copy
                                {
                                    curr_color[0]=0; curr_color[1]=1; curr_color[2]=0;
                                    glPushMatrix();
                                    glTranslatef(m_world_max_x*_Met2Pix,0,0);
                                }
                                else continue;//skip drawing
                            }break;
                        }
                        b2Body* tmp=m_pWorld->GetBodyList();

                        while(tmp)
                        {
                            glPushMatrix();
                            b2Vec2 center=tmp->GetWorldCenter();
                            glTranslatef(center.x*_Met2Pix,center.y*_Met2Pix,0);

                            b2MassData massD;
                            tmp->GetMassData(&massD);
                            b2Vec2 center_of_mass=massD.center;
                            //cout<<center.x<<endl;
                            //glTranslatef((center.x-center_of_mass.x)*_Met2Pix,(center.y-center_of_mass.y)*_Met2Pix,0);

                            glRotatef(tmp->GetAngle()*180/3.14159,0,0,1);
                            glTranslatef(-center_of_mass.x*_Met2Pix,-center_of_mass.y*_Met2Pix,0);
                            for( b2Fixture* fixture=tmp->GetFixtureList(); fixture; fixture=fixture->GetNext() )
                            {
                                //test if sensor
                                if( fixture->IsSensor() )
                                {
                                    //hot pink
                                    glColor3f(1,0,1);
                                }
                                else glColor3fv(curr_color);
                                //get shape type
                                b2Shape* shape=fixture->GetShape();
                                if( shape->m_type==b2Shape::e_chain )
                                {
                                    //skip terrain
                                    break;

                                    //chain
                                    int vertex_count=((b2ChainShape*)shape)->m_count;
                                    b2Vec2 points[vertex_count];

                                    for(int i=0;i<vertex_count;i++)
                                        points[i]=((b2ChainShape*)shape)->m_vertices[i];

                                    glBegin(GL_LINE_STRIP);
                                    for(int i=0;i<vertex_count;i++)
                                        glVertex2f(points[i].x*_Met2Pix,points[i].y*_Met2Pix);

                                    glEnd();
                                }
                                else if( shape->m_type==b2Shape::e_polygon )//polygon
                                {
                                    //skip menutext
                                    st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                                    if(data->s_info=="menutext") break;


                                    int vertex_count=((b2PolygonShape*)fixture->GetShape())->GetVertexCount();
                                    b2Vec2 points[vertex_count];

                                    for(int i=0;i<vertex_count;i++)
                                        points[i]=((b2PolygonShape*)fixture->GetShape())->GetVertex(i);

                                    glBegin(GL_LINE_STRIP);
                                    for(int i=0;i<vertex_count;i++)
                                        glVertex2f(points[i].x*_Met2Pix,points[i].y*_Met2Pix);

                                    glVertex2f(points[0].x*_Met2Pix,points[0].y*_Met2Pix);

                                    glEnd();
                                }
                                else if( shape->m_type==b2Shape::e_circle)//circle
                                {
                                    glBegin(GL_LINE_STRIP);
                                    for (float a = 0; a < 360 * _Deg2Rad; a += 30 * _Deg2Rad)
                                     glVertex2f( sinf(a)*shape->m_radius*_Met2Pix,
                                                 cosf(a)*shape->m_radius*_Met2Pix );
                                    glVertex2f(0,0);
                                    glEnd();
                                }
                            }

                            glPopMatrix();

                            tmp=tmp->GetNext();
                        }

                        glPopMatrix();//pop world shift
                    }
                }

                /*//draw rope and hooks
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                b2Body* tmp=m_pWorld->GetBodyList();
                while(tmp)
                {
                    //find rope or hook
                    st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                    if(data->s_info=="rope" && data->b_disconnected_part)
                    {
                        glColor3f(0.7,0.0,0.7);
                        glLineWidth(2);
                        glPushMatrix();
                        glTranslatef(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix,0.0);
                        glRotatef(tmp->GetAngle()*_Rad2Deg,0,0,1);
                        glBegin(GL_LINES);
                        glVertex2f(0.0,-_rope_part_length*_Met2Pix);
                        glVertex2f(0.0,_rope_part_length*_Met2Pix);
                        glEnd();
                        glLineWidth(1);
                        glPopMatrix();
                    }
                    else if(data->s_info=="hook" && data->b_disconnected_part)
                    {

                    }

                    tmp=tmp->GetNext();
                }
                glDisable(GL_BLEND);*/

                //draw players
                for(int player_i=0;player_i<4;player_i++)
                {
                    //draw ship and rope
                    m_vec_players[player_i].draw();
                }

                //draw enemies
                for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
                {
                    m_vec_pEnemies[enemy_i]->draw();
                }

                //draw objects
                for(int object_i=0;object_i<(int)m_vec_objects.size();object_i++)
                {
                    m_vec_objects[object_i].draw();
                }

                //draw text boxes
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, m_tex_menu);
                b2Body* tmp=m_pWorld->GetBodyList();
                while(tmp)
                {
                    //find text box
                    st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                    if(data->s_info=="menutext")
                    {
                        float brightness=1.0;
                        //select texture pos
                        //float tex_size=1024.0;
                        float texture_pos_min[]={0,0};
                        float texture_pos_max[]={1,1};
                        switch(data->i_id)
                        {
                            case 3:
                            {
                                texture_pos_min[0]=0.0/1024.0;
                                texture_pos_min[1]=(1024.0-0.0)/1024.0;
                                texture_pos_max[0]=374.0/1024.0;
                                texture_pos_max[1]=(1024.0-120.0)/1024.0;

                                brightness=(sinf(m_time_this_cycle+10)+1.0)/4.0+0.5;
                            }break;

                            case 2:
                            {
                                texture_pos_min[0]=0.0/1024.0;
                                texture_pos_min[1]=(1024.0-124.0)/1024.0;
                                texture_pos_max[0]=412.0/1024.0;
                                texture_pos_max[1]=(1024.0-242.0)/1024.0;

                                brightness=(sinf(m_time_this_cycle+30)+1.0)/4.0+0.5;
                            }break;

                            case 1:
                            {
                                texture_pos_min[0]=0.0/1024.0;
                                texture_pos_min[1]=(1024.0-243.0)/1024.0;
                                texture_pos_max[0]=588.0/1024.0;
                                texture_pos_max[1]=(1024.0-361.0)/1024.0;

                                brightness=(sinf(m_time_this_cycle+100.0)+1.0)/4.0+0.5;
                            }break;
                        }
                        glColor3f(brightness,brightness,brightness);

                        glPushMatrix();

                        b2Vec2 center=tmp->GetWorldCenter();
                        glTranslatef(center.x*_Met2Pix,center.y*_Met2Pix,0);
                        glRotatef(tmp->GetAngle()*180.0/_pi,0,0,1);
                        b2Fixture* fixture=tmp->GetFixtureList();// only one
                        int vertex_count=((b2PolygonShape*)fixture->GetShape())->GetVertexCount();//should be 4
                        b2Vec2 points[vertex_count];//top right, going cw

                        for(int i=0;i<vertex_count;i++)
                            points[i]=((b2PolygonShape*)fixture->GetShape())->GetVertex(i);

                        glBegin(GL_QUADS);
                        /*for(int i=0;i<vertex_count;i++)
                            glVertex2f(points[i].x*_Met2Pix,points[i].y*_Met2Pix);*/

                        glTexCoord2f(texture_pos_max[0],texture_pos_min[1]);
                        glVertex2f(points[0].x*_Met2Pix,points[0].y*_Met2Pix);
                        glTexCoord2f(texture_pos_max[0],texture_pos_max[1]);
                        glVertex2f(points[1].x*_Met2Pix,points[1].y*_Met2Pix);
                        glTexCoord2f(texture_pos_min[0],texture_pos_max[1]);
                        glVertex2f(points[2].x*_Met2Pix,points[2].y*_Met2Pix);
                        glTexCoord2f(texture_pos_min[0],texture_pos_min[1]);
                        glVertex2f(points[3].x*_Met2Pix,points[3].y*_Met2Pix);

                        glEnd();

                        glPopMatrix();
                    }

                    tmp=tmp->GetNext();
                }
                glDisable(GL_TEXTURE_2D);

                //draw main ship
                m_pMain_ship->draw();

                //draw projectiles
                draw_projectiles();

                //draw particles
                m_pParticle_engine->draw();

                //draw terrain and pop matrix
                draw_terrain(cam_pos);
                //matrix now popped to standard view

                //draw start messages
                glPushMatrix();
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_tex_menu);
                glEnable(GL_TEXTURE_2D);
                glTranslatef(-cam_pos[0],-cam_pos[1],0);//to cam pos
                glTranslatef(87.5*_Met2Pix,88.0*_Met2Pix,0);//to text pos
                glColor3f(1,1,1);
                glBegin(GL_QUADS);
                glTexCoord2f(1.0,(1024.0-973.0)/1024.0);
                glVertex2f(1024.0,0.0);
                glTexCoord2f(1.0,(1024.0-1003.0)/1024.0);
                glVertex2f(1024.0,30.0);
                glTexCoord2f(0.0,(1024.0-1003.0)/1024.0);
                glVertex2f(0.0,30.0);
                glTexCoord2f(0.0,(1024.0-973.0)/1024.0);
                glVertex2f(0.0,0.0);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();
                if(m_save_file_present)
                {
                    glPushMatrix();
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_tex_menu);
                    glEnable(GL_TEXTURE_2D);
                    glTranslatef(-cam_pos[0],-cam_pos[1],0);//to cam pos
                    glTranslatef(87.5*_Met2Pix,90.0*_Met2Pix,0);//to text pos
                    glColor3f(1,1,1);
                    glBegin(GL_QUADS);
                    glTexCoord2f(1.0,(1024.0-943.0)/1024.0);
                    glVertex2f(1024.0,0.0);
                    glTexCoord2f(1.0,(1024.0-973.0)/1024.0);
                    glVertex2f(1024.0,30.0);
                    glTexCoord2f(0.0,(1024.0-973.0)/1024.0);
                    glVertex2f(0.0,30.0);
                    glTexCoord2f(0.0,(1024.0-943.0)/1024.0);
                    glVertex2f(0.0,0.0);
                    glEnd();
                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);
                    glPopMatrix();
                }

                //draw credits
                glPushMatrix();
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE,GL_ONE);
                glBindTexture(GL_TEXTURE_2D, m_tex_menu);
                glEnable(GL_TEXTURE_2D);
                glTranslatef(-cam_pos[0],-cam_pos[1],0);//to cam pos
                glTranslatef(100.0*_Met2Pix,93.0*_Met2Pix,0);//to text pos
                //glTranslatef( ((float)m_screen_width-498.0)*0.5, ((float)m_screen_height-46.0)*0.5, 0 );//to center
                glColor3f(1,1,1);
                glBegin(GL_QUADS);
                glTexCoord2f(498.0/1024.0,(1024.0-713.0)/1024.0);
                glVertex2f(498.0,0.0);
                glTexCoord2f(498.0/1024.0,(1024.0-759.0)/1024.0);
                glVertex2f(498.0,46.0);
                glTexCoord2f(0.0,(1024.0-759.0)/1024.0);
                glVertex2f(0.0,46.0);
                glTexCoord2f(0.0,(1024.0-713.0)/1024.0);
                glVertex2f(0.0,0.0);
                glEnd();
                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
                glPopMatrix();

                //draw controller required sign, if no controllers are connected
                bool control_connected=false;
                for(int cont_i=0;cont_i<4;cont_i++)
                {
                    if( m_gamepad[cont_i].IsConnected() )
                    {
                        control_connected=true;
                        break;
                    }
                }
                if(!control_connected)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE,GL_ONE);
                    glBindTexture(GL_TEXTURE_2D, m_tex_menu);
                    glEnable(GL_TEXTURE_2D);
                    glColor3f(1,1,1);
                    glBegin(GL_QUADS);
                    glTexCoord2f(645.0/1024.0,(1024.0-853.0)/1024.0);
                    glVertex2f(645.0,0.0);
                    glTexCoord2f(645.0/1024.0,(1024.0-884.0)/1024.0);
                    glVertex2f(645.0,31.0);
                    glTexCoord2f(0.0,(1024.0-884.0)/1024.0);
                    glVertex2f(0.0,31.0);
                    glTexCoord2f(0.0,(1024.0-853.0)/1024.0);
                    glVertex2f(0.0,0.0);
                    glEnd();
                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);
                }

                //draw static
                draw_static();

                //draw screen fade
                if(m_screen_fade_prog>0.0)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0,0,0,m_screen_fade_prog);
                    glBegin(GL_QUADS);
                    glVertex2d(0,0);
                    glVertex2d(m_screen_width,0);
                    glVertex2d(m_screen_width,m_screen_height);
                    glVertex2d(0,m_screen_height);
                    glEnd();
                    glDisable(GL_BLEND);
                }

            }break;

            case gs_training:
            {
                //draw training level

                switch(m_training_state)
                {
                    case ts_ask:
                    {
                        glPushMatrix();
                        glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.4,0);

                        //draw question text
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_ONE,GL_ONE);
                        glBindTexture(GL_TEXTURE_2D, m_tex_moretext);
                        glEnable(GL_TEXTURE_2D);
                        glColor3f(1,1,1);
                        glBegin(GL_QUADS);
                        glTexCoord2f(1.0,(1024.0-0.0)/1024.0);
                        glVertex2f(1024.0,0.0);
                        glTexCoord2f(1.0,(1024.0-31.0)/1024.0);
                        glVertex2f(1024.0,31.0);
                        glTexCoord2f(0.0,(1024.0-31.0)/1024.0);
                        glVertex2f(0.0,31.0);
                        glTexCoord2f(0.0,(1024.0-0.0)/1024.0);
                        glVertex2f(0.0,0.0);
                        glEnd();

                        //draw reply options
                        glTranslatef(0.0,m_screen_height*0.1,0);
                        glBegin(GL_QUADS);
                        glTexCoord2f(1.0,(1024.0-32.0)/1024.0);
                        glVertex2f(1024.0,0.0);
                        glTexCoord2f(1.0,(1024.0-58.0)/1024.0);
                        glVertex2f(1024.0,26.0);
                        glTexCoord2f(0.0,(1024.0-58.0)/1024.0);
                        glVertex2f(0.0,26.0);
                        glTexCoord2f(0.0,(1024.0-32.0)/1024.0);
                        glVertex2f(0.0,0.0);
                        glEnd();

                        glDisable(GL_TEXTURE_2D);
                        glDisable(GL_BLEND);

                        //cover other answer, if replied no
                        if(m_fade_off)
                        {
                            glColor3f(0,0,0);
                            glBegin(GL_QUADS);
                            glVertex2f(583.0,0.0);
                            glVertex2f(583.0,26.0);
                            glVertex2f(0.0,26.0);
                            glVertex2f(0.0,0.0);
                            glEnd();
                        }


                        glPopMatrix();
                    }break;

                    case ts_reply:
                    {
                        //draw question text
                        glPushMatrix();
                        glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.4,0);

                        //draw question text
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_ONE,GL_ONE);
                        glBindTexture(GL_TEXTURE_2D, m_tex_moretext);
                        glEnable(GL_TEXTURE_2D);
                        glColor3f(1,1,1);
                        glBegin(GL_QUADS);
                        glTexCoord2f(1.0,(1024.0-0.0)/1024.0);
                        glVertex2f(1024.0,0.0);
                        glTexCoord2f(1.0,(1024.0-31.0)/1024.0);
                        glVertex2f(1024.0,31.0);
                        glTexCoord2f(0.0,(1024.0-31.0)/1024.0);
                        glVertex2f(0.0,31.0);
                        glTexCoord2f(0.0,(1024.0-0.0)/1024.0);
                        glVertex2f(0.0,0.0);
                        glEnd();

                        //draw reply options
                        glTranslatef(0.0,m_screen_height*0.1,0);
                        glBegin(GL_QUADS);
                        glTexCoord2f(1.0,(1024.0-32.0)/1024.0);
                        glVertex2f(1024.0,0.0);
                        glTexCoord2f(1.0,(1024.0-58.0)/1024.0);
                        glVertex2f(1024.0,26.0);
                        glTexCoord2f(0.0,(1024.0-58.0)/1024.0);
                        glVertex2f(0.0,26.0);
                        glTexCoord2f(0.0,(1024.0-32.0)/1024.0);
                        glVertex2f(0.0,0.0);
                        glEnd();

                        //draw reply
                        glTranslatef(0.0,m_screen_height*0.1,0);
                        glBegin(GL_QUADS);
                        glTexCoord2f(1.0,(1024.0-59.0)/1024.0);
                        glVertex2f(1024.0,0.0);
                        glTexCoord2f(1.0,(1024.0-85.0)/1024.0);
                        glVertex2f(1024.0,26.0);
                        glTexCoord2f(0.0,(1024.0-85.0)/1024.0);
                        glVertex2f(0.0,26.0);
                        glTexCoord2f(0.0,(1024.0-59.0)/1024.0);
                        glVertex2f(0.0,0.0);
                        glEnd();

                        glDisable(GL_TEXTURE_2D);
                        glDisable(GL_BLEND);

                        //cover other answer
                        glTranslatef(0.0,-m_screen_height*0.1,0);
                        glColor3f(0,0,0);
                        glBegin(GL_QUADS);
                        glVertex2f(1024.0,0.0);
                        glVertex2f(1024.0,26.0);
                        glVertex2f(512.0,26.0);
                        glVertex2f(512.0,0.0);

                        glVertex2f(434.0,0.0);
                        glVertex2f(434.0,26.0);
                        glVertex2f(0.0,26.0);
                        glVertex2f(0.0,0.0);
                        glEnd();

                        glPopMatrix();
                    }break;

                    case ts_rings_first:
                    {
                        ;
                    }//no break

                    case ts_rings_second:
                    {
                        //draw stars
                        glPushMatrix();
                        glTranslatef(((112.5*_Met2Pix-m_screen_width*0.5)-cam_pos[0])*0.7,-cam_pos[1]*0.7,0);
                        glBegin(GL_POINTS);
                        for(int i=0;i<(int)m_vec_stars.size();i++)
                        {
                            glColor3f(m_vec_stars[i].val_f,m_vec_stars[i].val_f,m_vec_stars[i].val_f);
                            glVertex2f( m_vec_stars[i].val_i1,m_vec_stars[i].val_i2 );
                        }
                        glEnd();
                        glPopMatrix();

                        //draw rings
                        glPushMatrix();
                        glTranslatef(-cam_pos[0],-cam_pos[1],0);//move to cam pos
                        for(int player_i=0;player_i<4;player_i++)
                        {
                            if(m_gamepad_connected[player_i])
                            {
                                switch(player_i)
                                {
                                    case 0:
                                    {
                                        for(unsigned int ring_i=0;ring_i<m_vec_training_rings_p1.size();ring_i++)
                                        {
                                            if(m_vec_training_rings_p1[ring_i].val_f<=0) continue;//completed

                                            float color[4]={0.7,0.2,0.2,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_big*(1.0-m_vec_training_rings_p1[ring_i].val_f/_training_ring_time);
                                            if(ring_i>2) radius=_training_ring_radius_small*(1.0-m_vec_training_rings_p1[ring_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_vec_training_rings_p1[ring_i].val_i1,m_vec_training_rings_p1[ring_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p1[ring_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_vec_training_rings_p1[ring_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.9,0.5,0.5,0.9);
                                            float full_radius=_training_ring_radius_big;
                                            if(ring_i>2) full_radius=_training_ring_radius_small;
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p1[ring_i].val_i1+cosf(angle*_Deg2Rad)*full_radius,
                                                            m_vec_training_rings_p1[ring_i].val_i2+sinf(angle*_Deg2Rad)*full_radius );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);

                                            break;//draw only one
                                        }

                                        //draw hook ring
                                        if(m_vec_training_rings_p1[4].val_f<=0.0 && m_arr_training_rings_hook[player_i].val_f>0.0)
                                        {
                                            float color[4]={0.7,0.2,0.2,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_small*(1.0-m_arr_training_rings_hook[player_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_arr_training_rings_hook[player_i].val_i1,m_arr_training_rings_hook[player_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.9,0.5,0.5,0.9);
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*_training_ring_radius_small,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*_training_ring_radius_small );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);
                                        }

                                    }break;

                                    case 1:
                                    {
                                        for(unsigned int ring_i=0;ring_i<m_vec_training_rings_p2.size();ring_i++)
                                        {
                                            if(m_vec_training_rings_p2[ring_i].val_f<=0) continue;//completed

                                            float color[4]={0.2,0.7,0.2,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_big*(1.0-m_vec_training_rings_p2[ring_i].val_f/_training_ring_time);
                                            if(ring_i>2) radius=_training_ring_radius_small*(1.0-m_vec_training_rings_p2[ring_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_vec_training_rings_p2[ring_i].val_i1,m_vec_training_rings_p2[ring_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p2[ring_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_vec_training_rings_p2[ring_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.5,0.9,0.5,0.9);
                                            float full_radius=_training_ring_radius_big;
                                            if(ring_i>2) full_radius=_training_ring_radius_small;
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p2[ring_i].val_i1+cosf(angle*_Deg2Rad)*full_radius,
                                                            m_vec_training_rings_p2[ring_i].val_i2+sinf(angle*_Deg2Rad)*full_radius );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);

                                            break;//draw only one
                                        }

                                        //draw hook ring
                                        if(m_vec_training_rings_p2[4].val_f<=0.0 && m_arr_training_rings_hook[player_i].val_f>0.0)
                                        {
                                            float color[4]={0.2,0.7,0.2,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_small*(1.0-m_arr_training_rings_hook[player_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_arr_training_rings_hook[player_i].val_i1,m_arr_training_rings_hook[player_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.5,0.9,0.5,0.9);
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*_training_ring_radius_small,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*_training_ring_radius_small );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);
                                        }
                                    }break;

                                    case 2:
                                    {
                                        for(unsigned int ring_i=0;ring_i<m_vec_training_rings_p3.size();ring_i++)
                                        {
                                            if(m_vec_training_rings_p3[ring_i].val_f<=0) continue;//completed

                                            float color[4]={0.2,0.2,0.7,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_big*(1.0-m_vec_training_rings_p3[ring_i].val_f/_training_ring_time);
                                            if(ring_i>2) radius=_training_ring_radius_small*(1.0-m_vec_training_rings_p3[ring_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_vec_training_rings_p3[ring_i].val_i1,m_vec_training_rings_p3[ring_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p3[ring_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_vec_training_rings_p3[ring_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.5,0.5,0.9,0.9);
                                            float full_radius=_training_ring_radius_big;
                                            if(ring_i>2) full_radius=_training_ring_radius_small;
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p3[ring_i].val_i1+cosf(angle*_Deg2Rad)*full_radius,
                                                            m_vec_training_rings_p3[ring_i].val_i2+sinf(angle*_Deg2Rad)*full_radius );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);

                                            break;//draw only one
                                        }

                                        //draw hook ring
                                        if(m_vec_training_rings_p3[4].val_f<=0.0 && m_arr_training_rings_hook[player_i].val_f>0.0)
                                        {
                                            float color[4]={0.2,0.2,0.7,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_small*(1.0-m_arr_training_rings_hook[player_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_arr_training_rings_hook[player_i].val_i1,m_arr_training_rings_hook[player_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.5,0.5,0.9,0.9);
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*_training_ring_radius_small,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*_training_ring_radius_small );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);
                                        }
                                    }break;

                                    case 3:
                                    {
                                        for(unsigned int ring_i=0;ring_i<m_vec_training_rings_p4.size();ring_i++)
                                        {
                                            if(m_vec_training_rings_p4[ring_i].val_f<=0) continue;//completed

                                            float color[4]={0.7,0.6,0.2,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_big*(1.0-m_vec_training_rings_p4[ring_i].val_f/_training_ring_time);
                                            if(ring_i>2) radius=_training_ring_radius_small*(1.0-m_vec_training_rings_p4[ring_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_vec_training_rings_p4[ring_i].val_i1,m_vec_training_rings_p4[ring_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p4[ring_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_vec_training_rings_p4[ring_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.9,0.8,0.5,0.9);
                                            float full_radius=_training_ring_radius_big;
                                            if(ring_i>2) full_radius=_training_ring_radius_small;
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_vec_training_rings_p4[ring_i].val_i1+cosf(angle*_Deg2Rad)*full_radius,
                                                            m_vec_training_rings_p4[ring_i].val_i2+sinf(angle*_Deg2Rad)*full_radius );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);

                                            break;//draw only one
                                        }

                                        //draw hook ring
                                        if(m_vec_training_rings_p4[4].val_f<=0.0 && m_arr_training_rings_hook[player_i].val_f>0.0)
                                        {
                                            float color[4]={0.7,0.6,0.2,0.2};
                                            //inner filled
                                            glEnable(GL_BLEND);
                                            float radius=_training_ring_radius_small*(1.0-m_arr_training_rings_hook[player_i].val_f/_training_ring_time);
                                            glColor4fv(color);
                                            glBegin(GL_TRIANGLE_FAN);
                                            glVertex2f(m_arr_training_rings_hook[player_i].val_i1,m_arr_training_rings_hook[player_i].val_i2);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*radius,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*radius );
                                            }
                                            glEnd();

                                            //outer line
                                            glLineWidth(2);
                                            glColor4f(0.9,0.8,0.5,0.9);
                                            glBegin(GL_LINE_STRIP);
                                            for(float angle=0;angle<=360.0;angle+=10.0)
                                            {
                                                glVertex2f( m_arr_training_rings_hook[player_i].val_i1+cosf(angle*_Deg2Rad)*_training_ring_radius_small,
                                                            m_arr_training_rings_hook[player_i].val_i2+sinf(angle*_Deg2Rad)*_training_ring_radius_small );
                                            }
                                            glEnd();
                                            glLineWidth(1);
                                            glDisable(GL_BLEND);
                                        }
                                    }break;
                                }
                            }
                        }
                        glPopMatrix();

                        //draw text
                        if(m_training_state==ts_rings_second)
                        {
                            glPushMatrix();
                            glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.05,0);
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE,GL_ONE);
                            glBindTexture(GL_TEXTURE_2D, m_tex_moretext);
                            glEnable(GL_TEXTURE_2D);
                            glColor3f(1,1,1);
                            glBegin(GL_QUADS);
                            glTexCoord2f(1.0,(1024.0-140.0)/1024.0);
                            glVertex2f(1024.0,0.0);
                            glTexCoord2f(1.0,(1024.0-221.0)/1024.0);
                            glVertex2f(1024.0,81.0);
                            glTexCoord2f(0.0,(1024.0-221.0)/1024.0);
                            glVertex2f(0.0,81.0);
                            glTexCoord2f(0.0,(1024.0-140.0)/1024.0);
                            glVertex2f(0.0,0.0);
                            glEnd();
                            glDisable(GL_TEXTURE_2D);
                            glDisable(GL_BLEND);
                            glPopMatrix();
                        }
                        else//draw text for first rings
                        {
                            glPushMatrix();
                            glTranslatef(m_screen_width*0.5-512.0,m_screen_height*0.05,0);
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE,GL_ONE);
                            glBindTexture(GL_TEXTURE_2D, m_tex_moretext);
                            glEnable(GL_TEXTURE_2D);
                            glColor3f(1,1,1);
                            glBegin(GL_QUADS);
                            glTexCoord2f(1.0,(1024.0-86.0)/1024.0);
                            glVertex2f(1024.0,0.0);
                            glTexCoord2f(1.0,(1024.0-140.0)/1024.0);
                            glVertex2f(1024.0,54.0);
                            glTexCoord2f(0.0,(1024.0-140.0)/1024.0);
                            glVertex2f(0.0,54.0);
                            glTexCoord2f(0.0,(1024.0-86.0)/1024.0);
                            glVertex2f(0.0,0.0);
                            glEnd();
                            glDisable(GL_TEXTURE_2D);
                            glDisable(GL_BLEND);
                            glPopMatrix();
                        }


                        glPushMatrix();
                        //move cam to cam_pos
                        glTranslatef(-cam_pos[0],-cam_pos[1],0);

                        //draw players
                        for(int player_i=0;player_i<4;player_i++)
                        {
                            //draw ship and rope
                            m_vec_players[player_i].draw();
                        }

                        //draw particles
                        m_pParticle_engine->draw();

                        glPopMatrix();
                    }break;
                }


                //draw screen fade
                if(m_screen_fade_prog>0.0)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0,0,0,m_screen_fade_prog);
                    glBegin(GL_QUADS);
                    glVertex2d(0,0);
                    glVertex2d(m_screen_width,0);
                    glVertex2d(m_screen_width,m_screen_height);
                    glVertex2d(0,m_screen_height);
                    glEnd();
                    glDisable(GL_BLEND);
                }

            }break;

            case gs_level_select:
            {
                //show gameover screen
                if(m_show_gameover && !m_fade_off)
                {
                    glPushMatrix();
                    glTranslatef( ((float)m_screen_width-1024.0)*0.5, ((float)m_screen_height-1024.0)*0.5, 0 );
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, m_tex_goal);

                    glColor4f(m_gameover_screen_fade_level,m_gameover_screen_fade_level,m_gameover_screen_fade_level,m_gameover_screen_fade_level);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0,1.0);
                    glVertex2f(0.0,0.0);
                    glTexCoord2f(0.0,0.0);
                    glVertex2f(0.0,1024.0);
                    glTexCoord2f(1.0,0.0);
                    glVertex2f(1024.0,1024.0);
                    glTexCoord2f(1.0,1.0);
                    glVertex2f(1024.0,0.0);
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);
                    glPopMatrix();

                    break;//draw nothing more
                }

                //show info screen
                if(m_show_info)
                {
                    glPushMatrix();
                    glTranslatef( ((float)m_screen_width-1024.0)*0.5, ((float)m_screen_height-1024.0)*0.5, 0 );
                    glEnable(GL_BLEND);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, m_tex_info);

                    glColor4f(m_screen_info_fade_level,m_screen_info_fade_level,m_screen_info_fade_level,m_screen_info_fade_level);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0,1.0);
                    glVertex2f(0.0,0.0);
                    glTexCoord2f(0.0,0.0);
                    glVertex2f(0.0,1024.0);
                    glTexCoord2f(1.0,0.0);
                    glVertex2f(1024.0,1024.0);
                    glTexCoord2f(1.0,1.0);
                    glVertex2f(1024.0,0.0);
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);
                    glPopMatrix();

                    //draw static
                    float static_fade=1.0-m_screen_info_fade_level;
                    if(static_fade>0.5) static_fade=m_screen_info_fade_level;
                    else if(static_fade<0.1) static_fade=0.1;
                    draw_static(static_fade);

                    break;//draw nothing more
                }

                //draw world map
                m_Starmap.draw();

                //draw hud (only for tutorial)
                if(m_on_tutorial_level) m_hud.draw();

                //draw screen fade
                if(m_screen_fade_prog>0.0)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0,0,0,m_screen_fade_prog);
                    glBegin(GL_QUADS);
                    glVertex2d(0,0);
                    glVertex2d(m_screen_width,0);
                    glVertex2d(m_screen_width,m_screen_height);
                    glVertex2d(0,m_screen_height);
                    glEnd();
                    glDisable(GL_BLEND);
                }

            }break;

            case gs_in_level:
            {
                //show manual
                if(m_show_manual)
                {
                    glPushMatrix();
                    glTranslatef( ((float)m_screen_width-1024.0)*0.5, ((float)m_screen_height-1024.0)*0.5, 0 );
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, m_tex_manual);

                    glColor3f(1,1,1);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0,1.0);
                    glVertex2f(0.0,0.0);
                    glTexCoord2f(0.0,0.0);
                    glVertex2f(0.0,1024.0);
                    glTexCoord2f(1.0,0.0);
                    glVertex2f(1024.0,1024.0);
                    glTexCoord2f(1.0,1.0);
                    glVertex2f(1024.0,0.0);
                    glEnd();

                    glDisable(GL_TEXTURE_2D);

                    //draw skip bar
                    if(m_on_tutorial_level)
                    {
                        //draw quit bar
                        float lowest_timer=_player_mship_control_key_timer;
                        for(int player_i=0;player_i<4;player_i++)
                        {
                            if( m_vec_players[player_i].m_key_hold_time_back < _player_mship_control_key_timer )
                             lowest_timer=m_vec_players[player_i].m_key_hold_time_back;
                        }
                        if(lowest_timer<_player_mship_control_key_timer)
                        {
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                            glColor4f(0.7,0.7,0.7,0.7);
                            glBegin(GL_QUADS);
                            glVertex2f(30.0,69.0);
                            glVertex2f(30.0,91.0);
                            glVertex2f(30.0+(327.0-30.0)*(1.0-lowest_timer/_player_mship_control_key_timer),91.0);
                            glVertex2f(30.0+(327.0-30.0)*(1.0-lowest_timer/_player_mship_control_key_timer),69.0);
                            glEnd();
                            glDisable(GL_BLEND);
                        }
                    }
                    else//hide skip tutorial text
                    {
                        glColor3f(0,0,0);
                        glBegin(GL_QUADS);
                        glVertex2f(30.0,69.0);
                        glVertex2f(30.0,91.0);
                        glVertex2f(327.0,91.0);
                        glVertex2f(327.0,69.0);
                        glEnd();
                    }

                    //draw quit bar
                    float lowest_timer=_player_mship_control_key_timer;
                    for(int player_i=0;player_i<4;player_i++)
                    {
                        if( m_vec_players[player_i].m_key_hold_time_start < _player_mship_control_key_timer )
                         lowest_timer=m_vec_players[player_i].m_key_hold_time_start;
                    }
                    if(lowest_timer<_player_mship_control_key_timer)
                    {
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        glColor4f(0.7,0.7,0.7,0.7);
                        glBegin(GL_QUADS);
                        glVertex2f(709.0,69.0);
                        glVertex2f(709.0,91.0);
                        glVertex2f(709.0+(926.0-709.0)*(1.0-lowest_timer/_player_mship_control_key_timer),91.0);
                        glVertex2f(709.0+(926.0-709.0)*(1.0-lowest_timer/_player_mship_control_key_timer),69.0);
                        glEnd();
                        glDisable(GL_BLEND);
                    }

                    glPopMatrix();

                    break;//draw nothing more
                }



                //show lost screen, will start when fade off is complete
                if(m_show_lost && !m_fade_off)
                {
                    glPushMatrix();
                    glTranslatef( ((float)m_screen_width-1024.0)*0.5, ((float)m_screen_height-1024.0)*0.5, 0 );
                    glEnable(GL_BLEND);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, m_tex_lost);

                    glColor4f(m_lost_screen_fade_level,m_lost_screen_fade_level,m_lost_screen_fade_level,m_lost_screen_fade_level);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0,1.0);
                    glVertex2f(0.0,0.0);
                    glTexCoord2f(0.0,0.0);
                    glVertex2f(0.0,1024.0);
                    glTexCoord2f(1.0,0.0);
                    glVertex2f(1024.0,1024.0);
                    glTexCoord2f(1.0,1.0);
                    glVertex2f(1024.0,0.0);
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);
                    glPopMatrix();

                    break;//draw nothing more
                }

                //draw current level

                //draw stars
                glPushMatrix();
                glTranslatef(((112.5*_Met2Pix-m_screen_width*0.5)-cam_pos[0])*0.7,-cam_pos[1]*0.7,0);
                glBegin(GL_POINTS);
                for(int i=0;i<(int)m_vec_stars.size();i++)
                {
                    glColor3f(m_vec_stars[i].val_f,m_vec_stars[i].val_f,m_vec_stars[i].val_f);
                    glVertex2f( m_vec_stars[i].val_i1,m_vec_stars[i].val_i2 );
                }
                glEnd();
                glPopMatrix();

                //draw caves
                draw_cave(cam_pos);

                glPushMatrix();
                //move cam to cam_pos
                glTranslatef(-cam_pos[0],-cam_pos[1],0);

                //debug draw
                if(debug_draw)
                {
                    float curr_color[3]={1.0,1.0,1.0};
                    for(int rep=1;rep<2;rep++)//draw world again next to original, if cam is close to edge
                    {
                        switch(rep)
                        {
                            case 0://left rep
                            {
                                //test cam pos
                                if( cam_pos[0]*_Pix2Met<0.0 )//draw left copy
                                {
                                    curr_color[0]=1; curr_color[1]=0; curr_color[2]=0;
                                    glPushMatrix();
                                    glTranslatef(-m_world_max_x*_Met2Pix,0,0);
                                }
                                else continue;//skip drawing
                            }break;

                            case 1://center rep
                            {
                                curr_color[0]=1; curr_color[1]=1; curr_color[2]=1;
                                glPushMatrix();//draw always
                            }break;

                            case 2://right rep
                            {
                                //test cam pos
                                if( cam_pos[0]*_Pix2Met+(float)m_screen_width*_Pix2Met>m_world_max_x )//draw left copy
                                {
                                    curr_color[0]=0; curr_color[1]=1; curr_color[2]=0;
                                    glPushMatrix();
                                    glTranslatef(m_world_max_x*_Met2Pix,0,0);
                                }
                                else continue;//skip drawing
                            }break;
                        }
                        b2Body* tmp=m_pWorld->GetBodyList();

                        //int counter=0;//temp
                        while(tmp)
                        {
                            glPushMatrix();
                            b2Vec2 center=tmp->GetWorldCenter();
                            glTranslatef(center.x*_Met2Pix,center.y*_Met2Pix,0);

                            b2MassData massD;
                            tmp->GetMassData(&massD);
                            b2Vec2 center_of_mass=massD.center;
                            //cout<<center.x<<endl;
                            //glTranslatef((center.x-center_of_mass.x)*_Met2Pix,(center.y-center_of_mass.y)*_Met2Pix,0);

                            glRotatef(tmp->GetAngle()*180/3.14159,0,0,1);
                            glTranslatef(-center_of_mass.x*_Met2Pix,-center_of_mass.y*_Met2Pix,0);
                            for( b2Fixture* fixture=tmp->GetFixtureList(); fixture; fixture=fixture->GetNext() )
                            {
                                //test if sensor
                                if( fixture->IsSensor() )
                                {
                                    //hot pink
                                    glColor3f(1,0,1);
                                }
                                else glColor3fv(curr_color);
                                //get shape type
                                b2Shape* shape=fixture->GetShape();
                                if( shape->m_type==b2Shape::e_chain )
                                {
                                    //skip terrain
                                    break;

                                    //chain
                                    int vertex_count=((b2ChainShape*)shape)->m_count;
                                    b2Vec2 points[vertex_count];

                                    for(int i=0;i<vertex_count;i++)
                                        points[i]=((b2ChainShape*)shape)->m_vertices[i];

                                    glBegin(GL_LINE_STRIP);
                                    for(int i=0;i<vertex_count;i++)
                                        glVertex2f(points[i].x*_Met2Pix,points[i].y*_Met2Pix);

                                    glEnd();
                                }
                                else if( shape->m_type==b2Shape::e_polygon )//polygon
                                {
                                    int vertex_count=((b2PolygonShape*)fixture->GetShape())->GetVertexCount();
                                    b2Vec2 points[vertex_count];

                                    for(int i=0;i<vertex_count;i++)
                                        points[i]=((b2PolygonShape*)fixture->GetShape())->GetVertex(i);

                                    glBegin(GL_LINE_STRIP);
                                    for(int i=0;i<vertex_count;i++)
                                        glVertex2f(points[i].x*_Met2Pix,points[i].y*_Met2Pix);

                                    glVertex2f(points[0].x*_Met2Pix,points[0].y*_Met2Pix);

                                    glEnd();
                                }
                                else if( shape->m_type==b2Shape::e_circle)//circle
                                {
                                    glBegin(GL_LINE_STRIP);
                                    for (float a = 0; a < 360 * _Deg2Rad; a += 30 * _Deg2Rad)
                                     glVertex2f( sinf(a)*shape->m_radius*_Met2Pix,
                                                 cosf(a)*shape->m_radius*_Met2Pix );
                                    glVertex2f(0,0);
                                    glEnd();
                                }
                            }

                            glPopMatrix();

                            tmp=tmp->GetNext();
                        }

                        //cout<<"Printed bodies: "<<counter<<endl;

                        glPopMatrix();//pop world shift
                    }

                    //draw all joints
                    if(debug_draw_joints)
                    {
                        glLineWidth(3);
                        glColor3f(0,1,0);
                        b2Joint* joint_tmp=m_pWorld->GetJointList();
                        while(joint_tmp)
                        {
                            b2Vec2 anchor_a=joint_tmp->GetAnchorA();
                            b2Vec2 anchor_b=joint_tmp->GetAnchorB();

                            glBegin(GL_LINES);
                            glVertex2f(anchor_a.x*_Met2Pix,anchor_a.y*_Met2Pix);
                            glVertex2f(anchor_b.x*_Met2Pix,anchor_b.y*_Met2Pix);
                            glEnd();

                            joint_tmp=joint_tmp->GetNext();
                        }
                        glLineWidth(1);
                    }
                }

                //draw players
                for(int player_i=0;player_i<4;player_i++)
                {
                    //draw ship and rope
                    m_vec_players[player_i].draw();
                }

                //draw enemies
                for(int enemy_i=0;enemy_i<(int)m_vec_pEnemies.size();enemy_i++)
                {
                    m_vec_pEnemies[enemy_i]->draw();
                }

                //draw objects
                for(int object_i=0;object_i<(int)m_vec_objects.size();object_i++)
                {
                    m_vec_objects[object_i].draw();
                }

                //draw main ship
                m_pMain_ship->draw();

                //draw projectiles
                draw_projectiles();

                //draw particles
                m_pParticle_engine->draw();

                //draw terrain and pop matrix
                draw_terrain(cam_pos);
                //matrix now popped to standard view

                //draw HUD
                if(multi_draw==-1)//do not draw for player windows
                 m_hud.draw();

                //draw stuck message
                if(m_on_tutorial_level && m_draw_stuck_timer>0.0)
                {
                    //draw message on screen
                    glPushMatrix();
                    glTranslatef(m_screen_width*0.5-206.0,m_screen_height*0.85,0);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D,m_tex_menu);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE,GL_ONE);
                    glColor4f(m_draw_stuck_timer,m_draw_stuck_timer,m_draw_stuck_timer,m_draw_stuck_timer);
                    glBegin(GL_QUADS);
                    glTexCoord2f(1.0,(1024.0-120.0)/1024.0);
                    glVertex2f(412.0,0.0);
                    glTexCoord2f(1.0,(1024.0-150.0)/1024.0);
                    glVertex2f(412.0,30.0);
                    glTexCoord2f(612.0/1024.0,(1024.0-150.0)/1024.0);
                    glVertex2f(0.0,30.0);
                    glTexCoord2f(612.0/1024.0,(1024.0-120.0)/1024.0);
                    glVertex2f(0.0,0.0);
                    glEnd();
                    glDisable(GL_TEXTURE_2D);
                    glPopMatrix();
                }

                //draw static
                draw_static();

                //draw screen fade
                if(m_screen_fade_prog>0.0)
                {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0,0,0,m_screen_fade_prog);
                    glBegin(GL_QUADS);
                    glVertex2d(0,0);
                    glVertex2d(m_screen_width,0);
                    glVertex2d(m_screen_width,m_screen_height);
                    glVertex2d(0,m_screen_height);
                    glEnd();
                    glDisable(GL_BLEND);
                }

                //draw convoy info
                if(m_waiting_for_convoy)
                {
                    //screen fade
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glColor4f(0,0,0,m_waiting_for_convoy_screen_fade_level);
                    glBegin(GL_QUADS);
                    glVertex2d(0,0);
                    glVertex2d(m_screen_width,0);
                    glVertex2d(m_screen_width,m_screen_height);
                    glVertex2d(0,m_screen_height);
                    glEnd();
                    glDisable(GL_BLEND);

                    //draw text
                    if(m_waiting_for_convoy_text_fade_level>0.0)
                    {
                        glPushMatrix();
                        glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, m_tex_menu);
                        glColor4f(m_waiting_for_convoy_text_fade_level,m_waiting_for_convoy_text_fade_level,
                                  m_waiting_for_convoy_text_fade_level,m_waiting_for_convoy_text_fade_level);
                        glTranslatef( ((float)m_screen_width-444.0)*0.5, ((float)m_screen_height-30.0)*0.5, 0.0  );
                        glBegin(GL_QUADS);
                        glTexCoord2f(0.0,(1024.0-824.0)/1024.0);
                        glVertex2f(0.0,0.0);
                        glTexCoord2f(0.0,(1024.0-854.0)/1024.0);
                        glVertex2f(0.0,30.0);
                        glTexCoord2f(444.0/1024.0,(1024.0-854.0)/1024.0);
                        glVertex2f(444.0,30.0);
                        glTexCoord2f(444.0/1024.0,(1024.0-824.0)/1024.0);
                        glVertex2f(444.0,0.0);
                        glEnd();
                        glDisable(GL_TEXTURE_2D);
                        glPopMatrix();
                    }
                }

            }break;

            case gs_game_over:
            {
                //draw game over screen

                //TEMP
                glColor3f(0.5,0.5,0.5);
                glBegin(GL_QUADS);
                glVertex2d(0,0);
                glVertex2d(m_screen_width,0);
                glVertex2d(m_screen_width,m_screen_height);
                glVertex2d(0,m_screen_height);
                glEnd();
            }break;
        }

        if(multi_draw!=-1)
        {
            //restore viewport
            glViewport(0,0,m_screen_width,m_screen_height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, m_screen_width, m_screen_height, 0,-1,1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            //draw vinjett
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_mask);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(1,1,1,1);
            glBegin(GL_QUADS);
            glTexCoord2f(1.0,0.0);
            glVertex2f(window_pos_x+window_size,window_pos_y);
            glTexCoord2f(1.0,1.0);
            glVertex2f(window_pos_x+window_size,window_pos_y+window_size);
            glTexCoord2f(0.0,1.0);
            glVertex2f(window_pos_x,window_pos_y+window_size);
            glTexCoord2f(0.0,0.0);
            glVertex2f(window_pos_x,window_pos_y);
            glEnd();
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_BLEND);

            //draw window border
            b2Vec2 player_pos=m_vec_players[multi_draw].get_body_ptr()->GetPosition();
            b2Vec2 screen_center=b2Vec2((m_cam_pos[0]+m_screen_width*0.5)*_Pix2Met, (m_cam_pos[1]+m_screen_height*0.5)*_Pix2Met);
            b2Vec2 rel_pos(player_pos);
            rel_pos-=screen_center;
            float border_color=rel_pos.Length()/50.0;
            if(border_color<0.3) border_color=0.3;
            if(border_color>1.0) border_color=1.0;
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE,GL_ONE);
            glLineWidth(2);
            glColor4f(border_color,border_color,border_color,0.9);
            glBegin(GL_LINE_STRIP);
            glVertex2f(window_pos_x+window_size,window_pos_y);
            glVertex2f(window_pos_x+window_size,window_pos_y+window_size);
            glVertex2f(window_pos_x,window_pos_y+window_size);
            glVertex2f(window_pos_x,window_pos_y);
            glVertex2f(window_pos_x+window_size,window_pos_y);
            glEnd();
            glLineWidth(1);
            glDisable(GL_BLEND);
        }
    }

    //cout<<"draw done\n";

    return true;
}


//Private

bool game::load_textures(void)
{
    cout<<"Loading Textures\n";

    //from text
    string s_decode=base64_decode( load_base64_file(file_texture_decal) );
    unsigned char* texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_decal = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_goal) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_goal = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_info) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_info = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_lost) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_lost = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_manual) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_manual = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_text) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_text = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_texture) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_menu = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_terrain0) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_terrains[0] = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_terrain1) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_terrains[1] = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_terrain2) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_terrains[2] = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_terrain3) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_terrains[3] = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_terrain4) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_terrains[4] = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_moretext) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_moretext = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );

    s_decode=base64_decode( load_base64_file(file_texture_mask) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_tex_mask = SOIL_load_OGL_texture_from_memory
    (
        texture_data,//buffer
        (int)s_decode.length(),//Buffer length
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT | SOIL_FLAG_MIPMAPS
    );


    /*//from files
    m_tex_decal=SOIL_load_OGL_texture
	(
		"texture\\decal.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT //SOIL_FLAG_MIPMAPS (mer suddig)
	);

	m_tex_menu=SOIL_load_OGL_texture
	(
		"texture\\texture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_manual=SOIL_load_OGL_texture
	(
		"texture\\manual.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_info=SOIL_load_OGL_texture
	(
		"texture\\info.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_lost=SOIL_load_OGL_texture
	(
		"texture\\lost.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_goal=SOIL_load_OGL_texture
	(
		"texture\\goal.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_text=SOIL_load_OGL_texture
	(
		"texture\\text.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_moretext=SOIL_load_OGL_texture
	(
		"texture\\moretext.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_mask=SOIL_load_OGL_texture
	(
		"texture\\mask.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

    //terrain textures
    for(int i=0;i<_starmap_numof_level_textures;i++)
    {
        //int to string
        string file_name("texture\\terrain");
        char buff[10];
        itoa(i,buff,10);
        string str=string(buff);
        file_name.append(str);
        file_name.append(".png");

        m_tex_terrains[i]=SOIL_load_OGL_texture
        (
            file_name.c_str(),
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
        );

        //set first to bad if any of the textures are bad
        if(m_tex_terrains[i]==0) m_tex_terrains[0]=0;
    }*/


	if(m_tex_decal==0 || m_tex_menu==0 || m_tex_manual==0 || m_tex_info==0 || m_tex_lost==0 ||
       m_tex_goal==0 || m_tex_text==0 || m_tex_moretext==0 || m_tex_mask==0 ||
       m_tex_terrains[0]==0 || m_tex_terrains[1]==0 || m_tex_terrains[2]==0 || m_tex_terrains[3]==0 || m_tex_terrains[4]==0)
	{
	    return false;
	}

    return true;
}

bool game::load_sounds(void)
{
    m_pSound=new sound();

    bool error_flag=false;

    //set window size
    int screen_size[2]={m_screen_width,m_screen_height};
    m_pSound->set_screen_test(screen_size,m_cam_pos);

    if( !m_pSound->load_WAVE_from_file( wav_hook_break,"sound\\hook_break.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_starmap_land,"sound\\starmap_land.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_starmap_travel,"sound\\starmap_travel.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_starmap_select,"sound\\starmap_select.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_starmap_startup,"sound\\starmap_startup.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_starmap_startdown,"sound\\starmap_startdown.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_hook_connect,"sound\\hook_connect.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_hook_disconnect,"sound\\hook_disconnect.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_connect,"sound\\mship_connect.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_disconnect,"sound\\mship_disconnect.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_input_fuel,"sound\\mship_input_fuel.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_input_ship,"sound\\mship_input_ship.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_drone_eject,"sound\\drone_eject.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_drone_join_ship,"sound\\drone_join_ship.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_drone_crash,"sound\\drone_crash.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_pea,"sound\\weapon_pea.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_spread,"sound\\weapon_spread.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_rocket,"sound\\weapon_rocket.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_grenade,"sound\\weapon_grenade.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_mine,"sound\\weapon_mine.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_cannon,"sound\\weapon_cannon.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_weapon_laser,"sound\\weapon_laser.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_ship_explosion,"sound\\ship_explosion.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_ship_detected,"sound\\enemy_ship_detected.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_enemy_ship_lost,"sound\\enemy_ship_lost.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_ship_col,"sound\\ship_col.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_bullet_hit,"sound\\bullet_hit.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_ship_upgrade,"sound\\ship_upgrade.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_bullet_explosion,"sound\\bullet_explosion.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_gear_enable,"sound\\gear_enable.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_gear_disable,"sound\\gear_disable.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_ship_raising,"sound\\player_ship_raising.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_motor,"sound\\player_motor.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_drone_motor,"sound\\drone_motor.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_motor_boost,"sound\\player_motor_boost.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_player_rope_motor,"sound\\player_rope_motor.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_turret_rotation,"sound\\turret_rotation.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_motor,"sound\\mship_motor.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_gear_motor,"sound\\mship_gear_motor.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_mship_recycle,"sound\\mship_recycle.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_alarm,"sound\\alarm.wav" ) ) error_flag=true;

    //if( !m_pSound->load_WAVE_from_file( wav_,"sound\\.wav" ) ) error_flag=true;

    //load ogg files
    if( !m_pSound->load_OGG_from_file( ogg_starmap_noise,"sound\\starmap_noise.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music0_intro,"sound\\music0_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music0_loop, "sound\\music0_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music1_intro,"sound\\music1_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music1_loop, "sound\\music1_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music2_intro,"sound\\music2_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music2_loop, "sound\\music2_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music3_intro,"sound\\music3_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music3_loop, "sound\\music3_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music4_intro,"sound\\music4_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music4_loop, "sound\\music4_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music5_intro,"sound\\music5_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music5_loop, "sound\\music5_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music6_intro,"sound\\music6_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music6_loop, "sound\\music6_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music7_intro,"sound\\music7_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music7_loop, "sound\\music7_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music8_intro,"sound\\music8_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music8_loop, "sound\\music8_loop.ogg" ) )  error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music9_intro,"sound\\music9_intro.ogg" ) ) error_flag=true;
    if( !m_pSound->load_OGG_from_file( ogg_music9_loop, "sound\\music9_loop.ogg" ) )  error_flag=true;

    if(error_flag)
    {
        cout<<"ERROR: Problem loading sound files\n";
        return false;
    }

    return true;
}

bool game::set_sound_loops(void)
{
    //load sound channels with sound
    m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p1);
    m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p2);
    m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p3);
    m_pSound->playSimpleSound(wav_player_motor,0.0,_sound_chan_motor_p4);
    m_pSound->playSimpleSound(wav_weapon_laser,0.0,_sound_chan_laser_p1);
    m_pSound->playSimpleSound(wav_weapon_laser,0.0,_sound_chan_laser_p2);
    m_pSound->playSimpleSound(wav_weapon_laser,0.0,_sound_chan_laser_p3);
    m_pSound->playSimpleSound(wav_weapon_laser,0.0,_sound_chan_laser_p4);
    m_pSound->playSimpleSound(wav_weapon_laser,0.0,_sound_chan_laser_enemy);
    m_pSound->playSimpleSound(wav_player_motor_boost,0.0,_sound_chan_boost_p1);
    m_pSound->playSimpleSound(wav_player_motor_boost,0.0,_sound_chan_boost_p2);
    m_pSound->playSimpleSound(wav_player_motor_boost,0.0,_sound_chan_boost_p3);
    m_pSound->playSimpleSound(wav_player_motor_boost,0.0,_sound_chan_boost_p4);
    m_pSound->playSimpleSound(wav_player_rope_motor,0.0,_sound_chan_motor_rope);
    m_pSound->playSimpleSound(wav_turret_rotation,0.0,_sound_chan_motor_turret);
    m_pSound->playSimpleSound(wav_mship_motor,0.0,_sound_chan_motor_mship);
    m_pSound->playSimpleSound(wav_mship_gear_motor,0.0,_sound_chan_gear_motor_mship);
    m_pSound->playSimpleSound(wav_alarm,0.0,_sound_chan_alarm);
    m_pSound->playSimpleSound(wav_player_motor_boost,0.0,_sound_chan_fuel_transfer,true);

    return true;
}

bool game::init_box2d(void)
{
cout<<"Initializing Box2D\n";
    m_world_gravity_curr=_world_gravity;//set gravity
    m_pWorld=new b2World( b2Vec2(0.0,m_world_gravity_curr) );

    //load level
    vector<st_float_float_int> vec_enemy_pos;
    vector<st_float_float_int> vec_object_pos;
    if( !load_level_data(-1,vec_enemy_pos,vec_object_pos) )
    {
        cout<<"ERROR: Problem loading level\n";
        return false;
    }

    //static cam
    m_cam_pos[0]=112.5*_Met2Pix-m_screen_width*0.5;
    m_cam_pos[1]=8.0*_Met2Pix;

    //add menu text boxes
    if(!m_run_test_level)
    {
        //box 1
        {
        b2Vec2 box_pos(112.5,81);
        float box_width=14.7;
        float box_height=2.95;
        //create object
        b2BodyDef bodydef;
        bodydef.position=box_pos;
        bodydef.type=b2_dynamicBody;
        bodydef.linearDamping=_object_damping_lin;
        bodydef.angularDamping=_object_damping_ang;
        b2Body* body_box=m_pWorld->CreateBody(&bodydef);
        //create fixture
        b2PolygonShape shape2;
        b2Vec2 edge_points[]={b2Vec2(-box_width,-box_height),
                              b2Vec2(box_width,-box_height),
                              b2Vec2(box_width,box_height),
                              b2Vec2(-box_width,box_height)};
        shape2.Set(edge_points,4);
        b2FixtureDef fixturedef;
        fixturedef.shape=&shape2;
        fixturedef.density=_object_density;
        body_box->CreateFixture(&fixturedef);
        //set data
        st_body_user_data* user_data=new st_body_user_data;
        user_data->s_info="menutext";
        user_data->i_id=1;
        body_box->SetUserData(user_data);
        }
        //box 2
        {
        b2Vec2 box_pos(112.5,74);
        float box_width=10.3;
        float box_height=2.95;
        //create object
        b2BodyDef bodydef;
        bodydef.position=box_pos;
        bodydef.type=b2_dynamicBody;
        bodydef.linearDamping=_object_damping_lin;
        bodydef.angularDamping=_object_damping_ang;
        b2Body* body_box=m_pWorld->CreateBody(&bodydef);
        //create fixture
        b2PolygonShape shape2;
        b2Vec2 edge_points[]={b2Vec2(-box_width,-box_height),
                              b2Vec2(box_width,-box_height),
                              b2Vec2(box_width,box_height),
                              b2Vec2(-box_width,box_height)};
        shape2.Set(edge_points,4);
        b2FixtureDef fixturedef;
        fixturedef.shape=&shape2;
        fixturedef.density=_object_density;
        body_box->CreateFixture(&fixturedef);
        //set data
        st_body_user_data* user_data=new st_body_user_data;
        user_data->s_info="menutext";
        user_data->i_id=2;
        body_box->SetUserData(user_data);
        }
        //box 3
        {
        b2Vec2 box_pos(113,67);
        float box_width=9.35;
        float box_height=3;
        //create object
        b2BodyDef bodydef;
        bodydef.position=box_pos;
        bodydef.type=b2_dynamicBody;
        bodydef.linearDamping=_object_damping_lin;
        bodydef.angularDamping=_object_damping_ang;
        b2Body* body_box=m_pWorld->CreateBody(&bodydef);
        //create fixture
        b2PolygonShape shape2;
        b2Vec2 edge_points[]={b2Vec2(-box_width,-box_height),
                              b2Vec2(box_width,-box_height),
                              b2Vec2(box_width,box_height),
                              b2Vec2(-box_width,box_height)};
        shape2.Set(edge_points,4);
        b2FixtureDef fixturedef;
        fixturedef.shape=&shape2;
        fixturedef.density=_object_density;
        body_box->CreateFixture(&fixturedef);
        //set data
        st_body_user_data* user_data=new st_body_user_data;
        user_data->s_info="menutext";
        user_data->i_id=3;
        body_box->SetUserData(user_data);
    }
    }

//cout<<"Adding main ship\n";
    //temp add main ship
    m_pMain_ship=new main_ship();
    m_pMain_ship->init(m_pWorld,m_pParticle_engine,m_world_gravity_curr,m_tex_decal,
                       &m_mship_led_prog,false,b2Vec2(m_player_start_pos[0],m_player_start_pos[1]));
//cout<<"Adding players\n";
    //temp add players
    int player_ind=0;
    m_player_active[player_ind]=false;
    //player new_player1=new player();
    m_vec_players.push_back( player() );
    m_vec_players.back().init( m_pWorld,m_pMain_ship->get_body_ptr(),m_pParticle_engine,m_pSound,m_tex_decal,player_ind,false,b2Vec2(m_player_start_pos[0],m_player_start_pos[1]) );
    //add another player
    player_ind=1;
    m_player_active[player_ind]=false;
    //player new_player2=new player();
    m_vec_players.push_back( player() );
    m_vec_players.back().init( m_pWorld,m_pMain_ship->get_body_ptr(),m_pParticle_engine,m_pSound,m_tex_decal,player_ind,false,b2Vec2(m_player_start_pos[0],m_player_start_pos[1]) );
    //add another player
    player_ind=2;
    m_player_active[player_ind]=false;
    //player new_player3=new player();
    m_vec_players.push_back( player() );
    m_vec_players.back().init( m_pWorld,m_pMain_ship->get_body_ptr(),m_pParticle_engine,m_pSound,m_tex_decal,player_ind,false,b2Vec2(m_player_start_pos[0],m_player_start_pos[1]) );
    //add another player
    player_ind=3;
    m_player_active[player_ind]=false;
    //player new_player4=new player();
    m_vec_players.push_back( player() );
    m_vec_players.back().init( m_pWorld,m_pMain_ship->get_body_ptr(),m_pParticle_engine,m_pSound,m_tex_decal,player_ind,false,b2Vec2(m_player_start_pos[0],m_player_start_pos[1]) );
//cout<<"Adding objects\n";
//add objects
    {
    b2Vec2 object_pos(90,70);
    float fuel_size=float(rand()%100)/166.0+0.2;
    //get fuel content, 0-100
    float fuel_content=float(rand()%20)+40.0;

    m_vec_objects.push_back( object() );
    m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                              fuel_size,//size
                              object_pos,
                              fuel_content);//fuel content
    }
    {
    b2Vec2 object_pos(140,70);
    float fuel_size=float(rand()%100)/166.0+0.2;
    //get fuel content, 0-100
    float fuel_content=float(rand()%20)+40.0;

    m_vec_objects.push_back( object() );
    m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                              fuel_size,//size
                              object_pos,
                              fuel_content);//fuel content
    }

//cout<<"Adding enemies\n";
    //add enemies for test level, otherwise empty pos vector (have to be added after players)
    if(m_run_test_level)
    {
        //place objects
        cout<<"Load Test Level: Placing Objects\n";
        //if test level, spawn all
        int numof_objects=100;//spawn max

        cout<<"numof objects to place: "<<numof_objects<<endl;
        for(int object_i=0;object_i<numof_objects;object_i++)
        {
            //test if spaces left
            if(vec_object_pos.empty()) break;

            //get pos
            int pos_index=rand()%(int)vec_object_pos.size();
            //get size, 0.2-0.8
            float fuel_size=float(rand()%100)/166.0+0.2;
            //get fuel content, 0-100
            float fuel_content=float(rand()%80)+20;

            m_vec_objects.push_back( object() );
            m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                      fuel_size,//size
                                      b2Vec2(vec_object_pos[pos_index].val_f1,vec_object_pos[pos_index].val_f2),
                                      fuel_content);//fuel content
            //remove pos, not to be reused
            vec_object_pos.erase(vec_object_pos.begin()+pos_index);

            //break;//TEMP
        }
        cout<<" Placed: "<<(int)m_vec_objects.size()<<endl;

        //place enemies
        cout<<"Load Test Level: Placing Enemies\n";
        //if test level, spawn all
        int numof_enemies=100;//spawn max

        cout<<"numof enemies to place: "<<numof_enemies<<endl;
        for(int enemy_i=0;enemy_i<numof_enemies;enemy_i++)
        {
            //test if spaces left
            if(vec_enemy_pos.empty()) break;

            //get type to place
            int enemy_type=rand()%15+2;//15 types, offset of 2 (2-16)
            //cout<<"Enemy type: "<<enemy_type<<endl;
            //get pos
            int pos_index=-1;
            if(enemy_type==et_burst_bot || enemy_type==et_flipper)
            {
                //find surface pos, type 2
                vector<int> vec_alternatives;
                for(int pos_i=0;pos_i<(int)vec_enemy_pos.size();pos_i++)
                {
                    if(vec_enemy_pos[pos_i].val_i==2)
                    {
                        vec_alternatives.push_back(pos_i);
                    }
                }
                if(!vec_alternatives.empty())
                {
                    int rand_val=rand()%int(vec_alternatives.size());
                    pos_index=vec_alternatives[rand_val];
                }
            }
            else//any place
            {
                pos_index=rand()%(int)vec_enemy_pos.size();
            }

            if(pos_index==-1)
            {
                cout<<"Enemy spawn: Could not place that enemy type\n";
                continue;
            }

            //place enemy
            //cout<<"place enemy...";
            m_vec_pEnemies.push_back( new enemy() );
            m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                        b2Vec2(vec_enemy_pos[pos_index].val_f1,vec_enemy_pos[pos_index].val_f2),
                                        enemy_type,m_tex_decal);
            //remove pos, not to be reused
            vec_enemy_pos.erase(vec_enemy_pos.begin()+pos_index);
            //cout<<"done\n";
        }
        cout<<" Placed: "<<(int)m_vec_pEnemies.size()<<endl;
    }

//cout<<"Init contact listener\n";
    //init contact listener
    m_ppBody_to_connect[0]=m_pBody_to_connect_p1;
    m_ppBody_to_connect[1]=m_pBody_to_connect_p2;
    m_ppBody_to_connect[2]=m_pBody_to_connect_p3;
    m_ppBody_to_connect[3]=m_pBody_to_connect_p4;
    m_ppPlayer_bodies[0]=m_vec_players[0].get_body_ptr();
    m_ppPlayer_bodies[1]=m_vec_players[1].get_body_ptr();
    m_ppPlayer_bodies[2]=m_vec_players[2].get_body_ptr();
    m_ppPlayer_bodies[3]=m_vec_players[3].get_body_ptr();
    m_ppRope_hook_sensors[0]=m_vec_players[0].get_rope_hook_sensor();
    m_ppRope_hook_sensors[1]=m_vec_players[1].get_rope_hook_sensor();
    m_ppRope_hook_sensors[2]=m_vec_players[2].get_rope_hook_sensor();
    m_ppRope_hook_sensors[3]=m_vec_players[3].get_rope_hook_sensor();
    //m_pEvent_flag_input_box=new bool();
    *m_pEvent_flag_input_box=false;
    //m_pEvent_flag_landing_gear=new bool[2];
    m_pEvent_flag_landing_gear[0]=m_pEvent_flag_landing_gear[1]=false;
    m_ppBody_in_mship_input[0]=m_pBody_to_connect_p1;

    m_pWorld->SetContactListener(&m_myContactListenerInstance);
    m_myContactListenerInstance.init(m_pWorld,
                                     m_ppRope_hook_sensors,
                                     m_pMain_ship->get_sensor_landing_ptr(),
                                     m_pMain_ship->get_sensor_input_ptr(),
                                     m_ppPlayer_bodies,
                                     m_pEvent_flag_hook,
                                     m_pEvent_flag_input_box,
                                     m_pMship_landing_sensor_flag,
                                     m_ppBody_in_mship_input,
                                     m_ppBody_to_connect,
                                     m_pMain_ship->get_sensor_landing_gear_left_ptr(),
                                     m_pMain_ship->get_sensor_landing_gear_right_ptr(),
                                     m_pEvent_flag_landing_gear,
                                     &m_vec_projectiles_to_remove,
                                     &m_vec_collision_events);

    m_pMain_ship->set_landing_gear_sensor_flags(m_pEvent_flag_landing_gear);


cout<<"Box2D Initialization Complete\n";

    return true;
}

bool game::load_level_data(int level_ind,
                           vector<st_float_float_int>& vec_enemy_pos,
                           vector<st_float_float_int>& vec_object_pos)
{
    cout<<"Loading Level Data\n";

    /*//open text level
    string file_name("levels\\level");
    if(m_run_test_level)
    {
        cout<<"Loading Test Level\n";
        file_name.append("_test.txt");
    }
    else if(level_ind==-1)
    {
        cout<<"Loading Menu Level\n";
        file_name.append("menu.txt");
    }
    else if(m_on_tutorial_level || level_ind==-2)
    {
        cout<<"Loading Tutorial Level\n";
        file_name.append("tutorial.txt");
    }
    else
    {
        char buff[10];
        itoa(level_ind,buff,10);
        string str=string(buff);
        file_name.append(str);
        file_name.append(".txt");
    }
    //open and read level file
    ifstream ifs_input_file(file_name.c_str());
    if(ifs_input_file==0)
    {
        cout<<"ERROR: Could not load level from "<<file_name<<endl;
        return false;
    }*/


    /*if(m_run_test_level)
    {
        cout<<"Loading Test Level\n";
        string file_name("levels\\level");

        file_name.append("_test.txt");

        ifstream ifs_input_file(file_name.c_str());
        if(ifs_input_file==0)
        {
            cout<<"ERROR: Could not load level from "<<file_name<<endl;
            return false;
        }
    }*/

    //open hardcode level
    int level_to_load=level_ind;
    if(m_on_tutorial_level && level_ind!=-1) level_to_load=-2;
    istringstream ifs_input_file( get_level_data(level_to_load) );

    vector< vector<b2Vec2> > vec_vec_edge_points;
    vec_vec_edge_points.push_back( vector<b2Vec2>() );
    vector< vector<b2Vec2> > vec_vec_cave_points;
    int curr_chain_ind=0;
    float scale_factor[2]={1.0,1.0};
    float connection_cut_off=0;//OFF
    float edge_extra=0;//OFF
    float max_x_coord=0;
    float min_x_coord=0;
    float max_y_coord=0;
    float min_y_coord=0;//negative y is higher
    bool first_point=true;
    float wrap_space=1;//at least 1 pixel between first and last coord, in next repeat
    string line;
    bool start_to_read=false;//read only values after BEGIN
    bool cave_last_row=false;//on if CAVE was found
    bool cave_read=false;//on if CAVE was found
    int curr_cave_ind=-1;
    b2Vec2 cave_start_pos(0,0);
    while( getline(ifs_input_file,line) )
    {
        //find "BEGIN"
        if(!start_to_read)
        {
            if(line=="BEGIN")
             start_to_read=true;
            continue;
        }
        //else, read data

        //read coordinates for edge, (x y)
        //get first word
        stringstream ss(line);
        string word;
        ss>>word;
        if(word=="v")//level coord
        {
            ss>>word;
            float x_val=atof(word.c_str());
            ss>>word;
            float y_val=atof(word.c_str());

            if(first_point)//get start values for extremes
            {
                first_point=false;
                max_x_coord=x_val;
                min_x_coord=x_val;
                max_y_coord=y_val;
                min_y_coord=y_val;
            }

            if(x_val>max_x_coord) max_x_coord=x_val;
            if(x_val<min_x_coord) min_x_coord=x_val;
            if(y_val>max_y_coord) max_y_coord=y_val;
            if(y_val<min_y_coord) min_y_coord=y_val;

            vec_vec_edge_points[curr_chain_ind].push_back( b2Vec2(x_val*_Pix2Met,y_val*_Pix2Met) );

            //start/end of a CAVE test
            if(cave_last_row)
            {
                if(cave_read)
                {
                    //cout<<"CAVE: Start of new cave found\n";
                    //start of a new cave
                    vec_vec_cave_points.push_back( vector<b2Vec2>() );
                    curr_cave_ind++;
                    //add first point
                    vec_vec_cave_points[curr_cave_ind].push_back( b2Vec2(x_val*_Pix2Met,y_val*_Pix2Met) );
                    //remember start point
                    cave_start_pos=b2Vec2(x_val*_Pix2Met,y_val*_Pix2Met);
                }
                else
                {
                    //end of the current cave
                    //cout<<"CAVE: End of current cave found\n";
                    //add final point
                    vec_vec_cave_points[curr_cave_ind].push_back( b2Vec2(x_val*_Pix2Met,y_val*_Pix2Met) );

                    //add extra points for cave opening
                    b2Vec2 cave_end_pos(x_val*_Pix2Met,y_val*_Pix2Met);
                    b2Vec2 cave_direction( cave_end_pos.x-cave_start_pos.x, cave_end_pos.y-cave_start_pos.y );
                    float cave_opening_length=cave_direction.Length();
                    //cout<<"CAVE: length of opening: "<<cave_opening_length*_Met2Pix<<endl;
                    //ccw rotation: tangent X is negative Y, and tangent Y is X.
                    b2Vec2 cave_hole_direction( -cave_direction.y, cave_direction.x );
                    cave_hole_direction.Normalize();
                    float cave_opening_depth=30.0;
                    cave_hole_direction*=cave_opening_depth*_Pix2Met;
                    float noise_level=0.1;
                    //make new points
                    int numof_points=int(cave_opening_length*_Met2Pix/10.0);
                    //cout<<"CAVE: points to create: "<<numof_points<<endl;

                    if(numof_points<1) numof_points=1;
                    for(int point_i=0;point_i<numof_points;point_i++)
                    {
                        float progress=1.0-((float)point_i+0.5)/(float)numof_points;
                        float noise_to_add=(float(rand()%1000)/500.0-1.0)*noise_level;
                        float new_x=cave_start_pos.x+progress*cave_direction.x
                                   +cave_hole_direction.x*sinf(progress*_pi)
                                   +cave_hole_direction.x*noise_to_add;
                        float new_y=cave_start_pos.y+progress*cave_direction.y
                                   +cave_hole_direction.y*sinf(progress*_pi)
                                   +cave_hole_direction.y*noise_to_add;

                        vec_vec_cave_points[curr_cave_ind].push_back( b2Vec2(new_x,new_y) );
                    }

                    //add first point to close shape
                    vec_vec_cave_points[curr_cave_ind].push_back( cave_start_pos );
                }
                cave_last_row=false;
            }
            //read cave point as normal
            else if(cave_read)
            {
                vec_vec_cave_points[curr_cave_ind].push_back( b2Vec2(x_val*_Pix2Met,y_val*_Pix2Met) );
            }


            continue;
        }
        if(word=="scale_factor")
        {
            ss>>word;
            float x_scale=atof(word.c_str());
            ss>>word;
            float y_scale=atof(word.c_str());

            scale_factor[0]=x_scale;
            scale_factor[1]=y_scale;
            continue;
        }
        if(word=="player_pos")//pixel coord of start pos for player
        {
            ss>>word;
            float x_val=atof(word.c_str());
            ss>>word;
            float y_val=atof(word.c_str());

            m_player_start_pos[0]=x_val*_Pix2Met*scale_factor[0];
            m_player_start_pos[1]=y_val*_Pix2Met*scale_factor[1];
            cout<<"Level Load: Player start pos: "<<m_player_start_pos[0]<<", "<<m_player_start_pos[1]<<endl;
            continue;
        }
        if(word=="connection_cut_off")
        {
            ss>>word;
            float value=atof(word.c_str());
            connection_cut_off=value*scale_factor[0];
            continue;
        }
        if(word=="edge_extra")
        {
            ss>>word;
            float value=atof(word.c_str());
            if(value>0) edge_extra=value*scale_factor[0];
            continue;
        }
        if(word=="wrap_space")
        {
            ss>>word;
            float value=atof(word.c_str());
            if(value>0) wrap_space=value*scale_factor[0];
            continue;
        }
        if(word=="edge_border_position")
        {
            ss>>word;
            float value_1=atof(word.c_str());
            ss>>word;
            float value_2=atof(word.c_str());
            m_level_soft_borders[0]=value_1*scale_factor[0];
            m_level_soft_borders[1]=value_2*scale_factor[0];
            continue;
        }
        if(word=="soft_border_length")
        {
            ss>>word;
            float value=atof(word.c_str());
            if(value>0) m_level_static_fade_distance=value*scale_factor[0];
            continue;
        }
        if(word=="hard_border_position")
        {
            ss>>word;
            float value_1=atof(word.c_str());
            ss>>word;
            float value_2=atof(word.c_str());
            m_level_hard_borders[0]=value_1*scale_factor[0];
            m_level_hard_borders[1]=value_2*scale_factor[0];
            continue;
        }
        if(word=="sky_height")
        {
            ss>>word;
            float value=atof(word.c_str());
            if(value>0) m_level_sky_height=value*scale_factor[1];
            continue;
        }
        if(word=="enemy_pos")
        {
            //get pos and type of enemy
            ss>>word;
            float x_val=atof(word.c_str());
            ss>>word;
            float y_val=atof(word.c_str());
            ss>>word;//type
            float type=atof(word.c_str());

            vec_enemy_pos.push_back( st_float_float_int( x_val*scale_factor[0]*_Pix2Met,
                                                         y_val*scale_factor[1]*_Pix2Met,type ) );
            continue;
        }
        if(word=="object_pos")
        {
            //get pos and type of enemy
            ss>>word;
            float x_val=atof(word.c_str());
            ss>>word;
            float y_val=atof(word.c_str());
            ss>>word;//type
            float type=atof(word.c_str());

            vec_object_pos.push_back( st_float_float_int( x_val*scale_factor[0]*_Pix2Met,
                                                          y_val*scale_factor[1]*_Pix2Met,type ) );
            continue;
        }
        if(word=="BREAK")
        {
            //go to next chain
            curr_chain_ind++;
            vec_vec_edge_points.push_back( vector<b2Vec2>() );
            continue;
        }
        if(word=="CAVE")
        {
            //raise or lower CAVE flag
            cave_last_row=true;
            cave_read=!cave_read;
            continue;
        }
    }
    //ifs_input_file.close();

    if(!start_to_read)
    {
        cout<<"ERROR: Did not find BEGIN in level.txt\n";
        return false;
    }
    if( vec_vec_edge_points[0].empty() )
    {
        cout<<"ERROR: Could not read coordinates from level.txt\n";
        return false;
    }

    //rescale extremes
    max_x_coord*=scale_factor[0];
    min_x_coord*=scale_factor[0];
    max_y_coord*=scale_factor[1];
    min_y_coord*=scale_factor[1];
    //store sky limit
    m_level_sky_height=min_y_coord-m_level_sky_height;//highest pos in pix
    m_level_bottom_height=max_y_coord;
    //remember max x pos in world
    m_world_max_x=(max_x_coord+wrap_space)*_Pix2Met;
    //set world frame shift limit
    m_world_offside_limit=edge_extra*0.3*_Pix2Met;

    //add extra vertices on sides from the other side
    //BUT only for bottom layer (chain 0)
    /*OFF, looped world not in use
    if(edge_extra>0)
    {
        vector<b2Vec2> vec_edge_right;
        vector<b2Vec2> vec_edge_left;
        //find points near edge
        for(int point_i=0;point_i<(int)vec_vec_edge_points[0].size();point_i++)
        {
            if(vec_vec_edge_points[0][point_i].x<edge_extra*_Pix2Met)
            {
                //close to left edge, add on right side, at end of vector
                vec_edge_left.push_back( b2Vec2( vec_vec_edge_points[0][point_i].x+(max_x_coord+wrap_space)*_Pix2Met,
                                                 vec_vec_edge_points[0][point_i].y ) );
            }
            if(vec_vec_edge_points[0][point_i].x>(max_x_coord-edge_extra)*_Pix2Met)
            {
                //close to right edge, add on left side
                vec_edge_right.push_back( b2Vec2( vec_vec_edge_points[0][point_i].x-(max_x_coord+wrap_space)*_Pix2Met,
                                                  vec_vec_edge_points[0][point_i].y ) );
            }
        }
        //add edges to main chain
        vector<b2Vec2> vec_chain_full;
        vec_chain_full.insert( vec_chain_full.end(),vec_edge_right.begin(),vec_edge_right.end() );
        vec_chain_full.insert( vec_chain_full.end(),vec_vec_edge_points[0].begin(),vec_vec_edge_points[0].end() );
        vec_chain_full.insert( vec_chain_full.end(),vec_edge_left.begin(),vec_edge_left.end() );
        vec_vec_edge_points[0]=vec_chain_full;
    }*/

    //fill edge from world edge to first point in terrain vector, same for right side of map
    //cout<<m_level_hard_borders[0]*_Pix2Met<<endl;
    b2Vec2 first_pt( m_level_hard_borders[0]*_Pix2Met/scale_factor[0], vec_vec_edge_points[0].front().y );
    b2Vec2 last_pt ( m_level_hard_borders[1]*_Pix2Met/scale_factor[0], vec_vec_edge_points[0].back ().y );
    vector<b2Vec2> vec_chain_w_start_end_pt;
    vec_chain_w_start_end_pt.push_back(first_pt);
    vec_chain_w_start_end_pt.insert( vec_chain_w_start_end_pt.end(), vec_vec_edge_points[0].begin(), vec_vec_edge_points[0].end() );
    vec_chain_w_start_end_pt.push_back(last_pt);
    vec_vec_edge_points[0]=vec_chain_w_start_end_pt;

    //build level m_vec_terrain
    for(int chain_i=0;chain_i<(int)vec_vec_edge_points.size();chain_i++)
    {
        b2BodyDef floorbodydef;
        floorbodydef.position.Set( 0,0 );
        floorbodydef.type=b2_staticBody;
        b2Body* pBody_temp=m_pWorld->CreateBody(&floorbodydef);
        m_vec_terrain.push_back(pBody_temp);
        //m_vec_terrain.back()=m_pWorld->CreateBody(&floorbodydef);

        int numof_points=(int)vec_vec_edge_points[chain_i].size();
        if(numof_points<2)
        {
            cout<<"ERROR: Too few points for terrain\n";
            return false;
        }
        b2Vec2 vs[numof_points];
        for(int i=0;i<(int)vec_vec_edge_points[chain_i].size();i++)
        {
            vs[i].x=vec_vec_edge_points[chain_i][i].x*scale_factor[0];
            vs[i].y=vec_vec_edge_points[chain_i][i].y*scale_factor[1];
        }
        b2ChainShape chain;
        if(chain_i==0)//first chain is wall to wall and will not be closed
         chain.CreateChain(vs,numof_points);
        else//all other chains will be looped, first connected to last
         chain.CreateLoop(vs,numof_points);

        b2FixtureDef floorfixturedef;
        floorfixturedef.shape=&chain;
        floorfixturedef.density=0.0;
        floorfixturedef.filter.maskBits=-1;//TEMP
        floorfixturedef.filter.categoryBits=_COLCAT_all;//TEMP
        m_vec_terrain.back()->CreateFixture(&floorfixturedef);
        //set data
        st_body_user_data* user_data=new st_body_user_data;
        user_data->s_info="terrain";
        m_vec_terrain.back()->SetUserData(user_data);
    }

    //make edge walls (hard borders) no passing
    b2BodyDef edgebodydef;
    edgebodydef.position.Set( 0,0 );
    edgebodydef.type=b2_staticBody;
    b2Body* pBody_edge=m_pWorld->CreateBody(&edgebodydef);
    b2ChainShape chain_edge;
    //cout<<"hard edge: "<<m_level_hard_borders[0]<<", "<<m_level_hard_borders[1]<<endl;
    b2Vec2 edge_points[4]={b2Vec2(m_level_hard_borders[0]*_Pix2Met,  max_y_coord*_Pix2Met*1.1),
                           b2Vec2(m_level_hard_borders[0]*_Pix2Met,  m_level_sky_height*_Pix2Met),
                           b2Vec2(m_level_hard_borders[1]*_Pix2Met,  m_level_sky_height*_Pix2Met),
                           b2Vec2(m_level_hard_borders[1]*_Pix2Met,  max_y_coord*_Pix2Met*1.1)};
    chain_edge.CreateChain(edge_points,4);
    b2FixtureDef edgefixturedef;
    edgefixturedef.shape=&chain_edge;
    edgefixturedef.density=0.0;
    edgefixturedef.filter.categoryBits=_COLCAT_all;
    edgefixturedef.filter.maskBits=-1;
    pBody_edge->CreateFixture(&edgefixturedef);
    //set data
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="edge";
    pBody_edge->SetUserData(user_data);

    //set highest map point
    m_level_highest_point=min_y_coord*_Pix2Met;

    //set cam pos
    m_cam_pos[0]=m_player_start_pos[0]*_Met2Pix-m_screen_width*0.5;
    m_cam_pos[1]=m_player_start_pos[1]*_Met2Pix;

    //store terrain data
    m_vec_vec_terrain_points.clear();
    for(int chain_i=0;chain_i<(int)vec_vec_edge_points.size();chain_i++)
    {
        m_vec_vec_terrain_points.push_back( vector<st_terrain_point>() );
        //add points
        for(int point_i=0;point_i<(int)vec_vec_edge_points[chain_i].size();point_i++)
        {
            m_vec_vec_terrain_points.back().push_back( st_terrain_point( vec_vec_edge_points[chain_i][point_i].x*scale_factor[0]*_Met2Pix,
                                                                         vec_vec_edge_points[chain_i][point_i].y*scale_factor[1]*_Met2Pix ) );
            m_vec_vec_terrain_points.back().back().height_var=rand()%20+40;
        }
    }

    m_vec_vec_cave_points.clear();
    for(int chain_i=0;chain_i<(int)vec_vec_cave_points.size();chain_i++)
    {
        m_vec_vec_cave_points.push_back( vector<st_terrain_point>() );
        //add points
        for(int point_i=0;point_i<(int)vec_vec_cave_points[chain_i].size();point_i++)
        {
            m_vec_vec_cave_points.back().push_back( st_terrain_point( vec_vec_cave_points[chain_i][point_i].x*scale_factor[0]*_Met2Pix,
                                                                      vec_vec_cave_points[chain_i][point_i].y*scale_factor[1]*_Met2Pix ) );
            m_vec_vec_cave_points.back().back().height_var=rand()%20+20;
        }
    }


    /*//calculate subpoint
    float distance=50.0;
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        //force first and last point, just below
        if(m_vec_vec_terrain_points[chain_i].size()<3)
        {
            cout<<"ERROR: Calculating terrain data: To few points\n";
            break;
        }
        //edit first and last point
        m_vec_vec_terrain_points[chain_i].front().subpos_x=m_vec_vec_terrain_points[chain_i].front().pos_x;
        m_vec_vec_terrain_points[chain_i].front().subpos_y=m_vec_vec_terrain_points[chain_i].front().pos_y+distance;
        m_vec_vec_terrain_points[chain_i].back().subpos_x=m_vec_vec_terrain_points[chain_i].back().pos_x;
        m_vec_vec_terrain_points[chain_i].back().subpos_y=m_vec_vec_terrain_points[chain_i].back().pos_y+distance;

        //skip first and last point
        //still some bug if vertical shift could create problems
        for(int point_i=1;point_i<(int)m_vec_vec_terrain_points[chain_i].size()-1;point_i++)
        {
            float ax=m_vec_vec_terrain_points[chain_i][point_i-1].pos_x;
            float ay=m_vec_vec_terrain_points[chain_i][point_i-1].pos_y;
            float bx=m_vec_vec_terrain_points[chain_i][point_i].pos_x;
            float by=m_vec_vec_terrain_points[chain_i][point_i].pos_y;
            float cx=m_vec_vec_terrain_points[chain_i][point_i+1].pos_x;
            float cy=m_vec_vec_terrain_points[chain_i][point_i+1].pos_y;

            //if no height diff, force standard drop dist
            if(ay==by && ay==cy)
            {
                m_vec_vec_terrain_points[chain_i][point_i].subpos_x=m_vec_vec_terrain_points[chain_i][point_i].pos_x;
                m_vec_vec_terrain_points[chain_i][point_i].subpos_y=m_vec_vec_terrain_points[chain_i][point_i].pos_y+distance;
                continue;
            }

            //calc B->A vector
            float bax=ax-bx;
            float bay=ay-by;
            float ba_length=sqrt( bax*bax + bay*bay );
            bax/=ba_length;
            bay/=ba_length;
            //calc B->C vector
            float bcx=cx-bx;
            float bcy=cy-by;
            float bc_length=sqrt( bcx*bcx + bcy*bcy );
            bcx/=bc_length;
            bcy/=bc_length;
            //calc B->S vector (subpoint vector, rel to A)
            float bsx=bax+bcx;
            float bsy=bay+bcy;

            //test if right/left angle
            bool left_angle=false;
            if(bax==0.0 || bcx==0.0) continue;
            else if( bay/bax > bcy/bcx ) left_angle=true;

            //get subpoint vector normalize
            float length=sqrt( bsx*bsx + bsy*bsy );
            if(length==0.0)
            {
                cout<<"ERROR: Calculating terrain data: Length is zero\n";
                continue;
            }
            float sx=bsx/length*distance;
            float sy=bsy/length*distance;
            if(left_angle)
            {
                sx*=-1.0;
                sy*=-1.0;
            }

            //store subpoint
            m_vec_vec_terrain_points[chain_i][point_i].subpos_x=m_vec_vec_terrain_points[chain_i][point_i].pos_x+sx;
            m_vec_vec_terrain_points[chain_i][point_i].subpos_y=m_vec_vec_terrain_points[chain_i][point_i].pos_y+sy;
        }
    }*/


    //gen stars
    int level_width=max_x_coord-min_x_coord;
    //cout<<"Level width: "<<level_width<<endl;
    int terrain_height=m_vec_vec_terrain_points.front().front().pos_y;
    m_vec_stars.clear();
    int numof_stars=rand()%int((float)level_width/100.0)+200;
    for(int i=0;i<numof_stars;i++)
    {
        int x_pos=rand()%int(m_level_soft_borders[1]-m_level_soft_borders[0])+m_level_soft_borders[0];
        int y_pos=rand()%int(terrain_height-m_level_sky_height)+m_level_sky_height;
        float light=float(rand()%500)/1000.0+0.5;

        m_vec_stars.push_back( st_int_int_float(x_pos,y_pos,light) );
    }


    cout<<"Loading Level Complete\n";

    return true;
}

bool game::draw_static(float set_fade_level)
{
    //return true;//disable

    float fade_level=set_fade_level;
    if(fade_level==-1)
    //calc fade level depending on cam pos
    {
        int cam_pos_x=m_cam_pos[0]+m_screen_width*0.5;
        if(cam_pos_x<m_level_soft_borders[0] || cam_pos_x>m_level_soft_borders[1] )
        {
            if(cam_pos_x<m_level_soft_borders[0]-m_level_static_fade_distance) fade_level=1.0;
            else if(cam_pos_x>m_level_soft_borders[1]+m_level_static_fade_distance) fade_level=1.0;
            else//calc fade level
            {
                if(cam_pos_x<m_level_soft_borders[0])
                {
                    fade_level= -(cam_pos_x-m_level_soft_borders[0])/m_level_static_fade_distance;
                }
                else if(cam_pos_x>m_level_soft_borders[1])
                {
                    fade_level= (cam_pos_x-m_level_soft_borders[1])/m_level_static_fade_distance;
                }
            }
        }
        //test if close to height limit
        int cam_pos_y=m_cam_pos[1]+m_screen_height*0.5;
        if(cam_pos_y<m_level_sky_height+m_level_static_fade_distance*2.0)
        {
            if(cam_pos_y<m_level_sky_height+m_level_static_fade_distance*1.0) fade_level=1.0;
            else//calc new fade level
            {
                float new_fade_level=-( cam_pos_y-(m_level_sky_height+m_level_static_fade_distance*2.0) )/m_level_static_fade_distance;
                //use highest value
                if(new_fade_level>fade_level) fade_level=new_fade_level;
            }
        }
    }
    //else use set level (0-1)

    int pixel_size=10;
    int start_pos=int((float)pixel_size*0.5);
    glPointSize(pixel_size);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_POINTS);
    for(int pos_x=start_pos;pos_x<m_screen_width;pos_x+=pixel_size)
     for(int pos_y=start_pos;pos_y<m_screen_height;pos_y+=pixel_size)
    {
        if( rand()%2==0 ) glColor4f(1.0,1.0,1.0,fade_level);
        else              glColor4f(0.0,0.0,0.0,fade_level);
        glVertex2i(pos_x,pos_y);
    }
    glEnd();
    glDisable(GL_BLEND);
    glPointSize(1);

    return true;
}

bool game::add_explotion(b2Body* exploading_body_ptr)
{
    st_body_user_data* data=(st_body_user_data*)exploading_body_ptr->GetUserData();

    float blast_radius=10.0;
    float blast_force_sens=100.0;
    b2Vec2 center_pos=exploading_body_ptr->GetPosition();

    //get list of bodies nearby
    b2AABB aabb_box;
    aabb_box.lowerBound=center_pos-b2Vec2(blast_radius,blast_radius);
    aabb_box.upperBound=center_pos+b2Vec2(blast_radius,blast_radius);
    MyQueryCallback aabb_callback;
    m_pWorld->QueryAABB(&aabb_callback,aabb_box);

    //translate to bodies
    vector<b2Body*> vec_bodies_involved;
    for(int fixture_i=0;fixture_i<(int)aabb_callback.m_vec_fixtures.size();fixture_i++)
    {
        b2Body* body_ptr=aabb_callback.m_vec_fixtures[fixture_i]->GetBody();

        //test if new
        bool is_new=true;
        for(int body_i=0;body_i<(int)vec_bodies_involved.size();body_i++)
        {
            if( vec_bodies_involved[body_i]==body_ptr )
            {
                is_new=false;
                break;
            }
        }
        if(!is_new) continue;//body already in vector

        //test if anything in the way (ON or OFF)
        /*MyRayCastCallback raycast;
        raycast.set_ignore_body(body_ptr);
        raycast.set_ignore_body_type("rope");
        raycast.set_ignore_body_type("hook");
        m_pWorld->RayCast(&raycast,center_pos,body_ptr->GetPosition());
        if(!raycast.m_any_hit)//nothing in the way*/
         vec_bodies_involved.push_back(body_ptr);
    }

    //add force to bodies
    for(int body_i=0;body_i<(int)vec_bodies_involved.size();body_i++)
    {
        if(vec_bodies_involved[body_i]==exploading_body_ptr) continue;//ignore itself

        st_body_user_data* body_data=(st_body_user_data*)vec_bodies_involved[body_i]->GetUserData();
        //ignore terrain type
        if(body_data->s_info=="terrain") continue;
        //ignore some projectiles (all except rockets, grenades and mines)
        if(body_data->s_info=="projectile")
        {
            if( body_data->i_id!=wt_rocket &&
                body_data->i_id!=wt_grenade &&
                body_data->i_id!=wt_mine ) continue;
        }
        //cout<<"Explosion effect on body: "<<body_data->s_info<<endl;

        //calc force direction
        b2Vec2 force=vec_bodies_involved[body_i]->GetPosition()-center_pos;
        float distance=force.Normalize();
        if(distance<1.0) distance=1.0;//min length to avoid extreme force

        //apply force
        b2Vec2 blast_force=(blast_force_sens/distance)*force;
        if(body_data->s_info=="player") blast_force=10*blast_force;//stronger due to higher mass
        vec_bodies_involved[body_i]->ApplyForce( blast_force,
                                                 vec_bodies_involved[body_i]->GetWorldPoint( b2Vec2(0.0,0.0) ),
                                                 true );

        //apply damage (to enemy,player,object)
        if(body_data->s_info=="enemy"||body_data->s_info=="player"||body_data->s_info=="object")
        {
            body_data->f_collision_damage_update+=data->f_projectile_damage_update/distance;
        }
    }

    //add extra visual explosion
    m_pParticle_engine->add_explosion(center_pos,100,300,0.5);

    return true;
}

bool game::unload_level(void)
{
    //unload only if a world is loaded
    if(!m_level_loaded) return false;

    //delete all joints
    cout<<"Deleting Joints...";
    b2Joint* tmp_joint=m_pWorld->GetJointList();
    while(tmp_joint)
    {
        b2Joint* tmp_joint_to_remove=tmp_joint;
        tmp_joint=tmp_joint->GetNext();

        m_pWorld->DestroyJoint(tmp_joint_to_remove);
    }
    cout<<"Complete\n";

    //delete all bodies
    cout<<"Deleting Bodies...";
    b2Body* tmp_body=m_pWorld->GetBodyList();
    while(tmp_body)
    {
        //destroy userdata
        st_body_user_data* data=(st_body_user_data*)tmp_body->GetUserData();
        delete data;

        b2Body* tmp_body_to_remove=tmp_body;
        tmp_body=tmp_body->GetNext();

        m_pWorld->DestroyBody(tmp_body_to_remove);
    }
    cout<<"Complete\n";

    //delete world
    cout<<"Deleting World...";
    //m_pWorld->~b2World();
    delete m_pWorld;
    cout<<"Complete\n";

    //reset variables
    m_vec_terrain.clear();
    m_vec_projectiles_to_remove.clear();
    m_vec_objects.clear();

    //remove enemies
    for(int i=0;i<(int)m_vec_pEnemies.size();i++)
    {
        m_vec_pEnemies[i]->delete_equipment();
    }
    m_vec_pEnemies.clear();

    m_cam_pos[0]=0.0;
    m_cam_pos[1]=0.0;
    m_cam_speed[0]=0.0;
    m_cam_speed[1]=0.0;
    m_world_max_x=0.0;
    m_world_offside_limit=0.0;
    m_came_mode=cm_follow_one;//who to follow
    //m_cam_player_to_follow=0;
    m_level_soft_borders[0]=-1.0;//left soft border
    m_level_soft_borders[1]= 1.0;//right soft border
    m_level_hard_borders[0]=-2.0;//left hard border
    m_level_hard_borders[1]= 2.0;//right hard border
    m_level_static_fade_distance=m_screen_width*0.5;
    m_level_sky_height=1000;
    m_id_counter_object=0;
    m_pEvent_flag_hook[0]=m_pEvent_flag_hook[1]=m_pEvent_flag_hook[2]=m_pEvent_flag_hook[3]=ev_idle;
    m_pMship_landing_sensor_flag[0]=m_pMship_landing_sensor_flag[1]=m_pMship_landing_sensor_flag[2]=m_pMship_landing_sensor_flag[3]=false;

    m_level_loaded=false;

    cout<<"Level Unloaded\n";

    return true;
}

bool game::load_selected_level(void)
{
    //unload current level, if not visiting the same as last
    if( m_last_played_level==m_Starmap.get_planet_now_level_index() && m_last_played_level!=-1 && !m_run_test_level )
    {//go back to current planet
        //move mship into pos
        m_pMain_ship->reset_to_land_pos();
        m_player_input_enabled=true;
        m_mship_landed=false;
        return true;
    }
    else//not last level, load new
    {
        //unload current level to load new
        unload_level();
    }

    cout<<"Loading selected level\n";

    bool tutorial_level_revisit=false;
    //int planet_ind_to_load=m_Starmap.get_planet_now();
    int level_ind=m_Starmap.get_planet_now_level_index();
    //
    if(level_ind==-1)//have not been visited
    {
        m_planets_visited_counter++;
        cout<<"Visited planets: "<<m_planets_visited_counter<<endl;
        //give unused or rare level
        int max_tries=20;
        int try_counter=0;
        while(true)
        {
            level_ind=rand()%_world_numof_levels;
            //test if already been used
            bool already_played=false;
            for(int level_i=(int)m_vec_played_levels.size()-1;level_i>=0;level_i--)
            {
                if(level_ind==m_vec_played_levels[level_i])
                {
                    already_played=true;
                    break;
                }
            }
            if(!already_played || try_counter>max_tries)
            {
                //level int selected
                if(!already_played)//level have not been played
                {
                    //remember level ind
                    m_vec_played_levels.push_back(level_ind);
                    m_last_played_level=level_ind;
                    m_Starmap.set_planet_now_level_index(level_ind);
                    cout<<"Loading current level index: "<<level_ind<<endl;
                    break;
                }
            }
            else//try again
            {
                try_counter++;
            }
        }
    }
    if(level_ind==-2)//tutorial level
    {
        tutorial_level_revisit=true;//will also be true if first visit
    }

    //create world
    m_world_gravity_curr=_world_gravity;//set gravity
    m_pWorld=new b2World( b2Vec2(0.0,m_world_gravity_curr) );

    //all elements with old world pointer have to be updated (which are not directly in the world)
    //weapons owned by mship
    for(int weap_i=0;weap_i<(int)m_vec_pWeapon_stored.size();weap_i++)
    {
        m_vec_pWeapon_stored[weap_i]->set_new_world(m_pWorld);
    }
    //gears owned by mship
    for(int gear_i=0;gear_i<(int)m_vec_pGear_stored.size();gear_i++)
    {
        m_vec_pGear_stored[gear_i]->set_new_world(m_pWorld);
    }
    //weapon and gear owned by players
    for(int player_i=0;player_i<4;player_i++)
    {
        m_vec_players[player_i].get_weapon_ptr()->set_new_world(m_pWorld);
        m_vec_players[player_i].get_gear_ptr()->set_new_world(m_pWorld);
    }


    //load level data
    vector<st_float_float_int> vec_enemy_pos;
    vector<st_float_float_int> vec_object_pos;
    if( !load_level_data(level_ind,vec_enemy_pos,vec_object_pos) )
    {
        cout<<"ERROR: Problem loading level\n";
        return false;
    }

    //create mship
    m_pMain_ship->init(m_pWorld,m_pParticle_engine,m_world_gravity_curr,m_tex_decal,
                       &m_mship_led_prog,true,b2Vec2(m_player_start_pos[0],m_player_start_pos[1]));

    //create players
    for(int player_i=0;player_i<4;player_i++)
    {
        m_vec_players[player_i].init( m_pWorld,m_pMain_ship->get_body_ptr(),
                                      m_pParticle_engine,m_pSound,m_tex_decal,player_i,true,
                                      b2Vec2(m_player_start_pos[0],m_player_start_pos[1]) );

        //reset drone mode
        m_vec_players[player_i].set_drone_mode(dm_off);

        //clear old disconnected ropes
        m_vec_players[player_i].erase_lost_ropes();
    }

    //force objects and enemies
    if(m_on_tutorial_level || tutorial_level_revisit)
    {
        //dont load object and enemies if already been here
        if(m_on_tutorial_level)
        {
            //force fuel size
            for(int object_i=0;object_i<(int)vec_object_pos.size();object_i++)
            {
                //get pos
                int pos_index=object_i;
                //get size, 0.2-0.8
                float fuel_size=0.5;
                //get fuel content, 0-100
                float fuel_content=80.0;

                m_vec_objects.push_back( object() );
                m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                          fuel_size,//size
                                          b2Vec2(vec_object_pos[pos_index].val_f1,vec_object_pos[pos_index].val_f2),
                                          fuel_content);//fuel content
            }
            vec_object_pos.clear();

            //force enemy type
            for(int enemy_i=0;enemy_i<(int)vec_enemy_pos.size();enemy_i++)
            {
                //get type to place
                int enemy_type=et_scanner;

                //get pos
                int pos_index=-1;

                pos_index=enemy_i;

                //place enemy
                //cout<<"place enemy...";
                m_vec_pEnemies.push_back( new enemy() );
                m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                            b2Vec2(vec_enemy_pos[pos_index].val_f1,vec_enemy_pos[pos_index].val_f2),
                                            enemy_type,m_tex_decal);
            }
            vec_enemy_pos.clear();

            //give mship body pointer to hud
            m_hud.set_mship_ptr( m_pMain_ship->get_body_ptr() );
            //give player body ptr to hud
            bool found_player=false;
            for(int i=0;i<4;i++)
            {
                if(m_player_active[i])
                {
                    //cout<<"found active player: "<<i+1<<endl;
                    found_player=true;
                    m_hud.set_player_ptr( m_vec_players[i].get_body_ptr() );
                    break;
                }
            }
            if(!found_player) m_hud.set_player_ptr( m_vec_players[0].get_body_ptr() );


            /*//add temp resources
            int numof_boxes=100;
            for(int box_i=0;box_i<numof_boxes;box_i++)
            {
                //const angle
                if(false)
                {
                    int numof_edges=rand()%4+5;
                    float deg=360.0/(float)numof_edges;
                    float radius=1.0;
                    b2Vec2 edges[numof_edges];
                    for(int e=0;e<numof_edges;e++)
                    {
                        edges[e].Set( float(rand()%10+1)/10.0*radius*cosf(deg*(float)e*_Deg2Rad),
                                      float(rand()%10+1)/10.0*radius*sinf(deg*(float)e*_Deg2Rad) );
                    }
                }

                //const radius
                if(false)
                {
                    int numof_edges=rand()%4+5;
                    float deg_target=360.0/(float)numof_edges;
                    float radius=1.0;
                    b2Vec2 edges[numof_edges];
                    float deg_curr=0;
                    for(int e=0;e<numof_edges;e++)
                    {
                        edges[e].Set( radius*cosf(deg_curr*(float)e*_Deg2Rad),
                                      radius*sinf(deg_curr*(float)e*_Deg2Rad) );
                        deg_curr+=deg_target+(rand()%10)/10.0*deg_target-deg_target*0.5;
                        if(deg_curr>=360.0) numof_edges=e+1;
                    }
                    if(numof_edges<3 || deg_curr<=180.0) continue;//bad shape
                }

                //convex hull
                int numof_edges=rand()%4+5;
                vector<b2Vec2> vec_points(numof_edges);
                for(int e=0;e<numof_edges;e++)
                {
                    //give random pos
                    vec_points.push_back( b2Vec2( float(rand()%10-5)/5.0, float(rand()%10-5)/5.0 ) );
                }
                //sort on x value
                while(true)
                {
                    bool updated=false;
                    for(unsigned int p=0;p<vec_points.size()-1;p++)
                    {
                        if( vec_points[p].x>vec_points[p+1].x )
                        {
                            updated=true;
                            b2Vec2 temp=vec_points[p];
                            vec_points[p]=vec_points[p+1];
                            vec_points[p+1]=temp;
                        }
                    }
                    if(!updated) break;
                }
                //create convex hull,
                vector<b2Vec2> hull_points(numof_edges*2);
                int k=0;
                //lower hull
                for(int i=0;i<numof_edges;++i)
                {
                    while (k >= 2 && cross(hull_points[k-2], hull_points[k-1], vec_points[i]) <= 0) k--;
                    hull_points[k++] = vec_points[i];
                }
                //upper hull
                for(int i=numof_edges-2, t=k+1; i>=0; i--)
                {
                    while (k >= t && cross(hull_points[k-2], hull_points[k-1], vec_points[i]) <= 0) k--;
                    hull_points[k++] = vec_points[i];
                }
                hull_points.resize(k);
                //test if ok
                if((int)hull_points.size()<5) continue;//skip
                b2Vec2 edges[(int)hull_points.size()];
                for(unsigned int e=0;e<hull_points.size();e++)
                {
                    edges[e].Set(hull_points[e].x, hull_points[e].y);
                }

                //place box
                m_vec_objects.push_back( object() );
                m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                          edges,(int)hull_points.size(),
                                          b2Vec2(m_player_start_pos[0]-10-rand()%50,
                                                 m_player_start_pos[1]+rand()%20-10) );
            }*/


            //add temp enemy
            /*int pos_shift=10;
            for(int et=et_burst_bot;et<=et_rammer;et++)
            {
                m_vec_pEnemies.push_back( new enemy() );
                m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                            b2Vec2(m_player_start_pos[0]+pos_shift,m_player_start_pos[1]),
                                            et,m_tex_decal);
                pos_shift+=10;
            }*/
            /*m_vec_pEnemies.push_back( new enemy() );
            m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                        b2Vec2(m_player_start_pos[0]+10,m_player_start_pos[1]),
                                        et_scanner,m_tex_decal);
            m_vec_objects.push_back( object() );
            m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                      0.5,//size
                                      b2Vec2(m_player_start_pos[0]-10,m_player_start_pos[1]),
                                      50);//fuel content*/

        }
    }
    else//normal placement
    {
        //place objects
        cout<<"Load Level: Placing Objects\n";
        int numof_objects_avg=m_Starmap.get_curr_planet_level_fuel();
        float numof_objects_variation=0.5;//50%
        int numof_objects=numof_objects_avg+(rand()%int((float)numof_objects_avg*numof_objects_variation*2.0)-
                                                    int((float)numof_objects_avg*numof_objects_variation));
        //if test level, spawn all
        if(m_run_test_level) numof_objects=100;//spawn max

        cout<<"numof objects to place: "<<numof_objects<<endl;
        for(int object_i=0;object_i<numof_objects;object_i++)
        {
            //test if spaces left
            if(vec_object_pos.empty()) break;

            //get pos
            int pos_index=rand()%(int)vec_object_pos.size();
            //get size, 0.2-0.8
            float fuel_size=float(rand()%100)/166.0+0.2;
            //get fuel content, 0-100
            float fuel_content=float(rand()%80)+20;

            m_vec_objects.push_back( object() );
            m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                      fuel_size,//size
                                      b2Vec2(vec_object_pos[pos_index].val_f1,vec_object_pos[pos_index].val_f2),
                                      fuel_content);//fuel content
            //remove pos, not to be reused
            vec_object_pos.erase(vec_object_pos.begin()+pos_index);

            //break;//TEMP
        }

        //place enemies
        cout<<"Load Level: Placing Enemies\n";
        int numof_enemies_avg=m_Starmap.get_curr_planet_level_enemy();
        float numof_enemies_variation=0.5;//50%
        int numof_enemies=numof_enemies_avg+(rand()%int((float)numof_enemies_avg*numof_enemies_variation*2.0)-
                                                    int((float)numof_enemies_avg*numof_enemies_variation));
        //if test level, spawn all
        if(m_run_test_level) numof_enemies=100;//spawn max

        cout<<"numof enemies to place: "<<numof_enemies<<endl;
        for(int enemy_i=0;enemy_i<numof_enemies;enemy_i++)
        {
            //test if spaces left
            if(vec_enemy_pos.empty()) break;

            //get type to place
            int enemy_type=rand()%15+2;//15 types, offset of 2 (2-16)
            //cout<<"Enemy type: "<<enemy_type<<endl;
            //get pos
            int pos_index=-1;
            if(enemy_type==et_burst_bot || enemy_type==et_flipper)
            {
                //find surface pos, type 2
                vector<int> vec_alternatives;
                for(int pos_i=0;pos_i<(int)vec_enemy_pos.size();pos_i++)
                {
                    if(vec_enemy_pos[pos_i].val_i==2)
                    {
                        vec_alternatives.push_back(pos_i);
                    }
                }
                if(!vec_alternatives.empty())
                {
                    int rand_val=rand()%int(vec_alternatives.size());
                    pos_index=vec_alternatives[rand_val];
                }
            }
            else//any place
            {
                pos_index=rand()%(int)vec_enemy_pos.size();
            }

            if(pos_index==-1)
            {
                cout<<"ERROR: Could not place enemy\n";
                continue;
            }

            //place enemy
            //cout<<"place enemy...";
            m_vec_pEnemies.push_back( new enemy() );
            m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                        b2Vec2(vec_enemy_pos[pos_index].val_f1,vec_enemy_pos[pos_index].val_f2),
                                        enemy_type,m_tex_decal);
            //remove pos, not to be reused
            vec_enemy_pos.erase(vec_enemy_pos.begin()+pos_index);
            //cout<<"done\n";
        }

        //spawn material on leftover enemy places
        for(unsigned int pos_i=0;pos_i<vec_enemy_pos.size();pos_i++)
        {
            //flip coin
            if( float(rand()%100)/100.0>_world_resource_at_enemy_pos_chance ) continue;//skip

            //create resource boxes
            int numof_boxes=rand()%5;
            for(int box_i=0;box_i<numof_boxes;box_i++)
            {
                //convex hull method
                int numof_edges=rand()%4+5;
                vector<b2Vec2> vec_points(numof_edges);
                for(int e=0;e<numof_edges;e++)
                {
                    //give random pos
                    vec_points.push_back( b2Vec2( float(rand()%10-5)/5.0, float(rand()%10-5)/5.0 ) );
                }
                //sort on x value
                while(true)
                {
                    bool updated=false;
                    for(unsigned int p=0;p<vec_points.size()-1;p++)
                    {
                        if( vec_points[p].x>vec_points[p+1].x )
                        {
                            updated=true;
                            b2Vec2 temp=vec_points[p];
                            vec_points[p]=vec_points[p+1];
                            vec_points[p+1]=temp;
                        }
                    }
                    if(!updated) break;
                }
                //create convex hull,
                vector<b2Vec2> hull_points(numof_edges*2);
                int k=0;
                //lower hull
                for(int i=0;i<numof_edges;++i)
                {
                    while (k >= 2 && cross(hull_points[k-2], hull_points[k-1], vec_points[i]) <= 0) k--;
                    hull_points[k++] = vec_points[i];
                }
                //upper hull
                for(int i=numof_edges-2, t=k+1; i>=0; i--)
                {
                    while (k >= t && cross(hull_points[k-2], hull_points[k-1], vec_points[i]) <= 0) k--;
                    hull_points[k++] = vec_points[i];
                }
                hull_points.resize(k);
                //test if ok
                if((int)hull_points.size()<5) continue;//skip
                b2Vec2 edges[(int)hull_points.size()];
                for(unsigned int e=0;e<hull_points.size();e++)
                {
                    edges[e].Set(hull_points[e].x, hull_points[e].y);
                }

                //place box
                m_vec_objects.push_back( object() );
                m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                          edges,(int)hull_points.size(),
                                          b2Vec2(vec_enemy_pos[pos_i].val_f1+float(rand()%50)/10.0-2.5,
                                                 vec_enemy_pos[pos_i].val_f2+float(rand()%50)/10.0-2.5) );
            }
        }
    }

    /*for(int enemy_i=0;enemy_i<(int)vec_enemy_pos.size();enemy_i++)
    {
        m_vec_pEnemies.push_back( new enemy() );
        m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                    b2Vec2(vec_enemy_pos[enemy_i].val_f1,vec_enemy_pos[enemy_i].val_f2),
                                    et_stand_turret);
    }*/

    //init contact listener
    m_ppBody_to_connect[0]=m_pBody_to_connect_p1;
    m_ppBody_to_connect[1]=m_pBody_to_connect_p2;
    m_ppBody_to_connect[2]=m_pBody_to_connect_p3;
    m_ppBody_to_connect[3]=m_pBody_to_connect_p4;
    m_ppPlayer_bodies[0]=m_vec_players[0].get_body_ptr();
    m_ppPlayer_bodies[1]=m_vec_players[1].get_body_ptr();
    m_ppPlayer_bodies[2]=m_vec_players[2].get_body_ptr();
    m_ppPlayer_bodies[3]=m_vec_players[3].get_body_ptr();
    m_ppRope_hook_sensors[0]=m_vec_players[0].get_rope_hook_sensor();
    m_ppRope_hook_sensors[1]=m_vec_players[1].get_rope_hook_sensor();
    m_ppRope_hook_sensors[2]=m_vec_players[2].get_rope_hook_sensor();
    m_ppRope_hook_sensors[3]=m_vec_players[3].get_rope_hook_sensor();
    //m_pEvent_flag_input_box=new bool();
    *m_pEvent_flag_input_box=false;
    //m_pEvent_flag_landing_gear=new bool[2];
    m_pEvent_flag_landing_gear[0]=m_pEvent_flag_landing_gear[1]=false;
    m_ppBody_in_mship_input[0]=m_pBody_to_connect_p1;

    m_pWorld->SetContactListener(&m_myContactListenerInstance);
    m_myContactListenerInstance.init(m_pWorld,
                                     m_ppRope_hook_sensors,
                                     m_pMain_ship->get_sensor_landing_ptr(),
                                     m_pMain_ship->get_sensor_input_ptr(),
                                     m_ppPlayer_bodies,
                                     m_pEvent_flag_hook,
                                     m_pEvent_flag_input_box,
                                     m_pMship_landing_sensor_flag,
                                     m_ppBody_in_mship_input,
                                     m_ppBody_to_connect,
                                     m_pMain_ship->get_sensor_landing_gear_left_ptr(),
                                     m_pMain_ship->get_sensor_landing_gear_right_ptr(),
                                     m_pEvent_flag_landing_gear,
                                     &m_vec_projectiles_to_remove,
                                     &m_vec_collision_events);

    m_pMain_ship->set_landing_gear_sensor_flags(m_pEvent_flag_landing_gear);

    m_level_loaded=true;
    m_player_input_enabled=true;
    m_mship_landed=false;

    //reset player activity, for respawn toggling
    for(int i=0;i<4;i++)
     m_player_active[i]=false;

    cout<<"Level loaded\n";

    return true;
}

bool game::draw_cave(float cam_pos[2])
{
    //get planet terrain details
    int terrain_texture_index=m_Starmap.get_curr_planet_level_texture();
    float terrain_color[3]={1,1,1};
    m_Starmap.get_curr_planet_level_color(terrain_color);
    //menu and tutorial are always the same (first)
    if(m_game_state==gs_menu || m_on_tutorial_level)
    {
        terrain_texture_index=0;
        terrain_color[0]=1.0;
        terrain_color[1]=1.0;
        terrain_color[2]=1.0;
    }
    float line_color_light[3]={0.8,0.8,0.8};
    float line_color_dark[3]={0.05,0.05,0.05};
    float quad_color_light[4]={0.8,0.8,0.8,0.9};
    float quad_color_light_edge[4]={0.8,0.8,0.8,0.0};
    float quad_color_dark[4]={0.1,0.1,0.1,0.9};
    float quad_color_dark_edge[4]={0.1,0.1,0.1,0.0};
    float cave_darken[3]={0.5,0.5,0.5};
    switch(terrain_texture_index)
    {
        case 1:
        {
            line_color_light[0]=line_color_light[1]=line_color_light[2]=0.1;
            line_color_dark[0]=line_color_dark[1]=line_color_dark[2]=0.0;
            quad_color_light[0]=quad_color_light[1]=quad_color_light[2]=0.05; quad_color_light[3]=0.9;
            quad_color_light_edge[0]=quad_color_light_edge[1]=quad_color_light_edge[2]=0.0; quad_color_light_edge[3]=0.0;
            quad_color_dark[0]=quad_color_dark[1]=quad_color_dark[2]=0.05; quad_color_dark[3]=0.9;
            quad_color_dark_edge[0]=quad_color_dark_edge[1]=quad_color_dark_edge[2]=0.0; quad_color_dark_edge[3]=0.0;
            cave_darken[0]=cave_darken[1]=cave_darken[2]=0.1;
        }break;

        case 3:
        {
            line_color_light[0]=line_color_light[1]=line_color_light[2]=0.1;
            line_color_dark[0]=line_color_dark[1]=line_color_dark[2]=0.0;
            quad_color_light[0]=quad_color_light[1]=quad_color_light[2]=0.05; quad_color_light[3]=0.9;
            quad_color_light_edge[0]=quad_color_light_edge[1]=quad_color_light_edge[2]=0.0; quad_color_light_edge[3]=0.0;
            quad_color_dark[0]=quad_color_dark[1]=quad_color_dark[2]=0.05; quad_color_dark[3]=0.9;
            quad_color_dark_edge[0]=quad_color_dark_edge[1]=quad_color_dark_edge[2]=0.0; quad_color_dark_edge[3]=0.0;
            cave_darken[0]=cave_darken[1]=cave_darken[2]=0.1;
        }break;
    }

    glPushMatrix();
    glTranslatef(-cam_pos[0],-cam_pos[1],0);

    //draw cave lines
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_tex_menu);
    glColor3f(line_color_dark[0]*terrain_color[0],
              line_color_dark[1]*terrain_color[1],
              line_color_dark[2]*terrain_color[2]);
    float dot_width=6.0;
    //float flip_side_min_dist=10.0;
    for(int chain_i=0;chain_i<(int)m_vec_vec_cave_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        for(int point_i=0;point_i<(int)m_vec_vec_cave_points[chain_i].size()-1;point_i++)
        {
            /*//color test
            if( m_vec_vec_cave_points[chain_i][point_i+1].pos_x <
                m_vec_vec_cave_points[chain_i][point_i].pos_x + flip_side_min_dist )
            {
                //under, darker color
                glColor3f(0.05,0.05,0.05);
            }
            else//above
            {
                glColor3f(0.8,0.8,0.8);
            }

            //draw start
            glTexCoord2f( (430.0)/1024.0, (1024.0-43.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i].pos_x-dot_width,
                        m_vec_vec_cave_points[chain_i][point_i].pos_y-dot_width );
            glTexCoord2f( (430.0)/1024.0, (1024.0-29.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i].pos_x-dot_width,
                        m_vec_vec_cave_points[chain_i][point_i].pos_y+dot_width );
            glTexCoord2f( (445.0)/1024.0, (1024.0-29.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i].pos_x+dot_width,
                        m_vec_vec_cave_points[chain_i][point_i].pos_y+dot_width );
            glTexCoord2f( (445.0)/1024.0, (1024.0-43.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i].pos_x+dot_width,
                        m_vec_vec_cave_points[chain_i][point_i].pos_y-dot_width );

            if( m_vec_vec_cave_points[chain_i][point_i+1].pos_x <
                m_vec_vec_cave_points[chain_i][point_i].pos_x + flip_side_min_dist )
            {
                //under, darker color
                glColor3f(0.05,0.05,0.05);
            }
            else//above
            {
                glColor3f(0.8,0.8,0.8);
            }*/


            //draw line
            glTexCoord2f( (438.0)/1024.0, (1024.0-60.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i].pos_x,
                        m_vec_vec_cave_points[chain_i][point_i].pos_y-dot_width );
            glTexCoord2f( (438.0)/1024.0, (1024.0-46.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i].pos_x,
                        m_vec_vec_cave_points[chain_i][point_i].pos_y+dot_width );
            glTexCoord2f( (510.0)/1024.0, (1024.0-46.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                        m_vec_vec_cave_points[chain_i][point_i+1].pos_y+dot_width );
            glTexCoord2f( (510.0)/1024.0, (1024.0-60.0)/1024.0 );
            glVertex2f( m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                        m_vec_vec_cave_points[chain_i][point_i+1].pos_y-dot_width );

            //draw end (will be drawn fo next point)
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    //draw to stencil buffer
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    glStencilFunc( GL_ALWAYS, 1, 0xFF );
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

    for(int chain_i=0;chain_i<(int)m_vec_vec_cave_points.size();chain_i++)
    {
        glBegin(GL_TRIANGLE_FAN);
        for(int point_i=0;point_i<(int)m_vec_vec_cave_points[chain_i].size();point_i++)
        {
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y);
        }
        glEnd();
    }

    glPopMatrix();

    //draw cave color
    float move_shift_x=1.0;
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    glColor3f(cave_darken[0]*terrain_color[0],
              cave_darken[1]*terrain_color[1],
              cave_darken[2]*terrain_color[2]);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_tex_terrains[terrain_texture_index]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(GL_QUADS);
    glTexCoord2f(cam_pos[0]/1024.0*move_shift_x,cam_pos[1]/1024.0);
    glVertex2d(0.0,0.0);
    glTexCoord2f(cam_pos[0]/1024.0*move_shift_x+m_screen_width/1024.0,cam_pos[1]/1024.0);
    glVertex2d(m_screen_width,0.0);
    glTexCoord2f(cam_pos[0]/1024.0*move_shift_x+m_screen_width/1024.0,cam_pos[1]/1024.0+m_screen_height/1024.0);
    glVertex2d(m_screen_width,m_screen_height);
    glTexCoord2f(cam_pos[0]/1024.0*move_shift_x,cam_pos[1]/1024.0+m_screen_height/1024.0);
    glVertex2d(0.0,m_screen_height);
    glEnd();

    //glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_2D);
    //glEnable(GL_DEPTH_TEST);

    //retranslate back
    glPushMatrix();
    glTranslatef(-cam_pos[0],-cam_pos[1],0);

    //draw terrain edge
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //float dist_sep=30.0;
    for(int chain_i=0;chain_i<(int)m_vec_vec_cave_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        for(int point_i=0;point_i<(int)m_vec_vec_cave_points[chain_i].size()-1;point_i++)
        {
            //above, with shadow
            glColor4f(quad_color_dark_edge[0]*terrain_color[0],
                      quad_color_dark_edge[1]*terrain_color[1],
                      quad_color_dark_edge[2]*terrain_color[2],
                      quad_color_dark_edge[3]);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y-
                       m_vec_vec_cave_points[chain_i][point_i].height_var);
            glColor4f(quad_color_dark[0]*terrain_color[0],
                      quad_color_dark[1]*terrain_color[1],
                      quad_color_dark[2]*terrain_color[2],
                      quad_color_dark[3]);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y);

            glVertex2f(m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i+1].pos_y);
            glColor4f(quad_color_dark_edge[0]*terrain_color[0],
                      quad_color_dark_edge[1]*terrain_color[1],
                      quad_color_dark_edge[2]*terrain_color[2],
                      quad_color_dark_edge[3]);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i+1].pos_y-
                       m_vec_vec_cave_points[chain_i][point_i+1].height_var);

            //below, dark
            glColor4f(quad_color_dark[0]*terrain_color[0],
                      quad_color_dark[1]*terrain_color[1],
                      quad_color_dark[2]*terrain_color[2],
                      quad_color_dark[3]);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y);
            glColor4f(quad_color_dark_edge[0]*terrain_color[0],
                      quad_color_dark_edge[1]*terrain_color[1],
                      quad_color_dark_edge[2]*terrain_color[2],
                      quad_color_dark_edge[3]);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y+
                       m_vec_vec_cave_points[chain_i][point_i].height_var);

            glVertex2f(m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i+1].pos_y+
                       m_vec_vec_cave_points[chain_i][point_i+1].height_var);
            glColor4f(quad_color_dark[0]*terrain_color[0],
                      quad_color_dark[1]*terrain_color[1],
                      quad_color_dark[2]*terrain_color[2],
                      quad_color_dark[3]);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i+1].pos_y);

            /*//below, bright
            glColor4f(0.8,0.8,0.8,0.9);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y);
            glColor4f(0.8,0.8,0.8,0.0);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i].pos_y+
                       m_vec_vec_cave_points[chain_i][point_i].height_var);

            glVertex2f(m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i+1].pos_y+
                       m_vec_vec_cave_points[chain_i][point_i+1].height_var);
            glColor4f(0.8,0.8,0.8,0.9);
            glVertex2f(m_vec_vec_cave_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_cave_points[chain_i][point_i+1].pos_y);*/
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    glPopMatrix();

    return true;
}

bool game::draw_terrain(float cam_pos[2])
{
    //glPopMatrix(); return true;//disable terrain

    //get planet terrain details
    int terrain_texture_index=m_Starmap.get_curr_planet_level_texture();
    float terrain_color[3]={1,1,1};
    m_Starmap.get_curr_planet_level_color(terrain_color);
    //menu and tutorial are always the same (first)
    if(m_game_state==gs_menu || m_on_tutorial_level)
    {
        terrain_texture_index=0;
        terrain_color[0]=1.0;
        terrain_color[1]=1.0;
        terrain_color[2]=1.0;
    }
    float line_color_light[3]={0.8,0.8,0.8};
    float line_color_dark[3]={0.05,0.05,0.05};
    float quad_color_light[4]={0.8,0.8,0.8,0.9};
    float quad_color_light_edge[4]={0.8,0.8,0.8,0.0};
    float quad_color_dark[4]={0.1,0.1,0.1,0.9};
    float quad_color_dark_edge[4]={0.1,0.1,0.1,0.0};

    switch(terrain_texture_index)
    {
        case 1:
        {
            line_color_light[0]=line_color_light[1]=line_color_light[2]=0.1;
            line_color_dark[0]=line_color_dark[1]=line_color_dark[2]=0.0;
            quad_color_light[0]=quad_color_light[1]=quad_color_light[2]=0.05; quad_color_light[3]=0.9;
            quad_color_light_edge[0]=quad_color_light_edge[1]=quad_color_light_edge[2]=0.0; quad_color_light_edge[3]=0.0;
            quad_color_dark[0]=quad_color_dark[1]=quad_color_dark[2]=0.05; quad_color_dark[3]=0.9;
            quad_color_dark_edge[0]=quad_color_dark_edge[1]=quad_color_dark_edge[2]=0.0; quad_color_dark_edge[3]=0.0;
        }break;

        case 3:
        {
            line_color_light[0]=line_color_light[1]=line_color_light[2]=0.1;
            line_color_dark[0]=line_color_dark[1]=line_color_dark[2]=0.0;
            quad_color_light[0]=quad_color_light[1]=quad_color_light[2]=0.05; quad_color_light[3]=0.9;
            quad_color_light_edge[0]=quad_color_light_edge[1]=quad_color_light_edge[2]=0.0; quad_color_light_edge[3]=0.0;
            quad_color_dark[0]=quad_color_dark[1]=quad_color_dark[2]=0.05; quad_color_dark[3]=0.9;
            quad_color_dark_edge[0]=quad_color_dark_edge[1]=quad_color_dark_edge[2]=0.0; quad_color_dark_edge[3]=0.0;
        }break;
    }

    //glClearStencil( 0 );//remove cave info
    glClear(GL_STENCIL_BUFFER_BIT);

    //draw terrain balls
    /*glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_tex_menu);
    glColor3f(1.0,1.0,1.0);
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size();point_i++)
        {
            //draw dot
            glTexCoord2f( (438.0-4.0)/1024.0, (1024.0-24.0+4.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x-3.0,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y-3.0 );
            glTexCoord2f( (438.0-4.0)/1024.0, (1024.0-24.0-4.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x-3.0,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y+3.0 );
            glTexCoord2f( (438.0+4.0)/1024.0, (1024.0-24.0-4.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x+3.0,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y+3.0 );
            glTexCoord2f( (438.0+4.0)/1024.0, (1024.0-24.0+4.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x+3.0,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y-3.0 );
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);*/

    //draw terrain lines
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_tex_menu);
    glColor3f(1.0,1.0,1.0);
    float dot_width=2.0;
    float flip_side_min_dist=10.0;
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size()-1;point_i++)
        {
            /*
            //color test
            if( m_vec_vec_terrain_points[chain_i][point_i+1].pos_x <
                m_vec_vec_terrain_points[chain_i][point_i].pos_x + flip_side_min_dist )
            {
                //under, darker color
                glColor3f(0.05,0.05,0.05);
            }
            else//above
            {
                glColor3f(0.8,0.8,0.8);
            }

            //draw start
            glTexCoord2f( (430.0)/1024.0, (1024.0-43.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x-dot_width,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y-dot_width );
            glTexCoord2f( (430.0)/1024.0, (1024.0-29.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x-dot_width,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y+dot_width );
            glTexCoord2f( (445.0)/1024.0, (1024.0-29.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x+dot_width,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y+dot_width );
            glTexCoord2f( (445.0)/1024.0, (1024.0-43.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x+dot_width,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y-dot_width );*/

            if( m_vec_vec_terrain_points[chain_i][point_i+1].pos_x <
                m_vec_vec_terrain_points[chain_i][point_i].pos_x + flip_side_min_dist )
            {
                //under, darker color
                glColor3f(line_color_dark[0]*terrain_color[0],
                          line_color_dark[1]*terrain_color[1],
                          line_color_dark[2]*terrain_color[2]);
            }
            else//above
            {
                glColor3f(line_color_light[0]*terrain_color[0],
                          line_color_light[1]*terrain_color[1],
                          line_color_light[2]*terrain_color[2]);
            }

            //draw line
            glTexCoord2f( (438.0)/1024.0, (1024.0-60.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y-dot_width );
            glTexCoord2f( (438.0)/1024.0, (1024.0-46.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                        m_vec_vec_terrain_points[chain_i][point_i].pos_y+dot_width );
            glTexCoord2f( (510.0)/1024.0, (1024.0-46.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                        m_vec_vec_terrain_points[chain_i][point_i+1].pos_y+dot_width );
            glTexCoord2f( (510.0)/1024.0, (1024.0-60.0)/1024.0 );
            glVertex2f( m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                        m_vec_vec_terrain_points[chain_i][point_i+1].pos_y-dot_width );

            //draw end (will be drawn fo next point)
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    //draw to stencil buffer
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
    glStencilFunc( GL_ALWAYS, 1, 0xFF );
    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_TRIANGLE_FAN);

        if(chain_i==0)
        {
            //first triangle point from, top left (only for base terrain)
            glVertex2f(m_level_hard_borders[0]*_Met2Pix,-m_level_sky_height*_Met2Pix);
        }

        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size();point_i++)
        {
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);
        }

        if(chain_i==0)
        {
            //last triangle point, top right (only for base terrain)
            glVertex2f(m_level_hard_borders[1]*_Met2Pix,-m_level_sky_height*_Met2Pix);
        }

        glEnd();
    }

    /*//debug draw subpos, subpos have to be calculated
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_LINES);
        glColor3f(1,0,1);
        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size();point_i++)
        {
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].subpos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].subpos_y);
        }
        glEnd();
    }*/

    /*//debug draw subpos quads
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_STENCIL_TEST);
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glEnable(GL_BLEND);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        glColor4f(1.0,0.0,1.0,1.0);
        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size()-1;point_i++)
        {
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);
            glColor4f(1.0,0.0,1.0,0.1);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].subpos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].subpos_y);

            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].subpos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].subpos_y);
            glColor4f(1.0,0.0,1.0,1.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y);
        }
        glEnd();
    }
    glDisable(GL_BLEND);*/

    /*//debug draw big quads
    //glDisable(GL_TEXTURE_2D);
    //glDisable(GL_STENCIL_TEST);
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    glEnable(GL_BLEND);
    float dist_sep=50.0;
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size()-1;point_i++)
        {
            //above
            glColor4f(1.0,0.0,1.0,0.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y-dist_sep);
            glColor4f(1.0,0.0,1.0,1.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);

            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y);
            glColor4f(1.0,0.0,1.0,0.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y-dist_sep);

            //below
            glColor4f(1.0,0.0,1.0,1.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);
            glColor4f(1.0,0.0,1.0,0.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y+dist_sep);

            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y+dist_sep);
            glColor4f(1.0,0.0,1.0,1.0);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y);
        }
        glEnd();
    }
    glDisable(GL_BLEND);*/

    //pop matrix to draw terrain color across the screen
    glPopMatrix();

    //draw terrain color
    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
    glStencilFunc(GL_EQUAL, 1, 1);
    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
    glColor3fv(terrain_color);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_tex_terrains[terrain_texture_index]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBegin(GL_QUADS);
    glTexCoord2f(cam_pos[0]/1024.0,cam_pos[1]/1024.0);
    glVertex2d(0.0,0.0);
    glTexCoord2f(cam_pos[0]/1024.0+m_screen_width/1024.0,cam_pos[1]/1024.0);
    glVertex2d(m_screen_width,0.0);
    glTexCoord2f(cam_pos[0]/1024.0+m_screen_width/1024.0,cam_pos[1]/1024.0+m_screen_height/1024.0);
    glVertex2d(m_screen_width,m_screen_height);
    glTexCoord2f(cam_pos[0]/1024.0,cam_pos[1]/1024.0+m_screen_height/1024.0);
    glVertex2d(0.0,m_screen_height);
    glEnd();

    //glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_2D);
    //glEnable(GL_DEPTH_TEST);

    //retranslate back
    glPushMatrix();
    glTranslatef(-cam_pos[0],-cam_pos[1],0);
    //draw terrain edge
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //float dist_sep=30.0;
    for(int chain_i=0;chain_i<(int)m_vec_vec_terrain_points.size();chain_i++)
    {
        glBegin(GL_QUADS);
        for(int point_i=0;point_i<(int)m_vec_vec_terrain_points[chain_i].size()-1;point_i++)
        {
            //above, with shadow
            glColor4f(quad_color_dark_edge[0]*terrain_color[0],
                      quad_color_dark_edge[1]*terrain_color[1],
                      quad_color_dark_edge[2]*terrain_color[2],
                      quad_color_dark_edge[3]);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y-
                       m_vec_vec_terrain_points[chain_i][point_i].height_var);
            glColor4f(quad_color_dark[0]*terrain_color[0],
                      quad_color_dark[1]*terrain_color[1],
                      quad_color_dark[2]*terrain_color[2],
                      quad_color_dark[3]);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);

            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y);
            glColor4f(quad_color_dark_edge[0]*terrain_color[0],
                      quad_color_dark_edge[1]*terrain_color[1],
                      quad_color_dark_edge[2]*terrain_color[2],
                      quad_color_dark_edge[3]);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y-
                       m_vec_vec_terrain_points[chain_i][point_i+1].height_var);

            //below
            glColor4f(quad_color_light[0]*terrain_color[0],
                      quad_color_light[1]*terrain_color[1],
                      quad_color_light[2]*terrain_color[2],
                      quad_color_light[3]);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y);
            glColor4f(quad_color_light_edge[0]*terrain_color[0],
                      quad_color_light_edge[1]*terrain_color[1],
                      quad_color_light_edge[2]*terrain_color[2],
                      quad_color_light_edge[3]);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i].pos_y+
                       m_vec_vec_terrain_points[chain_i][point_i].height_var);

            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y+
                       m_vec_vec_terrain_points[chain_i][point_i+1].height_var);
            glColor4f(quad_color_light[0]*terrain_color[0],
                      quad_color_light[1]*terrain_color[1],
                      quad_color_light[2]*terrain_color[2],
                      quad_color_light[3]);
            glVertex2f(m_vec_vec_terrain_points[chain_i][point_i+1].pos_x,
                       m_vec_vec_terrain_points[chain_i][point_i+1].pos_y);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

    glPopMatrix();

    return true;
}

bool game::draw_projectiles(void)
{
    float bullet_size=3.0;
    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_tex_decal);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    b2Body* tmp=m_pWorld->GetBodyList();
    while(tmp)
    {
        st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
        if(data->s_info=="projectile")
        {
            switch(data->i_id)
            {
                case wt_rocket:
                {
                    //glVertex2f(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix);

                    glPushMatrix();
                    glTranslatef(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix,0.0);
                    glRotatef(tmp->GetAngle()*_Rad2Deg,0.0,0.0,1.0);
                    glBegin(GL_QUADS);
                    glTexCoord2f( (299.5-3.0)/1024.0, (1024.0-140.5+5.0)/1024.0 );
                    glVertex2f(-3.0,-5.0);
                    glTexCoord2f( (299.5-3.0)/1024.0, (1024.0-140.5-5.0)/1024.0 );
                    glVertex2f(-3.0,5.0);
                    glTexCoord2f( (299.5+3.0)/1024.0, (1024.0-140.5-5.0)/1024.0 );
                    glVertex2f(3.0,5.0);
                    glTexCoord2f( (299.5+3.0)/1024.0, (1024.0-140.5+5.0)/1024.0 );
                    glVertex2f(3.0,-5.0);
                    glEnd();
                    glPopMatrix();
                }break;

                case wt_grenade:
                {
                    //glVertex2f(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix);

                    glPushMatrix();
                    glTranslatef(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix,0.0);
                    glRotatef(tmp->GetAngle()*_Rad2Deg,0.0,0.0,1.0);
                    glBegin(GL_QUADS);
                    glTexCoord2f( (310.5-3.5)/1024.0, (1024.0-140.5+3.5)/1024.0 );
                    glVertex2f(-3.5,-3.5);
                    glTexCoord2f( (310.5-3.5)/1024.0, (1024.0-140.5-3.5)/1024.0 );
                    glVertex2f(-3.5,3.5);
                    glTexCoord2f( (310.5+3.5)/1024.0, (1024.0-140.5-3.5)/1024.0 );
                    glVertex2f(3.5,3.5);
                    glTexCoord2f( (310.5+3.5)/1024.0, (1024.0-140.5+3.5)/1024.0 );
                    glVertex2f(3.5,-3.5);
                    glEnd();
                    glPopMatrix();
                }break;

                case wt_mine:
                {
                    //glVertex2f(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix);

                    glPushMatrix();
                    glTranslatef(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix,0.0);
                    glRotatef(tmp->GetAngle()*_Rad2Deg,0.0,0.0,1.0);
                    glBegin(GL_QUADS);
                    glTexCoord2f( (289.5-4.5)/1024.0, (1024.0-141.5+4.5)/1024.0 );
                    glVertex2f(-4.5,-4.5);
                    glTexCoord2f( (289.5-4.5)/1024.0, (1024.0-141.5-4.5)/1024.0 );
                    glVertex2f(-4.5,4.5);
                    glTexCoord2f( (289.5+4.5)/1024.0, (1024.0-141.5-4.5)/1024.0 );
                    glVertex2f(4.5,4.5);
                    glTexCoord2f( (289.5+4.5)/1024.0, (1024.0-141.5+4.5)/1024.0 );
                    glVertex2f(4.5,-4.5);
                    glEnd();
                    glPopMatrix();
                }break;

                default://normal projectile
                {
                    //glVertex2f(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix);

                    glPushMatrix();
                    glTranslatef(tmp->GetPosition().x*_Met2Pix,tmp->GetPosition().y*_Met2Pix,0.0);
                    //glRotatef(tmp->GetAngle()*_Rad2Deg,0.0,0.0,1.0);
                    glBegin(GL_QUADS);
                    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                    glVertex2f(-bullet_size,-bullet_size);
                    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                    glVertex2f(-bullet_size,bullet_size);
                    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                    glVertex2f(bullet_size,bullet_size);
                    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                    glVertex2f(bullet_size,-bullet_size);
                    glEnd();
                    glPopMatrix();

                }break;
            }
        }

        tmp=tmp->GetNext();
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    return true;
}

bool game::require_convoy_test(void)
{
    //no convoys on the tutorial mission
    if(m_on_tutorial_level) return false;

    //test if any fuel objects left
    if(!m_vec_objects.empty()) return false;

    //test if current fuel is enough to go to next station
    if( m_fuel_curr>m_Starmap.get_dist_to_closest_planet_with_fuel() )
    {
        return false;
    }

    return true;//convoy required
}

bool game::create_convoy(void)
{
    cout<<"Convoy: Creating convoy\n";
    int numof_fuel_lifters=1+rand()%5;
    int numof_enemies=1+rand()%10;

    //set start point
    bool start_left=false;
    if(rand()%2==0) start_left=true;
    float start_x=m_level_soft_borders[1]*_Pix2Met;
    if(start_left) start_x=m_level_soft_borders[0]*_Pix2Met;
    float start_y=m_level_highest_point-10.0;

    //set end point
    float end_x=m_level_soft_borders[0];
    if(start_left) end_x=m_level_soft_borders[1]*_Pix2Met;
    float end_y=m_level_highest_point-10.0;

    float enemy_spacing=-5.0;
    if(start_left) enemy_spacing=5;

    //spawn enemies
    int convoy_enemy_types[11]={et_burst_bot,
                                et_auto_flat,
                                et_flipper,
                                et_rocket_tank,
                                et_grenade_ship,
                                et_cannon_tank,
                                et_beamer,
                                et_cloaker,
                                et_scanner,
                                et_flying_turret,
                                et_aim_bot};
    int spawn_counter=0;
    while(true)
    {
        if(numof_fuel_lifters<=0 && numof_enemies<=0) break;//done with spawning

        //carrier or enemy
        bool spawn_lifter=false;
        if(rand()%2==0) spawn_lifter=true;

        if(spawn_lifter)
        {
            if(numof_fuel_lifters<=0) continue;//no lifters left to spawn
            numof_fuel_lifters--;
            spawn_counter++;

            int enemy_type=et_lifter;
            b2Vec2 enemy_pos=b2Vec2(start_x+(float)spawn_counter*enemy_spacing,start_y);
            m_vec_pEnemies.push_back( new enemy() );
            m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                        enemy_pos,
                                        enemy_type,m_tex_decal);
            //set convoy mode
            m_vec_pEnemies.back()->set_convoy_pos( b2Vec2(end_x,end_y) );

            //spawn fuel object
            b2Vec2 fuel_pos=b2Vec2(start_x+(float)spawn_counter*enemy_spacing,start_y+0.7);
            //get size, 0.2-0.8
            float fuel_size=float(rand()%100)/166.0+0.2;
            //get fuel content, 0-100
            float fuel_content=float(rand()%80)+20;

            m_vec_objects.push_back( object() );
            m_vec_objects.back().init(m_pWorld,m_tex_decal,m_id_counter_object++,
                                      fuel_size,//size
                                      fuel_pos,
                                      fuel_content);//fuel content

            //link fuel to lifter
            m_vec_pEnemies.back()->force_hook( m_vec_objects.back().get_object_ptr(),b2Vec2(0.0,-fuel_size) );
        }
        else//spawn enemy
        {
            if(numof_enemies<=0) continue;//no enemies left to spawn
            numof_enemies--;
            spawn_counter++;

            int enemy_type=convoy_enemy_types[ rand()%11 ];
            m_vec_pEnemies.push_back( new enemy() );
            m_vec_pEnemies.back()->init(m_pWorld,m_pSound,m_pParticle_engine,
                                        b2Vec2( start_x+(float)spawn_counter*enemy_spacing,start_y ),
                                        enemy_type,m_tex_decal);
            //set convoy mode
            m_vec_pEnemies.back()->set_convoy_pos( b2Vec2(end_x,end_y) );
        }
    }

    return true;
}

bool game::reset(void)
{
    cout<<"Game reset\n";

    //remove player equipment
    for(int i=0;i<4;i++)
    {
        //remove weapon, default and current
        if(m_vec_players[i].get_weapon_ptr()==m_vec_players[i].get_default_weapon_ptr())
        {
            //same
            delete m_vec_players[i].get_weapon_ptr();
        }
        else//remove both
        {
            delete m_vec_players[i].get_weapon_ptr();
            delete m_vec_players[i].get_default_weapon_ptr();
        }

        //remove gear, default and current
        if(m_vec_players[i].get_gear_ptr()==m_vec_players[i].get_default_gear_ptr())
        {
            //same
            delete m_vec_players[i].get_gear_ptr();
        }
        else//remove both
        {
            delete m_vec_players[i].get_gear_ptr();
            delete m_vec_players[i].get_default_gear_ptr();
        }
    }

    //remove players
    m_vec_players.clear();

    //remove objects
    m_vec_objects.clear();

    //remove enemies
    for(int i=0;i<(int)m_vec_pEnemies.size();i++)
    {
        m_vec_pEnemies[i]->delete_equipment();
    }
    m_vec_pEnemies.clear();

    //remove stored weapons
    for(int i=0;i<(int)m_vec_pWeapon_stored.size();i++)
    {
        delete m_vec_pWeapon_stored[i];
    }
    m_vec_pWeapon_stored.clear();

    //remove stored gear
    for(int i=0;i<(int)m_vec_pGear_stored.size();i++)
    {
        delete m_vec_pGear_stored[i];
    }
    m_vec_pGear_stored.clear();

    //remove particle engine
    delete m_pParticle_engine;

    //clear vectors
    m_vec_terrain.clear();
    m_vec_projectiles_to_remove.clear();
    m_vec_played_levels.clear();
    m_vec_stars.clear();

    //remove joints
    b2Joint* tmp_joint=m_pWorld->GetJointList();
    while(tmp_joint)
    {
        b2Joint* tmp_joint_to_remove=tmp_joint;
        tmp_joint=tmp_joint->GetNext();

        m_pWorld->DestroyJoint(tmp_joint_to_remove);
    }

    //remove bodies
    b2Body* tmp_body=m_pWorld->GetBodyList();
    while(tmp_body)
    {
        //destroy userdata
        st_body_user_data* data=(st_body_user_data*)tmp_body->GetUserData();
        delete data;

        b2Body* tmp_body_to_remove=tmp_body;
        tmp_body=tmp_body->GetNext();

        m_pWorld->DestroyBody(tmp_body_to_remove);
    }

    //remove world
    delete m_pWorld;

    //stop all sounds
    for(int sound_i=0;sound_i<_numof_sound_sources;sound_i++)
    {
        m_pSound->stopSound(sound_i);
    }

    return true;
}

bool game::lost_test(void)
{
    if(!m_show_lost)
    {
        //any active players
        for(int player_i=0;player_i<4;player_i++)
         if( m_player_active[player_i] ) return false;

        //no active players, test if more gamepads than destroyed drones
        int gamepad_counter=0;
        for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
         if(m_gamepad_connected[gamepad_i]) gamepad_counter++;

        int destroyed_drone_counter=0;
        for(int player_i=0;player_i<4;player_i++)
         if(m_vec_players[player_i].get_drone_mode()==dm_destroyed) destroyed_drone_counter++;

        if(gamepad_counter>destroyed_drone_counter) return false;

        //game is lost
        m_show_lost=true;
        m_fade_off=true;

        //stop loop sounds
        for(int sound_i=10;sound_i<30;sound_i++) m_pSound->set_volume(sound_i,0.0);
        //disable sounds
        m_pSound->enable_sound(false);

        //erase saved game state
        remove(m_save_filename.c_str());

        return true;
    }

    return false;
}

bool game::save_game_to_file(void)
{
    cout<<"Saving world to save file\n";

    //create or open save file (overwritten every time)
    ofstream save_file( m_save_filename.c_str() );
    if(save_file==0)
    {
        cout<<"ERROR: Could not create save file\n";
        return false;
    }
    string game_data;

    //store mship info
    game_data.append("Mship ");
    game_data.append( num_to_string(m_pMain_ship->m_fuel_tank_curr) );
    game_data.append(" ");
    game_data.append( num_to_string(m_pMain_ship->m_fuel_tank_max) );
    game_data.append(" ");
    game_data.append( num_to_string(m_pMain_ship->m_resources_curr) );
    game_data.append(" ");
    game_data.append( num_to_string(m_pMain_ship->m_resources_max) );
    game_data.append("\n");
    //gear, stored
    for(unsigned int gear_i=0;gear_i<m_vec_pGear_stored.size();gear_i++)
    {
        game_data.append("gear ");
        game_data.append( num_to_string(m_vec_pGear_stored[gear_i]->get_type()) );
        if(m_vec_pGear_stored[gear_i]->get_type()==gt_shield)
        {
            float shield_data[3]={1,1,1};
            m_vec_pGear_stored[gear_i]->get_shield_data(shield_data);
            game_data.append(" ");
            game_data.append( num_to_string(shield_data[0]) );
            game_data.append(" ");
            game_data.append( num_to_string(shield_data[1]) );
            game_data.append(" ");
            game_data.append( num_to_string(shield_data[2]) );
        }
        game_data.append("\n");
    }
    //gear from players
    for(unsigned int player_i=0;player_i<4;player_i++)
    {
        if( m_vec_players[player_i].get_gear_ptr()!=m_vec_players[player_i].get_default_gear_ptr() )
        {
            game_data.append("gear ");
            game_data.append( num_to_string(m_vec_players[player_i].get_gear_ptr()->get_type()) );
            if(m_vec_players[player_i].get_gear_ptr()->get_type()==gt_shield)
            {
                float shield_data[3]={1,1,1};
                m_vec_players[player_i].get_gear_ptr()->get_shield_data(shield_data);
                game_data.append(" ");
                game_data.append( num_to_string(shield_data[0]) );
                game_data.append(" ");
                game_data.append( num_to_string(shield_data[1]) );
                game_data.append(" ");
                game_data.append( num_to_string(shield_data[2]) );
            }
            game_data.append("\n");
        }
    }
    //weapon, stored
    for(unsigned int weapon_i=0;weapon_i<m_vec_pWeapon_stored.size();weapon_i++)
    {
        game_data.append("weapon ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->get_type()) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_subtype) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_burst_counter_max) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_bullet_damage) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_bullet_speed) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_ammo_cost_per_shot) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_spread_factor) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_grenade_timer) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_range_max) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_pWeapon_stored[weapon_i]->m_beam_damage) );

        game_data.append("\n");
    }
    //weapons from players
    for(unsigned int player_i=0;player_i<4;player_i++)
    {
        if( m_vec_players[player_i].get_weapon_ptr()!=m_vec_players[player_i].get_default_weapon_ptr() )
        {
            game_data.append("weapon ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->get_type()) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_subtype) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_burst_counter_max) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_bullet_damage) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_bullet_speed) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_ammo_cost_per_shot) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_spread_factor) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_grenade_timer) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_range_max) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_players[player_i].get_weapon_ptr()->m_beam_damage) );

            game_data.append("\n");
        }
    }

    //store player info
    for(unsigned int player_i=0;player_i<4;player_i++)
    {
        game_data.append("player ");
        game_data.append( num_to_string(player_i) );
        game_data.append(" ");
        game_data.append( num_to_string( m_vec_players[player_i].m_motor_thrust_power_max ) );
        game_data.append(" ");
        game_data.append( num_to_string( m_vec_players[player_i].m_hp_max ) );
        game_data.append(" ");
        game_data.append( num_to_string( m_vec_players[player_i].m_fuel_max ) );
        game_data.append(" ");
        game_data.append( num_to_string( m_vec_players[player_i].m_ammo_max ) );
        game_data.append(" ");
        game_data.append( num_to_string( m_vec_players[player_i].m_upgrade_counter ) );

        game_data.append("\n");
    }

    //store starmap info
    m_Starmap.save_data(game_data);

    //encrypt
    for(unsigned int i=0;i<game_data.size();i++)
    {
        game_data[i]+=m_save_key[i%m_save_key.size()];
    }


    save_file<<game_data;
    save_file.close();

    return true;
}

bool game::load_game_from_file(void)
{
    cout<<"Restoring world from save file\n";

    //open save file
    ifstream save_file(m_save_filename.c_str());
    if(save_file==0)
    {
        cout<<"ERROR: Could not find save file\n";
        return false;
    }
    string game_data,line;
    while(getline(save_file,line))
    {
        game_data.append(line);
        game_data.append("\n");
    }
    save_file.close();


    //decrypt
    for(unsigned int i=0;i<game_data.size();i++)
    {
        game_data[i]-=m_save_key[i%m_save_key.size()];
    }


    //restore mship and player info
    string word;
    stringstream ss(game_data);
    while(ss>>word)
    {
        if(word=="Mship")
        {
            ss>>word;
            m_pMain_ship->m_fuel_tank_curr=atof( word.c_str() );
            ss>>word;
            m_pMain_ship->m_fuel_tank_max=atof( word.c_str() );
            ss>>word;
            m_pMain_ship->m_resources_curr=atof( word.c_str() );
            ss>>word;
            m_pMain_ship->m_resources_max=atof( word.c_str() );
        }
        else if(word=="gear")
        {
            ss>>word;
            int gear_type=atof( word.c_str() );
            m_vec_pGear_stored.push_back( new gear(m_pWorld,gear_type,1) );
            //settings
            if(gear_type==gt_shield)
            {
                ss>>word;
                m_vec_pGear_stored.back()->m_shield_regen_delay=atof( word.c_str() );
                ss>>word;
                m_vec_pGear_stored.back()->m_shield_hp_max=atof( word.c_str() );
                ss>>word;
                m_vec_pGear_stored.back()->m_shield_regen_speed=atof( word.c_str() );
            }
        }
        else if(word=="weapon")
        {
            ss>>word;
            int weapon_type=atof( word.c_str() );

            m_vec_pWeapon_stored.push_back( new weapon(m_pWorld,weapon_type,1) );
            //settings
            ss>>word;
            m_vec_pWeapon_stored.back()->m_subtype=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_burst_counter_max=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_bullet_damage=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_bullet_speed=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_ammo_cost_per_shot=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_spread_factor=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_grenade_timer=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_range_max=atof( word.c_str() );
            ss>>word;
            m_vec_pWeapon_stored.back()->m_beam_damage=atof( word.c_str() );
        }
        else if(word=="player")
        {
            ss>>word;
            int player_ind=atof( word.c_str() );
            ss>>word;
            m_vec_players[player_ind].m_motor_thrust_power_max=atof( word.c_str() );
            ss>>word;
            m_vec_players[player_ind].m_hp_max=atof( word.c_str() );
            ss>>word;
            m_vec_players[player_ind].m_fuel_max=atof( word.c_str() );
            ss>>word;
            m_vec_players[player_ind].m_ammo_max=atof( word.c_str() );
            ss>>word;
            m_vec_players[player_ind].m_upgrade_counter=atof( word.c_str() );
        }
    }

    //restore starmap
    int screen_size[2]={m_screen_width,m_screen_height};
    if(!m_Starmap.init(screen_size,m_tex_menu,m_pSound,game_data))
    {
        cout<<"ERROR: Could not initialize starmap\n";
        return false;
    }

    return true;
}
