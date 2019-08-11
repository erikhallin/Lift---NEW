#include "main_ship.h"

main_ship::main_ship()
{
    m_landing_time_limit=20.0;//sec before landing timeout
    m_takeoff_time_limit=20.0;
    m_landing_gear_delay_limit=4.0;//time before landing gear extends
}

bool main_ship::init(b2World* world_ptr,particle_engine* pPart_eng,float gravity,int texture,
                     float* pMship_led_prog,bool reinit,b2Vec2 pos)
{
    m_pWorld=world_ptr;
    m_pParticle_engine=pPart_eng;
    m_texture=texture;
    m_in_landing_phase=true;
    m_in_takeoff_phase=false;
    m_land_phase=lp_init;
    m_takeoff_phase=tp_init;
    m_landing_start_height_pos=pos.y;
    m_landstrip_motor_in_use[0]=m_landstrip_motor_in_use[1]=m_landstrip_motor_in_use[2]=m_landstrip_motor_in_use[3]=false;
    m_balance_extra_thrust_factor=1.0;
    m_tilt_factor_last=0.5;
    m_world_gravity_curr=gravity;
    m_start_pos[0]=pos.x;
    m_start_pos[1]=pos.y;
    m_landing_time=_mship_landing_timeout;
    m_landing_gear_left_lock_on=m_landing_gear_right_lock_on=false;
    m_player_in_control=-1;
    m_auto_land=m_all_players_on_ship=true;
    m_fuel_tank_drawn=m_resources_drawn=0.0;
    for(int i=0;i<4;i++) m_player_inside[i]=true;
    m_pMship_led_prog=pMship_led_prog;
    m_sound_col_timer=0;

    m_lamp_landing_timer_inc=false;
    m_lamp_landing_timer=m_lamp_landing_time=1.0;

    if(!reinit)
    {
        m_fuel_tank_curr=_mship_fuel_start;
        m_fuel_tank_max=_mship_fuel_max;
        m_resources_curr=_mship_resources_start;
        m_resources_max=_mship_resources_max;
    }

    //create body
    b2BodyDef bodydef;
    bodydef.position.Set(pos.x,pos.y);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=_mship_damping_lin;
    bodydef.angularDamping=_mship_damping_ang;
    m_pBody=m_pWorld->CreateBody(&bodydef);
    //create fixture1
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-1.0,-2.0); b2Vec2 p2(-1.0,2.0); b2Vec2 p3(5.0,2.0); b2Vec2 p4(5.0,-2.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.density=_mship_density;
    m_pBody->CreateFixture(&fixturedef1);
    fixturedef1.filter.maskBits=~(_COLCAT_player1|_COLCAT_player2|_COLCAT_player3|_COLCAT_player4);//TEMP
    fixturedef1.filter.categoryBits=_COLCAT_mship;//TEMP
    }
    //create fixture2
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-4.0,0.0); b2Vec2 p2(-4.0,2.0); b2Vec2 p3(-1.0,2.0); b2Vec2 p4(-1.0,0.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.density=_mship_density;
    m_pBody->CreateFixture(&fixturedef1);
    fixturedef1.filter.maskBits=~(_COLCAT_player1|_COLCAT_player2|_COLCAT_player3|_COLCAT_player4);//TEMP
    fixturedef1.filter.categoryBits=_COLCAT_mship;//TEMP
    }
    //create fixture3
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-5.0,-2.0); b2Vec2 p2(-5.0,2.0); b2Vec2 p3(-4.0,2.0); b2Vec2 p4(-4.0,-2.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.density=_mship_density;
    m_pBody->CreateFixture(&fixturedef1);
    fixturedef1.filter.maskBits=~(_COLCAT_player1|_COLCAT_player2|_COLCAT_player3|_COLCAT_player4);//TEMP
    fixturedef1.filter.categoryBits=_COLCAT_mship;//TEMP
    }
    //create land gear left fixture
    {
    bodydef.position.Set(pos.x-2.5,pos.y+1.0);
    m_pLanding_gear_left=m_pWorld->CreateBody(&bodydef);
    b2PolygonShape shape1;
    //b2Vec2 p1(-3.5,0.0); b2Vec2 p2(-3.5,2.0); b2Vec2 p3(-1.5,2.0); b2Vec2 p4(-1.5,0.0);
    b2Vec2 p1(-1,-1.0); b2Vec2 p2(-1,1.0); b2Vec2 p3(1,1.0); b2Vec2 p4(1,-1.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.density=_mship_density;
    fixturedef1.friction=2.0;
    fixturedef1.filter.maskBits=~(_COLCAT_player1|_COLCAT_player2|_COLCAT_player3|_COLCAT_player4);//TEMP
    fixturedef1.filter.categoryBits=_COLCAT_mship;//TEMP
    m_pLanding_gear_left->CreateFixture(&fixturedef1);
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="main_ship_landing_gear";
    m_pLanding_gear_left->SetUserData(user_data);
    //landing gear sensor
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-1,1.0); b2Vec2 p2(-1,1.1); b2Vec2 p3(1,1.1); b2Vec2 p4(1,1.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.isSensor=true;
    fixturedef1.filter.categoryBits=_COLCAT_all;
    fixturedef1.filter.maskBits=-1;
    m_pLanding_gear_sensor_left=m_pLanding_gear_left->CreateFixture(&fixturedef1);
    }
    //prism joint with
    //prismatic joint, link rope with ship
    b2PrismaticJointDef prismJointDef;
    prismJointDef.bodyA = m_pBody;
    prismJointDef.bodyB = m_pLanding_gear_left;
    prismJointDef.localAxisA.Set(0.0,1.0);
    prismJointDef.collideConnected = false;
    prismJointDef.localAnchorA.Set(-2.5,1.0);
    prismJointDef.localAnchorB.Set(0,0);
    prismJointDef.enableLimit=true;
    prismJointDef.lowerTranslation=0.0;
    prismJointDef.upperTranslation=2.0;
    prismJointDef.enableMotor=true;
    prismJointDef.motorSpeed=-1000.0;
    prismJointDef.maxMotorForce=1000;
    m_pLanding_motor_join_left = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );
    }
    //create land gear right fixture
    {
    bodydef.position.Set(pos.x+2.5,pos.y+1.0);
    m_pLanding_gear_right=m_pWorld->CreateBody(&bodydef);
    b2PolygonShape shape1;
    //b2Vec2 p1(-3.5,0.0); b2Vec2 p2(-3.5,2.0); b2Vec2 p3(-1.5,2.0); b2Vec2 p4(-1.5,0.0);
    b2Vec2 p1(-1,-1.0); b2Vec2 p2(-1,1.0); b2Vec2 p3(1,1.0); b2Vec2 p4(1,-1.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.density=_mship_density;
    fixturedef1.friction=2.0;
    fixturedef1.filter.maskBits=~(_COLCAT_player1|_COLCAT_player2|_COLCAT_player3|_COLCAT_player4);//TEMP
    fixturedef1.filter.categoryBits=_COLCAT_mship;//TEMP
    m_pLanding_gear_right->CreateFixture(&fixturedef1);
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="main_ship_landing_gear";
    m_pLanding_gear_right->SetUserData(user_data);
    //landing gear sensor
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-1,1.0); b2Vec2 p2(-1,1.1); b2Vec2 p3(1,1.1); b2Vec2 p4(1,1.0);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.isSensor=true;
    m_pLanding_gear_sensor_right=m_pLanding_gear_right->CreateFixture(&fixturedef1);
    }
    //prism joint with
    //prismatic joint, link rope with ship
    b2PrismaticJointDef prismJointDef;
    prismJointDef.bodyA = m_pBody;
    prismJointDef.bodyB = m_pLanding_gear_right;
    prismJointDef.localAxisA.Set(0.0,1.0);
    prismJointDef.collideConnected = false;
    prismJointDef.localAnchorA.Set(2.5,1.0);
    prismJointDef.localAnchorB.Set(0,0);
    prismJointDef.enableLimit=true;
    prismJointDef.lowerTranslation=0.0;
    prismJointDef.upperTranslation=2.0;
    prismJointDef.enableMotor=true;
    prismJointDef.motorSpeed=-1000.0;
    prismJointDef.maxMotorForce=1000;
    m_pLanding_motor_join_right = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );
    }
    //create sensor input
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-3.8,-0.4); b2Vec2 p2(-3.8,-0.2); b2Vec2 p3(-1.2,-0.2); b2Vec2 p4(-1.2,-0.4);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.isSensor=true;
    fixturedef1.filter.categoryBits=_COLCAT_all;
    fixturedef1.filter.maskBits=-1;
    m_pInput_sensor=m_pBody->CreateFixture(&fixturedef1);
    }
    //create sensor landingstrip
    {
    b2PolygonShape shape1;
    b2Vec2 p1(-0.4,-2.6); b2Vec2 p2(-0.4,-2.2); b2Vec2 p3(4.4,-2.2); b2Vec2 p4(4.4,-2.6);
    //b2Vec2 p1(-3.4,-5.6); b2Vec2 p2(-3.4,-2.2); b2Vec2 p3(7.4,-2.2); b2Vec2 p4(7.4,-5.6);
    b2Vec2 arr[]={p1,p2,p3,p4};
    shape1.Set(arr,4);
    b2FixtureDef fixturedef1;
    fixturedef1.shape=&shape1;
    fixturedef1.isSensor=true;
    fixturedef1.filter.categoryBits=_COLCAT_all;
    fixturedef1.filter.maskBits=-1;
    m_pLand_sensor=m_pBody->CreateFixture(&fixturedef1);
    }
    //set center of rotation
    b2MassData massD;
    m_pBody->GetMassData(&massD);
    b2Vec2 centerV(0,0);
    massD.center = centerV;
    m_pBody->SetMassData(&massD);

    //set data
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="main_ship";
    m_pBody->SetUserData(user_data);
    m_pBody->SetLinearVelocity(b2Vec2(0,0));
    m_pBody->SetAngularVelocity(0.0);
    //move into pos
    //m_pBody->SetTransform( pos, _pi*0.2 );

    return true;
}

int main_ship::update(float time_dif)
{
    if(m_sound_col_timer>0) m_sound_col_timer-=time_dif;

    //fuel and resources bar
    float bar_speed=100.0;
    if(m_fuel_tank_drawn<m_fuel_tank_curr)
    {
        m_fuel_tank_drawn+=time_dif*bar_speed;
        if(m_fuel_tank_drawn>m_fuel_tank_curr) m_fuel_tank_drawn=m_fuel_tank_curr;
    }
    else if(m_fuel_tank_drawn>m_fuel_tank_curr)
    {
        m_fuel_tank_drawn-=time_dif*bar_speed;
        if(m_fuel_tank_drawn<m_fuel_tank_curr) m_fuel_tank_drawn=m_fuel_tank_curr;
    }
    if(m_resources_drawn<m_resources_curr)
    {
        m_resources_drawn+=time_dif*bar_speed;
        if(m_resources_drawn>m_resources_curr) m_resources_drawn=m_resources_curr;
    }
    else if(m_resources_drawn>m_resources_curr)
    {
        m_resources_drawn-=time_dif*bar_speed;
        if(m_resources_drawn<m_resources_curr) m_resources_drawn=m_resources_curr;
    }

    //ship lamp update
    if(m_in_landing_phase)
    {
        if(m_lamp_landing_timer_inc)//inc
        {
            m_lamp_landing_timer+=time_dif;
            if(m_lamp_landing_timer>m_lamp_landing_time)
            {
                m_lamp_landing_timer=m_lamp_landing_time;
                m_lamp_landing_timer_inc=false;
            }
        }
        else//dec
        {
            m_lamp_landing_timer-=time_dif;
            if(m_lamp_landing_timer<0.0)
            {
                m_lamp_landing_timer=0.0;
                m_lamp_landing_timer_inc=true;
            }
        }
    }
    else if(!m_lamp_landing_timer_inc)
    {
        m_lamp_landing_timer_inc=true;
        m_lamp_landing_timer=0.0;
    }

    //engine
    if(m_in_landing_phase && m_auto_land)
    {
        //land timeout
        if(m_landing_time>0 && (m_land_phase!=lp_on_ground||m_land_phase!=lp_full_stop))
         m_landing_time-=time_dif;
        if(m_landing_time<=0.0 && (m_land_phase!=lp_on_ground||m_land_phase!=lp_full_stop))
        {
            //force landing gear extension
            m_land_phase=lp_full_stop;
            m_landing_time=_mship_landing_timeout;
        }

        //update height pos
        m_landing_pos_curr=m_pBody->GetPosition().y;
        float landing_length_travelled=fabs(m_landing_start_height_pos-m_landing_pos_curr);
        //update fall speed
        b2Vec2 velocity=m_pBody->GetLinearVelocity();
        m_landing_fall_speed=velocity.y;
        //update dist to ground
        m_landing_dist_to_ground=m_landing_dist_max-landing_length_travelled;

        //motor pos
        b2Vec2 motor_pos_left(-4.2,2.3);
        b2Vec2 motor_pos_right(4.2,2.3);

        switch(m_land_phase)
        {
            case lp_init:
            {
                cout<<"Landing: fall\n";
                //measure distance to ground with raycast
                b2Vec2 ship_pos=m_pBody->GetPosition();
                float offset_dist_below_ship=5.0;
                ship_pos.y+=offset_dist_below_ship;//to avoid collision with ship
                b2Vec2 point_below=ship_pos;
                float search_distance=1000;
                point_below.y+=search_distance;//must hit ground
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,ship_pos,point_below);
                if(callback.m_any_hit)
                {
                    m_landing_dist_max=callback.m_fraction*search_distance+offset_dist_below_ship;
                    //cout<<m_landing_dist_max<<endl;
                }
                else
                {
                    cout<<"Landing: no ground found within "<<search_distance<<" meters\n";
                    m_landing_dist_max=search_distance;
                }
                cout<<"Landing: Distance to ground: "<<m_landing_dist_max<<endl;
                m_landing_dist_to_ground=m_landing_dist_max-landing_length_travelled;

                //set init values
                m_tilt_factor_last=m_pBody->GetAngle()/_pi*0.5+0.5;//from 0 to 1
                m_motor_thrust_curr=0;
                m_landing_thrust_min=-200.0;
                m_landing_last_height_pos=m_landing_start_height_pos;
                m_landing_fall_speed=0;
                m_landing_target_fall_speed=1.0;//0.07;
                m_landing_target_height_stop=2.5;//5.4;//units above ground
                m_landing_target_height_start_motor=50.0;
                m_landing_target_height_controlled_decline=30.0;
                m_landing_gear_left_lock_on=m_landing_gear_right_lock_on=m_landing_gear_delay_done=false;
                m_landing_gear_delay_timer=m_landing_timer=0.0;
                //m_landing_extra_stop_force=3.0;
                m_land_phase=lp_fall;

            }break;

            case lp_fall:
            {
                //if have passed half of the total length, stop ship from falling
                if(m_landing_dist_to_ground<m_landing_target_height_start_motor)
                {
                    m_land_phase=lp_motor_warm_up;
                    cout<<"Landing: motor warm up\n";
                }
                //else cout<<landing_length_travelled<<endl;
            }break;

            case lp_motor_warm_up:
            {
                //cout<<m_landing_fall_speed<<endl;
                if(m_landing_dist_to_ground<m_landing_target_height_controlled_decline)
                {
                    m_land_phase=lp_stop_fall;
                    cout<<"Landing: stop fall\n";
                }
                //if not minimum fall speed reached, no thrust
                if( m_landing_fall_speed<m_landing_target_fall_speed )
                {
                    m_motor_thrust_curr*=0.5;
                    //break;
                }

                //increase strength of motor until height reached
                float sens=10.0*time_dif*_world_gravity;
                m_motor_thrust_curr-=sens;

                b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                       sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                //adjust tilt -1 to 1 -> 0.5 to 1
                float tilt_angle=m_pBody->GetAngle();
                //wrap
                while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                //cout<<tilt_factor<<endl;
                b2Vec2 force_left =(1.0-tilt_factor)*force;
                b2Vec2 force_right=(    tilt_factor)*force;
                m_tilt_factor_last=tilt_factor;

                //add thrust to body
                m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*(-force_left) );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*(-force_right) );

            }break;

            case lp_stop_fall:
            {
                if(m_landing_fall_speed>0)//going down
                {
                    //increase thruster force relative to falling speed
                    float sens=2000.0*time_dif*_world_gravity;
                    m_motor_thrust_curr=-m_landing_fall_speed*sens-sens*2.0;
                    if(m_motor_thrust_curr>m_landing_thrust_min) m_motor_thrust_curr=m_landing_thrust_min;

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*(-force_right) );

                    //cout<<m_landing_fall_speed<<endl;
                }
                //test if target fall speed is reached
                //cout<<m_landing_fall_speed<<endl;
                if( m_landing_fall_speed<m_landing_target_fall_speed )
                {
                    cout<<"Landing: decline slow\n";
                    m_land_phase=lp_decline_slow;
                }

            }break;

            case lp_decline_slow:
            {
                //apply force only if fall speed to high
                if( m_landing_fall_speed>m_landing_target_fall_speed )
                {
                    //cout<<m_landing_fall_speed<<endl;

                    //increase thruster force relative to falling speed
                    float sens=1000*time_dif*_world_gravity;
                    m_motor_thrust_curr=m_landing_fall_speed*-sens-sens*2.0;
                    //cout<<m_motor_thrust_curr<<endl;

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*(-force_right) );
                }
                if(m_landing_dist_to_ground-5.0<m_landing_target_height_stop)
                {
                    cout<<"Landing: full stop\n";
                    //cout<<"min thrust: "<<m_motor_thrust_curr<<endl;
                    m_landing_thrust_min=m_motor_thrust_curr;
                    m_land_phase=lp_full_stop;
                }
                //else cout<<m_landing_dist_to_ground<<endl;

            }break;

            case lp_full_stop:
            {
                //cout<<m_landing_fall_speed<<endl;

                /*//reduce extra stop force to 1.0
                if(m_landing_extra_stop_force>1.0)
                {
                    m_landing_extra_stop_force-=time_dif*2.0;
                    if(m_landing_extra_stop_force<1.0) m_landing_extra_stop_force=1.0;

                    cout<<m_landing_extra_stop_force<<endl;
                }*/

                //try to hoover above targeted height
                if(m_landing_dist_to_ground<m_landing_target_height_stop)//too low
                {
                    //float dist_dif=-(landing_length_travelled-m_landing_dist_max+m_landing_target_height_stop);
                    //cout<<dist_dif<<endl;
                    //increase thrusters
                    m_motor_thrust_curr=-4500.0*time_dif*_world_gravity;
                    //if(m_motor_thrust_curr>m_landing_thrust_min) m_motor_thrust_curr=m_landing_thrust_min;//min thrust value
                    //cout<<m_motor_thrust_curr<<endl;

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_factor=m_pBody->GetAngle()/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*(-force_right) );
                }
                //else weak blast to keep height, if falling
                else /*if(m_landing_fall_speed>0.0)*/
                {
                    float sens=3900.0*time_dif*_world_gravity;
                    m_motor_thrust_curr=-sens;

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*(-force_right) );
                }

                //test if landing gear timer is done
                if(m_landing_gear_delay_timer<m_landing_gear_delay_limit) m_landing_gear_delay_timer+=time_dif;
                else if(!m_landing_gear_delay_done)//start gear motors
                {
                    cout<<"Landing: Gear extension\n";
                    m_landing_gear_delay_done=true;
                    //gear extension
                    m_pLanding_motor_join_left->SetMotorSpeed(0.2);
                    m_pLanding_motor_join_right->SetMotorSpeed(0.3);
                }

                //stop gear if touching ground
                if(m_pLanding_gear_touch[0])
                {
                    landing_gear_motor_left_lock(true);
                }
                if(m_pLanding_gear_touch[1])
                {
                    landing_gear_motor_right_lock(true);
                }

                //test if gears have reached ground
                if(m_pLanding_gear_touch[0] && m_pLanding_gear_touch[1])
                {
                    //now on ground
                    cout<<"Landing: Gear sensors triggered\n";
                    landing_gear_motor_left_lock(true);
                    landing_gear_motor_right_lock(true);
                    m_land_phase=lp_on_ground;
                }

                //test if landing gear is done
                if(m_landing_gear_left_lock_on&&m_landing_gear_right_lock_on)
                {
                    m_land_phase=lp_on_ground;
                    return 0;
                }
                else// test if should force land (both gears fully extended)
                {
                    if(m_pLanding_motor_join_left->GetJointTranslation() >= m_pLanding_motor_join_left->GetUpperLimit() &&
                       m_pLanding_motor_join_right->GetJointTranslation() >= m_pLanding_motor_join_right->GetUpperLimit() )
                    {
                        cout<<"Landing gears fully extended\n";

                        m_land_phase=lp_on_ground;
                        //force lock of both gear

                        //stop motors (done in functions)
                        //m_pLanding_motor_join_left->SetMotorSpeed(0);
                        //m_pLanding_motor_join_right->SetMotorSpeed(0);

                        //weld gears
                        landing_gear_motor_left_lock(true);
                        landing_gear_motor_right_lock(true);
                        /*if(!m_landing_gear_left_lock_on)
                        {
                            b2WeldJointDef weldJointDef;
                            weldJointDef.bodyA = m_pBody;
                            weldJointDef.bodyB = m_pLanding_gear_left;
                            weldJointDef.collideConnected = false;
                            b2Vec2 rel_pos=m_pBody->GetLocalPoint(m_pLanding_gear_left->GetPosition());
                            weldJointDef.localAnchorA=rel_pos;
                            weldJointDef.localAnchorB.Set(0,0);
                            m_pLanding_gear_left_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
                            m_landing_gear_left_lock_on=true;
                        }
                        if(!m_landing_gear_right_lock_on)
                        {
                            b2WeldJointDef weldJointDef;
                            weldJointDef.bodyA = m_pBody;
                            weldJointDef.bodyB = m_pLanding_gear_right;
                            weldJointDef.collideConnected = false;
                            b2Vec2 rel_pos=m_pBody->GetLocalPoint(m_pLanding_gear_right->GetPosition());
                            weldJointDef.localAnchorA=rel_pos;
                            weldJointDef.localAnchorB.Set(0,0);
                            m_pLanding_gear_right_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
                            m_landing_gear_right_lock_on=true;
                        }*/
                    }
                    else//or timer runs out
                    {
                        m_landing_timer+=time_dif;
                        //cout<<m_pLanding_motor_join_left->GetJointTranslation()<<" < "<<m_pLanding_motor_join_left->GetUpperLimit()<<endl;
                        if(m_landing_timer>m_landing_time_limit)
                        {
                            cout<<"Landing timeout\n";

                            m_land_phase=lp_on_ground;
                            //force lock of both gear

                            //stop motors
                            //m_pLanding_motor_join_left->SetMotorSpeed(0);
                            //m_pLanding_motor_join_right->SetMotorSpeed(0);

                            //weld gears
                            landing_gear_motor_left_lock(true);
                            landing_gear_motor_right_lock(true);
                            /*if(!m_landing_gear_left_lock_on)
                            {
                                b2WeldJointDef weldJointDef;
                                weldJointDef.bodyA = m_pBody;
                                weldJointDef.bodyB = m_pLanding_gear_left;
                                weldJointDef.collideConnected = false;
                                //b2Vec2 rel_pos=m_pLanding_gear_left->GetPosition()-m_pBody->GetPosition();
                                b2Vec2 rel_pos=m_pBody->GetLocalPoint(m_pLanding_gear_left->GetPosition());
                                weldJointDef.localAnchorA=rel_pos;
                                weldJointDef.localAnchorB.Set(0,0);
                                m_pLanding_gear_left_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
                                m_landing_gear_left_lock_on=true;
                            }
                            if(!m_landing_gear_right_lock_on)
                            {
                                b2WeldJointDef weldJointDef;
                                weldJointDef.bodyA = m_pBody;
                                weldJointDef.bodyB = m_pLanding_gear_right;
                                weldJointDef.collideConnected = false;
                                //b2Vec2 rel_pos=m_pLanding_gear_right->GetPosition()-m_pBody->GetPosition();
                                b2Vec2 rel_pos=m_pBody->GetLocalPoint(m_pLanding_gear_right->GetPosition());
                                weldJointDef.localAnchorA=rel_pos;
                                weldJointDef.localAnchorB.Set(0,0);
                                m_pLanding_gear_right_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
                                m_landing_gear_right_lock_on=true;
                            }*/
                        }
                    }
                }

                //return 1;//landing gear sensor test will be done in game (not anymore, tested locally)

            }break;

            case lp_on_ground:
            {
                //fade out thruster power
                m_motor_thrust_curr+=200.0*time_dif;
                if(m_motor_thrust_curr>=0)
                {
                    //landing completed
                    m_land_phase=lp_complete;
                    m_in_landing_phase=false;
                    return 2;
                }
                else//add some weak thrust
                {
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    b2Vec2 force_left =b2Vec2(force.x*(1.0-tilt_factor)*(1.0-tilt_factor),force.y*(1.0-tilt_factor)*(1.0-tilt_factor));
                    b2Vec2 force_right=b2Vec2(force.x*tilt_factor*tilt_factor,force.y*tilt_factor*tilt_factor);

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*(-force_right) );
                }
            }
        }


    }
    else if(m_in_takeoff_phase && m_auto_land && m_all_players_on_ship)
    {
        //update heigt pos
        m_landing_pos_curr=m_pBody->GetPosition().y;
        float landing_length_travelled=fabs(m_landing_start_height_pos-m_landing_pos_curr);
        //uppdate fall speed (average by 10 cycles)
        static int cycle_counter=10;
        if(--cycle_counter<=0)
        {
            cycle_counter=10;
            //cout<<landing_length_travelled<<endl;
            m_landing_fall_speed=m_landing_last_height_pos-m_landing_pos_curr;//negative if going down
            m_landing_last_height_pos=m_landing_pos_curr;
        }
        //update dist to ground
        m_landing_dist_to_ground=m_landing_dist_max-landing_length_travelled;

        //motor pos
        b2Vec2 motor_pos_left(-4.2,2.3);
        b2Vec2 motor_pos_right(4.2,2.3);

        switch(m_takeoff_phase)
        {
            case tp_init:
            {
                cout<<"Takeoff: begin takeoff\n";
                //set init values
                m_motor_thrust_curr=m_landing_thrust_min;
                m_takeoff_timer=0.0;
                m_takeoff_phase=tp_to_target_height;
                m_landing_time=_mship_landing_timeout;
                m_takeoff_thrust_adjust=0;
                m_takeoff_height_pos=m_pBody->GetPosition().y;
                m_takeoff_height_above_ground=3.0;
            }break;

            case tp_to_target_height:
            {
                //test if targeted height reached
                if(m_landing_pos_curr<m_takeoff_height_pos-m_takeoff_height_above_ground)
                {
                    cout<<"Takeoff: gear up\n";
                    m_takeoff_phase=tp_gear_up;
                    m_landing_thrust_min=m_motor_thrust_curr;
                    m_takeoff_thrust_adjust=0;

                    //unlock gears
                    //if(m_landing_gear_left_lock_on) m_pWorld->DestroyJoint(m_pLanding_gear_left_lock);
                    //if(m_landing_gear_right_lock_on) m_pWorld->DestroyJoint(m_pLanding_gear_right_lock);

                    //gears up
                    m_pLanding_motor_join_left->SetMotorSpeed(-1.0);
                    m_pLanding_motor_join_right->SetMotorSpeed(-1.0);
                }
                else//run thrusters
                {
                    //float dist_dif=-(landing_length_travelled-m_landing_dist_max+m_landing_target_height_stop);
                    //increase thrusters
                    m_takeoff_thrust_adjust+=10.0*time_dif*_world_gravity;//will be increased as long as below targeted height

                    m_motor_thrust_curr=-111.0*_world_gravity*time_dif-m_takeoff_thrust_adjust;
                    //cout<<m_motor_thrust_curr<<endl;
                    if(m_motor_thrust_curr>m_landing_thrust_min) m_motor_thrust_curr=m_landing_thrust_min;//min thrust value

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.10*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.10*(-force_right) );
                }

            }break;

            case tp_gear_up:
            {
                //test if gears are up
                if(m_pLanding_motor_join_left->GetJointTranslation() <= m_pLanding_motor_join_left->GetLowerLimit() &&
                   m_pLanding_motor_join_right->GetJointTranslation() <= m_pLanding_motor_join_right->GetLowerLimit() )
                {
                    cout<<"Takeoff: blastoff\n";
                    //gears are up, begin blastoff
                    m_takeoff_phase=tp_blastoff;

                    //stop motors
                    //m_pLanding_motor_join_left->SetMotorSpeed(0);
                    //m_pLanding_motor_join_right->SetMotorSpeed(0);

                    //weld gears
                    landing_gear_motor_left_lock(true);
                    landing_gear_motor_right_lock(true);
                    /*{
                    b2WeldJointDef weldJointDef;
                    weldJointDef.bodyA = m_pBody;
                    weldJointDef.bodyB = m_pLanding_gear_left;
                    weldJointDef.collideConnected = false;
                    b2Vec2 rel_pos=m_pLanding_gear_left->GetPosition()-m_pBody->GetPosition();
                    weldJointDef.localAnchorA=rel_pos;
                    weldJointDef.localAnchorB.Set(0,0);
                    m_pLanding_gear_left_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
                    }
                    {
                    b2WeldJointDef weldJointDef;
                    weldJointDef.bodyA = m_pBody;
                    weldJointDef.bodyB = m_pLanding_gear_right;
                    weldJointDef.collideConnected = false;
                    b2Vec2 rel_pos=m_pLanding_gear_right->GetPosition()-m_pBody->GetPosition();
                    weldJointDef.localAnchorA=rel_pos;
                    weldJointDef.localAnchorB.Set(0,0);
                    m_pLanding_gear_right_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
                    }

                    m_landing_gear_left_lock_on=true;
                    m_landing_gear_right_lock_on=true;*/
                }
                else//maintain height
                {
                    //float dist_dif=-(landing_length_travelled-m_landing_dist_max+m_landing_target_height_stop);
                    //increase thrusters
                    //m_motor_thrust_curr=200*dist_dif;
                    if(m_motor_thrust_curr>m_landing_thrust_min) m_motor_thrust_curr=m_landing_thrust_min;//min thrust value

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.10*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.10*(-force_right) );
                }
            }break;

            case tp_blastoff:
            {
                //test if height limit is reached
                if(m_landing_pos_curr<m_landing_start_height_pos-_mship_takeoff_hight_limit_dif)//higher that starting position
                {
                    cout<<"Takeoff: complete\n";
                    m_takeoff_phase=tp_blastoff_fadeout;
                    return 3;
                }
                else//more thrust
                {
                    //time limit
                    m_takeoff_timer+=time_dif;
                    if(m_takeoff_timer>m_takeoff_time_limit)
                    {
                        //force takeoff
                        cout<<"Takeoff: time out\n";
                        m_takeoff_phase=tp_blastoff_fadeout;
                        return 3;
                    }

                    //gradually increase thruster power
                    m_motor_thrust_curr-=222.0*time_dif*_world_gravity;

                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                    //adjust tilt -1 to 1 -> 0.5 to 1
                    float tilt_angle=m_pBody->GetAngle();
                    //wrap
                    while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                    while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                    float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                    //cout<<tilt_factor<<endl;
                    if(tilt_factor>0.5)
                    {
                        if(tilt_factor>=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    else if(tilt_factor<0.5)
                    {
                        if(tilt_factor<=m_tilt_factor_last)
                         m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                    }
                    //cout<<m_balance_extra_thrust_factor<<endl;

                    b2Vec2 force_left =(1.0-tilt_factor)*force;
                    b2Vec2 force_right=(    tilt_factor)*force;
                    force_right*=m_balance_extra_thrust_factor;
                    m_tilt_factor_last=tilt_factor;

                    //add thrust to body
                    m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                    m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.10*(-force_left) );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.10*(-force_right) );
                }

            }break;

            case tp_blastoff_fadeout:
            {
                //constant thruster power
                b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr ,
                                       sin(m_pBody->GetAngle()-_pi*0.5) * m_motor_thrust_curr );

                //adjust tilt -1 to 1 -> 0.5 to 1
                float tilt_angle=m_pBody->GetAngle();
                //wrap
                while(tilt_angle> _pi) tilt_angle-=_pi*2.0;
                while(tilt_angle<-_pi) tilt_angle+=_pi*2.0;
                float tilt_factor=tilt_angle/_pi*0.5+0.5;//from 0 to 1
                //cout<<tilt_factor<<endl;
                if(tilt_factor>0.5)
                {
                    if(tilt_factor>=m_tilt_factor_last)
                     m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                }
                else if(tilt_factor<0.5)
                {
                    if(tilt_factor<=m_tilt_factor_last)
                     m_balance_extra_thrust_factor+=5.0*(tilt_factor-m_tilt_factor_last);
                }
                //cout<<m_balance_extra_thrust_factor<<endl;

                b2Vec2 force_left =(1.0-tilt_factor)*force;
                b2Vec2 force_right=(    tilt_factor)*force;
                force_right*=m_balance_extra_thrust_factor;
                m_tilt_factor_last=tilt_factor;

                //add thrust to body
                m_pBody->ApplyForce( -force_left, m_pBody->GetWorldPoint( b2Vec2(-4.0,2.0) ), true );
                m_pBody->ApplyForce( -force_right, m_pBody->GetWorldPoint( b2Vec2(4.0,2.0) ), true );

                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.05*(-force_left) );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.05*(-force_right) );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.10*(-force_left) );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.10*(-force_right) );
            }break;

        }
    }

    //

    //land test
    if(!m_auto_land)
    {
        //if both gear sensors is true, ship have landed
        if(m_pLanding_gear_touch[0] && m_pLanding_gear_touch[1])
        {
            m_in_landing_phase=false;
            m_in_takeoff_phase=false;
            return 2;
        }
        else m_in_landing_phase=true;

        //if both gear sensors are false, ship can leave
        if(!m_pLanding_gear_touch[0] && !m_pLanding_gear_touch[1])
        {
            m_in_takeoff_phase=true;
            m_takeoff_phase=tp_init;

            //test if mship height is above takeoff limit
            b2Vec2 mship_pos=m_pBody->GetPosition();
            if(mship_pos.y<m_start_pos[1]-_mship_takeoff_hight_limit_dif && m_all_players_on_ship)
            {
                //leave map, if all players are on ship
                cout<<"Manual takeoff: Main ship is high enough to leave the level\n";
                return 4;
            }
            //else cout<<mship_pos.y<<"\t"<<m_start_pos[1]-_mship_takeoff_hight_limit_dif<<endl;
        }
        else m_in_takeoff_phase=false;
    }

    return 0;
}

bool main_ship::draw(void)
{
    b2Vec2 body_pos=m_pBody->GetWorldCenter();
    float angle=m_pBody->GetAngle();

    //draw fuel tank
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
    glRotatef(angle*180/_pi,0,0,1);
    float fuel_height=-45.0*(m_fuel_tank_drawn/m_fuel_tank_max);
    //rest is black
    glColor4f(0.0,0.0,0.0,1.0);
    glBegin(GL_QUADS);
    glVertex2f(-12.0,-25.0);
    glVertex2f(-12.0,25.0);
    glVertex2f(4.0,25.0);
    glVertex2f(4.0,-25.0);
    //fuel color
    glColor4f(0.6,0.6,0.2,1.0);
    glVertex2f(-8.0,fuel_height+21.0);
    glVertex2f(-8.0, 21.0);
    glVertex2f(-2.0, 21.0);
    glVertex2f(-2.0,fuel_height+21.0);
    glEnd();

    //draw resource tank
    float resources_height=-45.0*(m_resources_drawn/m_resources_max);
    //rest is black
    glColor4f(0.0,0.0,0.0,1.0);
    glBegin(GL_QUADS);
    glVertex2f(8.0,-25.0);
    glVertex2f(8.0,25.0);
    glVertex2f(22.0,25.0);
    glVertex2f(22.0,-25.0);
    //material color
    glColor4f(0.3,0.7,0.2,1.0);
    glVertex2f(12.0,resources_height+21.0);
    glVertex2f(12.0, 21.0);
    glVertex2f(18.0, 21.0);
    glVertex2f(18.0,resources_height+21.0);
    glEnd();
    glDisable(GL_BLEND);
    glPopMatrix();

    //draw landing gear
    glColor3f(1.0,1.0,1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE,GL_ONE);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D,m_texture);

    //left
    glPushMatrix();
    body_pos=m_pLanding_gear_left->GetWorldCenter();
    angle=m_pLanding_gear_left->GetAngle();
    glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
    glRotatef(angle*180/_pi,0,0,1);
    //mask
    glBlendFunc(GL_DST_COLOR,GL_ZERO);
    glBegin(GL_QUADS);
    glTexCoord2f(269.0/1024.0,(1024.0-13.0-58.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0,-1.0*_Met2Pix-8.0);
    glTexCoord2f(269.0/1024.0,(1024.0-68.0-58.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-68.0-58.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-13.0-58.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0,-1.0*_Met2Pix-8.0);
    glEnd();
    //texture
    glBlendFunc(GL_ONE,GL_ONE);
    glBegin(GL_QUADS);
    glTexCoord2f(269.0/1024.0,(1024.0-13.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0,-1.0*_Met2Pix-8.0);
    glTexCoord2f(269.0/1024.0,(1024.0-68.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-68.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-13.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0,-1.0*_Met2Pix-8.0);
    glEnd();
    glPopMatrix();

    //right
    glPushMatrix();
    body_pos=m_pLanding_gear_right->GetWorldCenter();
    angle=m_pLanding_gear_right->GetAngle();
    glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
    glRotatef(angle*180/_pi,0,0,1);
    //mask
    glBlendFunc(GL_DST_COLOR,GL_ZERO);
    glBegin(GL_QUADS);
    glTexCoord2f(269.0/1024.0,(1024.0-13.0-58.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0,-1.0*_Met2Pix-8.0);
    glTexCoord2f(269.0/1024.0,(1024.0-68.0-58.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-68.0-58.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-13.0-58.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0,-1.0*_Met2Pix-8.0);
    glEnd();
    //texture
    glBlendFunc(GL_ONE,GL_ONE);
    glBegin(GL_QUADS);
    glTexCoord2f(269.0/1024.0,(1024.0-13.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0,-1.0*_Met2Pix-8.0);
    glTexCoord2f(269.0/1024.0,(1024.0-68.0)/1024.0);
    glVertex2f(-1.0*_Met2Pix-8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-68.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0, 1.0*_Met2Pix+8.0);
    glTexCoord2f(324.0/1024.0,(1024.0-13.0)/1024.0);
    glVertex2f( 1.0*_Met2Pix+8.0,-1.0*_Met2Pix-8.0);
    glEnd();
    glPopMatrix();

    //draw main ship body
    glPushMatrix();
    body_pos=m_pBody->GetWorldCenter();
    angle=m_pBody->GetAngle();
    glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
    glRotatef(angle*180/_pi,0,0,1);

    //draw mask
    glBlendFunc(GL_DST_COLOR,GL_ZERO);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0/1024.0,(1024.0-1.0-122.0)/1024.0);
    glVertex2f(-5.0*_Met2Pix-26.0,-2.0*_Met2Pix-21.0);
    glTexCoord2f(1.0/1024.0,(1024.0-121.0-122.0)/1024.0);
    glVertex2f(-5.0*_Met2Pix-26.0, 2.0*_Met2Pix+21.0);
    glTexCoord2f(253.0/1024.0,(1024.0-121.0-122.0)/1024.0);
    glVertex2f( 5.0*_Met2Pix+26.0, 2.0*_Met2Pix+21.0);
    glTexCoord2f(253.0/1024.0,(1024.0-1.0-122.0)/1024.0);
    glVertex2f( 5.0*_Met2Pix+26.0,-2.0*_Met2Pix-21.0);
    glEnd();

    glBlendFunc(GL_ONE,GL_ONE);

    glBegin(GL_QUADS);
    glTexCoord2f(1.0/1024.0,(1024.0-1.0)/1024.0);
    glVertex2f(-5.0*_Met2Pix-26.0,-2.0*_Met2Pix-21.0);
    glTexCoord2f(1.0/1024.0,(1024.0-121.0)/1024.0);
    glVertex2f(-5.0*_Met2Pix-26.0, 2.0*_Met2Pix+21.0);
    glTexCoord2f(253.0/1024.0,(1024.0-121.0)/1024.0);
    glVertex2f( 5.0*_Met2Pix+26.0, 2.0*_Met2Pix+21.0);
    glTexCoord2f(253.0/1024.0,(1024.0-1.0)/1024.0);
    glVertex2f( 5.0*_Met2Pix+26.0,-2.0*_Met2Pix-21.0);
    glEnd();
    //glDisable(GL_TEXTURE_2D);
    //glDisable(GL_BLEND);

    //draw lamps
    //right
    if(!m_lamp_landing_timer_inc) glColor3f(0.2,0.2,0.2);
    else glColor3f(1.0,0.0,0.1);
    if(!m_in_landing_phase) glColor3f(0.0,1.0,0.0);
    glBegin(GL_QUADS);
    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
    glVertex2f(95.0-16.0,23.0-16.0);
    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
    glVertex2f(95.0-16.0,23.0+16.0);
    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
    glVertex2f(95.0+16.0,23.0+16.0);
    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
    glVertex2f(95.0+16.0,23.0-16.0);
    glEnd();
    //left low
    glBegin(GL_QUADS);
    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
    glVertex2f(-96.0-16.0,23.0-16.0);
    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
    glVertex2f(-96.0-16.0,23.0+16.0);
    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
    glVertex2f(-96.0+16.0,23.0+16.0);
    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
    glVertex2f(-96.0+16.0,23.0-16.0);
    glEnd();
    //left high (takeoff)
    float led_brightness=*m_pMship_led_prog;
    if(led_brightness<0.1) led_brightness=0.1;
    if(!m_lamp_landing_timer_inc) glColor3f(0.2,0.2,0.2);
    else glColor3f(led_brightness,0.0,0.0);
    if(!m_in_takeoff_phase || m_in_landing_phase) glColor3f(led_brightness,0.0,0.0);
    if(!m_auto_land) glColor3f(led_brightness,led_brightness,0.0);
    glBegin(GL_QUADS);
    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
    glVertex2f(-96.0-16.0,-24.0-16.0);
    glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
    glVertex2f(-96.0-16.0,-24.0+16.0);
    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
    glVertex2f(-96.0+16.0,-24.0+16.0);
    glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
    glVertex2f(-96.0+16.0,-24.0-16.0);
    glEnd();

    //draw player leds
    for(int player_i=0;player_i<4;player_i++)
    {
        b2Vec2 led_pos;
        switch(player_i)
        {
            case 0: led_pos.Set(-0.1*_Met2Pix,-1.9*_Met2Pix); break;
            case 1: led_pos.Set(1.15*_Met2Pix,-1.9*_Met2Pix); break;
            case 2: led_pos.Set(2.50*_Met2Pix,-1.9*_Met2Pix); break;
            case 3: led_pos.Set(3.90*_Met2Pix,-1.9*_Met2Pix); break;
        }

        if(m_player_in_control==player_i)
         glColor3f(0.2,0.2,0.8);
        else if(m_player_inside[player_i])
         glColor3f(0.2,0.8,0.2);
        else
         glColor3f(0.9,0.3,0.3);

        glBegin(GL_QUADS);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(led_pos.x-16.0,led_pos.y-8.0);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(led_pos.x-16.0,led_pos.y+8.0);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(led_pos.x+16.0,led_pos.y+8.0);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(led_pos.x+16.0,led_pos.y-8.0);
        glEnd();

    }
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();

    return true;
}

bool main_ship::landing_gear_motor_left_lock(bool flag)
{
    if(!m_landing_gear_left_lock_on && flag)//lock if not locked and flag is true
    {
        //stop motor
        m_pLanding_motor_join_left->SetMotorSpeed(0);

        //stop body motion
        m_pLanding_gear_left->SetLinearVelocity(b2Vec2(0,0));
        m_pLanding_gear_left->SetAngularVelocity(0.0);

        /*//weld gear
        b2WeldJointDef weldJointDef;
        weldJointDef.bodyA = m_pBody;
        weldJointDef.bodyB = m_pLanding_gear_left;
        weldJointDef.collideConnected = false;
        //b2Vec2 rel_pos=m_pLanding_gear_left->GetPosition()-m_pBody->GetPosition();
        b2Vec2 rel_pos=m_pBody->GetLocalPoint(m_pLanding_gear_left->GetPosition());
        weldJointDef.localAnchorA=rel_pos;
        weldJointDef.localAnchorB.Set(0,0);
        m_pLanding_gear_left_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );*/

        //m_landing_gear_left_lock_on=true;//never locks...
    }

    if(m_landing_gear_left_lock_on && !flag)//unlock if locked and flag false
    {
        //unlock
        m_pWorld->DestroyJoint(m_pLanding_gear_left_lock);

        //start motor
        m_pLanding_motor_join_left->SetMotorSpeed(-1.0);

        m_landing_gear_left_lock_on=false;
    }

    return true;
}

bool main_ship::landing_gear_motor_right_lock(bool flag)
{
    if(!m_landing_gear_right_lock_on && flag)//lock if not locked and flag is true
    {
        //stop motor
        m_pLanding_motor_join_right->SetMotorSpeed(0);

        //stop body motion
        m_pLanding_gear_right->SetLinearVelocity(b2Vec2(0,0));
        m_pLanding_gear_right->SetAngularVelocity(0.0);

        /*//weld gear
        b2WeldJointDef weldJointDef;
        weldJointDef.bodyA = m_pBody;
        weldJointDef.bodyB = m_pLanding_gear_right;
        weldJointDef.collideConnected = false;
        //b2Vec2 rel_pos=m_pLanding_gear_right->GetPosition()-m_pBody->GetPosition();
        b2Vec2 rel_pos=m_pBody->GetLocalPoint(m_pLanding_gear_right->GetPosition());
        weldJointDef.localAnchorA=rel_pos;
        weldJointDef.localAnchorB.Set(0,0);
        m_pLanding_gear_right_lock = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );*/

        //m_landing_gear_right_lock_on=true;//never locks...
    }

    if(m_landing_gear_right_lock_on && !flag)//unlock if locked and flag false
    {
        //unlock
        m_pWorld->DestroyJoint(m_pLanding_gear_right_lock);

        //start motor
        m_pLanding_motor_join_right->SetMotorSpeed(-1.0);

        m_landing_gear_right_lock_on=false;
    }

    return true;
}

bool main_ship::begin_takeoff(void)//but landing will be finished before starting to takeoff
{
    m_in_takeoff_phase=true;

    return true;
}

float main_ship::get_resources(void)
{
    return m_resources_curr;
}

bool main_ship::change_resources(float value)
{
    if(m_resources_curr+value<0.0) return false;//not enough

    m_resources_curr+=value;

    //cull max value
    if(m_resources_curr>m_resources_max) m_resources_curr=m_resources_max;

    return true;
}

float main_ship::get_fuel(void)
{
    return m_fuel_tank_curr;
}

bool main_ship::set_fuel(float new_value)
{
    m_fuel_tank_curr=new_value;

    return true;
}

bool main_ship::change_fuel(float value)
{
    if(m_fuel_tank_curr+value<0.0) return false;//not enough

    m_fuel_tank_curr+=value;

    //cull max value
    if(m_fuel_tank_curr>m_fuel_tank_max) m_fuel_tank_curr=m_fuel_tank_max;

    return true;
}

b2Body* main_ship::get_body_ptr(void)
{
    return m_pBody;
}

b2Fixture* main_ship::get_sensor_input_ptr(void)
{
    return m_pInput_sensor;
}

b2Fixture* main_ship::get_sensor_landing_ptr(void)
{
    return m_pLand_sensor;
}

b2Fixture* main_ship::get_sensor_landing_gear_left_ptr(void)
{
    return m_pLanding_gear_sensor_left;
}

b2Fixture* main_ship::get_sensor_landing_gear_right_ptr(void)
{
    return m_pLanding_gear_sensor_right;
}

bool main_ship::reset_to_land_pos(void)
{
    //move to start pos
    m_pBody->SetTransform( b2Vec2(m_start_pos[0],m_start_pos[1]), 0.0 );
    m_pBody->SetAngularVelocity(0.0);
    m_pBody->SetLinearVelocity(b2Vec2(0.0,0.0));
    //move gears
    m_pLanding_gear_left->SetTransform( b2Vec2(m_start_pos[0]-2.5,m_start_pos[1]+1.0), 0.0);
    m_pLanding_gear_left->SetAngularVelocity(0.0);
    m_pLanding_gear_left->SetLinearVelocity( b2Vec2(0.0,0.0));
    m_pLanding_gear_right->SetTransform( b2Vec2(m_start_pos[0]+2.5,m_start_pos[1]+1.0), 0.0);
    m_pLanding_gear_right->SetAngularVelocity(0.0);
    m_pLanding_gear_right->SetLinearVelocity(b2Vec2(0.0,0.0));
    //move players
    b2Body* tmp=m_pWorld->GetBodyList();
    while(tmp)
    {
        st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
        if(data->s_info=="player" || data->s_info=="hook")
        {
            switch(data->i_id)
            {
                case 0: tmp->SetTransform( b2Vec2(m_start_pos[0]+0.00,m_start_pos[1]-1.0), 0.0 ); break;
                case 1: tmp->SetTransform( b2Vec2(m_start_pos[0]+1.25,m_start_pos[1]-1.0), 0.0 ); break;
                case 2: tmp->SetTransform( b2Vec2(m_start_pos[0]+2.75,m_start_pos[1]-1.0), 0.0 ); break;
                case 3: tmp->SetTransform( b2Vec2(m_start_pos[0]+4.00,m_start_pos[1]-1.0), 0.0 ); break;
            }
            tmp->SetAngularVelocity(0.0);
            tmp->SetLinearVelocity(b2Vec2(0.0,0.0));
        }

        tmp=tmp->GetNext();
    }

    //m_pLanding_motor_join_left->SetMotorSpeed(-0.5);
    //m_pLanding_motor_join_right->SetMotorSpeed(-0.5);


    //reset variables
    m_in_landing_phase=true;
    m_in_takeoff_phase=false;
    m_land_phase=lp_init;
    m_takeoff_phase=tp_init;
    m_landing_start_height_pos=m_start_pos[1];
    m_landstrip_motor_in_use[0]=m_landstrip_motor_in_use[1]=m_landstrip_motor_in_use[2]=m_landstrip_motor_in_use[3]=false;
    m_balance_extra_thrust_factor=1.0;
    m_tilt_factor_last=0.5;
    m_player_in_control=-1;
    m_auto_land=m_all_players_on_ship=true;
    m_pLanding_gear_touch[0]=m_pLanding_gear_touch[1]=false;

    return true;
}

bool main_ship::is_landing(void)
{
    return m_in_landing_phase;
}

bool main_ship::is_takeoff(void)
{
    return m_in_takeoff_phase;
}

int main_ship::get_takeoff_phase(void)
{
    return m_takeoff_phase;
}

bool main_ship::set_landing_gear_motor_speed_left(float motor_speed)
{
    m_pLanding_motor_join_left->SetMotorSpeed(-motor_speed);

    if(m_pLanding_motor_join_left->GetJointTranslation()>=m_pLanding_motor_join_left->GetUpperLimit() ||
       m_pLanding_motor_join_left->GetJointTranslation()<=m_pLanding_motor_join_left->GetLowerLimit())
     return false;//max reached

    return true;//is moving
}

bool main_ship::set_landing_gear_motor_speed_right(float motor_speed)
{
    m_pLanding_motor_join_right->SetMotorSpeed(-motor_speed);

    if(m_pLanding_motor_join_right->GetJointTranslation()>=m_pLanding_motor_join_right->GetUpperLimit() ||
       m_pLanding_motor_join_right->GetJointTranslation()<=m_pLanding_motor_join_right->GetLowerLimit())
     return false;//max reached

    return true;//is moving
}

bool main_ship::set_landing_gear_sensor_flags(bool* flag_landing_gear)
{
    m_pLanding_gear_touch=flag_landing_gear;

    return true;
}

int main_ship::get_auto_pilot(void)
{
    return m_player_in_control;
}

bool main_ship::set_auto_pilot(int player_in_control)
{
    m_player_in_control=player_in_control;

    if(player_in_control==-1)
    {
        m_auto_land=true;

        //sens if further landing is needed
        if(!m_pLanding_gear_touch[0] || !m_pLanding_gear_touch[1])
        {
            m_in_landing_phase=true;
            m_takeoff_phase=lp_init;
        }
        else m_in_landing_phase=false;

        //cancel possible takeoff
        m_in_takeoff_phase=false;
        m_takeoff_phase=lp_init;

        /*//reset landing/takeoff process
        if(m_in_landing_phase)
        {
            m_land_phase=lp_fall;
            m_takeoff_phase=lp_init;
        }
        else if(m_in_takeoff_phase)//never resume a takeoff, is canceled in autopilot
        {
            m_land_phase=lp_init;
            m_takeoff_phase=tp_to_target_height;
        }*/
    }
    else
    {
        m_auto_land=false;
        //m_pBody->SetGravityScale(2.0);
    }

    return true;
}

bool main_ship::set_all_players_on_ship(bool flag)
{
    //cout<<"All ships on mship: "<<flag<<endl;

    m_all_players_on_ship=flag;

    //reset takeoff phase
    if(!flag)
    {
        m_in_takeoff_phase=false;
        m_takeoff_phase=tp_init;
    }


    return true;
}

bool main_ship::player_inside(int player_ind,bool inside)
{
    m_player_inside[player_ind]=inside;

    return true;
}

float main_ship::get_motor_thrust(void)
{
    return m_motor_thrust_curr;
}

float main_ship::get_gear_motor_speed(void)
{
    //1 if both are moving, 0.5 = 1, 0 = none
    float motor_speed=0;
    if( fabs(m_pLanding_motor_join_right->GetMotorSpeed())>0.0 &&
        m_pLanding_motor_join_right->GetJointTranslation()<=m_pLanding_motor_join_right->GetUpperLimit() &&
        m_pLanding_motor_join_right->GetJointTranslation()>=m_pLanding_motor_join_right->GetLowerLimit() )
     motor_speed+=0.5;

    if( fabs(m_pLanding_motor_join_left->GetMotorSpeed())>0.0 &&
        m_pLanding_motor_join_left->GetJointTranslation()<=m_pLanding_motor_join_left->GetUpperLimit() &&
        m_pLanding_motor_join_left->GetJointTranslation()>=m_pLanding_motor_join_left->GetLowerLimit() )
     motor_speed+=0.5;

    return motor_speed;
}

bool main_ship::restock(void)
{
    m_resources_curr=m_resources_max;
    m_resources_drawn=m_resources_curr;
    m_fuel_tank_curr=m_fuel_tank_max;
    m_fuel_tank_drawn=m_fuel_tank_curr;

    return true;
}
