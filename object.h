#ifndef OBJECT_H
#define OBJECT_H

#include <iostream> //temp
#include <vector>
#include <Box2D/Box2D.h>
#include <gl/gl.h>
#include "definitions.h"

using namespace std;

class object
{
    public:
        object();

        bool  init(b2World* world_ptr,int texture,int id,b2Vec2 edge_points[],int numof_edges,b2Vec2 pos);
        bool  init(b2World* world_ptr,int texture,int id,float size,b2Vec2 pos,float fuel_content);
        bool  update();
        bool  draw();
        bool  shift_object_pos(float x,float y);
        bool  get_id(void);
        float get_content(void);
        b2Body* get_object_ptr(void);

        int   m_object_type;
        vector<b2Vec2> m_vec_edge_points;


    private:

        int      m_id,m_texture;
        float    m_fuel_content,m_rel_fuel_content,m_size,m_fuel_texture_prog;
        b2Body*  m_pBody;
        b2World* m_pWorld;
        b2Vec2   m_extreme_point_min,m_extreme_point_max;


};

#endif // OBJECT_H
