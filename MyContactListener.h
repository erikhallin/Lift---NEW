#ifndef MYCONTACTLISTENER_H
#define MYCONTACTLISTENER_H

#include <iostream>
#include <vector>
#include <Box2D/Box2D.h>
#include "definitions.h"

enum events
{
    ev_idle=0,
    ev_connectable
};

class MyContactListener : public b2ContactListener
{
    public:

        bool init(b2World*   world_ptr,
                  b2Fixture** ppRope_hook_sensors,
                  //b2Fixture** ppSensor_rope_hook_p1,
                  //b2Fixture** ppSensor_rope_hook_p2,
                  //b2Fixture** ppSensor_rope_hook_p3,
                  //b2Fixture** ppSensor_rope_hook_p4,
                  b2Fixture* pSensor_mship_landing,
                  b2Fixture* pSensor_mship_input,
                  b2Body**   ppPlayer_bodies,
                  int*       event_flag_hook,
                  bool*      event_flag_input_box,
                  /*bool*      sensor_flag_rope_hook,*/
                  bool*      sensor_flag_mship_landing,
                  b2Body**   ppBody_in_mship_input,
                  b2Body**   ppBody_to_connect,
                  b2Fixture* pSensor_mship_landing_gear_left,
                  b2Fixture* pSensor_mship_landing_gear_right,
                  bool*      event_flag_landing_gear,
                  std::vector<b2Body*>* pVec_projectiles_to_remove,
                  std::vector<st_collision_event>* pVec_collision_events);

        bool set_player_bodies(b2Body** ppPlayer_bodies);

        std::vector<b2Body*> m_vec_pBodies_in_input_box;

    private:

        b2World*                 m_pWorld;
        b2Fixture**              m_ppRope_hook_sensors;
        int*                     m_pEvent_flag_hook;
        bool*                    m_pEvent_flag_input_box;
        //bool*                    m_pSensor_flag_rope_hook;
        bool*                    m_pSensor_flag_mship_landing;
        b2Body**                 m_ppBody_to_connect;//one per player
        b2Body**                 m_ppBody_in_mship_input;
        b2Body**                 m_ppPlayer_bodies;
        b2Fixture*               m_pSensor_mship_landing;
        b2Fixture*               m_pSensor_mship_input;
        b2Fixture*               m_pSensor_mship_landing_gear_left;
        b2Fixture*               m_pSensor_mship_landing_gear_right;
        bool*                    m_pEvent_flag_landing_gear;//0-left, 1-right (true if made contact)

        //std::vector<b2Fixture**>         m_vec_ppSensor_rope_hook;
        std::vector<b2Body*>*            m_pVec_projectiles_to_remove;
        std::vector<st_collision_event>* m_pVec_collision_events;

        void BeginContact(b2Contact* contact);
        void EndContact(b2Contact* contact);
        void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
        void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};

#endif // MYCONTACTLISTENER_H
