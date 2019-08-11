#include "MyQueryCallback.h"

MyQueryCallback::MyQueryCallback()
{
    m_any_hit=false;
}

bool MyQueryCallback::ReportFixture(b2Fixture* fixture)
{
    m_any_hit=true;
    m_vec_fixtures.push_back(fixture);

    return true;//have to be true if continue to get fixtures within area
}
