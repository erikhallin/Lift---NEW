#include "weapon.h"

weapon::weapon()
{
    m_type=wt_unarmed;
    m_beam_pos_updated=false;
}

weapon::weapon(b2World* world_ptr,int type,float variation_tolerance,int subtype)
{
    m_pWorld=world_ptr;
    m_type=type;
    m_subtype=subtype;
    //create variables
    m_time_last_fired=(float)clock()/(float)CLOCKS_PER_SEC;
    m_time_when_ready_to_fire=m_time_last_fired;
    switch(m_type)
    {
        case wt_pea:
        {
            m_fire_cooldown=0.5;
            m_ammo_cost_per_shot=1.0;
            m_bullet_damage=-10.0;
            m_bullet_speed=20.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_burst:
        {
            m_fire_cooldown=1.0;
            m_fire_cooldown_part=0.1;
            m_burst_counter=0;
            m_burst_counter_max=3;
            m_ammo_cost_per_shot=1.0;
            m_bullet_damage=-10.0;
            m_bullet_speed=10.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_fire_cooldown_part+=m_fire_cooldown_part*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_burst_counter_max+=m_burst_counter_max*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_rapid:
        {
            m_fire_cooldown=0.12;
            m_spread_factor=0.1;
            m_ammo_cost_per_shot=1.0;
            m_bullet_damage=-10.0;
            m_bullet_speed=10.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_spread:
        {
            m_fire_cooldown=1.0;
            m_spread_factor=0.3;
            m_burst_counter=0;
            m_burst_counter_max=5;
            m_ammo_cost_per_shot=5.0;
            m_bullet_damage=-5.0;
            m_bullet_speed=10.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_burst_counter_max+=m_burst_counter_max*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_rocket:
        {
            m_fire_cooldown=1.0;
            m_spread_factor=0.1;
            m_ammo_cost_per_shot=10.0;
            m_bullet_damage=-100.0;
            m_bullet_speed=10.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_grenade:
        {
            m_fire_cooldown=2.0;
            m_spread_factor=0.1;
            m_grenade_timer=3.0;
            m_ammo_cost_per_shot=10.0;
            m_bullet_damage=-10.0;
            m_bullet_speed=15.0;
            m_subtype=rand()%wst_second_timed+wst_impact;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_grenade_timer+=m_grenade_timer*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_cannon:
        {
            m_fire_cooldown=2.0;
            m_spread_factor=0.1;
            m_range_max=100.0;
            m_ammo_cost_per_shot=10.0;
            m_bullet_damage=-50.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_range_max+=m_range_max*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            //m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_beam:
        {
            m_fire_cooldown=0.0;
            m_spread_factor=0.1;
            m_range_max=100.0;
            m_beam_damage=-500.0;
            m_ammo_cost_per_shot=3.0;

            //m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_range_max+=m_range_max*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_beam_damage+=m_beam_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            //m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            //m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;

        case wt_mine:
        {
            m_fire_cooldown=2.0;
            m_spread_factor=0.1;
            m_ammo_cost_per_shot=15.0;
            m_bullet_damage=-10.0;
            m_bullet_speed=2.0;

            m_fire_cooldown+=m_fire_cooldown*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_spread_factor+=m_spread_factor*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_ammo_cost_per_shot+=m_ammo_cost_per_shot*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_damage+=m_bullet_damage*variation_tolerance*(float(rand()%1000)/1000.0-0.5);
            m_bullet_speed+=m_bullet_speed*variation_tolerance*(float(rand()%1000)/1000.0-0.5);

        }break;
    }

    //give random color temp
    m_weapon_color[0]=float(rand()%500)/1000.0+0.5;
    m_weapon_color[1]=float(rand()%500)/1000.0+0.5;
    m_weapon_color[2]=float(rand()%500)/1000.0+0.5;
}

bool weapon::is_ready_to_fire()
{
    //cout<<"ready test...";

    if( (float)clock()/(float)CLOCKS_PER_SEC>=m_time_when_ready_to_fire )
     return true;

    return false;
}

int weapon::fire_weapon(b2Vec2 pos,b2Vec2 fire_direction,float time_dif)
{
    //cout<<"Fire..";
    m_time_last_fired=(float)clock()/(float)CLOCKS_PER_SEC;

    //create projectile
    switch(m_type)
    {
        case wt_pea:
        {
            //cout<<"1";
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            float bullet_speed_sens=m_bullet_speed;//20
            //float bullet_age=10.0;//in sec, is not currently updated, removed if collision with world edge
            float bullet_damage=m_bullet_damage;
            //cout<<"2";
            b2BodyDef bodydef;
            bodydef.position=pos;
            bodydef.type=b2_dynamicBody;
            //cout<<"3";
            bodydef.linearDamping=0.0;
            bodydef.angularDamping=0.0;
            bodydef.linearVelocity=bullet_speed_sens*fire_direction;
            bodydef.gravityScale=0.0f;
            //cout<<"4";
            b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);//crash here!!!
            //set data
            //cout<<"5";
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="projectile";
            user_data->i_id=m_type;
            //cout<<"6";
            user_data->f_projectile_damage_update=bullet_damage;
            projectile_body->SetUserData(user_data);
            //cout<<"7";
            //fixture
            b2CircleShape shape;
            shape.m_radius=0.05;
            b2FixtureDef fixturedef;
            //cout<<"8";
            fixturedef.shape=&shape;
            fixturedef.density=0.1;
            //fixturedef.isSensor=true;
            //cout<<"9";
            projectile_body->CreateFixture(&fixturedef);//or crash here!!!
            //cout<<"projectile made..";
            //rotate
            //float fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;
            //projectile_body->SetTransform(projectile_body->GetPosition(),(fire_angle-90.0)*_Deg2Rad);
        }break;

        case wt_burst:
        {
            if(++m_burst_counter>=m_burst_counter_max)
            {//wait for next burst
                m_burst_counter=0;
                m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;
            }
            else m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown_part;//waint only for part of burst

            float bullet_speed_sens=m_bullet_speed;
            //float bullet_age=10.0;//in sec, in snot currently updated, removed if collision with world edge
            float bullet_damage=m_bullet_damage;
            b2BodyDef bodydef;
            bodydef.position=pos;
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=0.0;
            bodydef.angularDamping=0.0;
            bodydef.linearVelocity=bullet_speed_sens*fire_direction;
            bodydef.gravityScale=0.0f;
            b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="projectile";
            user_data->i_id=m_type;
            user_data->f_projectile_damage_update=bullet_damage;
            projectile_body->SetUserData(user_data);
            //fixture
            b2CircleShape shape;
            shape.m_radius=0.05;
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape;
            fixturedef.density=0.1;
            //fixturedef.isSensor=true;
            projectile_body->CreateFixture(&fixturedef);
        }break;

        case wt_rapid:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            //recalc fire direction due to spread
            float new_fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;
            new_fire_angle+=new_fire_angle*(float(rand()%1000)/1000.0-0.5)*m_spread_factor;
            b2Vec2 fire_direction_spread( cosf(new_fire_angle*_Deg2Rad), sinf(new_fire_angle*_Deg2Rad) );

            float bullet_speed_sens=m_bullet_speed;
            //float bullet_age=10.0;//in sec, in snot currently updated, removed if collision with world edge
            float bullet_damage=m_bullet_damage;
            b2BodyDef bodydef;
            bodydef.position=pos;
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=0.0;
            bodydef.angularDamping=0.0;
            bodydef.linearVelocity=bullet_speed_sens*fire_direction_spread;
            bodydef.gravityScale=0.0f;
            b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="projectile";
            user_data->i_id=m_type;
            user_data->f_projectile_damage_update=bullet_damage;
            projectile_body->SetUserData(user_data);
            //fixture
            b2CircleShape shape;
            shape.m_radius=0.03;
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape;
            fixturedef.density=0.1;
            //fixturedef.isSensor=true;
            projectile_body->CreateFixture(&fixturedef);
        }break;

        case wt_spread:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            float bullet_size=0.03;
            float bullet_spaceing=0.03;
            float distance_per_step=bullet_size*2.0+bullet_spaceing;
            //pack bullet along barrel
            vector<b2Vec2> vec_bullet_pos;
            b2Vec2 fire_dir_norm=fire_direction;
            fire_dir_norm.Normalize();
            for(int bullet_i=0;bullet_i<m_burst_counter_max;bullet_i++)
            {
                vec_bullet_pos.push_back( b2Vec2( pos.x+fire_dir_norm.x*(float)bullet_i*distance_per_step,
                                                  pos.y+fire_dir_norm.y*(float)bullet_i*distance_per_step ) );
            }

            //create projectile
            for(int bullet_i=0;bullet_i<m_burst_counter_max;bullet_i++)
            {
                //recalc fire direction due to spread
                float new_fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;
                new_fire_angle+=100.0*(float(rand()%1000)/1000.0-0.5)*m_spread_factor;
                b2Vec2 fire_direction_spread( cosf(new_fire_angle*_Deg2Rad), sinf(new_fire_angle*_Deg2Rad) );

                float bullet_speed_sens=m_bullet_speed;
                //float bullet_age=10.0;//in sec, is not currently updated, removed if collision with world edge
                float bullet_damage=m_bullet_damage;
                b2BodyDef bodydef;
                bodydef.position=vec_bullet_pos[bullet_i];
                bodydef.type=b2_dynamicBody;
                bodydef.linearDamping=0.0;
                bodydef.angularDamping=0.0;
                bodydef.linearVelocity=bullet_speed_sens*fire_direction_spread;
                bodydef.gravityScale=0.0f;
                b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
                //set data
                st_body_user_data* user_data=new st_body_user_data;
                user_data->s_info="projectile";
                user_data->i_id=m_type;
                user_data->f_projectile_damage_update=bullet_damage;
                projectile_body->SetUserData(user_data);
                //fixture
                b2CircleShape shape;
                shape.m_radius=bullet_size;
                b2FixtureDef fixturedef;
                fixturedef.shape=&shape;
                fixturedef.density=0.1;
                //fixturedef.isSensor=true;
                projectile_body->CreateFixture(&fixturedef);
            }
        }break;

        case wt_rocket:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            float fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;

            float bullet_speed_sens=m_bullet_speed;
            float bullet_damage=m_bullet_damage;
            b2BodyDef bodydef;
            bodydef.position=pos;
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=0.0;
            bodydef.angularDamping=0.0;
            bodydef.linearVelocity=bullet_speed_sens*fire_direction;
            bodydef.gravityScale=0.0f;
            b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="projectile";
            user_data->i_id=m_type;
            user_data->f_projectile_damage_update=bullet_damage;
            projectile_body->SetUserData(user_data);
            //fixture
            b2PolygonShape shape;
            shape.SetAsBox(0.15,0.07,b2Vec2(0,0),fire_angle*_Deg2Rad);
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape;
            fixturedef.density=0.1;
            //fixturedef.isSensor=true;
            projectile_body->CreateFixture(&fixturedef);
            //rotate
            projectile_body->SetTransform(projectile_body->GetPosition(),(fire_angle+90.0)*_Deg2Rad);
        }break;

        case wt_grenade:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            float fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;

            float bullet_speed_sens=m_bullet_speed;
            float bullet_damage=m_bullet_damage;
            b2BodyDef bodydef;
            bodydef.position=pos;
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_weapon_genade_damping_lin;
            bodydef.angularDamping=_weapon_genade_damping_ang;
            bodydef.linearVelocity=bullet_speed_sens*fire_direction;
            //bodydef.gravityScale=0.0f;
            b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="projectile";
            user_data->i_id=m_type;
            user_data->i_subtype=m_subtype;
            user_data->f_projectile_damage_update=bullet_damage;
            user_data->f_time_left=m_grenade_timer;
            projectile_body->SetUserData(user_data);
            //fixture
            b2CircleShape shape;
            shape.m_radius=0.1;
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape;
            fixturedef.density=0.1;
            fixturedef.restitution=0.9;
            //fixturedef.isSensor=true;
            projectile_body->CreateFixture(&fixturedef);
            //rotate
            projectile_body->SetTransform(projectile_body->GetPosition(),(fire_angle)*_Deg2Rad);
            projectile_body->SetAngularVelocity( float(rand()%200-100)/10.0 );
        }break;

        case wt_cannon:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            //float fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;

            //raycast to find hit pos
            b2Vec2 pos_target=b2Vec2( pos.x+fire_direction.x*m_range_max,
                                      pos.y+fire_direction.y*m_range_max );
            MyRayCastCallback callback;
            m_pWorld->RayCast(&callback,pos,pos_target);
            if(callback.m_any_hit)
            {
                //update target pos
                pos_target=b2Vec2( pos.x+fire_direction.x*m_range_max*callback.m_fraction,
                                   pos.y+fire_direction.y*m_range_max*callback.m_fraction );

                //deal direct hit damage
                b2Body* target_body=callback.m_pFixture->GetBody();
                st_body_user_data* data=(st_body_user_data*)target_body->GetUserData();
                if(data->s_info=="player"||data->s_info=="enemy"||data->s_info=="object")
                {
                    data->f_projectile_damage_update+=-50.0;
                }

                //spawn impact grenade at target
                float bullet_damage=m_bullet_damage;
                b2BodyDef bodydef;
                bodydef.position=pos_target;
                bodydef.type=b2_dynamicBody;
                bodydef.linearDamping=_weapon_genade_damping_lin;
                bodydef.angularDamping=_weapon_genade_damping_ang;
                b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
                //set data
                st_body_user_data* user_data=new st_body_user_data;
                user_data->s_info="projectile";
                user_data->i_id=wt_grenade;
                user_data->i_subtype=wst_impact;
                user_data->f_projectile_damage_update=bullet_damage;
                projectile_body->SetUserData(user_data);
                //fixture
                b2CircleShape shape;
                shape.m_radius=0.1;
                b2FixtureDef fixturedef;
                fixturedef.shape=&shape;
                fixturedef.density=0.1;
                fixturedef.restitution=0.9;
                projectile_body->CreateFixture(&fixturedef);

                return 1;//max range was enough
            }

        }break;

        case wt_beam:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            //float fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;

            //raycast to find hit pos
            b2Vec2 pos_target=b2Vec2( pos.x+fire_direction.x*m_range_max,
                                      pos.y+fire_direction.y*m_range_max );
            MyRayCastCallback callback;
            m_pWorld->RayCast(&callback,pos,pos_target);
            if(callback.m_any_hit)
            {
                //update target pos
                pos_target=b2Vec2( pos.x+fire_direction.x*m_range_max*callback.m_fraction,
                                   pos.y+fire_direction.y*m_range_max*callback.m_fraction );

                //deal direct hit damage
                b2Body* target_body=callback.m_pFixture->GetBody();
                st_body_user_data* data=(st_body_user_data*)target_body->GetUserData();
                if(data->s_info=="player"||data->s_info=="enemy"||data->s_info=="object")
                {
                    data->f_projectile_damage_update+=m_beam_damage*time_dif;//damage depends on time
                }

            }

            //store beam start and end pos
            m_vBeam_pos_start=b2Vec2( pos.x-fire_direction.x*_player_ship_to_barrel_dist*0.3,
                                      pos.y-fire_direction.y*_player_ship_to_barrel_dist*0.3 );
            m_vBeam_pos_end=pos_target;
            m_beam_pos_updated=true;

            if(callback.m_any_hit) return 1;//signal beam to emmit particles
            else return 0;

        }break;

        case wt_mine:
        {
            m_time_when_ready_to_fire=m_time_last_fired+m_fire_cooldown;

            float fire_angle=atan2f( fire_direction.x , -fire_direction.y )*_Rad2Deg-90.0;

            float bullet_speed_sens=m_bullet_speed;
            float bullet_damage=m_bullet_damage;
            b2BodyDef bodydef;
            bodydef.position=pos;
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_weapon_mine_damping_lin;
            bodydef.angularDamping=_weapon_mine_damping_ang;
            bodydef.linearVelocity=bullet_speed_sens*fire_direction;
            bodydef.gravityScale=0.0f;
            b2Body* projectile_body=m_pWorld->CreateBody(&bodydef);
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="projectile";
            user_data->i_id=m_type;
            user_data->i_subtype=m_subtype;
            user_data->f_projectile_damage_update=bullet_damage;
            user_data->f_time_left=m_grenade_timer;
            projectile_body->SetUserData(user_data);
            //fixture
            b2CircleShape shape;
            shape.m_radius=0.1;
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape;
            fixturedef.density=0.1;
            fixturedef.restitution=0.9;
            //fixturedef.isSensor=true;
            projectile_body->CreateFixture(&fixturedef);
            //rotate
            projectile_body->SetTransform(projectile_body->GetPosition(),(fire_angle)*_Deg2Rad);
            projectile_body->SetAngularVelocity( float(rand()%200-100)/100.0 );
        }break;
    }

    return 0;
}

int weapon::get_type(void)
{
    return m_type;
}

bool weapon::get_beam_pos(b2Vec2& start_pos,b2Vec2& end_pos)
{
    if(!m_beam_pos_updated) return false;

    start_pos=m_vBeam_pos_start;
    end_pos=m_vBeam_pos_end;

    m_beam_pos_updated=false;

    return true;
}

float weapon::get_ammo_cost(void)
{
    return m_ammo_cost_per_shot;
}

bool weapon::set_new_world(b2World* new_world_prt)
{
    m_pWorld=new_world_prt;

    return true;
}

