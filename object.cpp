#include "object.h"

object::object()
{
    //ctor
}

bool object::init(b2World* world_ptr,int texture,int id,b2Vec2 edge_points[],int numof_edges,b2Vec2 pos)
{
    m_pWorld=world_ptr;
    m_id=id;
    m_size=1.0;//not used
    m_texture=texture;
    m_fuel_content=0.0;//no fuel
    m_rel_fuel_content=0.0;
    m_fuel_texture_prog=0.0;
    m_object_type=ot_resource;

    //store edge points
    for(int i=0;i<numof_edges;i++)
    {
        m_vec_edge_points.push_back( edge_points[i] );
    }

    //create object
    b2BodyDef bodydef;
    bodydef.position.Set(pos.x,pos.y);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=_object_damping_lin;
    bodydef.angularDamping=_object_damping_ang;
    m_pBody=m_pWorld->CreateBody(&bodydef);
    //create fixture
    b2PolygonShape shape2;
    shape2.Set(edge_points,numof_edges);
    b2FixtureDef fixturedef;
    fixturedef.shape=&shape2;
    fixturedef.density=_object_density;
    fixturedef.filter.categoryBits=_COLCAT_all;
    fixturedef.filter.maskBits=-1;
    m_pBody->CreateFixture(&fixturedef);
    //move into pos
    //m_pBody->SetTransform( pos, 0 );
    //set data
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="object";
    user_data->i_id=m_id;
    m_pBody->SetUserData(user_data);

    //set center of rotation
    /*b2MassData massD;
    m_pBody->GetMassData(&massD);
    b2Vec2 centerV(0,0);
    massD.center = centerV;
    m_pBody->SetMassData(&massD);*/

    return true;
}

bool object::init(b2World* world_ptr,int texture,int id,float size,b2Vec2 pos,float fuel_content)
{
    m_pWorld=world_ptr;
    m_id=id;
    m_size=size;
    m_texture=texture;
    m_rel_fuel_content=fuel_content/100.0;
    m_fuel_content=size*fuel_content*5.0;
    m_fuel_texture_prog=0.0;
    m_object_type=ot_fuel;

    b2Vec2 edge_points[]={b2Vec2(-size,-size),b2Vec2(size,-size),b2Vec2(size,size),b2Vec2(-size,size)};

    //create object
    b2BodyDef bodydef;
    bodydef.position.Set(pos.x,pos.y);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=_object_damping_lin;
    bodydef.angularDamping=_object_damping_ang;
    m_pBody=m_pWorld->CreateBody(&bodydef);
    //create fixture
    b2PolygonShape shape2;
    shape2.Set(edge_points,4);
    b2FixtureDef fixturedef;
    fixturedef.shape=&shape2;
    fixturedef.density=_object_density+_fuel_density*fuel_content;
    //fixturedef.restitution=0.0;
    fixturedef.filter.categoryBits=_COLCAT_all;
    fixturedef.filter.maskBits=-1;
    m_pBody->CreateFixture(&fixturedef);

    //move into pos
    //m_pBody->SetTransform( pos, 0 );
    //set data
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="object";
    user_data->i_id=m_id;
    m_pBody->SetUserData(user_data);

    //set center of rotation
    b2MassData massD;
    m_pBody->GetMassData(&massD);
    b2Vec2 centerV(0,0);
    massD.center = centerV;
    m_pBody->SetMassData(&massD);

    return true;
}

bool object::update()
{
    m_fuel_texture_prog+=_world_step_time*0.1;
    if(m_fuel_texture_prog>1.0) m_fuel_texture_prog-=1.0;

    if(!m_pBody->IsAwake()) return false;//no update nedded

    //find extreme points
    b2Fixture* pFix=m_pBody->GetFixtureList();//only one
    b2Vec2 start_point=((b2PolygonShape*)pFix->GetShape())->GetVertex(0);
    m_extreme_point_min=start_point;
    m_extreme_point_max=start_point;
    for(int i=1;i<4;i++)//4 edges
    {
        b2Vec2 point=((b2PolygonShape*)pFix->GetShape())->GetVertex(i);
        if(point.x>m_extreme_point_max.x) m_extreme_point_max.x=point.x;
        else if(point.x<m_extreme_point_min.x) m_extreme_point_min.x=point.x;
        if(point.y>m_extreme_point_max.y) m_extreme_point_max.y=point.y;
        else if(point.y<m_extreme_point_min.y) m_extreme_point_min.y=point.y;
    }

    //enlarge size
    //float box_height=m_extreme_point_max.y-m_extreme_point_min.y;
    //float box_width=m_extreme_point_max.x-m_extreme_point_min.x;
    if(m_object_type==ot_fuel)
    {
        m_extreme_point_max.x+=m_size;
        m_extreme_point_min.x-=m_size;
        m_extreme_point_max.y+=m_size;
        m_extreme_point_min.y-=m_size;
    }

    //calc fuel height
    float new_height=m_extreme_point_max.y-m_extreme_point_min.y;
    m_extreme_point_min.y=m_extreme_point_max.y-new_height*m_rel_fuel_content;

    return true;
}

bool object::draw()
{
    /*//temp draw box
    glPushMatrix();
    b2Vec2 pos=m_pBody->GetPosition();
    float angle=m_pBody->GetAngle()*_Rad2Deg;
    glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
    glRotatef(angle,0,0,1);
    glBegin(GL_LINE_STRIP);
    glColor3f(0.3,0.8,0.5);
    glVertex2f(-1.0*_Met2Pix,1.0*_Met2Pix);
    glVertex2f(1.0*_Met2Pix,1.0*_Met2Pix);
    glColor3f(0.8,0.3,0.8);
    glVertex2f(1.0*_Met2Pix,-1.0*_Met2Pix);
    glVertex2f(-1.0*_Met2Pix,-1.0*_Met2Pix);
    glColor3f(0.3,0.8,0.5);
    glVertex2f(-1.0*_Met2Pix,1.0*_Met2Pix);
    glEnd();
    glPopMatrix();*/

    switch(m_object_type)
    {
        case ot_fuel:
        {
            //move to pos
            float size_pix=m_size*_Met2Pix/15.0;
            b2Vec2 pos=m_pBody->GetPosition();
            float angle=m_pBody->GetAngle()*_Rad2Deg;

            //draw stencil mask
            glClear(GL_STENCIL_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
            glStencilFunc( GL_ALWAYS, 1, 0xFF );
            glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
            //b2Fixture* pFix=m_pBody->GetFixtureList();//only one
            glColor3f(1,1,1);
            glPushMatrix();
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(angle,0,0,1);
            glBegin(GL_QUADS);
            glVertex2f(-m_size*0.9*_Met2Pix,m_size*0.9*_Met2Pix);
            glVertex2f(-m_size*0.9*_Met2Pix,-m_size*0.9*_Met2Pix);
            glVertex2f(m_size*0.9*_Met2Pix,-m_size*0.9*_Met2Pix);
            glVertex2f(m_size*0.9*_Met2Pix,m_size*0.9*_Met2Pix);
            glEnd();
            glPopMatrix();

            //draw fuel level color
            glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
            glStencilFunc(GL_EQUAL, 1, 1);
            glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            float texture_shift_y=60.0*m_fuel_texture_prog;
            glPushMatrix();
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glColor3f(0.8,0.8,0.4);
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(281.0/1024.0,(1024.0-178.0-texture_shift_y)/1024.0);
            glVertex2f(m_extreme_point_min.x*_Met2Pix,m_extreme_point_min.y*_Met2Pix);
            glTexCoord2f(281.0/1024.0,(1024.0-178.0-(m_extreme_point_max.y-m_extreme_point_min.y)*_Met2Pix-texture_shift_y)/1024.0);
            glVertex2f(m_extreme_point_min.x*_Met2Pix,m_extreme_point_max.y*_Met2Pix);
            glTexCoord2f(310.0/1024.0,(1024.0-178.0-(m_extreme_point_max.y-m_extreme_point_min.y)*_Met2Pix-texture_shift_y)/1024.0);
            glVertex2f(m_extreme_point_max.x*_Met2Pix,m_extreme_point_max.y*_Met2Pix);
            glTexCoord2f(310.0/1024.0,(1024.0-178.0-texture_shift_y)/1024.0);
            glVertex2f(m_extreme_point_max.x*_Met2Pix,m_extreme_point_min.y*_Met2Pix);
            glEnd();
            glDisable(GL_STENCIL_TEST);
            //glPopMatrix();

            //draw shell box
            //glPushMatrix();
            //glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(angle,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            //glEnable(GL_TEXTURE_2D);
            //glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(971.0/1024.0,(1024.0-1.0-41.0)/1024.0);
            glVertex2f(-20.0*size_pix,-20.0*size_pix);
            glTexCoord2f(971.0/1024.0,(1024.0-40.0-41.0)/1024.0);
            glVertex2f(-20.0*size_pix, 20.0*size_pix);
            glTexCoord2f(1010.0/1024.0,(1024.0-40.0-41.0)/1024.0);
            glVertex2f( 20.0*size_pix, 20.0*size_pix);
            glTexCoord2f(1010.0/1024.0,(1024.0-1.0-41.0)/1024.0);
            glVertex2f( 20.0*size_pix,-20.0*size_pix);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(971.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-20.0*size_pix,-20.0*size_pix);
            glTexCoord2f(971.0/1024.0,(1024.0-40.0)/1024.0);
            glVertex2f(-20.0*size_pix, 20.0*size_pix);
            glTexCoord2f(1010.0/1024.0,(1024.0-40.0)/1024.0);
            glVertex2f( 20.0*size_pix, 20.0*size_pix);
            glTexCoord2f(1010.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 20.0*size_pix,-20.0*size_pix);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);

            glPopMatrix();
        }break;

        case ot_resource:
        {
            //temp
            //b2MassData massD;
            //m_pBody->GetMassData(&massD);

            glPushMatrix();
            b2Vec2 pos=m_pBody->GetPosition();
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(angle,0,0,1);

            //to center of mass
            //glTranslatef( -massD.center.x*_Met2Pix, -massD.center.y*_Met2Pix, 0.0);

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);

            glBegin(GL_TRIANGLE_FAN);
            glColor3f(0.5,0.5,0.5);
            glTexCoord2f(296.0/1024.0,(1024.0-193.0)/1024.0);
            glVertex2f(0.0,0.0);//center
            glColor3f(0.2,0.2,0.2);
            for(unsigned int i=0;i<m_vec_edge_points.size();i++)
            {
                glTexCoord2f((296.0+m_vec_edge_points[i].x*_Met2Pix)/1024.0,
                             (1024.0-193.0-m_vec_edge_points[i].y*_Met2Pix)/1024.0);
                glVertex2f(m_vec_edge_points[i].x*_Met2Pix,
                           m_vec_edge_points[i].y*_Met2Pix);
            }
            glTexCoord2f((296.0+m_vec_edge_points.front().x*_Met2Pix)/1024.0,
                         (1024.0-193.0-m_vec_edge_points.front().y*_Met2Pix)/1024.0);
            glVertex2f(m_vec_edge_points.front().x*_Met2Pix,
                       m_vec_edge_points.front().y*_Met2Pix);
            glEnd();
            glPopMatrix();

            glDisable(GL_TEXTURE_2D);
        }break;
    }

    return true;
}

bool object::shift_object_pos(float x,float y)
{
    //move object
    float angle=m_pBody->GetAngle();
    b2Vec2 pos=m_pBody->GetWorldCenter();
    m_pBody->SetTransform( b2Vec2( pos.x+x,pos.y+y ), angle );

    return true;
}

bool object::get_id(void)
{
    return m_id;
}

float object::get_content(void)
{
    switch(m_object_type)
    {
        case ot_fuel:
        {
            return m_fuel_content;
        }break;

        case ot_resource:
        {
            /*//calc polygon area (ERROR)
            int numof_points=(int)m_vec_edge_points.size();
            m_vec_edge_points.push_back( m_vec_edge_points.front() );//wrap-around, end point is first

            int i,j,k;
            float area=0;
            if( numof_points<3 ) return 0;//not a polygon

            for(i=1, j=2, k=0; i<(int)m_vec_edge_points.size(); i++, j++, k++)
            {
                area+=m_vec_edge_points[i].x*(m_vec_edge_points[j].y-m_vec_edge_points[k].y);
            }
            area+=m_vec_edge_points[numof_points].x*(m_vec_edge_points[1].y-
                                                     m_vec_edge_points[numof_points-1].y);//wrap-around term

            //remove end copy
            m_vec_edge_points.pop_back();

            return area*0.5;*/

            //get mass
            b2MassData massD;
            m_pBody->GetMassData(&massD);

            return massD.mass*100.0;
        }break;
    }

    //error
    return 0.0;
}

b2Body* object::get_object_ptr(void)
{
    return m_pBody;
}
