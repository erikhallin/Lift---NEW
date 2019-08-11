#ifndef MYQUERYCALLBACK_H
#define MYQUERYCALLBACK_H

#include <Box2D/Box2D.h>
#include <vector>

class MyQueryCallback : public b2QueryCallback
{
    public:
        MyQueryCallback();

        bool m_any_hit;
        std::vector<b2Fixture*> m_vec_fixtures;

    private:

        bool ReportFixture(b2Fixture* fixture);
};

#endif // MYQUERYCALLBACK_H
