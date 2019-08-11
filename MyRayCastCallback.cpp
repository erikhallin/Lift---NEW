#include "MyRayCastCallback.h"

MyRayCastCallback::MyRayCastCallback()
{
    m_any_hit=false;
    m_ignore_body=false;
    m_ignore_body_type=false;
}

void MyRayCastCallback::set_ignore_body(b2Body* body_to_ignore)
{
    m_ignore_body=true;
    m_pBody_to_ignore=body_to_ignore;
}

void MyRayCastCallback::set_ignore_body_type(std::string body_type)
{
    m_ignore_body_type=true;
    m_vec_ignored_types.push_back(body_type);
}

float32 MyRayCastCallback::ReportFixture(b2Fixture* fixture,const b2Vec2& point,const b2Vec2& normal,float32 fraction)
{
    //ignore sensors
    if(fixture->IsSensor()) return 1;

    //ignore given body
    if(m_ignore_body)
     if( fixture->GetBody()==m_pBody_to_ignore ) return 1;

    //ignore given body types
    if(m_ignore_body_type)
    {
        st_body_user_data* data=(st_body_user_data*)fixture->GetBody()->GetUserData();

        for(int type_i=0;type_i<(int)m_vec_ignored_types.size();type_i++)
        {
            if(m_vec_ignored_types[type_i]==data->s_info) return 1;
        }
    }


    m_any_hit=true;
    m_pFixture=fixture;
    m_point=point;
    m_normal=normal;
    m_fraction=fraction;

    return fraction;//stores only first intersection (return value of 1, will update for all intersections)
}
