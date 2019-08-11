#include "particle_engine.h"

particle_engine::particle_engine()
{
    m_rand_accuracy=1000;
    m_direction_spread=0.5;
    m_direction_spread_min=0.9;
    m_time_left_variation=0.9;
    m_default_time_left=1.0;
    m_gravity[0]=0.0;
    m_gravity[1]=20.0;
    m_ignore_counter=0;
    m_ignore_limit=10;
    m_directed_angle_state_return_chance=0.05;
}

bool particle_engine::init(void)
{
    srand(time(0));

    //clear all
    for(int part_i=0;part_i<_max_particles;part_i++)
    {
        m_time_left[part_i]=0.0;
    }

    return true;
}

bool particle_engine::add_particle(b2Vec2 pos,b2Vec2 direction,float life_time)//in meter format, will be converted to pixel format
{
    //if skip_random is true, ignore some calls
    /*if(skip_random)
    {
        //update ignore counter
        if(++m_ignore_counter>m_ignore_limit) m_ignore_counter=0;//make particle
        else return false;//no particle this time
    }*/
    //else always add a particle

    //find free slot
    int free_slot=-1;
    for(int part_i=0;part_i<_max_particles;part_i++)
    {
        if(m_time_left[part_i]<=0)
        {
            free_slot=part_i;
            break;
        }
    }
    if(free_slot==-1) return false;//no free slots

    //set start pos
    m_pos[free_slot][0]=pos.x*_Met2Pix;
    m_pos[free_slot][1]=pos.y*_Met2Pix;
    //set direction
    float direction_sum=fabs(direction.x)+fabs(direction.y)+m_direction_spread_min;
    float new_direction_x=direction.x+direction_sum*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread;
    float new_direction_y=direction.y+direction_sum*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread;
    m_direction[free_slot][0]=-new_direction_x*_Met2Pix;
    m_direction[free_slot][1]=-new_direction_y*_Met2Pix;

    //set color
    //m_color[free_slot][0]=m_color[free_slot][1]=m_color[free_slot][2]=0.7;

    //set time left
    m_time_start[free_slot]=m_time_left[free_slot]=life_time+life_time*
                            ((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_time_left_variation;
    //m_time_start[free_slot]=m_time_left[free_slot]=m_default_time_left+m_default_time_left*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_time_left_variation;

    return true;
}

bool particle_engine::add_explosion(b2Vec2 pos,int numof_part,float force,float life_time,float direction_angle)
{
    bool directed_explosion=true;
    if(direction_angle==-1.0) directed_explosion=false;

    //calc direction x,y
    b2Vec2 direction_focus(0,0);
    b2Vec2 direction_focus_start(0,0);
    float directed_sum=0.0;
    if(directed_explosion)
    {
        direction_focus.x=cosf((direction_angle-90.0)*_Deg2Rad)*0.6;
        direction_focus.y=sinf((direction_angle-90.0)*_Deg2Rad)*0.6;
        //std::cout<<"DIRECTION: "<<direction_focus.x<<", "<<direction_focus.y<<" : Angle: "<<direction_angle<<std::endl;
        directed_sum=fabs(direction_focus.x)+fabs(direction_focus.y);
    }
    direction_focus_start=direction_focus;

    //make n particles with random direction
    for(int new_part=0;new_part<numof_part;new_part++)
    {
        //find free slot
        int free_slot=-1;
        for(int part_i=0;part_i<_max_particles;part_i++)
        {
            if(m_time_left[part_i]<=0)
            {
                free_slot=part_i;
                break;
            }
        }
        if(free_slot==-1) return false;//no more free slots

        //set pos
        m_pos[free_slot][0]=pos.x*_Met2Pix;
        m_pos[free_slot][1]=pos.y*_Met2Pix;
        //set direction

        if(directed_explosion)
        {
            //semi normal distrub. with drift
            float rand_val=float(rand()%100)/100.0;// 0.0 - 1.0
                 if(rand_val<0.025) direction_focus.x-=0.4;
            else if(rand_val<0.05)  direction_focus.x-=0.3;
            else if(rand_val<0.10)  direction_focus.x-=0.2;
            else if(rand_val<0.225) direction_focus.x-=0.1;
            //else if(rand_val<0.50) direction_focus.x+=0.0;
            else if(rand_val<0.725) direction_focus.x+=0.0;
            else if(rand_val<0.90)  direction_focus.x+=0.1;
            else if(rand_val<0.95)  direction_focus.x+=0.2;
            else if(rand_val<0.975) direction_focus.x+=0.3;
            else                    direction_focus.x+=0.4;
            //and for y
            rand_val=float(rand()%100)/100.0;// 0.0 - 1.0
                 if(rand_val<0.025) direction_focus.y-=0.4;
            else if(rand_val<0.05)  direction_focus.y-=0.3;
            else if(rand_val<0.10)  direction_focus.y-=0.2;
            else if(rand_val<0.225) direction_focus.y-=0.1;
            //else if(rand_val<0.50) direction_focus.x+=0.0;
            else if(rand_val<0.725) direction_focus.y+=0.0;
            else if(rand_val<0.90)  direction_focus.y+=0.1;
            else if(rand_val<0.95)  direction_focus.y+=0.2;
            else if(rand_val<0.975) direction_focus.y+=0.3;
            else                    direction_focus.y+=0.4;

            //directed_sum will always be positive (focus on down left)
            //float direction_x=force*(directed_sum+direction_focus.x+((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread );
            //float direction_y=force*(directed_sum+direction_focus.y+((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread );

            //without directed_sum
            float direction_x=force*(direction_focus.x+direction_focus.x*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread );
            float direction_y=force*(direction_focus.y+direction_focus.y*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_direction_spread );

            m_direction[free_slot][0]=direction_x;
            m_direction[free_slot][1]=direction_y;

            //chance to return to start direction
            if(rand_val<0.50+m_directed_angle_state_return_chance && rand_val>0.5-m_directed_angle_state_return_chance)
            {
                direction_focus=direction_focus_start;
            }
        }
        else//full random
        {
            float direction_x=force*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5);
            float direction_y=force*((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5);

            m_direction[free_slot][0]=direction_x;
            m_direction[free_slot][1]=direction_y;
        }

        //no random
        //float direction_x=force*direction_focus.x;
        //float direction_y=force*direction_focus.y;

        //set time left
        m_time_start[free_slot]=m_time_left[free_slot]=life_time+life_time*
                                ((float(rand()%m_rand_accuracy)/(float)m_rand_accuracy)-0.5)*m_time_left_variation;
    }

    return true;
}

bool particle_engine::update(float time_dif)
{
    for(int part_i=0;part_i<_max_particles;part_i++)
    {
        if(m_time_left[part_i]>0)
        {
            //update age
            m_time_left[part_i]-=time_dif;
            //update pos
            m_pos[part_i][0]+=(m_direction[part_i][0]+m_gravity[0])*time_dif;
            m_pos[part_i][1]+=(m_direction[part_i][1]+m_gravity[1])*time_dif;
            //update color

        }
    }

    return true;
}

bool particle_engine::draw(void)
{
    glPointSize(1);
    glBegin(GL_POINTS);
    for(int part_i=0;part_i<_max_particles;part_i++)
    {
        if(m_time_left[part_i]>0)//only draw alive particles
        {
            glColor3f(m_time_left[part_i]/m_time_start[part_i],
                      m_time_left[part_i]/m_time_start[part_i],
                      m_time_left[part_i]/m_time_start[part_i]);
            glVertex2fv(m_pos[part_i]);
        }
    }
    glEnd();
    glPointSize(1);

    return true;
}


