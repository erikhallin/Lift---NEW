#ifndef PARTICLE_ENGINE_H
#define PARTICLE_ENGINE_H

#define _default_time_left 1.0

#include <iostream> //TEMP
#include <gl/gl.h>
#include <time.h>
#include <stdlib.h>
#include <Box2D/Box2D.h>
#include "definitions.h"

class particle_engine
{
    public:
        particle_engine();

        bool init(void);//set number of particles?
        bool add_particle(b2Vec2 pos,b2Vec2 direction,float life_time=_default_time_left);
        bool add_explosion(b2Vec2 pos,int numof_part,float force,float life_time,float direction_angle=-1.0);//angle from 0-360 (-1 OFF)
        bool update(float time_dif);
        bool draw(void);

    private:

        float m_time_start[_max_particles];
        float m_time_left[_max_particles];
        float m_pos[_max_particles][2];
        float m_direction[_max_particles][2];
        //float m_speed[_max_particles][2]; direction is speed
        float m_color[_max_particles][3];

        int   m_rand_accuracy,m_ignore_counter,m_ignore_limit;
        float m_direction_spread_min;
        float m_direction_spread;
        //float m_speed_variation;
        float m_time_left_variation;
        float m_default_time_left;
        float m_gravity[2];
        float m_directed_angle_state_return_chance;
};

#endif // PARTICLE_ENGINE_H
