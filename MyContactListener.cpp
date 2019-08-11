#include "MyContactListener.h"

bool MyContactListener::init( b2World*   world_ptr,
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
                              std::vector<st_collision_event>* pVec_collision_events)
{
    m_pWorld=world_ptr;
    m_ppRope_hook_sensors=ppRope_hook_sensors;
    //for(int sens_i=0;sens_i<4;sens_i++)
    // m_vec_ppSensor_rope_hook.push_back(ppRope_hook_sensors[sens_i]);
    /*m_vec_ppSensor_rope_hook.push_back(ppSensor_rope_hook_p1);
    m_vec_ppSensor_rope_hook.push_back(ppSensor_rope_hook_p2);
    m_vec_ppSensor_rope_hook.push_back(ppSensor_rope_hook_p3);
    m_vec_ppSensor_rope_hook.push_back(ppSensor_rope_hook_p4);*/
    m_pSensor_mship_landing=pSensor_mship_landing;
    m_pSensor_mship_input=pSensor_mship_input;
    m_pEvent_flag_hook=event_flag_hook;
    m_pEvent_flag_input_box=event_flag_input_box;
    /*m_pSensor_flag_rope_hook=sensor_flag_rope_hook;*/
    m_pSensor_flag_mship_landing=sensor_flag_mship_landing;
    m_ppBody_to_connect=ppBody_to_connect;
    m_ppPlayer_bodies=ppPlayer_bodies;
    m_ppBody_in_mship_input=ppBody_in_mship_input;
    m_pSensor_mship_landing_gear_left=pSensor_mship_landing_gear_left;
    m_pSensor_mship_landing_gear_right=pSensor_mship_landing_gear_right;
    m_pEvent_flag_landing_gear=event_flag_landing_gear;
    m_pVec_projectiles_to_remove=pVec_projectiles_to_remove;
    m_pVec_collision_events=pVec_collision_events;
    //std::cout<<"init Vec address: "<<pVec_projectiles_to_remove<<endl;
    //pVec_projectiles_to_remove.clear();
    m_vec_pBodies_in_input_box.clear();
    //m_pVec_collision_events.clear();

    return true;
}

bool MyContactListener::set_player_bodies(b2Body** ppPlayer_bodies)
{
    m_ppPlayer_bodies=ppPlayer_bodies;

    return true;
}

void MyContactListener::BeginContact(b2Contact* contact)
{
    //cout<<"Collission test start\n";
    //get colliders fixtures and bodies
    b2Fixture* fixA=contact->GetFixtureA();
    b2Fixture* fixB=contact->GetFixtureB();
    b2Body* bodyA=fixA->GetBody();
    b2Body* bodyB=fixB->GetBody();
    st_body_user_data* dataA=(st_body_user_data*)bodyA->GetUserData();
    st_body_user_data* dataB=(st_body_user_data*)bodyB->GetUserData();

    //ignore if projectile is

    //cout<<"Projectile test\n";
    //if projectile, remove and give damage
    if( (dataA->s_info=="projectile") ^ (dataB->s_info=="projectile") )
    {
        if( dataA->s_info=="projectile" )//report damage to B, remove projectile A
        {
            //ignore sensors
            if(fixB->IsSensor()) {/*cout<<" end\n";*/ return;}

            //cout<<"Projectile caused damage to "<<dataB->s_info<<endl;
            dataB->f_projectile_damage_update+=dataA->f_projectile_damage_update;
            //mark A for deletion, if not already marked
            bool already_marked=false;
            for(int proj_i=0;proj_i<(int)m_pVec_projectiles_to_remove->size();proj_i++)
            {
                if( (*m_pVec_projectiles_to_remove)[proj_i]==bodyA )
                {
                    already_marked=true;
                    break;
                }
            }
            if(!already_marked) m_pVec_projectiles_to_remove->push_back(bodyA);

            //cout<<" end\n";
            return;
        }
        else//report damage to A, remove projectile B
        {
            //ignore sensors
            if(fixA->IsSensor()) {/*cout<<" end\n";*/ return;}

            //cout<<"Projectile caused damage to "<<dataA->s_info<<endl;
            dataA->f_projectile_damage_update+=dataB->f_projectile_damage_update;
            //mark B for deletion, if not already marked
            bool already_marked=false;
            for(int proj_i=0;proj_i<(int)m_pVec_projectiles_to_remove->size();proj_i++)
            {
                if( (*m_pVec_projectiles_to_remove)[proj_i]==bodyB )
                {
                    already_marked=true;
                    break;
                }
            }
            if(!already_marked) m_pVec_projectiles_to_remove->push_back(bodyB);

            //cout<<" end\n";
            return;
        }
    }
    else if(dataA->s_info=="projectile"&&dataB->s_info=="projectile")//projectile-projectile collision
    {
        //mark A and B for deletion, if not already marked
        bool already_marked_A=false;
        bool already_marked_B=false;
        for(int proj_i=0;proj_i<(int)m_pVec_projectiles_to_remove->size();proj_i++)
        {
            if( (*m_pVec_projectiles_to_remove)[proj_i]==bodyA )
            {
                already_marked_A=true;
            }
            else if( (*m_pVec_projectiles_to_remove)[proj_i]==bodyB )
            {
                already_marked_B=true;
            }
        }

        if(!already_marked_A) m_pVec_projectiles_to_remove->push_back(bodyA);
        if(!already_marked_B) m_pVec_projectiles_to_remove->push_back(bodyB);
        //delete dataA;
        //delete dataB;
        //m_pWorld->DestroyBody(bodyA);
        //m_pWorld->DestroyBody(bodyB);
        //cout<<" end\n";
        return;
    }

    //drone
    if( (dataA->s_info=="drone") || (dataB->s_info=="drone") )
    {
        cout<<"Collision: "<<dataA->s_info<<", and "<<dataB->s_info<<endl;

        if(dataA->s_info=="drone")//A is drone
        {
            //if collision with player ship
            if(dataB->s_info=="player")
            {
                m_pVec_collision_events->push_back( st_collision_event(bodyA,bodyB,ct_drone_player) );

                return;
            }

            //if collision with input box
            if(fixB==m_pSensor_mship_input)
            {
                m_pVec_collision_events->push_back( st_collision_event(bodyA,ct_drone_inputbox) );

                return;
            }

            //collision with something else (not sensors)
            if(!fixB->IsSensor())
            {
                //cout<<"a\n";
                //mark drone for removal
                m_pVec_collision_events->push_back( st_collision_event(bodyA,ct_unknown) );
                //if both A and B are drones, mark both
                if(dataB->s_info=="drone")
                 m_pVec_collision_events->push_back( st_collision_event(bodyB,ct_unknown) );

                return;
            }
        }
        else//B is drone
        {
            //if collision with player ship
            if(dataA->s_info=="player")
            {
                m_pVec_collision_events->push_back( st_collision_event(bodyB,bodyA,ct_drone_player) );

                return;
            }

            //if collision with input box
            if(fixA==m_pSensor_mship_input)
            {
                m_pVec_collision_events->push_back( st_collision_event(bodyB,ct_drone_inputbox) );

                return;
            }

            //collision with something else (not sensors)
            if(!fixA->IsSensor())
            {
                //cout<<"b\n";
                //mark drone for removal
                m_pVec_collision_events->push_back( st_collision_event(bodyB,ct_unknown) );
                //if both A and B are drones, mark both
                if(dataA->s_info=="drone")
                 m_pVec_collision_events->push_back( st_collision_event(bodyA,ct_unknown) );

                //cout<<(int)m_pVec_collision_events->size()<<endl;

                return;
            }
        }
    }

    //cout<<"Land strip sensor\n";
    //if landing strip sensor, report flag
    if(fixA==m_pSensor_mship_landing||fixB==m_pSensor_mship_landing)
    {
        if(fixA==m_pSensor_mship_landing)//test if B is a player
        {
            b2Body* bodyB=fixB->GetBody();
            for(int player_i=0;player_i<4;player_i++)
            {
                if(bodyB==m_ppPlayer_bodies[player_i])
                {
                    m_pSensor_flag_mship_landing[player_i]=true;
                    //cout<<" end\n";
                    return;
                }
            }
        }
        else//test if A is a player
        {
            b2Body* bodyA=fixA->GetBody();
            for(int player_i=0;player_i<4;player_i++)
            {
                if(bodyA==m_ppPlayer_bodies[player_i])
                {
                    m_pSensor_flag_mship_landing[player_i]=true;
                    //cout<<" end\n";
                    return;
                }
            }
        }
        //cout<<" end\n";
        return;
    }
    //cout<<" done\n";

    //cout<<"Input box sensor\n";
    //if input box sensor, report flag and body
    if(fixA==m_pSensor_mship_input||fixB==m_pSensor_mship_input)
    {
        if(fixA==m_pSensor_mship_input)//report B in input box
        {
            //skip if B is a sensor
            if(fixB->IsSensor()) return;

            if(*m_pEvent_flag_input_box)
            {
                //in bodies entered at the same frame
                b2Body* bodyB=fixB->GetBody();
                m_vec_pBodies_in_input_box.push_back(bodyB);
            }
            else
            {
                b2Body* bodyB=fixB->GetBody();
                *m_ppBody_in_mship_input=bodyB;
                *m_pEvent_flag_input_box=true;
            }

            //cout<<" end\n";
            return;
        }
        else//report A in input box
        {
            if(fixA->IsSensor()) return;

            if(*m_pEvent_flag_input_box)
            {
                //in bodies entered at the same frame
                b2Body* bodyA=fixA->GetBody();
                m_vec_pBodies_in_input_box.push_back(bodyA);
            }
            else
            {
                b2Body* bodyA=fixA->GetBody();
                *m_ppBody_in_mship_input=bodyA;
                *m_pEvent_flag_input_box=true;
            }

            //cout<<" end\n";
            return;
        }
    }
    //cout<<" done\n";

    //ship rope hook test
    //cout<<"Hook sensor\n";
    //test if rope sensors are active
    for(int player_i=0;player_i<4;player_i++)
    {
        //test if players hook sensor was involved
        if(fixA==m_ppRope_hook_sensors[player_i] || fixB==m_ppRope_hook_sensors[player_i])
        {
            //std::cout<<"Hook connectable for player: "<<player_i<<std::endl;

            //store other body for later connection
            if(fixA==m_ppRope_hook_sensors[player_i])
            {
                if(fixB->IsSensor()) return;//no connection to sensors
                //only allow connection to players, objects, terrain
                if( dataB->s_info!="player"  &&
                    dataB->s_info!="object"  &&
                    dataB->s_info!="terrain" &&
                    dataB->s_info!="enemy"   &&
                    dataB->s_info!="menutext"&&
                    dataB->s_info!="main_ship") {/*cout<<" end\n";*/ return;}

                m_ppBody_to_connect[player_i]=fixB->GetBody();

                //b2Vec2 pos=fixB->GetBody()->GetPosition();
                //std::cout<<"iPOS: "<<pos.x<<", "<<pos.y<<std::endl;
            }
            else if(fixB==m_ppRope_hook_sensors[player_i])
            {
                if(fixA->IsSensor()) return;//no connection to sensors

                //only allow connection to players, objects, terrain
                if( dataA->s_info!="player"  &&
                    dataA->s_info!="object"  &&
                    dataA->s_info!="terrain" &&
                    dataA->s_info!="enemy"   &&
                    dataA->s_info!="menutext"&&
                    dataA->s_info!="main_ship") {/*cout<<" end\n";*/ return;}

                m_ppBody_to_connect[player_i]=fixA->GetBody();

                //b2Vec2 pos=fixA->GetBody()->GetPosition();
                //std::cout<<"iPOS: "<<pos.x<<", "<<pos.y<<std::endl;
            }
            else//not connectable
            {
                m_pEvent_flag_hook[player_i]=ev_idle;//error
                //cout<<" end\n";
                return;
            }

            //raise flag that hook is connectable
            m_pEvent_flag_hook[player_i]=ev_connectable;
            //cout<<" end\n";
            return;
        }
    }

    //test if landing gear left made connection
    if(fixA==m_pSensor_mship_landing_gear_left||fixB==m_pSensor_mship_landing_gear_left)
    {
        m_pEvent_flag_landing_gear[0]=true;
        cout<<"Left landing gear sensor triggered\n";
        //cout<<" end\n";
        return;
    }
    if(fixA==m_pSensor_mship_landing_gear_right||fixB==m_pSensor_mship_landing_gear_right)
    {
        m_pEvent_flag_landing_gear[1]=true;
        cout<<"Right landing gear sensor triggered\n";
        //cout<<" end\n";
        return;
    }

    //cout<<"Collission test end\n";
}

void MyContactListener::EndContact(b2Contact* contact)
{
    //cout<<"CollisionEnd test start\n";
    //find which sensor
    b2Fixture* fixA=contact->GetFixtureA();
    b2Fixture* fixB=contact->GetFixtureB();
    //b2Body* bodyA=fixA->GetBody();
    //b2Body* bodyB=fixB->GetBody();
    //WARNING! The data may have been deleted if this call was made due to a body was destroyed
    //If the DATA is needed, make sure that the body is not marked for deletion
    /*st_body_user_data* dataA=(st_body_user_data*)bodyA->GetUserData();
    st_body_user_data* dataB=(st_body_user_data*)bodyB->GetUserData();
    cout<<"ColEnd between: "<<dataA->s_info<<" and "<<dataB->s_info<<endl;*/

    //mship landing sensor
    //cout<<"ColEnd mship landing sensor\n";
    if(fixA==m_pSensor_mship_landing||fixB==m_pSensor_mship_landing)
    {
        if(fixA==m_pSensor_mship_landing)//test if B is a player
        {
            b2Body* bodyB=fixB->GetBody();
            for(int player_i=0;player_i<4;player_i++)
            {
                if(bodyB==m_ppPlayer_bodies[player_i])
                {
                    m_pSensor_flag_mship_landing[player_i]=false;
                    break;
                }
            }
            //cout<<" end\n";
            return;
        }
        else//test if A is a player
        {
            b2Body* bodyA=fixA->GetBody();
            for(int player_i=0;player_i<4;player_i++)
            {
                if(bodyA==m_ppPlayer_bodies[player_i])
                {
                    m_pSensor_flag_mship_landing[player_i]=false;
                    break;
                }
            }
            //cout<<" end\n";
            return;
        }
    }

    //input box sensor, report flag and body
    //cout<<"ColEnd mship input sensor\n";
    if(fixA==m_pSensor_mship_input||fixB==m_pSensor_mship_input)
    {
        if(fixA==m_pSensor_mship_input)//report B in input box
        {
            *m_pEvent_flag_input_box=false;
        }
        else//report A in input box
        {
            *m_pEvent_flag_input_box=false;
        }
        //cout<<" end\n";
        return;
    }

    //hook sensor
    //cout<<"ColEnd hook sensor\n";
    for(int player_i=0;player_i<4;player_i++)
    {
        if(fixA==m_ppRope_hook_sensors[player_i] || fixB==m_ppRope_hook_sensors[player_i])
        {
            //std::cout<<"Hook not connectable for player: "<<player_i<<std::endl;;
            //end sensor contact
            m_pEvent_flag_hook[player_i]=ev_idle;
        }
    }

    //test if landing gear left lost connection
    if(fixA==m_pSensor_mship_landing_gear_left||fixB==m_pSensor_mship_landing_gear_left)
    {
        m_pEvent_flag_landing_gear[0]=false;
        cout<<"Left landing gear sensor untriggered\n";
        //cout<<" end\n";
        return;
    }
    //test if landing gear right lost connection
    if(fixA==m_pSensor_mship_landing_gear_right||fixB==m_pSensor_mship_landing_gear_right)
    {
        m_pEvent_flag_landing_gear[1]=false;
        cout<<"Right landing gear sensor untriggered\n";
        //cout<<" end\n";
        return;
    }

    //cout<<"CollisionEnd test end\n";
}

void MyContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    b2WorldManifold worldManifold;

    contact->GetWorldManifold(&worldManifold);

    b2PointState state1[2], state2[2];

    b2GetPointStates(state1, state2, oldManifold, contact->GetManifold());
    //NSLog(@"Presolving");

    if (state2[0] == b2_addState)
    {
        const b2Body* bodyA = contact->GetFixtureA()->GetBody();
        const b2Body* bodyB = contact->GetFixtureB()->GetBody();
        b2Vec2 point = worldManifold.points[0];
        b2Vec2 vA = bodyA->GetLinearVelocityFromWorldPoint(point);
        b2Vec2 vB = bodyB->GetLinearVelocityFromWorldPoint(point);
        b2Vec2 rV = vB - vA;
        float32 approachVelocity = b2Dot(rV, worldManifold.normal);

        //cout<<"Vel: "<<approachVelocity<<endl;
        if(approachVelocity>-_collision_damage_limit) return;//below limit (velocity is negative and more negative if higher speed)

        //report damage to body, if body is player or...
        st_body_user_data* dataA=(st_body_user_data*)bodyA->GetUserData();
        st_body_user_data* dataB=(st_body_user_data*)bodyB->GetUserData();
        if(dataA->s_info=="player"||dataA->s_info=="enemy")
        {//set damage
            dataA->f_collision_damage_update+=approachVelocity;
        }
        if(dataB->s_info=="player"||dataB->s_info=="enemy")
        {//set damage
            dataB->f_collision_damage_update+=approachVelocity;
        }

        /*if (-5.0f < approachVelocity && approachVelocity < 1.0f)
        {

            //MyPlayCollisionSound();
            //NSLog(@"Not Playing Sound");
        }
        else
        {
            //NSLog(@"playing the sound");
        }*/

    }
}

void MyContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)
{
    ;
}
