#include "star_map.h"

star_map::star_map()
{
    //set default values
    m_min_planet_dist=5;
    m_max_planet_dist=100;//?
    m_min_numof_planets=2000;
    m_max_numof_planets=10000;
    m_stored_time=0;
}

bool star_map::init(int screen_size[2],int texture,sound* pSound,string file_data)
{
    cout<<"Initializing Starmap\n";

    m_pSound=pSound;

    m_vec_planets.clear();
    m_vec_planets_ind_within_scan_range.clear();

    m_screen_width=screen_size[0];
    m_screen_height=screen_size[1];
    m_texture=texture;
    m_cam_speed[0]=m_cam_speed[1]=0;
    m_cam_pos[0]=m_cam_pos[1]=0;
    m_zoom_level=_starmap_zoom_max;
    m_planet_start=m_planet_now=m_planet_selected=m_planet_selection_ind=0;
    m_travel_pack_up_now=m_planet_travel_now=m_travel_done_set_up_now=m_travel_towards_goal=false;
    m_max_numof_enemy_content=10;
    m_max_numof_fuel_content=10;
    m_curr_planet_enemy_content_drag=m_curr_planet_fuel_content_drag=0;
    m_world_rotation=m_radar_sweep_time=0.0;
    m_travel_pack_up_time=m_travel_done_set_up_time=0.0;

    //create starmap
    if(file_data.empty())
    {
        //place planets
        m_numof_planets=rand()%(m_max_numof_planets-m_min_numof_planets)+m_min_numof_planets;
        for(int planet_i=0;planet_i<m_numof_planets;planet_i++)
        {
            //radial distribution
            float R=rand()%(int)_starmap_radius_world;
            if(R<1000) R=1000;
            float a=(rand()%1000)/1000.0;
            float b=(rand()%1000)/1000.0;
            if(b<a)
            {
                float c=0.0;
                //swap
                c=a;
                a=b;
                b=c;
            }
            float x=b*R*cos(2.0*_pi*a/b);
            float y=b*R*sin(2.0*_pi*a/b);


            st_planet planet_temp;

            planet_temp.pos[0]=x;
            planet_temp.pos[1]=y;

            planet_temp.flash_speed=float(rand()%100)/50.0;

            m_vec_planets.push_back(planet_temp);
        }

        //measure distances to other planets
        for(int planet_i1=0;planet_i1<(int)m_vec_planets.size();planet_i1++)
        {
            float shortest_dist=sqrt( (m_vec_planets[planet_i1].pos[0]-m_vec_planets[0].pos[0])*
                                      (m_vec_planets[planet_i1].pos[0]-m_vec_planets[0].pos[0]) +
                                      (m_vec_planets[planet_i1].pos[1]-m_vec_planets[0].pos[1])*
                                      (m_vec_planets[planet_i1].pos[1]-m_vec_planets[0].pos[1]) );
            int   shortest_ind=0;
            for(int planet_i2=0;planet_i2<(int)m_vec_planets.size();planet_i2++)
            {
                if(planet_i1==planet_i2) continue;

                float dist=sqrt( (m_vec_planets[planet_i1].pos[0]-m_vec_planets[planet_i2].pos[0])*
                                 (m_vec_planets[planet_i1].pos[0]-m_vec_planets[planet_i2].pos[0]) +
                                 (m_vec_planets[planet_i1].pos[1]-m_vec_planets[planet_i2].pos[1])*
                                 (m_vec_planets[planet_i1].pos[1]-m_vec_planets[planet_i2].pos[1]) );
                if(dist<shortest_dist)
                {
                    //test if shorter than minimum
                    if(dist<m_min_planet_dist)
                    {
                        //remove planet
                        m_vec_planets.erase( m_vec_planets.begin()+planet_i2 );
                        planet_i2--;
                    }
                    else//store new shortest dist
                    {
                        shortest_dist=dist;
                        shortest_ind=planet_i2;
                    }
                }
            }

            m_vec_planets[planet_i1].dist_to_closest_planet=shortest_dist;
        }

        //test distances from current planet to others
        int numof_planets_within_scan_range=5;
        bool pos_found=false;
        while(!pos_found)
        {
            int planets_within_range=0;

            //place start planet
            float dist_to_center=rand()%(int)_starmap_radius_tol_start_planet+_starmap_radius_start_planet;
            float planet_angle=float(rand()%360);
            float x_pos=dist_to_center*cosf(planet_angle*_Deg2Rad);
            float y_pos=dist_to_center*sinf(planet_angle*_Deg2Rad);

            for(int planet_i=0;planet_i<(int)m_vec_planets.size();planet_i++)
            {
                float dist=sqrt( (m_vec_planets[planet_i].pos[0]-x_pos)*
                                 (m_vec_planets[planet_i].pos[0]-x_pos) +
                                 (m_vec_planets[planet_i].pos[1]-y_pos)*
                                 (m_vec_planets[planet_i].pos[1]-y_pos) );
                if(dist<_mship_scan_range)
                {
                    planets_within_range++;
                    if(planets_within_range>=numof_planets_within_scan_range)
                    {
                        //test if one within fuel range
                        for(int planet_i2=0;planet_i2<(int)m_vec_planets.size();planet_i2++)
                        {
                            float dist2=sqrt( (m_vec_planets[planet_i2].pos[0]-x_pos)*
                                              (m_vec_planets[planet_i2].pos[0]-x_pos) +
                                              (m_vec_planets[planet_i2].pos[1]-y_pos)*
                                              (m_vec_planets[planet_i2].pos[1]-y_pos) );
                            if(dist2<_mship_fuel_start)
                            {
                                //test if other planet is too near startpos
                                pos_found=true;
                                st_planet planet_temp;
                                planet_temp.pos[0]=x_pos;
                                planet_temp.pos[1]=y_pos;
                                planet_temp.flash_speed=float(rand()%100)/50.0;
                                m_vec_planets.push_back(planet_temp);
                                m_planet_start=m_planet_now=m_planet_selected=(int)m_vec_planets.size()-1;

                                //place goal planet
                                st_planet planet_temp2;
                                planet_temp2.pos[0]=dist_to_center*cosf((planet_angle-180.0)*_Deg2Rad);;
                                planet_temp2.pos[1]=dist_to_center*sinf((planet_angle-180.0)*_Deg2Rad);;
                                planet_temp2.flash_speed=float(rand()%100)/50.0;
                                m_vec_planets.push_back(planet_temp2);
                                m_planet_goal=(int)m_vec_planets.size()-1;

                                break;
                            }
                        }

                        break;
                    }
                }
            }
        }

        //place route to goal planets
        float pos_curr_x=m_vec_planets[m_planet_now].pos[0];
        float pos_curr_y=m_vec_planets[m_planet_now].pos[1];
        float pos_goal_x=m_vec_planets[m_planet_goal].pos[0];
        float pos_goal_y=m_vec_planets[m_planet_goal].pos[1];
        //cout<<"Goal pos: "<<pos_goal_x<<", "<<pos_goal_y<<endl;
        for(int tryi=0;tryi<1000;tryi++)
        {
            //cout<<"Route planet: "<<pos_curr_x<<", "<<pos_curr_y;;
            float goal_dir_x=pos_goal_x-pos_curr_x;
            float goal_dir_y=pos_goal_y-pos_curr_y;
            float goal_dist=sqrt(goal_dir_x*goal_dir_x+goal_dir_y*goal_dir_y);
            if(goal_dist==0) break;//placed ontop of the goal station
            goal_dir_x/=goal_dist;
            goal_dir_y/=goal_dist;
            //calc angle
            float angle=atan2(goal_dir_y,goal_dir_x)*_Rad2Deg;
            //cout<<"\t"<<angle<<endl;
            //give random varation
            angle+=(rand()%140)-70;
            //calc pos of new planet, within 90 % distance of current pos
            pos_curr_x+=cosf(angle*_Deg2Rad)*_mship_scan_range*0.9;
            pos_curr_y+=sinf(angle*_Deg2Rad)*_mship_scan_range*0.9;
            //place that planet
            st_planet planet_temp;
            planet_temp.pos[0]=pos_curr_x;
            planet_temp.pos[1]=pos_curr_y;
            planet_temp.flash_speed=float(rand()%100)/50.0;
            m_vec_planets.push_back(planet_temp);
            //measure distance to goal planet
            float dist_to_goal=sqrt( (pos_goal_x-pos_curr_x)*(pos_goal_x-pos_curr_x)+
                                     (pos_goal_y-pos_curr_y)*(pos_goal_y-pos_curr_y) );
            if(dist_to_goal<_mship_scan_range*0.9)
            {
                //done with route
                break;
            }

            /*//test if to close to any other planet, for removal
            for(unsigned int planet_i=1;planet_i<m_vec_planets.size()-1;planet_i++)//-1 to avoid test with itself
            {
                float dist=sqrt( (m_vec_planets.back().pos[0]-m_vec_planets[planet_i].pos[0])*
                                 (m_vec_planets.back().pos[0]-m_vec_planets[planet_i].pos[0]) +
                                 (m_vec_planets.back().pos[1]-m_vec_planets[planet_i].pos[1])*
                                 (m_vec_planets.back().pos[1]-m_vec_planets[planet_i].pos[1]) );
                if(dist<m_min_planet_dist)
                {
                    //remove that planet
                    m_vec_planets.erase( m_vec_planets.begin()+planet_i );
                    planet_i--;
                }
            }*/
        }

        //place fuel to match distances (some average)
        for(unsigned int planet_i=0;planet_i<m_vec_planets.size();planet_i++)
        {
            m_vec_planets[planet_i].fuel_content=rand()%(int)m_max_numof_fuel_content+1;
        }

        //place enemies to match fuel content
        for(unsigned int planet_i=0;planet_i<m_vec_planets.size();planet_i++)
        {
            m_vec_planets[planet_i].enemy_content=rand()%(int)m_max_numof_fuel_content+1;
            //reroll try, if fewer enemies than fuel
            if(m_vec_planets[planet_i].enemy_content<m_vec_planets[planet_i].fuel_content)
            {
                m_vec_planets[planet_i].enemy_content=rand()%(int)m_max_numof_fuel_content+1;
            }
        }

        //give texture and color index
        for(unsigned int planet_i=0;planet_i<m_vec_planets.size();planet_i++)
        {
            int color_index=rand()%_starmap_numof_level_colors;
            switch(color_index)
            {
                case 0:
                {
                    m_vec_planets[planet_i].level_color[0]=1.0;
                    m_vec_planets[planet_i].level_color[1]=1.0;
                    m_vec_planets[planet_i].level_color[2]=1.0;
                }break;

                case 1:
                {
                    m_vec_planets[planet_i].level_color[0]=1.0;
                    m_vec_planets[planet_i].level_color[1]=0.6;
                    m_vec_planets[planet_i].level_color[2]=0.4;
                }break;

                case 2:
                {
                    m_vec_planets[planet_i].level_color[0]=0.6;
                    m_vec_planets[planet_i].level_color[1]=0.6;
                    m_vec_planets[planet_i].level_color[2]=0.2;
                }break;

                case 3:
                {
                    m_vec_planets[planet_i].level_color[0]=0.7;
                    m_vec_planets[planet_i].level_color[1]=0.7;
                    m_vec_planets[planet_i].level_color[2]=0.9;
                }break;

                case 4:
                {
                    m_vec_planets[planet_i].level_color[0]=0.7;
                    m_vec_planets[planet_i].level_color[1]=0.7;
                    m_vec_planets[planet_i].level_color[2]=0.9;
                }break;

                case 5:
                {
                    m_vec_planets[planet_i].level_color[0]=0.9;
                    m_vec_planets[planet_i].level_color[1]=0.7;
                    m_vec_planets[planet_i].level_color[2]=0.7;
                }break;

                case 6:
                {
                    m_vec_planets[planet_i].level_color[0]=0.3;
                    m_vec_planets[planet_i].level_color[1]=0.2;
                    m_vec_planets[planet_i].level_color[2]=0.2;
                }break;

                case 7:
                {
                    m_vec_planets[planet_i].level_color[0]=0.2;
                    m_vec_planets[planet_i].level_color[1]=0.3;
                    m_vec_planets[planet_i].level_color[2]=0.2;
                }break;

                case 8:
                {
                    m_vec_planets[planet_i].level_color[0]=0.2;
                    m_vec_planets[planet_i].level_color[1]=0.3;
                    m_vec_planets[planet_i].level_color[2]=0.2;
                }break;

                case 9:
                {
                    m_vec_planets[planet_i].level_color[0]=0.6;
                    m_vec_planets[planet_i].level_color[1]=0.4;
                    m_vec_planets[planet_i].level_color[2]=0.3;
                }break;

                case 10:
                {
                    m_vec_planets[planet_i].level_color[0]=0.7;
                    m_vec_planets[planet_i].level_color[1]=0.7;
                    m_vec_planets[planet_i].level_color[2]=0.7;
                }break;

                case 11:
                {
                    m_vec_planets[planet_i].level_color[0]=0.8;
                    m_vec_planets[planet_i].level_color[1]=0.8;
                    m_vec_planets[planet_i].level_color[2]=0.8;
                }break;

                case 12:
                {
                    m_vec_planets[planet_i].level_color[0]=0.5;
                    m_vec_planets[planet_i].level_color[1]=0.3;
                    m_vec_planets[planet_i].level_color[2]=0.0;
                }break;

                case 13:
                {
                    m_vec_planets[planet_i].level_color[0]=0.6;
                    m_vec_planets[planet_i].level_color[1]=0.6;
                    m_vec_planets[planet_i].level_color[2]=0.6;
                }break;

                case 14:
                {
                    m_vec_planets[planet_i].level_color[0]=0.5;
                    m_vec_planets[planet_i].level_color[1]=0.5;
                    m_vec_planets[planet_i].level_color[2]=0.5;
                }break;

                default:
                {
                    m_vec_planets[planet_i].level_color[0]=1.0;
                    m_vec_planets[planet_i].level_color[1]=1.0;
                    m_vec_planets[planet_i].level_color[2]=1.0;
                }
            }

            m_vec_planets[planet_i].level_texture_index=rand()%_starmap_numof_level_textures;
        }
        //set tutorial color/terrain to default
        m_vec_planets[m_planet_now].level_texture_index=0;
        m_vec_planets[m_planet_now].level_color[0]=1.0;
        m_vec_planets[m_planet_now].level_color[1]=1.0;
        m_vec_planets[m_planet_now].level_color[2]=1.0;
    }
    else
    //create starmap from file
    {
        cout<<"Starmap: Loading data from file\n";
        //find "Starmap"
        int starmap_pos=-1;
        for(unsigned int i=0;i<file_data.length()-7;i++)
        {
            if(file_data[i+0]=='S'&&file_data[i+1]=='t'&&file_data[i+2]=='a'&&file_data[i+3]=='r'&&
               file_data[i+4]=='m'&&file_data[i+5]=='a'&&file_data[i+6]=='p'&&file_data[i+7]==' ')
            {
                starmap_pos=i;
                break;
            }
        }
        if(starmap_pos==-1)
        {
            cout<<"ERROR: Could not load starmap from data file\n";
            return false;
        }
        string starmap_data=string(file_data,starmap_pos);
        stringstream ss(starmap_data);
        string word;
        ss>>word;//skip starmap
        ss>>word;
        m_planet_now=m_planet_selected=atof(word.c_str());
        ss>>word;
        m_planet_goal=atof(word.c_str());
        ss>>word;
        m_planet_start=atof(word.c_str());

        //place planets
        while(ss>>word)
        {
            //find planet info
            if(word=="Planet")
            {
                //save planet info
                st_planet temp_planet;
                ss>>word;
                temp_planet.pos[0]=atof(word.c_str());
                ss>>word;
                temp_planet.pos[1]=atof(word.c_str());
                ss>>word;
                temp_planet.fuel_content=atof(word.c_str());
                ss>>word;
                temp_planet.enemy_content=atof(word.c_str());
                ss>>word;
                if(word=="1")
                {
                    //save more planet info
                    temp_planet.world_initialized=true;
                    ss>>word;
                    temp_planet.level_index=atof(word.c_str());
                    ss>>word;
                    temp_planet.level_color[0]=atof(word.c_str());
                    ss>>word;
                    temp_planet.level_color[1]=atof(word.c_str());
                    ss>>word;
                    temp_planet.level_color[2]=atof(word.c_str());
                    ss>>word;
                    temp_planet.level_texture_index=atof(word.c_str());
                }
                else
                {
                    //generate missing info
                    temp_planet.world_initialized=false;
                    temp_planet.level_color[0]=1.0;
                    temp_planet.level_color[1]=1.0;
                    temp_planet.level_color[2]=1.0;
                    temp_planet.level_index=-1;
                    temp_planet.level_texture_index=0;
                }
                temp_planet.flash_speed=float(rand()%100)/50.0;

                //save
                m_vec_planets.push_back(temp_planet);
            }
        }

        //set start planet to tutorial location
        m_vec_planets[m_planet_start].level_index=-2;
    }

    //move cam to visiting planet
    m_cam_pos[0]=m_vec_planets[m_planet_now].pos[0]-m_screen_width*0.5;
    m_cam_pos[1]=m_vec_planets[m_planet_now].pos[1]-m_screen_height*0.5;

    m_numof_planets=(int)m_vec_planets.size();

    calc_planets_within_range(_mship_scan_range);
    //select current planet
    for(int planet_i=0;planet_i<(int)m_vec_planets_ind_within_scan_range.size();planet_i++)
    {
        if(m_vec_planets_ind_within_scan_range[planet_i]==m_planet_now)
        {
            m_planet_selection_ind=planet_i;
            break;
        }
    }

    return true;
}

int star_map::update(float time_dif,float& mship_fuel)
{
    m_stored_time+=time_dif;
    m_world_rotation+=time_dif*0.2;

    //calc pack up progress
    float pack_up_progress=m_travel_pack_up_time/_mship_travel_pack_up_time_max;
    if(!m_travel_pack_up_now) pack_up_progress=0.0;
    if(m_travel_done_set_up_now) pack_up_progress=1.0-m_travel_done_set_up_time/_mship_travel_pack_up_time_max;
    m_radar_sweep_time+=time_dif*20.0+time_dif*pack_up_progress*200.0;
    //m_radar_sweep_time+=time_dif*20.0;

    if(m_stored_time>360.0) m_stored_time-=360.0;
    if(m_world_rotation>360.0) m_world_rotation-=360.0;
    if(m_radar_sweep_time>360.0) m_radar_sweep_time-=360.0;

    //update travel
    if(m_travel_pack_up_now)
    {
        //update timer
        m_travel_pack_up_time+=time_dif;
        if(m_travel_pack_up_time>_mship_travel_pack_up_time_max)
        {
            m_travel_pack_up_time=_mship_travel_pack_up_time_max;
            m_travel_pack_up_now=false;
            m_planet_travel_now=true;
            m_travel_time=0.0;

            //play sound
            m_pSound->playSimpleSound(wav_starmap_travel,1.0,_sound_chan_starmap_travel,true);
        }
    }
    else if(m_planet_travel_now)
    {
        //consume fuel
        mship_fuel-=m_travel_fuel_cost*(time_dif/m_travel_time_max);
        //time_dif/_mship_travel_speed;
        //cout<<mship_fuel<<endl;

        //update timer
        m_travel_time+=time_dif;
        if(m_travel_time>m_travel_time_max)
        {
            //travel complete
            m_travel_time=m_travel_time_max;
            m_planet_travel_now=false;
            m_travel_done_set_up_now=true;
            m_travel_done_set_up_time=0.0;

            //stop sound
            m_pSound->stopSound(_sound_chan_starmap_travel);

            //play sound
            m_pSound->playSimpleSound(wav_starmap_startdown,1.0);

            //move ship
            m_planet_now=m_planet_selected;
            calc_planets_within_range(_mship_scan_range);
            //select current planet
            for(int planet_i=0;planet_i<(int)m_vec_planets_ind_within_scan_range.size();planet_i++)
            {
                if(m_vec_planets_ind_within_scan_range[planet_i]==m_planet_now)
                {
                    m_planet_selection_ind=planet_i;
                    break;
                }
            }
        }

        //update cam pos
        float cam_center_pos[2]={ m_cam_pos[0]+m_screen_width*0.5,
                                  m_cam_pos[1]+m_screen_height*0.5 };
        float travel_progress=m_travel_time/m_travel_time_max;
        float travel_x=travel_progress*(m_vec_planets[m_travel_planet_end].pos[0]-m_vec_planets[m_travel_planet_start].pos[0]);
        float travel_y=travel_progress*(m_vec_planets[m_travel_planet_end].pos[1]-m_vec_planets[m_travel_planet_start].pos[1]);
        float mship_pos[2]={ m_vec_planets[m_travel_planet_start].pos[0]+travel_x,
                             m_vec_planets[m_travel_planet_start].pos[1]+travel_y };
        for(int i=0;i<2;i++)
        {
            if(cam_center_pos[i]>mship_pos[i])
            {
                m_cam_pos[i]-=0.2;
                m_cam_speed[i]=-0.2;
            }
            else if(cam_center_pos[i]<mship_pos[i])
            {
                m_cam_pos[i]+=0.2;
                m_cam_speed[i]=0.2;
            }
        }

    }
    else if(m_travel_done_set_up_now)
    {
        //update timer
        m_travel_done_set_up_time+=time_dif;
        if(m_travel_done_set_up_time>_mship_travel_pack_up_time_max)
        {
            //set up complete
            m_travel_done_set_up_time=_mship_travel_pack_up_time_max;
            m_travel_done_set_up_now=false;

            //test if goal planet is reached
            if(m_travel_towards_goal) return 2;//game over
        }
    }

    m_mship_fuel=mship_fuel;

    //slow down cam speed
    for(int i=0;i<2;i++)
     if(m_cam_speed[i]!=0.0)
    {
        m_cam_pos[i]+=m_cam_speed[i];
        m_cam_speed[i]*=0.97;
        if(fabs(m_cam_speed[i])<_starmap_cam_speed_limit) m_cam_speed[i]=0.0;
    }

    for(int planet_i=0;planet_i<(int)m_vec_planets.size();planet_i++)
    {
        m_vec_planets[planet_i].brightness=(sinf(m_stored_time*m_vec_planets[planet_i].flash_speed)+1.0)/4.0+0.5;
    }

    //update planet fuel/enemy bar, with drag
    float drag_speed_fuel=0.3+0.2*(sinf(m_stored_time*sinf(m_stored_time*1.5)));
    float drag_speed_enemy=0.3+0.2*(sinf(m_stored_time*sinf(m_stored_time*0.2)));
    float curr_enemy_cont=0.0;
    if(!m_vec_planets_ind_within_scan_range.empty())
     curr_enemy_cont=m_vec_planets[m_vec_planets_ind_within_scan_range[m_planet_selection_ind]].enemy_content/m_max_numof_enemy_content;
    float curr_fuel_cont=0.0;
    if(!m_vec_planets_ind_within_scan_range.empty())
     curr_fuel_cont=m_vec_planets[m_vec_planets_ind_within_scan_range[m_planet_selection_ind]].fuel_content/m_max_numof_fuel_content;

    if(m_curr_planet_enemy_content_drag<curr_enemy_cont)
    {
        m_curr_planet_enemy_content_drag+=drag_speed_enemy*time_dif;
        if(m_curr_planet_enemy_content_drag>1.0) m_curr_planet_enemy_content_drag=1.0;
    }
    else if(m_curr_planet_enemy_content_drag>curr_enemy_cont)
    {
        m_curr_planet_enemy_content_drag-=drag_speed_enemy*time_dif;
    }
    if(m_curr_planet_fuel_content_drag<curr_fuel_cont)
    {
        m_curr_planet_fuel_content_drag+=drag_speed_fuel*time_dif;
        if(m_curr_planet_fuel_content_drag>1.0) m_curr_planet_fuel_content_drag=1.0;
    }
    else if(m_curr_planet_fuel_content_drag>curr_fuel_cont)
    {
        m_curr_planet_fuel_content_drag-=drag_speed_fuel*time_dif;
    }

    return 1;
}

bool star_map::draw(void)
{
    glPushMatrix();
    //glTranslatef(m_cam_pos[0],m_cam_pos[1],0);
    glTranslatef(m_screen_width*0.5,m_screen_height*0.5,0);
    glRotatef(m_world_rotation,0,0,1);
    //zoom compensation
    glTranslatef( -(1.0+m_zoom_level)*((m_cam_pos[0]+m_screen_width*0.5)/m_screen_width)*m_screen_width ,
                  -(1.0+m_zoom_level)*((m_cam_pos[1]+m_screen_height*0.5)/m_screen_height)*m_screen_height , 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    //draw background?

    //draw planets
    for(int planet_i=0;planet_i<(int)m_vec_planets.size();planet_i++)
    {
        if(m_vec_planets[planet_i].world_initialized)
        {
            glColor4f(m_vec_planets[planet_i].brightness,m_vec_planets[planet_i].brightness*0.2,m_vec_planets[planet_i].brightness*0.2,0.9);
            glPointSize(3);
        }
        else
        {
            glColor4f(m_vec_planets[planet_i].brightness,m_vec_planets[planet_i].brightness,m_vec_planets[planet_i].brightness,0.9);
            glPointSize(1);
        }

        glBegin(GL_POINTS);
        glVertex2f(m_vec_planets[planet_i].pos[0]*(1.0+m_zoom_level),m_vec_planets[planet_i].pos[1]*(1.0+m_zoom_level));
        glEnd();
    }

    //draw travel graphics
    if(m_planet_travel_now)
    {
        float travel_progress=m_travel_time/m_travel_time_max;
        float travel_x=travel_progress*(m_vec_planets[m_travel_planet_end].pos[0]-m_vec_planets[m_travel_planet_start].pos[0]);
        float travel_y=travel_progress*(m_vec_planets[m_travel_planet_end].pos[1]-m_vec_planets[m_travel_planet_start].pos[1]);
        //draw line between planets
        glColor3f(0.3,0.3,0.3);
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex2f(m_vec_planets[m_travel_planet_start].pos[0]*(1.0+m_zoom_level),
                   m_vec_planets[m_travel_planet_start].pos[1]*(1.0+m_zoom_level));
        glVertex2f(m_vec_planets[m_travel_planet_end].pos[0]*(1.0+m_zoom_level),
                   m_vec_planets[m_travel_planet_end].pos[1]*(1.0+m_zoom_level));
        glEnd();
        //draw progress
        glColor3f(0.9,0.9,0.9);
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2f(m_vec_planets[m_travel_planet_start].pos[0]*(1.0+m_zoom_level),
                   m_vec_planets[m_travel_planet_start].pos[1]*(1.0+m_zoom_level));
        glVertex2f((m_vec_planets[m_travel_planet_start].pos[0]+travel_x)*(1.0+m_zoom_level),
                   (m_vec_planets[m_travel_planet_start].pos[1]+travel_y)*(1.0+m_zoom_level) );
        glEnd();

        //draw fuel radius
        glTranslatef((m_vec_planets[m_planet_now].pos[0]+travel_x)*(1.0+m_zoom_level),
                     (m_vec_planets[m_planet_now].pos[1]+travel_y)*(1.0+m_zoom_level),0);
        float fuel_radius=m_mship_fuel*(1.0+m_zoom_level);
        glColor4f(0.7,0.7,0.0,0.2);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0,0);
        glColor4f(0.7,0.7,0.0,0.05);
        for(int i=0;i<=100;i++)
        {
            float angle=2*_pi*(float)i/100.0;
            //glColor4f(0.6,0.6,0.5*cosf(0.5*m_stored_time+(float)i*0.5),0.2);
            glVertex2f(fuel_radius*cosf(angle),fuel_radius*sinf(angle));
        }
        glEnd();
        glPopMatrix();
    }
    else//draw starmap HUD
    {
        //calc pack up progress
        float pack_up_progress=m_travel_pack_up_time/_mship_travel_pack_up_time_max;
        if(!m_travel_pack_up_now) pack_up_progress=0.0;
        if(m_travel_done_set_up_now) pack_up_progress=1.0-m_travel_done_set_up_time/_mship_travel_pack_up_time_max;

        //draw selected level info

        //go to planet now pos
        glLineWidth(1);
        glPushMatrix();
        glTranslatef(m_vec_planets[m_planet_now].pos[0]*(1.0+m_zoom_level),
                     m_vec_planets[m_planet_now].pos[1]*(1.0+m_zoom_level),0);

        /*//draw visiting planet circle
        float radius=(5.0+2.0*cosf(2.0*(m_stored_time+360.0)));//(1.0+m_zoom_level);
        glColor3f(0.6,0.2,0.1);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<=30;i++)
        {
            float angle=2*_pi*(float)i/30.0;
            glVertex2f(radius*cosf(angle),radius*sinf(angle));
        }
        glEnd();*/

        //draw visiting planet (bigger star)
        glPointSize(2);
        glColor3f(m_vec_planets[m_planet_now].brightness,m_vec_planets[m_planet_now].brightness,m_vec_planets[m_planet_now].brightness);
        glBegin(GL_POINTS);
        glVertex2f(0,0);
        glEnd();
        glPointSize(1);

        //draw fuel radius
        float fuel_radius=m_mship_fuel*(1.0+m_zoom_level);
        glColor4f(0.7,0.7,0.0,0.2);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0,0);
        glColor4f(0.7,0.7,0.0,0.05);
        for(int i=0;i<=100;i++)
        {
            float angle=2*_pi*(float)i/100.0;
            //glColor4f(0.6,0.6,0.5*cosf(0.5*m_stored_time+(float)i*0.5),0.2);
            glVertex2f(fuel_radius*cosf(angle),fuel_radius*sinf(angle));
        }
        glEnd();

        //draw scan range
        float scan_radius=_mship_scan_range*(1.0+m_zoom_level)*(1.0-pack_up_progress);
        //sweep radar
        float color1[4]={0.0,0.0,0.7,1};
        float color2[4]={0.0,0.0,0.7,0.0};
        float sweep_angle1=m_radar_sweep_time*_Deg2Rad;
        float sweep_angle2=(m_radar_sweep_time-15.0)*_Deg2Rad;
        glBegin(GL_QUAD_STRIP);
        glVertex2f(0,0);
        glVertex2f(0,0);
        for(int i=0;i<=5;i++)
        {
            float radius=scan_radius*float(i)/5.0;
            glColor4fv(color1);
            glVertex2f(radius*cosf(sweep_angle1),radius*sinf(sweep_angle1));
            glColor4fv(color2);
            glVertex2f(radius*cosf(sweep_angle2),radius*sinf(sweep_angle2));

            glColor4fv(color1);
            glVertex2f(radius*cosf(sweep_angle1),radius*sinf(sweep_angle1));
            glColor4fv(color2);
            glVertex2f(radius*cosf(sweep_angle2),radius*sinf(sweep_angle2));
        }
        glEnd();
        glColor4f(0.0,0.0,0.9,1);
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2f(0,0);
        glVertex2f(scan_radius*cosf(sweep_angle1),scan_radius*sinf(sweep_angle1));
        glEnd();

        //radar circle
        glColor4f(0.0,0.0,0.9,1);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<=100;i++)
        {
            float angle=2*_pi*(float)i/100.0;
            //glColor3f(0.5*cosf(0.5*m_stored_time+(float)i*0.5),0.5*cosf(0.5*m_stored_time+(float)i*0.5),0.9);
            glVertex2f(scan_radius*cosf(angle),scan_radius*sinf(angle));
        }
        glEnd();
        glLineWidth(1);

        glPopMatrix();

        //draw selected planet circle
        if(!m_vec_planets_ind_within_scan_range.empty())
        {
            glPushMatrix();
            glTranslatef(m_vec_planets[m_vec_planets_ind_within_scan_range[m_planet_selection_ind]].pos[0]*(1.0+m_zoom_level),
                         m_vec_planets[m_vec_planets_ind_within_scan_range[m_planet_selection_ind]].pos[1]*(1.0+m_zoom_level),0);
            float selected_radius=(7.0+3.0*cosf(2.0*(m_stored_time+360.0)));//*(1.0+m_zoom_level);
            glColor3f(0.6,0.6,0.6);
            glBegin(GL_LINE_LOOP);
            for(int i=0;i<=30;i++)
            {
                float angle=2*_pi*(float)i/30.0;
                glVertex2f(selected_radius*cosf(angle),selected_radius*sinf(angle));
            }
            glEnd();
            glPopMatrix();
        }

        //draw goal planet circle
        glPushMatrix();
        glTranslatef(m_vec_planets[m_planet_goal].pos[0]*(1.0+m_zoom_level),
                     m_vec_planets[m_planet_goal].pos[1]*(1.0+m_zoom_level),0);
        float selected_radius=(7.0+3.0*cosf(2.0*m_stored_time));//*(1.0+m_zoom_level);
        glColor3f(0,0.9,0);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<=30;i++)
        {
            float angle=2*_pi*(float)i/30.0;
            glVertex2f(selected_radius*cosf(angle),selected_radius*sinf(angle));
        }
        glEnd();
        glPopMatrix();

        //draw main ship?
    }
    glDisable(GL_BLEND);

    glPopMatrix();

    //draw planet data
    if(!m_vec_planets_ind_within_scan_range.empty())
    {
        /*//just a line
        float screen_gap=0.01;
        glLineWidth(3);
        glColor3f(1.0,0.3,0.3);
        glBegin(GL_LINES);
        glVertex2f( m_screen_width*0.5-m_curr_planet_enemy_content_drag*m_screen_width*0.4-screen_gap*m_screen_width, 0.05*m_screen_height );
        glVertex2f( m_screen_width*0.5-screen_gap*m_screen_width, 0.05*m_screen_height );
        glEnd();
        glColor3f(0.3,1.0,0.3);
        glBegin(GL_LINES);
        glVertex2f( m_screen_width*0.5+screen_gap*m_screen_width, 0.05*m_screen_height );
        glVertex2f( m_screen_width*0.5+m_curr_planet_fuel_content_drag*m_screen_width*0.4+screen_gap*m_screen_width, 0.05*m_screen_height );
        glEnd();*/

        //texture line
        glPushMatrix();
        //float screen_gap=0.01;
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glColor3f(0.6,0.1,0.1);
        //go to center
        glTranslatef(m_screen_width*0.5,m_screen_height*0.05,0);
        //draw start edge
        glBegin(GL_QUADS);
        glTexCoord2f(429.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0,-0.01759*m_screen_height);
        glTexCoord2f(429.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0,0.0);
        glTexCoord2f(438.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width,0.0);
        glTexCoord2f(438.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width,-0.01759*m_screen_height);
        //draw line
        glTexCoord2f(438.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width,-0.01759*m_screen_height);
        glTexCoord2f(438.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width,0.0);
        glTexCoord2f(510.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width+m_curr_planet_enemy_content_drag*m_screen_width*0.4,0.0);
        glTexCoord2f(510.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width+m_curr_planet_enemy_content_drag*m_screen_width*0.4,-0.01759*m_screen_height);
        //draw end edge
        glTexCoord2f(510.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width+m_curr_planet_enemy_content_drag*m_screen_width*0.4,-0.01759*m_screen_height);
        glTexCoord2f(510.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width+m_curr_planet_enemy_content_drag*m_screen_width*0.4,0.0);
        glTexCoord2f(519.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width*2.0+m_curr_planet_enemy_content_drag*m_screen_width*0.4,0.0);
        glTexCoord2f(519.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0046875*m_screen_width*2.0+m_curr_planet_enemy_content_drag*m_screen_width*0.4,-0.01759*m_screen_height);

        //fuel bar
        glColor3f(0.1,0.6,0.1);
        glTexCoord2f(429.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(0.0,-0.01759*m_screen_height);
        glTexCoord2f(429.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(0.0,0.0);
        glTexCoord2f(438.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width,0.0);
        glTexCoord2f(438.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width,-0.01759*m_screen_height);
        //draw line
        glTexCoord2f(438.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width,-0.01759*m_screen_height);
        glTexCoord2f(438.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width,0.0);
        glTexCoord2f(510.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width-m_curr_planet_fuel_content_drag*m_screen_width*0.4,0.0);
        glTexCoord2f(510.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width-m_curr_planet_fuel_content_drag*m_screen_width*0.4,-0.01759*m_screen_height);
        //draw end edge
        glTexCoord2f(438.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width-m_curr_planet_fuel_content_drag*m_screen_width*0.4,-0.01759*m_screen_height);
        glTexCoord2f(438.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width-m_curr_planet_fuel_content_drag*m_screen_width*0.4,0.0);
        glTexCoord2f(429.0/1024.0,(1024.0-64.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width*2.0-m_curr_planet_fuel_content_drag*m_screen_width*0.4,0.0);
        glTexCoord2f(429.0/1024.0,(1024.0-45.0)/1024.0);
        glVertex2f(-0.0046875*m_screen_width*2.0-m_curr_planet_fuel_content_drag*m_screen_width*0.4,-0.01759*m_screen_height);

        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        glPopMatrix();
    }

    //draw planet name?

    return true;
}

bool star_map::move_cam_pos(float move_value[3],bool extra_zoom)//returns new values
{
    if(m_travel_pack_up_now||m_travel_done_set_up_now||m_planet_travel_now) return false;//no cam movement during travels

    //recalc move vector depending on world rotation
    float x_shift=move_value[0]*cosf(-m_world_rotation*_Deg2Rad)-move_value[1]*sinf(-m_world_rotation*_Deg2Rad);
    float y_shift=move_value[0]*sinf(-m_world_rotation*_Deg2Rad)+move_value[1]*cosf(-m_world_rotation*_Deg2Rad);
    move_value[0]=x_shift;
    move_value[1]=y_shift;

    //test if cam will be outside the world radius
    float cam_center_pos[2]={ m_cam_pos[0]+m_screen_width*0.5,
                              m_cam_pos[1]+m_screen_height*0.5 };
    float dist_from_center=sqrt( (cam_center_pos[0])*(cam_center_pos[0]) +
                                 (cam_center_pos[1])*(cam_center_pos[1]) );
    if(dist_from_center>_starmap_radius_world)
    {
        //only allow movement towards the center
        if(cam_center_pos[0]>0.0 && move_value[0]>0.0) move_value[0]=0.0;
        if(cam_center_pos[0]<0.0 && move_value[0]<0.0) move_value[0]=0.0;
        if(cam_center_pos[1]>0.0 && move_value[1]>0.0) move_value[1]=0.0;
        if(cam_center_pos[1]<0.0 && move_value[1]<0.0) move_value[1]=0.0;
    }

    m_cam_pos[0]+=move_value[0]/(1.0+m_zoom_level);
    m_cam_pos[1]+=move_value[1]/(1.0+m_zoom_level);
    m_zoom_level+=move_value[2];
    if(m_zoom_level<_starmap_zoom_min) m_zoom_level=_starmap_zoom_min;
    //different zoom level cut off values, one for the beginning, second for user updated
    if(extra_zoom)
    {
        if(m_zoom_level>_starmap_zoom_max) m_zoom_level=_starmap_zoom_max;
    }
    else//user updated, use lower limit
    {
        if(m_zoom_level>_starmap_zoom_max2) m_zoom_level=_starmap_zoom_max2;
    }



    //update cam speed
    if(fabs(move_value[0]/(1.0+m_zoom_level))>fabs(m_cam_speed[0])) m_cam_speed[0]=move_value[0]/(1.0+m_zoom_level);
    if(fabs(move_value[1]/(1.0+m_zoom_level))>fabs(m_cam_speed[1])) m_cam_speed[1]=move_value[1]/(1.0+m_zoom_level);

    move_value[0]=m_cam_pos[0];
    move_value[1]=m_cam_pos[1];
    move_value[2]=m_zoom_level;

    //planet selection with cam, select closest within range
    int old_selection=m_planet_selection_ind;
    int shortest_ind=0;//0 is current planet
    float shortest_dist=sqrt( (m_vec_planets[m_vec_planets_ind_within_scan_range[shortest_ind]].pos[0]-cam_center_pos[0])*
                              (m_vec_planets[m_vec_planets_ind_within_scan_range[shortest_ind]].pos[0]-cam_center_pos[0]) +
                              (m_vec_planets[m_vec_planets_ind_within_scan_range[shortest_ind]].pos[1]-cam_center_pos[1])*
                              (m_vec_planets[m_vec_planets_ind_within_scan_range[shortest_ind]].pos[1]-cam_center_pos[1]) );

    for(int planet_i=0;planet_i<(int)m_vec_planets_ind_within_scan_range.size();planet_i++)
    {
        float dist=sqrt( (m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0]-cam_center_pos[0])*
                         (m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0]-cam_center_pos[0]) +
                         (m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1]-cam_center_pos[1])*
                         (m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1]-cam_center_pos[1]) );
        if(dist<shortest_dist)
        {
            shortest_dist=dist;
            shortest_ind=planet_i;
        }
    }

    //a new planet?
    m_planet_selection_ind=shortest_ind;
    if(m_planet_selection_ind!=old_selection)
    {
        m_planet_selected=m_vec_planets_ind_within_scan_range[m_planet_selection_ind];

        //play sound
        m_pSound->playSimpleSound(wav_starmap_select,1.0);
    }


    return true;
}

bool star_map::get_cam_pos(float cam_pos[3])
{
    cam_pos[0]=m_cam_pos[0];
    cam_pos[1]=m_cam_pos[1];
    cam_pos[2]=m_zoom_level;

    return true;
}

bool star_map::planet_selection(int direction)
{
    if(m_travel_pack_up_now||m_travel_done_set_up_now||m_planet_travel_now) return false;//no selection during travels

    if(m_vec_planets_ind_within_scan_range.empty()) return false;

    //direction switch due to world rotation
    for(float flips=m_world_rotation;flips>45.0;flips-=90)
    {
        direction-=1;
        //if(direction>dir_left) direction=dir_up;
        if(direction<dir_up) direction=dir_left;
    }

    int shortest_ind=-1;
    //float shortest_dist=_mship_scan_range*3.0;
    vector<st_int_float> vec_possible_planets;
    for(int planet_i=0;planet_i<(int)m_vec_planets_ind_within_scan_range.size();planet_i++)
    {
        if(m_vec_planets_ind_within_scan_range[planet_i]==m_planet_selected) continue;

        float dist=sqrt( (m_vec_planets[m_planet_selected].pos[0]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0])*
                         (m_vec_planets[m_planet_selected].pos[0]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0]) +
                         (m_vec_planets[m_planet_selected].pos[1]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1])*
                         (m_vec_planets[m_planet_selected].pos[1]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1]) );
        //direction test, store if within correct angle
        switch(direction)
        {
            case dir_up:
            {
                if(m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1]<m_vec_planets[m_planet_selected].pos[1])
                {
                    vec_possible_planets.push_back( st_int_float(planet_i,dist) );
                }
            }break;

            case dir_right:
            {
                if(m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0]>m_vec_planets[m_planet_selected].pos[0])
                {
                    vec_possible_planets.push_back( st_int_float(planet_i,dist) );
                }
            }break;

            case dir_down:
            {
                if(m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1]>m_vec_planets[m_planet_selected].pos[1])
                {
                    vec_possible_planets.push_back( st_int_float(planet_i,dist) );
                }
            }break;

            case dir_left:
            {
                if(m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0]<m_vec_planets[m_planet_selected].pos[0])
                {
                    vec_possible_planets.push_back( st_int_float(planet_i,dist) );
                }
            }break;
        }
    }

    //sort possible planets, shortest dist first
    //cout<<"possible planets: "<<(int)vec_possible_planets.size()<<endl;
    st_int_float temp_st;
    while(true)
    {
        bool update=false;
        for(int i=0;i<(int)vec_possible_planets.size()-1;i++)
        {
            if( vec_possible_planets[i].val_f>vec_possible_planets[i+1].val_f )
            {
                temp_st=vec_possible_planets[i];
                vec_possible_planets[i]=vec_possible_planets[i+1];
                vec_possible_planets[i+1]=temp_st;
                update=true;
            }
        }
        if(!update) break;
    }

    //find closest planets that fulfills direction selection
    bool selection_made=false;
    for(int i=0;i<(int)vec_possible_planets.size();i++)
    {
        //cone test
        float dx=m_vec_planets[m_vec_planets_ind_within_scan_range[vec_possible_planets[i].val_i]].pos[0]-m_vec_planets[m_planet_selected].pos[0];
        float dy=m_vec_planets[m_vec_planets_ind_within_scan_range[vec_possible_planets[i].val_i]].pos[1]-m_vec_planets[m_planet_selected].pos[1];
        float angle=90.0;
        //if(dy!=0.0) angle=atan(dx/dy)*_Rad2Deg;
        if(dy!=0.0) angle=atan2(-dy,dx)*_Rad2Deg;
        //if(dx<0) angle+=180.0;
        while(angle<0)    angle+=360.0;
        while(angle>=360) angle-=360.0;

        //cout<<"dist: "<<vec_possible_planets[i].val_f<<"\td: "<<dx<<", "<<dy<<"\tang: "<<angle;
        switch(direction)
        {
            case dir_up:
            {
                //if(angle>=120 && angle<240)
                if(angle>=30 && angle<150)
                {
                    //shortest_dist=dist;
                    shortest_ind=vec_possible_planets[i].val_i;
                    selection_made=true;
                }
            }break;

            case dir_right:
            {
                //if(angle>=30 && angle<150)
                if( (angle>=0 && angle<60) || angle>=300 )
                {
                    //shortest_dist=dist;
                    shortest_ind=vec_possible_planets[i].val_i;
                    selection_made=true;
                }
            }break;

            case dir_down:
            {
                //if( (angle>=0 && angle<60) || angle>=300 )
                if(angle>=210 && angle<330)
                {
                    //shortest_dist=dist;
                    shortest_ind=vec_possible_planets[i].val_i;
                    selection_made=true;
                }
            }break;

            case dir_left:
            {
                //if(angle>=210 && angle<330)
                if(angle>=120 && angle<240)
                {
                    //shortest_dist=dist;
                    shortest_ind=vec_possible_planets[i].val_i;
                    selection_made=true;
                }
            }break;
        }
        if(selection_made) break;
    }

    if(selection_made)
    {
        m_planet_selection_ind=shortest_ind;
        m_planet_selected=m_vec_planets_ind_within_scan_range[m_planet_selection_ind];

        //play sound
        m_pSound->playSimpleSound(wav_starmap_select,1.0);
    }

    //cout<<"selected: "<<m_planet_selected<<endl;

    /*//select with random order within range
    m_planet_selection_ind+=direction;
    if(m_planet_selection_ind<0) m_planet_selection_ind=(int)m_vec_planets_ind_within_scan_range.size()-1;
    if(m_planet_selection_ind>=(int)m_vec_planets_ind_within_scan_range.size()) m_planet_selection_ind=0;*/

    return true;
}

bool star_map::planet_go_to(void)
{
    if(m_travel_pack_up_now||m_travel_done_set_up_now||m_planet_travel_now) return false;//already travelling

    if(m_planet_now==m_planet_selected) return false;//no travel

    //calc travel distance
    m_travel_planet_start=m_planet_now;
    m_travel_planet_end=m_planet_selected;
    float dist=sqrt( (m_vec_planets[m_travel_planet_start].pos[0]-m_vec_planets[m_travel_planet_end].pos[0])*
                     (m_vec_planets[m_travel_planet_start].pos[0]-m_vec_planets[m_travel_planet_end].pos[0]) +
                     (m_vec_planets[m_travel_planet_start].pos[1]-m_vec_planets[m_travel_planet_end].pos[1])*
                     (m_vec_planets[m_travel_planet_start].pos[1]-m_vec_planets[m_travel_planet_end].pos[1]) );

    //test if fuel is enough
    //cout<<"Fuel: "<<m_mship_fuel<<"\tDist: "<<dist<<endl;
    if(dist>m_mship_fuel) return false;//not enough fuel
    m_travel_fuel_cost=dist;

    //start travel

    //start timers
    m_travel_time_max=dist*_mship_travel_speed;
    m_travel_time=0.0;
    //m_planet_travel_now=false;
    m_travel_pack_up_time=0.0;
    m_travel_pack_up_now=true;

    //play sound
    m_pSound->playSimpleSound(wav_starmap_startup,1.0);

    //test if going to goal planet
    if(m_travel_planet_end==m_planet_goal)
    {
        m_travel_towards_goal=true;
    }

    return true;
}

int star_map::get_planet_now(void)
{
    return m_planet_now;
}

int star_map::get_planet_now_level_index(void)
{
    //will be called when a level is loaded, reset starmap cam pos
    m_cam_pos[0]=m_vec_planets[m_planet_now].pos[0]-m_screen_width*0.5;
    m_cam_pos[1]=m_vec_planets[m_planet_now].pos[1]-m_screen_height*0.5;
    m_zoom_level=_starmap_zoom_max;

    return m_vec_planets[m_planet_now].level_index;
}

bool star_map::set_planet_now_level_index(int level_index)
{
    m_vec_planets[m_planet_now].world_initialized=true;
    m_vec_planets[m_planet_now].level_index=level_index;

    if(level_index==-2)//tutorial mission
    {
        m_vec_planets[m_planet_now].fuel_content=0;
        m_vec_planets[m_planet_now].enemy_content=0;
    }

    return true;
}

int star_map::get_curr_planet_level_fuel(void)
{
    return (int)m_vec_planets[m_planet_now].fuel_content;
}

bool star_map::change_curr_planet_level_fuel(int value)
{
    m_vec_planets[m_planet_now].fuel_content+=value;
    if(m_vec_planets[m_planet_now].fuel_content<0) m_vec_planets[m_planet_now].fuel_content=0;

    return true;
}

int star_map::get_curr_planet_level_enemy(void)
{
    return (int)m_vec_planets[m_planet_now].enemy_content;
}

bool star_map::change_curr_planet_level_enemy(int value)
{
    m_vec_planets[m_planet_now].enemy_content+=value;
    if(m_vec_planets[m_planet_now].enemy_content<0) m_vec_planets[m_planet_now].enemy_content=0;

    return true;
}

float star_map::get_dist_to_closest_planet_with_fuel(void)
{
    //measure dist to all planets within scan range
    float closest_dist=_mship_scan_range+1.0;
    for(int planet_i=0;planet_i<(int)m_vec_planets_ind_within_scan_range.size();planet_i++)
    {
        if(m_planet_now==m_vec_planets_ind_within_scan_range[planet_i]) continue;

        float dist=sqrt( (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0])*
                         (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[0]) +
                         (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1])*
                         (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[m_vec_planets_ind_within_scan_range[planet_i]].pos[1]) );
        if(dist<closest_dist)
        {
            //test if it have any fuel
            if(m_vec_planets[ m_vec_planets_ind_within_scan_range[planet_i] ].fuel_content>0.0) closest_dist=dist;
            //if it has not any fuel, get next in line
        }
    }
    //if any station had fuel, return dist
    if(closest_dist<=_mship_scan_range)
    {
        return closest_dist;
    }

    //if none of the stations within scan range have any fuel, test all stations (takes time)
    closest_dist=sqrt( (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[m_planet_goal].pos[0])*
                       (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[m_planet_goal].pos[0]) +
                       (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[m_planet_goal].pos[1])*
                       (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[m_planet_goal].pos[1]) );
    for(int planet_i=0;planet_i<(int)m_vec_planets.size();planet_i++)
    {
        if(m_planet_now==planet_i) continue;

        float dist=sqrt( (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[planet_i].pos[0])*
                         (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[planet_i].pos[0]) +
                         (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[planet_i].pos[1])*
                         (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[planet_i].pos[1]) );
        if(dist<closest_dist)
        {
            //test if it have any fuel
            if(m_vec_planets[planet_i].fuel_content>0.0) closest_dist=dist;
            //if it has not any fuel, get next in line
        }
    }

    return closest_dist;
}

bool star_map::move_cam_towards_planet_now(float progression)//from 0-1
{
    if(progression==0.0)
    {
        //store start pos
        m_cam_pos_old[0]=m_cam_pos[0];
        m_cam_pos_old[1]=m_cam_pos[1];
        //cout<<"Starmap: Startpos: "<<m_cam_pos_old[0]<<"\t"<<m_cam_pos_old[1]<<endl;
    }

    //calc move direction
    float cam_end_pos[2]={ m_vec_planets[m_planet_now].pos[0]-m_screen_width*0.5,
                           m_vec_planets[m_planet_now].pos[1]-m_screen_height*0.5 };
    float cam_move_direction[2]={ cam_end_pos[0]-m_cam_pos_old[0],
                                  cam_end_pos[1]-m_cam_pos_old[1] };
    //cout<<"Starmap: Direction: "<<cam_move_direction[0]<<"\t"<<cam_move_direction[1]<<endl;
    m_cam_pos[0]=m_cam_pos_old[0]+cam_move_direction[0]*progression;
    m_cam_pos[1]=m_cam_pos_old[1]+cam_move_direction[1]*progression;
    //cout<<m_cam_pos[0]<<"\t"<<m_cam_pos[1]<<endl;

    return true;
}

bool star_map::is_travelling(void)
{
    return m_planet_travel_now;
}

bool star_map::reset_view(void)
{
    m_cam_pos[0]=m_cam_pos_old[0]=0.0;
    m_cam_pos[1]=m_cam_pos_old[1]=0.0;
    m_cam_speed[0]=0.0;
    m_cam_speed[1]=0.0;
    m_zoom_level=_starmap_zoom_min;

    return true;
}

bool star_map::make_new_goal(void)
{
    //select a station far away
    float min_dist=10000;
    while(true)
    {
        int rand_station=rand()%int(m_vec_planets.size());

        //measure distance
        if( sqrt( (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[rand_station].pos[0])*
                  (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[rand_station].pos[0])+
                  (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[rand_station].pos[1])*
                  (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[rand_station].pos[1]) ) > min_dist )
        {
            m_planet_goal=rand_station;
            m_travel_towards_goal=false;
            break;
        }

    }

    return true;
}

bool star_map::save_data(string& game_data)
{
    cout<<"SAVE: Starmap data\n";
    //save map data
    game_data.append("Starmap ");
    game_data.append( num_to_string(m_planet_now) );
    game_data.append(" ");
    game_data.append( num_to_string(m_planet_goal) );
    game_data.append(" ");
    game_data.append( num_to_string(m_planet_start) );
    game_data.append("\n");

    //save planet data
    for(unsigned int planet_i=0;planet_i<m_vec_planets.size();planet_i++)
    {
        game_data.append("Planet ");
        game_data.append( num_to_string(m_vec_planets[planet_i].pos[0]) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_planets[planet_i].pos[1]) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_planets[planet_i].fuel_content) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_planets[planet_i].enemy_content) );
        game_data.append(" ");
        game_data.append( num_to_string(m_vec_planets[planet_i].world_initialized) );
        if(m_vec_planets[planet_i].world_initialized)
        {
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_planets[planet_i].level_index) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_planets[planet_i].level_color[0]) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_planets[planet_i].level_color[1]) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_planets[planet_i].level_color[2]) );
            game_data.append(" ");
            game_data.append( num_to_string(m_vec_planets[planet_i].level_texture_index) );
        }
        game_data.append("\n");

    }

    cout<<"SAVE: Starmap data done\n";

    return true;
}


//PRIVATE

bool star_map::calc_planets_within_range(float range)
{
    m_vec_planets_ind_within_scan_range.clear();

    //measure dist from player planet to all others, store those within range
    for(int planet_i=0;planet_i<(int)m_vec_planets.size();planet_i++)
    {
        //if(planet_i==m_planet_now) continue;//not possible to select current planet

        float dist=sqrt( (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[planet_i].pos[0])*
                         (m_vec_planets[m_planet_now].pos[0]-m_vec_planets[planet_i].pos[0]) +
                         (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[planet_i].pos[1])*
                         (m_vec_planets[m_planet_now].pos[1]-m_vec_planets[planet_i].pos[1]) );
        if(dist<range)
        {
            m_vec_planets_ind_within_scan_range.push_back(planet_i);
        }
    }

    return true;
}

bool star_map::get_curr_planet_level_color(float color[3])
{
    color[0]=m_vec_planets[m_planet_now].level_color[0];
    color[1]=m_vec_planets[m_planet_now].level_color[1];
    color[2]=m_vec_planets[m_planet_now].level_color[2];

    return true;
}

int star_map::get_curr_planet_level_texture(void)
{
    return m_vec_planets[m_planet_now].level_texture_index;
}
