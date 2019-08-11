#ifndef MYRAYCASTCALLBACK_H
#define MYRAYCASTCALLBACK_H

//#include <iostream>//TEMP

#include <Box2D/Box2D.h>
#include <vector>
#include <string>
#include "definitions.h"

class MyRayCastCallback : public b2RayCastCallback
{
    public:
        MyRayCastCallback();

        void       set_ignore_body(b2Body* body_to_ignore);
        void       set_ignore_body_type(std::string body_type);

        bool       m_any_hit,m_ignore_body,m_ignore_body_type;
        b2Fixture* m_pFixture;
        b2Body*    m_pBody_to_ignore;
        b2Vec2     m_point;
        b2Vec2     m_normal;
        float32    m_fraction;

        std::vector<std::string> m_vec_ignored_types;

    private:

        float32 ReportFixture(b2Fixture* fixture,const b2Vec2& point,const b2Vec2& normal,float32 fraction);


/*ReportFixture Return Value Options:

To find only the closest intersection:
- return the fraction value from the callback
- use the most recent intersection as the result

To find all intersections along the ray:
- return 1 from the callback
- store the intersections in a list

To simply find if the ray hits anything:
- if you get a callback, something was hit (but it may not be the closest)
- return 0 from the callback for efficiency
*/

};

#endif
