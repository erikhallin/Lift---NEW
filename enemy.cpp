#include "enemy.h"

enemy::enemy()
{
    m_ai_think_time_default=1.0;
    m_speed_limit_up=0.1;
}

bool enemy::init(b2World* pWorld,sound* pSound,particle_engine* pPart_eng,b2Vec2 pos,int type,int texture)
{
    m_pWorld=pWorld;
    m_type=type;
    m_texture=texture;
    m_pParticle_engine=pPart_eng;
    m_pSound=pSound;
    m_plan_phase=pp_patrol;
    m_is_dead=false;
    int coin=rand()%2;
    if(coin==0) m_patrol_right=false;
    else        m_patrol_right=true;
    m_patrol_timer=m_patrol_next_time=0.0;
    m_patrol_bad_pos_counter=0;
    m_patrol_bad_pos_max=5;//tries until swap direction
    m_pure_idle_timer=m_pure_idle_delay=float(rand()%1000)/1000.0*20.0+10.0;// 10-20 sec
    m_target_hooked=false;
    m_convoy_mode=false;
    m_led_time=0.5;
    m_led_timer=float(rand()%100)/100.0*m_led_time;
    m_led_flip=m_led_flip2=false;
    m_led_glow=0.0;
    m_sound_timer=0;
    m_sound_col_timer=0;
    m_play_beam_sound=false;

    //find player bodies
    b2Body* tmp=m_pWorld->GetBodyList();
    while(tmp)
    {
        st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
        if(data->s_info=="player")
         m_vec_pPlayer_bodies.push_back(tmp);

        tmp=tmp->GetNext();
    }

    switch(m_type)
    {
        case et_default:
        {
            m_ship_target_pos.Set(pos.x+30,pos.y);
            m_tilt_limit_max=60.0;
            m_tilt_limit_ok=5.0;
            m_height_limit_max=10.0;
            m_height_limit_ok=5.0;
            m_offside_limit_max=10.0;
            m_offside_limit_ok=5.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=60.0;
            m_ai_detection_range=40.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_pea,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture
            b2PolygonShape shape1;
            b2Vec2 p1(-1.0,-1.0); b2Vec2 p2(-1.0,1.0); b2Vec2 p3(1.0,1.0); b2Vec2 p4(1.0,-1.0);
            b2Vec2 arr[]={p1,p2,p3,p4};
            shape1.Set(arr,4);
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape1;
            fixturedef.density=_enemy_density;
            fixturedef.filter.categoryBits=_COLCAT_all;
            fixturedef.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef);
            //move into pos
            m_pBody->SetTransform( pos, _pi*0.9 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_burst_bot:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=60.0;
            m_tilt_limit_ok=5.0;
            m_height_limit_max=5.0;
            m_height_limit_ok=4.0;
            m_offside_limit_max=5.0;
            m_offside_limit_ok=3.0;

            m_size=1.5;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=40.0;
            m_ai_detection_range=30.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.1;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_burst,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture
            b2PolygonShape shape1;
            b2Vec2 p1(-0.4*m_size,0.2*m_size);
            b2Vec2 p2(-0.4*m_size,0.5*m_size);
            b2Vec2 p3(0.4*m_size,0.5*m_size);
            b2Vec2 p4(0.4*m_size,0.2*m_size);
            b2Vec2 p5(0.0*m_size,-0.2*m_size);
            b2Vec2 arr[]={p1,p2,p3,p4,p5};
            shape1.Set(arr,5);
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape1;
            fixturedef.density=_enemy_density;
            fixturedef.filter.categoryBits=_COLCAT_all;
            fixturedef.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.9 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_auto_flat:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=10.0;
            m_tilt_limit_ok=2.0;
            m_height_limit_max=0.1;
            m_height_limit_ok=0.0;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=3.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=40.0;
            m_ai_detection_range=30.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=3.0;
            m_ai_aim_angle_tolerance=20.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_rapid,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.2*m_size);
            b2Vec2 p2(-0.5*m_size,0.0*m_size);
            b2Vec2 p3(-0.2*m_size,0.3*m_size);
            b2Vec2 p4(0.2*m_size,0.3*m_size);
            b2Vec2 p5(0.5*m_size,0.0*m_size);
            b2Vec2 p6(0.5*m_size,-0.2*m_size);
            b2Vec2 arr[]={p1,p2,p3,p4,p5,p6};
            shape1.Set(arr,6);
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape1;
            fixturedef.density=_enemy_density;
            fixturedef.filter.categoryBits=_COLCAT_all;
            fixturedef.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.06 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_lifter:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=60.0;
            m_tilt_limit_ok=2.0;
            m_height_limit_max=10.0;
            m_height_limit_ok=2.0;
            m_offside_limit_max=1.0;
            m_offside_limit_ok=0.2;
            m_height_above_target=10.0;
            m_above_target_flag=m_target_hooked=false;
            m_hook_distance=1.0;

            m_size=2.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=20.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_lift_target_movement_sens=0.01;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //no weapon
            m_pWeapon_curr=new weapon( m_pWorld,wt_unarmed,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_boost,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.25*m_size);
            b2Vec2 p2(-0.5*m_size,0.3*m_size);
            b2Vec2 p3(0.5*m_size,0.3*m_size);
            b2Vec2 p4(0.5*m_size,-0.25*m_size);
            b2Vec2 p5(0.25*m_size,-0.5*m_size);
            b2Vec2 p6(-0.25*m_size,-0.5*m_size);
            b2Vec2 arr[]={p1,p2,p3,p4,p5,p6};
            shape1.Set(arr,6);
            b2FixtureDef fixturedef;
            fixturedef.shape=&shape1;
            fixturedef.density=_enemy_density;
            fixturedef.filter.categoryBits=_COLCAT_all;
            fixturedef.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_flipper:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=50.0;
            m_tilt_limit_ok=2.0;
            m_height_limit_max=2.0;
            m_height_limit_ok=0.1;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=1.0;
            m_height_above_target=10.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=30.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_body_flipped=false;//direction to the right
            m_flip_target_left=false;
            m_movement_phase=0;
            m_counter_rotation_limit=0.001;
            m_speed_limit_min=1.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_spread,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.5*m_size);
            b2Vec2 p2(-0.5*m_size,0.5*m_size);
            b2Vec2 p3(0.5*m_size,0.5*m_size);
            b2Vec2 p4(0.5*m_size,-0.5*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture tail
            b2PolygonShape shape2;
            b2Vec2 p5(-1.5*m_size,-0.15*m_size);
            b2Vec2 p6(-1.5*m_size,0.15*m_size);
            b2Vec2 p7(-0.5*m_size,0.15*m_size);
            b2Vec2 p8(-0.5*m_size,-0.15*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //move into pos
            m_pBody->SetTransform( pos, _pi*0.1 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_stand_turret:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=90.0;
            m_tilt_limit_ok=0.0;
            m_height_limit_max=0.0;
            m_height_limit_ok=0.0;
            m_offside_limit_max=0.0;
            m_offside_limit_ok=0.0;
            m_height_above_target=0.0;

            m_size=1.5;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.0;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=0.4;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_turret_angle=0.0; m_turret_angle_min=-90.0; m_turret_angle_max=90.0;
            m_turret_turn_speed=50.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;


            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_pea,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_turret_rotation,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body top
            b2PolygonShape shape1;
            b2Vec2 p1(-0.15*m_size,-0.5*m_size);
            b2Vec2 p2(-0.30*m_size,-0.2*m_size);
            b2Vec2 p3(0.30*m_size,-0.2*m_size);
            b2Vec2 p4(0.15*m_size,-0.5*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture body main
            b2PolygonShape shape2;
            b2Vec2 p5(-0.5*m_size,-0.2*m_size);
            b2Vec2 p6(-0.5*m_size,0.2*m_size);
            b2Vec2 p7(0.5*m_size,0.2*m_size);
            b2Vec2 p8(0.5*m_size,-0.2*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.friction=2.0;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //create fixture leg left
            b2PolygonShape shape3;
            b2Vec2 p9 (-0.5*m_size,0.2*m_size);
            b2Vec2 p10(-0.7*m_size,0.5*m_size);
            b2Vec2 p11(-0.5*m_size,0.5*m_size);
            b2Vec2 p12(-0.3*m_size,0.2*m_size);
            b2Vec2 arr3[]={p9,p10,p11,p12};
            shape3.Set(arr3,4);
            b2FixtureDef fixturedef3;
            fixturedef3.shape=&shape3;
            fixturedef3.density=_enemy_density;
            fixturedef3.friction=2.0;
            m_pBody->CreateFixture(&fixturedef3);
            //create fixture leg right
            b2PolygonShape shape4;
            b2Vec2 p13(0.3*m_size,0.2*m_size);
            b2Vec2 p14(0.5*m_size,0.5*m_size);
            b2Vec2 p15(0.7*m_size,0.5*m_size);
            b2Vec2 p16(0.5*m_size,0.2*m_size);
            b2Vec2 arr4[]={p13,p14,p15,p16};
            shape4.Set(arr4,4);
            b2FixtureDef fixturedef4;
            fixturedef4.shape=&shape4;
            fixturedef4.density=_enemy_density;
            m_pBody->CreateFixture(&fixturedef4);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_rocket_tank:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=2.0;
            m_tilt_limit_ok=0.1;
            m_height_limit_max=2.0;
            m_height_limit_ok=0.1;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=1.0;
            m_height_above_target=10.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=3.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_body_flipped=false;//turret direction to the right
            m_flip_target_left=false;
            m_movement_phase=0;
            m_tilt_adjust_timer=m_tilt_adjust_delay=0.0;//sec before tilt is adjusted
            m_turret_flip_timer=m_turret_flip_delay=3.0;//delay when turret is flipped
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_rocket,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.2*m_size);
            b2Vec2 p2(-0.5*m_size,0.2*m_size);
            b2Vec2 p3(0.5*m_size,0.2*m_size);
            b2Vec2 p4(0.5*m_size,-0.2*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture head
            b2PolygonShape shape2;
            b2Vec2 p5(-0.25*m_size,-0.5*m_size);
            b2Vec2 p6(-0.25*m_size,-0.2*m_size);
            b2Vec2 p7(0.25*m_size,-0.2*m_size);
            b2Vec2 p8(0.25*m_size,-0.5*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.1 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_grenade_ship:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=10.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=5.0;
            m_height_limit_ok=1.0;
            m_offside_limit_max=30.0;
            m_offside_limit_ok=1.0;
            m_height_above_target=10.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_movement_phase=0;
            m_fire_range_min=5.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_grenade,1.0 ,wst_impact);
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,0.0*m_size);
            b2Vec2 p2(-0.3*m_size,0.3*m_size);
            b2Vec2 p3(0.3*m_size,0.3*m_size);
            b2Vec2 p4(0.5*m_size,0.0*m_size);
            b2Vec2 p5(0.0*m_size,-0.3*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4,p5};
            shape1.Set(arr1,5);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.1 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_flying_turret:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=20.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=10.0;
            m_height_limit_ok=1.0;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=1.0;
            m_height_above_target=15.0;

            m_size=1.5;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.0;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=0.4;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_turret_angle=180.0; m_turret_angle_min=90.0; m_turret_angle_max=270.0;
            m_turret_turn_speed=50.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //weapon
            m_pWeapon_curr=new weapon( m_pWorld,wt_pea,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_turret_aim,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body main
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.15*m_size);
            b2Vec2 p2(-0.5*m_size,0.15*m_size);
            b2Vec2 p3(0.5*m_size,0.15*m_size);
            b2Vec2 p4(0.5*m_size,-0.15*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture body turret head
            b2PolygonShape shape2;
            b2Vec2 p5(-0.25*m_size,0.15*m_size);
            b2Vec2 p6(-0.15*m_size,0.40*m_size);
            b2Vec2 p7(0.15*m_size,0.40*m_size);
            b2Vec2 p8(0.25*m_size,0.15*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.friction=2.0;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_cannon_tank:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=2.0;
            m_tilt_limit_ok=0.1;
            m_height_limit_max=2.0;
            m_height_limit_ok=0.1;
            m_offside_limit_max=30.0;
            m_offside_limit_ok=1.0;
            m_height_above_target=10.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=3.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_body_flipped=false;//turret direction to the right
            m_flip_target_left=false;
            m_movement_phase=0;
            m_tilt_adjust_timer=m_tilt_adjust_delay=0.0;//sec before tilt is adjusted
            m_turret_flip_timer=m_turret_flip_delay=3.0;//delay when turret is flipped
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_cannon,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.2*m_size);
            b2Vec2 p2(-0.5*m_size,0.2*m_size);
            b2Vec2 p3(0.5*m_size,0.2*m_size);
            b2Vec2 p4(0.5*m_size,-0.2*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture head
            b2PolygonShape shape2;
            b2Vec2 p5(-0.25*m_size,-0.5*m_size);
            b2Vec2 p6(-0.25*m_size,-0.2*m_size);
            b2Vec2 p7(0.25*m_size,-0.2*m_size);
            b2Vec2 p8(0.25*m_size,-0.5*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.1 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_miner:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=10.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=20.0;
            m_height_limit_ok=10.0;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=10.0;
            m_height_above_target=10.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=5.0;
            m_fire_ship_to_barrel_dist=1.0;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_movement_phase=0;
            m_speed_limit_min=0.01;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_mine,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body
            b2PolygonShape shape1;
            b2Vec2 p1(-0.20*m_size,-0.25*m_size);
            b2Vec2 p2(-0.15*m_size,0.0*m_size);
            b2Vec2 p3(0.15*m_size,0.0*m_size);
            b2Vec2 p4(0.20*m_size,-0.25*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture leg left
            b2PolygonShape shape2;
            b2Vec2 p5(-0.2*m_size,-0.25*m_size);
            b2Vec2 p6(-0.5*m_size,0.25*m_size);
            b2Vec2 p7(-0.35*m_size,0.33*m_size);
            b2Vec2 p8(-0.15*m_size,0.0*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //create fixture leg right
            b2PolygonShape shape3;
            b2Vec2 p9 (0.2*m_size,-0.25*m_size);
            b2Vec2 p10(0.15*m_size,0.0*m_size);
            b2Vec2 p11(0.35*m_size,0.33*m_size);
            b2Vec2 p12(0.5*m_size,0.25*m_size);
            b2Vec2 arr3[]={p9,p10,p11,p12};
            shape3.Set(arr3,4);
            b2FixtureDef fixturedef3;
            fixturedef3.shape=&shape3;
            fixturedef3.density=_enemy_density;
            fixturedef3.filter.categoryBits=_COLCAT_all;
            fixturedef3.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef3);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.1 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_aim_bot:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=5.0;
            m_tilt_limit_ok=0.1;
            m_height_limit_max=20.0;
            m_height_limit_ok=1.0;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=1.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=5.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=0.7;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_turret_angle=0.0;
            m_turret_turn_speed=100.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //weapon
            m_pWeapon_curr=new weapon( m_pWorld,wt_pea,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_turret_auto_aim,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body main
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.25*m_size);
            b2Vec2 p2(-0.5*m_size,0.25*m_size);
            b2Vec2 p3(0.0*m_size,0.50*m_size);
            b2Vec2 p4(0.5*m_size,0.25*m_size);
            b2Vec2 p5(0.5*m_size,-0.25*m_size);
            b2Vec2 p6(0.0*m_size,-0.50*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4,p5,p6};
            shape1.Set(arr1,6);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_rammer:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=10.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=1.0;
            m_height_limit_ok=1.0;
            m_offside_limit_max=15.0;
            m_offside_limit_ok=1.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.1;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=3.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=0.7;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_turret_angle=0.0;
            m_turret_turn_speed=100.0;
            m_target_on_right_side=m_target_swap_side=false;
            m_ram_reset_distance=5.0;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;
            m_shield_on=m_shield_target_on=false;
            m_shield_timer=0.0;
            m_shield_delay=1.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_unarmed,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_shield,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body main
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.5*m_size);
            b2Vec2 p2(-0.8*m_size,0.0*m_size);
            b2Vec2 p3(-0.5*m_size,0.5*m_size);
            b2Vec2 p4(0.5*m_size,0.5*m_size);
            b2Vec2 p5(0.8*m_size,0.0*m_size);
            b2Vec2 p6(0.5*m_size,-0.50*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4,p5,p6};
            shape1.Set(arr1,6);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_beamer:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=20.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=10.0;
            m_height_limit_ok=1.0;
            m_offside_limit_max=20.0;
            m_offside_limit_ok=2.0;
            m_height_above_target=10.0;

            m_size=1.5;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.0;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=10.0;
            m_ai_aim_angle_tolerance=10.0;
            m_fire_ship_to_barrel_dist=0.2;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_draw_beam=false;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_beam,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_unarmed,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body main
            b2PolygonShape shape1;
            b2Vec2 p1(-0.5*m_size,-0.10*m_size);
            b2Vec2 p2(-0.5*m_size,0.10*m_size);
            b2Vec2 p3(0.5*m_size,0.10*m_size);
            b2Vec2 p4(0.5*m_size,-0.10*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture top part
            b2PolygonShape shape2;
            b2Vec2 p5(-0.25*m_size,-0.10*m_size);
            b2Vec2 p6(0.25*m_size,-0.10*m_size);
            b2Vec2 p7(0.0*m_size,-0.30*m_size);
            b2Vec2 arr2[]={p5,p6,p7};
            shape2.Set(arr2,3);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_cloaker:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=20.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=2.0;
            m_height_limit_ok=0.1;
            m_offside_limit_max=10.0;
            m_offside_limit_ok=1.0;

            m_size=1.0;
            m_hp_max=m_hp_curr=100;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.0;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=1.0;
            m_ai_aim_angle_tolerance=5.0;
            m_fire_ship_to_barrel_dist=0.7;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_cloak_target_off=true;
            m_cloak_delay=1.0;
            m_cloak_timer=m_cloak_delay;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=20.0;

            //arm
            m_pWeapon_curr=new weapon( m_pWorld,wt_pea,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_cloak,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            bodydef.gravityScale=0.0f;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body main
            b2PolygonShape shape1;
            b2Vec2 p1(-0.3*m_size,-0.10*m_size);
            b2Vec2 p2(-0.3*m_size,0.10*m_size);
            b2Vec2 p3(0.3*m_size,0.10*m_size);
            b2Vec2 p4(0.3*m_size,-0.10*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //create fixture left part
            b2PolygonShape shape2;
            b2Vec2 p5(-0.5*m_size,-0.20*m_size);
            b2Vec2 p6(-0.5*m_size,0.20*m_size);
            b2Vec2 p7(-0.3*m_size,0.20*m_size);
            b2Vec2 p8(-0.3*m_size,-0.20*m_size);
            b2Vec2 arr2[]={p5,p6,p7,p8};
            shape2.Set(arr2,4);
            b2FixtureDef fixturedef2;
            fixturedef2.shape=&shape2;
            fixturedef2.density=_enemy_density;
            fixturedef2.friction=2.0;
            fixturedef2.filter.categoryBits=_COLCAT_all;
            fixturedef2.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef2);
            //create fixture right part
            b2PolygonShape shape3;
            b2Vec2 p9 (0.3*m_size,-0.20*m_size);
            b2Vec2 p10(0.3*m_size,0.20*m_size);
            b2Vec2 p11(0.5*m_size,0.20*m_size);
            b2Vec2 p12(0.5*m_size,-0.20*m_size);
            b2Vec2 arr3[]={p9,p10,p11,p12};
            shape3.Set(arr3,4);
            b2FixtureDef fixturedef3;
            fixturedef3.shape=&shape3;
            fixturedef3.density=_enemy_density;
            fixturedef3.filter.categoryBits=_COLCAT_all;
            fixturedef3.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef3);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;

        case et_scanner:
        {
            m_ship_target_pos.Set(pos.x,pos.y);
            m_tilt_limit_max=20.0;
            m_tilt_limit_ok=1.0;
            m_height_limit_max=1.0;
            m_height_limit_ok=0.1;
            m_offside_limit_max=1.0;
            m_offside_limit_ok=0.1;

            m_size=1.0;
            m_hp_max=m_hp_curr=20;
            m_ai_think_timer=m_ai_think_delay=m_ai_think_time_default+m_ai_think_time_default*(float(rand()%1000)/1000.0-0.5);
            m_ai_sight_range=80.0;
            m_ai_detection_range=60.0;
            m_ai_flee_step_length=10.0;
            m_ai_low_hp_flee_limit=0.0;
            m_ai_flee_think_factor=3.0;
            m_height_above_ground=1000;
            m_height_above_ground_min=1.0;
            m_ai_aim_angle_tolerance=5.0;
            m_fire_ship_to_barrel_dist=0.7;
            m_measure_ground_dist_timer=0.0;
            m_measure_ground_dist_delay=0.2;
            m_pref_height_above_ground=30.0;
            m_pref_height_above_ground_tol=25.0;

            //weapon
            m_pWeapon_curr=new weapon( m_pWorld,wt_unarmed,1.0 );
            m_pGear_curr=new gear( m_pWorld,gt_gyro,1.0 );

            //create body
            b2BodyDef bodydef;
            bodydef.position.Set(pos.x,pos.y);
            bodydef.type=b2_dynamicBody;
            bodydef.linearDamping=_enemy_damping_lin;
            bodydef.angularDamping=_enemy_damping_ang;
            bodydef.gravityScale=0.0f;
            m_pBody=m_pWorld->CreateBody(&bodydef);
            //create fixture body main
            b2PolygonShape shape1;
            b2Vec2 p1(-0.2*m_size,-0.20*m_size);
            b2Vec2 p2(-0.2*m_size,0.20*m_size);
            b2Vec2 p3(0.2*m_size,0.20*m_size);
            b2Vec2 p4(0.2*m_size,-0.20*m_size);
            b2Vec2 arr1[]={p1,p2,p3,p4};
            shape1.Set(arr1,4);
            b2FixtureDef fixturedef1;
            fixturedef1.shape=&shape1;
            fixturedef1.density=_enemy_density;
            fixturedef1.filter.categoryBits=_COLCAT_all;
            fixturedef1.filter.maskBits=-1;
            m_pBody->CreateFixture(&fixturedef1);
            //move into pos
            //m_pBody->SetTransform( pos, _pi*0.5 );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="enemy";
            user_data->i_id=type;
            user_data->vp_this=this;
            m_pBody->SetUserData(user_data);

            //set center of rotation
            b2MassData massD;
            m_pBody->GetMassData(&massD);
            b2Vec2 centerV(0,0);
            massD.center = centerV;
            m_pBody->SetMassData(&massD);
        }break;
    }

    return true;
}

int enemy::update(float time_dif,float view_pos[4])
{
    if(m_sound_col_timer>0) m_sound_col_timer-=time_dif;

    //if no hp, inactive
    if(m_is_dead) return 0;

    //update player body pointers
    m_vec_pPlayer_bodies.clear();
    b2Body* tmp=m_pWorld->GetBodyList();
    while(tmp)
    {
        st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
        if(data->s_info=="player")
         m_vec_pPlayer_bodies.push_back(tmp);

        tmp=tmp->GetNext();
    }

    b2Vec2 body_pos=m_pBody->GetPosition();

    //calc sound area
    int sound_box=0;//sound off
    float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
    if(body_pos.x*_Met2Pix>view_pos[0] &&
       body_pos.x*_Met2Pix<view_pos[2] &&
       body_pos.y*_Met2Pix>view_pos[1] &&
       body_pos.y*_Met2Pix<view_pos[3] )
    {
        sound_box=1;//on screen
    }
    else if(body_pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
            body_pos.x*_Met2Pix<view_pos[0] &&
            body_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
            body_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
    {
        sound_box=2;//left side
    }
    else if(body_pos.x*_Met2Pix>view_pos[2] &&
            body_pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
            body_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
            body_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
    {
        sound_box=3;//right side
    }
    else if(body_pos.x*_Met2Pix>view_pos[0] &&
            body_pos.x*_Met2Pix<view_pos[2] &&
            body_pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
            body_pos.y*_Met2Pix<view_pos[1] )
    {
        sound_box=4;//top side
    }
    else if(body_pos.x*_Met2Pix>view_pos[0] &&
            body_pos.x*_Met2Pix<view_pos[2] &&
            body_pos.y*_Met2Pix>view_pos[3] &&
            body_pos.y*_Met2Pix<view_pos[3]+extra_side_area )
    {
        sound_box=5;//top side
    }

    //play laser sound if drawing laser beam
    if(m_play_beam_sound && m_type==et_beamer)
    {
        m_play_beam_sound=false;

        b2Vec2 pos=m_pBody->GetPosition();

        if(sound_box!=0)
        {
            float sound_data[]={0,0,0, 0,0,0, 0,0,-1,
                                0,1,0, 0,0,-1, 0,0,0,
                                1,  1,  0};
            switch(sound_box)
            {
                case 0: break;//no sound
                case 1:
                {
                    sound_data[14]=0;
                }break;
                case 2://left
                {
                    sound_data[12]=-_sound_box_side_shift;
                    sound_data[19]=_sound_box_level_outside;
                }break;
                case 3://right
                {
                    sound_data[12]=_sound_box_side_shift;
                    sound_data[19]=_sound_box_level_outside;
                }break;
                case 4:
                {
                    sound_data[12]=0;
                    sound_data[19]=_sound_box_level_outside;
                }break;
                case 5:
                {
                    sound_data[12]=0;
                    sound_data[19]=_sound_box_level_outside;
                }break;
            }

            //test if new sound is louder than current
            float old_volume=m_pSound->get_volume(_sound_chan_laser_enemy);
            if(sound_data[19]>old_volume)
             m_pSound->updateSound(_sound_chan_laser_enemy,sound_data);
        }
    }

    //destroyed if no HP
    if(m_hp_curr<=0.0)
    {
        m_is_dead=true;
        m_led_timer=0.0;
        m_led_glow=0.0;
        m_draw_beam=false;

        switch(m_type)
        {
            case et_cloaker:
            {
                //turn on gravity
                m_pBody->SetGravityScale(1.0);
            }break;

            case et_scanner:
            {
                //turn on gravity
                m_pBody->SetGravityScale(1.0);
            }break;
        }

        //play sound
        if(sound_box!=0)
        {
            float data[]={0,0,0, 0,0,0, 0,0,-1,
                          0,1,0, 0,0,-1, 0,0,0,
                          1,  1,  0};
            switch(sound_box)
            {
                case 0: break;//no sound
                case 1:
                {
                    data[14]=0;
                }break;
                case 2://left
                {
                    data[12]=-_sound_box_side_shift;
                    data[19]=_sound_box_level_outside;
                }break;
                case 3://right
                {
                    data[12]=_sound_box_side_shift;
                    data[19]=_sound_box_level_outside;
                }break;
                case 4:
                {
                    data[12]=0;
                    data[19]=_sound_box_level_outside;
                }break;
                case 5:
                {
                    data[12]=0;
                    data[19]=_sound_box_level_outside;
                }break;
            }
            m_pSound->playWAVE(wav_ship_explosion,data);
        }

        return 0;
    }

    //cout<<"mode: "<<m_plan_phase<<endl;

    //sound timer
    if(m_sound_timer>0.0) m_sound_timer-=time_dif;

    //if low hp, flee if in attack mode
    if(m_hp_curr/m_hp_max<m_ai_low_hp_flee_limit && m_plan_phase==pp_attack) m_plan_phase=pp_run_away;

    //update led
    switch(m_type)
    {
        case et_lifter: m_led_timer-=time_dif*0.5; break;
        default: m_led_timer-=time_dif; break;
    }
    if(m_led_timer<=0.0)
    {
        m_led_timer+=m_led_time;
        m_led_flip=!m_led_flip;
        if(m_led_flip) m_led_flip2=!m_led_flip2;
    }
    if(m_led_glow>0.0)
    {
        m_led_glow-=time_dif*2.0;
        if(m_led_glow<0.0) m_led_glow=0.0;
    }
    if(m_draw_beam) m_led_glow=1.0;

    //if time to think
    if(m_ai_think_timer>0) m_ai_think_timer-=time_dif;
    else//think
    {
        m_ai_think_timer=m_ai_think_delay;

        //make decisions
        switch(m_plan_phase)
        {
            case pp_patrol:
            {
                //test if time to update target pos
                if(m_patrol_timer>m_patrol_next_time)
                {
                    //test if direction should be changed
                    if(m_patrol_bad_pos_counter>m_patrol_bad_pos_max)
                    {
                        m_patrol_right=!m_patrol_right;
                        m_patrol_bad_pos_counter=0;
                    }

                    //test current height above ground
                    float search_distance=1000.0;
                    float curr_height=search_distance;
                    b2Vec2 pos_below_curr( body_pos.x, body_pos.y+search_distance );
                    MyRayCastCallback callback0;
                    callback0.set_ignore_body(m_pBody);
                    m_pWorld->RayCast(&callback0,body_pos,pos_below_curr);
                    if(callback0.m_any_hit)
                    {
                        curr_height=search_distance*callback0.m_fraction;
                    }
                    //test if current height is ok
                    if(curr_height+m_pref_height_above_ground_tol>m_pref_height_above_ground &&
                       curr_height-m_pref_height_above_ground_tol<m_pref_height_above_ground)//height ok
                    {
                        ;//continue with finding a new target pos, sideways
                    }
                    else//height not ok
                    {
                        //to high
                        if(curr_height>m_pref_height_above_ground+m_pref_height_above_ground_tol)
                        {
                            //go down
                            float step_dist=float(rand()%100)/100.0*9.0+0.1;// 0.1 - 9.1 meters
                            b2Vec2 step_below( body_pos.x, body_pos.y+step_dist );
                            //test if possible to go there
                            MyRayCastCallback callback01;
                            callback01.set_ignore_body(m_pBody);
                            m_pWorld->RayCast(&callback01,body_pos,step_below);
                            if(!callback01.m_any_hit)
                            {
                                //set new target pos
                                m_ship_target_pos=step_below;
                                //cout<<"New patrol pos down: "<<m_ship_target_pos.x<<", "<<m_ship_target_pos.y<<"\t"<<step_dist<<endl;
                                //pos could be updated further down, if allowed height is reached
                            }
                        }
                        //to low
                        else if(curr_height<m_pref_height_above_ground-m_pref_height_above_ground_tol)
                        {
                            //go up
                            float step_dist=float(rand()%100)/100.0*9.0+0.1;// 0.1 - 9.1 meters
                            b2Vec2 step_below( body_pos.x, body_pos.y-step_dist );
                            //test if possible to go there
                            MyRayCastCallback callback01;
                            callback01.set_ignore_body(m_pBody);
                            m_pWorld->RayCast(&callback01,body_pos,step_below);
                            if(!callback01.m_any_hit)
                            {
                                //set new target pos
                                m_ship_target_pos=step_below;
                                //cout<<"New patrol pos up: "<<m_ship_target_pos.x<<", "<<m_ship_target_pos.y<<endl;
                                //pos could be updated further down, if allowed height is reached
                            }
                        }
                    }

                    //set new target pos
                    float step_x=float(rand()%100)/100.0*19.0+1.0;// 1 - 20 meters
                    if(!m_patrol_right) step_x=-step_x;//go left
                    float step_y=(float(rand()%100)/100.0-0.4)*10.0;// -5 - 5 meters
                    //cout<<"step_y: "<<step_y<<endl;
                    b2Vec2 new_pos( body_pos.x+step_x, body_pos.y+step_y );
                    //measure height above ground
                    float new_height=search_distance;
                    b2Vec2 pos_below( new_pos.x, new_pos.y+search_distance );
                    MyRayCastCallback callback1;
                    callback1.set_ignore_body(m_pBody);
                    m_pWorld->RayCast(&callback1,new_pos,pos_below);
                    if(callback1.m_any_hit)
                    {
                        new_height=search_distance*callback1.m_fraction;
                    }
                    //test if new height is ok
                    if(new_height+m_pref_height_above_ground_tol>m_pref_height_above_ground &&
                       new_height-m_pref_height_above_ground_tol<m_pref_height_above_ground)//height ok
                    {
                        //test if possible to go there
                        MyRayCastCallback callback2;
                        callback2.set_ignore_body(m_pBody);
                        m_pWorld->RayCast(&callback2,body_pos,new_pos);
                        if(callback2.m_any_hit)
                        {
                            //new pos not ok
                            m_patrol_bad_pos_counter++;
                        }
                        else
                        {
                            m_patrol_bad_pos_counter=0;
                            //set new target pos
                            m_ship_target_pos=new_pos;
                            //cout<<"New patrol pos: "<<m_ship_target_pos.x<<", "<<m_ship_target_pos.y<<endl;
                        }
                    }
                    else//height not ok
                    {
                        //no pos update this round
                        m_patrol_bad_pos_counter++;
                    }

                    //set time for next patrol update
                    m_patrol_timer=0.0;
                    m_patrol_next_time=float(rand()%100)/100.0*4.0+1.0;// 1 - 6 sec delay
                }
                else if(!m_convoy_mode) m_patrol_timer+=m_ai_think_delay;//update patrol timer, not updated in convoy mode

                //cout<<"Patrol timer: "<<m_patrol_timer<<endl;

            }//no break will allow idle behaviour to, to search for players

            case pp_idle:
            {
                if(m_convoy_mode)
                {
                    //set target to end pos
                    m_ship_target_pos=m_convoy_end_pos;

                    //test if end pos reached
                    if(body_pos.x-m_offside_limit_ok*2.0<m_ship_target_pos.x &&
                       body_pos.x+m_offside_limit_ok*2.0>m_ship_target_pos.x /*&&
                       body_pos.y-m_height_limit_ok*2.0<m_ship_target_pos.y &&
                       body_pos.y+m_height_limit_ok*2.0>m_ship_target_pos.y*/ )
                    {
                        //signal to leave map
                        return 2;
                    }
                }

                //chance to start patrol
                if(m_plan_phase==pp_idle)//pure idle (not in patrol)
                {
                    if(m_pure_idle_timer<0.0)
                    {
                        m_plan_phase=pp_patrol;
                        m_pure_idle_timer=m_pure_idle_delay;
                    }
                    else m_pure_idle_timer-=m_ai_think_delay;
                }

                if(m_type==et_lifter)//lifters search for allies
                {
                    if(m_convoy_mode) break;//not if in convoy mode

                    //get bodies of potential targets
                    vector<b2Body*> vec_pAllied_bodies;
                    b2Body* tmp=m_pWorld->GetBodyList();
                    while(tmp)
                    {
                        if(tmp!=m_pBody)//cant help yourself
                        {
                            st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
                            if(data->s_info=="enemy"&&data->b_alive)
                            {
                                if(data->i_id!=et_stand_turret)//dont lift stand_turrets
                                 vec_pAllied_bodies.push_back(tmp);
                            }

                        }

                        tmp=tmp->GetNext();
                    }
                    //test if any allied within searchdist
                    for(int allied_i=0;allied_i<(int)vec_pAllied_bodies.size();allied_i++)
                    {
                        //test if players within search box
                        float search_dist=m_ai_detection_range;
                        b2Vec2 allied_pos=vec_pAllied_bodies[allied_i]->GetPosition();
                        if( body_pos.x+search_dist>allied_pos.x && body_pos.x-search_dist<allied_pos.x &&
                            body_pos.y+search_dist>allied_pos.y && body_pos.y-search_dist<allied_pos.y )
                        {
                            //test if target is within view
                            MyRayCastCallback callback;
                            callback.set_ignore_body(m_pBody);
                            m_pWorld->RayCast(&callback,body_pos,allied_pos);
                            if(callback.m_any_hit)
                            {
                                //if hit, make sure that it belongs to the player
                                b2Body* body_in_view=callback.m_pFixture->GetBody();
                                st_body_user_data* data=(st_body_user_data*)body_in_view->GetUserData();
                                if(data->s_info=="enemy" )//already tested above
                                {//target in view
                                    //is the target not moving
                                    b2Vec2 lin_speed=vec_pAllied_bodies[allied_i]->GetLinearVelocity();
                                    if( fabs(lin_speed.x)<m_lift_target_movement_sens &&
                                        fabs(lin_speed.y)<m_lift_target_movement_sens)
                                    {
                                        cout<<"Target spotted for lift\n";
                                        m_pTarget_body=vec_pAllied_bodies[allied_i];
                                        m_ship_target_pos=allied_pos;
                                        m_plan_phase=pp_lift;
                                    }
                                    else//too much motion
                                    {
                                        ;//cout<<"Target is moving: "<<lin_speed.x<<", "<<lin_speed.y<<endl;
                                    }
                                }
                            }
                            else//no hit, strange...
                            {
                                cout<<"ERROR: Lift searches for allies: No bodies in sight range\n";
                            }
                        }
                    }

                    //if players in sight, run away
                    for(int player_i=0;player_i<(int)m_vec_pPlayer_bodies.size();player_i++)
                    {
                        st_body_user_data* player_data=(st_body_user_data*)m_vec_pPlayer_bodies[player_i]->GetUserData();

                        //ignore cloaked and dead players
                        if(player_data->b_cloaked || !player_data->b_alive) continue;

                        //test if players within search box
                        float search_dist=m_ai_detection_range;
                        b2Vec2 player_pos=m_vec_pPlayer_bodies[player_i]->GetPosition();
                        if( body_pos.x+search_dist>player_pos.x && body_pos.x-search_dist<player_pos.x &&
                            body_pos.y+search_dist>player_pos.y && body_pos.y-search_dist<player_pos.y )
                        {
                            //test if target is within view
                            MyRayCastCallback callback;
                            m_pWorld->RayCast(&callback,body_pos,player_pos);
                            if(callback.m_any_hit)
                            {
                                b2Body* body_in_view=callback.m_pFixture->GetBody();
                                st_body_user_data* data=(st_body_user_data*)body_in_view->GetUserData();
                                //if hit, make sure that it belongs to the player
                                if(data->s_info=="player" ||
                                   data->s_info=="rope" ||
                                   data->s_info=="hook" )
                                {//target in view
                                    //set new target to attack
                                    cout<<"Lifter spotted player, run away\n";
                                    m_pTarget_body=m_vec_pPlayer_bodies[player_i];
                                    m_ship_target_pos=body_pos;
                                    m_plan_phase=pp_run_away;
                                    break;//dont test for other players
                                }
                            }
                            else//no hit, strange...
                            {
                                cout<<"ERROR: Enemy flees from player: No bodies in sight range\n";
                            }
                        }
                    }
                }

                //scanners report pos and draws a beam
                else if(m_type==et_scanner)
                {
                    b2Vec2 center_pos=body_pos;
                    float scan_radius=20.0;

                    //get list of bodies nearby
                    b2AABB aabb_box;
                    aabb_box.lowerBound=center_pos-b2Vec2(scan_radius,scan_radius);
                    aabb_box.upperBound=center_pos+b2Vec2(scan_radius,scan_radius);
                    MyQueryCallback aabb_callback;
                    m_pWorld->QueryAABB(&aabb_callback,aabb_box);

                    //find player bodies
                    vector<b2Body*> vec_player_bodies_involved;
                    for(int fixture_i=0;fixture_i<(int)aabb_callback.m_vec_fixtures.size();fixture_i++)
                    {
                        b2Body* body_ptr=aabb_callback.m_vec_fixtures[fixture_i]->GetBody();
                        st_body_user_data* data=(st_body_user_data*)body_ptr->GetUserData();

                        //test if body is a player
                        if(data->s_info!="player") continue;
                        if(data->b_cloaked) continue;

                        //test if new
                        bool is_new=true;
                        for(int body_i=0;body_i<(int)vec_player_bodies_involved.size();body_i++)
                        {
                            if( vec_player_bodies_involved[body_i]==body_ptr )
                            {
                                is_new=false;
                                break;
                            }
                        }
                        if(!is_new) continue;//body already in vector

                        //test if anything in the way (ON or OFF)
                        MyRayCastCallback raycast;
                        raycast.set_ignore_body(body_ptr);
                        raycast.set_ignore_body_type("rope");
                        raycast.set_ignore_body_type("hook");
                        m_pWorld->RayCast(&raycast,center_pos,body_ptr->GetPosition());
                        if(!raycast.m_any_hit)//nothing in the way
                         vec_player_bodies_involved.push_back(body_ptr);
                    }

                    if(!vec_player_bodies_involved.empty())
                    {
                        //pick random player
                        int selected_player=rand()%(int)vec_player_bodies_involved.size();
                        m_pTarget_body=vec_player_bodies_involved[selected_player];
                        //make beam
                        m_draw_beam=true;
                        m_vBeam_start=center_pos;
                        m_vBeam_end=vec_player_bodies_involved[selected_player]->GetPosition();

                        //stop
                        m_plan_phase=pp_idle;

                        //send alarm to other enemies whitin sight
                        b2Body* tmp=m_pWorld->GetBodyList();
                        while(tmp)
                        {
                            st_body_user_data* body_data=(st_body_user_data*)tmp->GetUserData();
                            if(body_data->s_info=="enemy" && tmp!=m_pBody)
                            {
                                //test if in sight
                                MyRayCastCallback raycast;
                                raycast.set_ignore_body(tmp);
                                m_pWorld->RayCast(&raycast,center_pos,tmp->GetPosition());
                                if(!raycast.m_any_hit)//nothing in the way
                                {
                                    //send info to that enemy
                                    enemy* enemy_in_sight=(enemy*)body_data->vp_this;
                                    enemy_in_sight->scanner_alarm(body_pos,vec_player_bodies_involved[selected_player]);
                                }
                            }

                            tmp=tmp->GetNext();
                        }

                        //play sound
                        if(sound_box!=0 && m_sound_timer<=0)
                        {
                            m_sound_timer=_enemy_sound_max_time;
                            float data[]={0,0,0, 0,0,0, 0,0,-1,
                                          0,1,0, 0,0,-1, 0,0,0,
                                          1,  1,  0};
                            switch(sound_box)
                            {
                                case 0: break;//no sound
                                case 1:
                                {
                                    data[14]=0;
                                }break;
                                case 2://left
                                {
                                    data[12]=-_sound_box_side_shift;
                                    data[19]=_sound_box_level_outside;
                                }break;
                                case 3://right
                                {
                                    data[12]=_sound_box_side_shift;
                                    data[19]=_sound_box_level_outside;
                                }break;
                                case 4:
                                {
                                    data[12]=0;
                                    data[19]=_sound_box_level_outside;
                                }break;
                                case 5:
                                {
                                    data[12]=0;
                                    data[19]=_sound_box_level_outside;
                                }break;
                            }
                            m_pSound->playWAVE(wav_enemy_ship_detected,data);
                        }
                    }
                }

                //other enemies search for players to attack
                else
                {
                    for(int player_i=0;player_i<(int)m_vec_pPlayer_bodies.size();player_i++)
                    {
                        st_body_user_data* player_data=(st_body_user_data*)m_vec_pPlayer_bodies[player_i]->GetUserData();

                        //ignore cloaked and dead players
                        if(player_data->b_cloaked || !player_data->b_alive) continue;

                        //test if players within search box
                        float search_dist=m_ai_detection_range;
                        b2Vec2 player_pos=m_vec_pPlayer_bodies[player_i]->GetPosition();
                        if( body_pos.x+search_dist>player_pos.x && body_pos.x-search_dist<player_pos.x &&
                            body_pos.y+search_dist>player_pos.y && body_pos.y-search_dist<player_pos.y )
                        {
                            //test if target is within view
                            MyRayCastCallback callback;
                            callback.set_ignore_body(m_pBody);
                            m_pWorld->RayCast(&callback,body_pos,player_pos);
                            if(callback.m_any_hit)
                            {
                                b2Body* body_in_view=callback.m_pFixture->GetBody();
                                st_body_user_data* data=(st_body_user_data*)body_in_view->GetUserData();
                                //if hit, make sure that it belongs to the player
                                if(data->s_info=="player" ||
                                   data->s_info=="rope" ||
                                   data->s_info=="hook" )
                                {//target in view
                                    //set new target to attack
                                    cout<<"Enemy: Target spotted\n";

                                    //play sound
                                    if(sound_box!=0 && m_sound_timer<=0 && m_plan_phase!=pp_attack)
                                    {
                                        m_sound_timer=_enemy_sound_max_time;
                                        float data[]={0,0,0, 0,0,0, 0,0,-1,
                                                      0,1,0, 0,0,-1, 0,0,0,
                                                      1,  1,  0};
                                        switch(sound_box)
                                        {
                                            case 0: break;//no sound
                                            case 1:
                                            {
                                                data[14]=0;
                                            }break;
                                            case 2://left
                                            {
                                                data[12]=-_sound_box_side_shift;
                                                data[19]=_sound_box_level_outside;
                                            }break;
                                            case 3://right
                                            {
                                                data[12]=_sound_box_side_shift;
                                                data[19]=_sound_box_level_outside;
                                            }break;
                                            case 4:
                                            {
                                                data[12]=0;
                                                data[19]=_sound_box_level_outside;
                                            }break;
                                            case 5:
                                            {
                                                data[12]=0;
                                                data[19]=_sound_box_level_outside;
                                            }break;
                                        }
                                        m_pSound->playWAVE(wav_enemy_ship_detected,data);
                                    }

                                    m_pTarget_body=m_vec_pPlayer_bodies[player_i];
                                    m_ship_target_pos=player_pos;
                                    m_plan_phase=pp_attack;
                                    m_pure_idle_timer=m_pure_idle_delay;//reset idle timer
                                    if(body_pos.x<player_pos.x) m_target_on_right_side=true;
                                    else                        m_target_on_right_side=false;

                                    break;//dont test for other players
                                }
                                //else cout<<"Body in the way of player: "<<data->s_info<<endl;
                            }
                            else//no hit, strange...
                            {
                                cout<<"ERROR: Enemy search for player to attack: No bodies in sight range\n";
                            }
                        }
                    }
                }

            }break;

            case pp_attack:
            {
                //test if target still is within sight
                float search_dist=m_ai_sight_range;
                b2Vec2 player_pos=m_pTarget_body->GetPosition();
                if( body_pos.x+search_dist>player_pos.x && body_pos.x-search_dist<player_pos.x &&
                    body_pos.y+search_dist>player_pos.y && body_pos.y-search_dist<player_pos.y )
                {
                    //test if within sight line
                    //test if target is within view
                    MyRayCastCallback callback;
                    callback.set_ignore_body(m_pBody);
                    m_pWorld->RayCast(&callback,body_pos,player_pos);
                    if(callback.m_any_hit)
                    {
                        //if hit, make sure that it belongs to the player
                        b2Body* body_in_view=callback.m_pFixture->GetBody();
                        st_body_user_data* data=(st_body_user_data*)body_in_view->GetUserData();
                        if(!data->b_cloaked && data->b_alive &&
                           ( data->s_info=="player" ||
                             data->s_info=="rope"   ||
                             data->s_info=="hook" ) )
                        {//target in view
                            //update target pos
                            m_ship_target_pos=player_pos;
                            if(body_pos.x<player_pos.x)
                            {
                                //swap update only if target is aligned
                                if(body_pos.y+m_height_limit_max>player_pos.y &&
                                   body_pos.y-m_height_limit_max<player_pos.y)
                                {
                                    if(!m_target_on_right_side) m_target_swap_side=true;
                                    else m_target_swap_side=false;
                                }
                                else m_target_swap_side=false;
                                m_target_on_right_side=true;
                            }
                            else
                            {
                                //swap update only if target is aligned
                                if(body_pos.y+m_height_limit_max>player_pos.y &&
                                   body_pos.y-m_height_limit_max<player_pos.y)
                                {
                                    if(m_target_on_right_side) m_target_swap_side=true;
                                    else m_target_swap_side=false;
                                }
                                else m_target_swap_side=false;
                                m_target_on_right_side=false;
                            }
                        }
                        else
                        {
                            //cout<<"Lost sight of player, body in sight: "<<data->s_info<<endl;
                            m_plan_phase=pp_idle;
                            m_patrol_right=m_target_on_right_side;
                        }
                    }
                    else//no hit, strange...
                    {
                        cout<<"No bodies in sight range\n";
                    }
                }
                else//target lost
                {
                    //cout<<"Target list due to distance\n";
                    m_plan_phase=pp_idle;
                    m_patrol_right=m_target_on_right_side;

                    //play sound
                    if(sound_box!=0 && m_sound_timer<=0)
                    {
                        m_sound_timer=_enemy_sound_max_time;
                        float data[]={0,0,0, 0,0,0, 0,0,-1,
                                      0,1,0, 0,0,-1, 0,0,0,
                                      1,  1,  0};
                        switch(sound_box)
                        {
                            case 0: break;//no sound
                            case 1:
                            {
                                data[14]=0;
                            }break;
                            case 2://left
                            {
                                data[12]=-_sound_box_side_shift;
                                data[19]=_sound_box_level_outside;
                            }break;
                            case 3://right
                            {
                                data[12]=_sound_box_side_shift;
                                data[19]=_sound_box_level_outside;
                            }break;
                            case 4:
                            {
                                data[12]=0;
                                data[19]=_sound_box_level_outside;
                            }break;
                            case 5:
                            {
                                data[12]=0;
                                data[19]=_sound_box_level_outside;
                            }break;
                        }
                        m_pSound->playWAVE(wav_enemy_ship_lost,data);
                    }
                }

            }break;

            case pp_run_away:
            {
                //cout<<"Run away\n";
                //think timer extended due to stress
                m_ai_think_timer*=m_ai_flee_think_factor;

                //test if feared target is in sight box
                bool target_out_of_sight=false;
                b2Vec2 target_pos=m_pTarget_body->GetPosition();
                float search_dist=m_ai_sight_range;
                if( body_pos.x+search_dist>target_pos.x && body_pos.x-search_dist<target_pos.x &&
                    body_pos.y+search_dist>target_pos.y && body_pos.y-search_dist<target_pos.y )
                {
                    st_body_user_data* target_data=(st_body_user_data*)m_pTarget_body->GetUserData();

                    //ignore cloaked target
                    if(target_data->b_cloaked || !target_data->b_alive) target_out_of_sight=true;
                    else//continue test
                    {
                        //test if in sight range
                        MyRayCastCallback callback;
                        callback.set_ignore_body(m_pBody);
                        m_pWorld->RayCast(&callback,body_pos,target_pos);
                        if(callback.m_any_hit)
                        {
                            b2Body* body_in_view=callback.m_pFixture->GetBody();
                            st_body_user_data* data=(st_body_user_data*)body_in_view->GetUserData();
                            //test if target
                            if(data->s_info=="player" ||
                               data->s_info=="rope" ||
                               data->s_info=="hook" )
                            {
                                if(body_pos.x<target_pos.x) m_target_on_right_side=true;
                                else                        m_target_on_right_side=false;
                                //target still close, find new target to go to
                                b2Vec2 flee_direction=body_pos-target_pos;
                                flee_direction.Normalize();
                                float flee_angle=atan2f( flee_direction.x , -flee_direction.y )*_Rad2Deg-90.0;
                                vector<b2Vec2> vec_possible_directions;
                                for(float angle=flee_angle-90.0;angle<flee_angle+90.0;angle+=30.0)
                                {
                                    b2Vec2 direction( cosf(angle*_Deg2Rad), sinf(angle*_Deg2Rad) );
                                    b2Vec2 new_pos=body_pos+(m_ai_flee_step_length*direction);
                                    //test if anything in the way
                                    MyRayCastCallback callback;
                                    m_pWorld->RayCast(&callback,body_pos,new_pos);
                                    if(!callback.m_any_hit) vec_possible_directions.push_back(new_pos);
                                }
                                if(vec_possible_directions.empty())//no options
                                {
                                    //try to go towards the threat
                                    for(float angle=flee_angle-90.0;angle>flee_angle-270.0;angle-=30.0)
                                    {
                                        b2Vec2 direction( cosf(angle*_Deg2Rad), sinf(angle*_Deg2Rad) );
                                        b2Vec2 new_pos=body_pos+(m_ai_flee_step_length*direction);
                                        //test if anything in the way
                                        MyRayCastCallback callback;
                                        m_pWorld->RayCast(&callback,body_pos,new_pos);
                                        if(!callback.m_any_hit) vec_possible_directions.push_back(new_pos);
                                    }
                                    if(vec_possible_directions.empty())//still no options
                                    {
                                        //set to attack mode
                                        m_plan_phase=pp_attack;
                                    }

                                }
                                else//pick random pos
                                {
                                    int pick_i=rand()%(int)vec_possible_directions.size();
                                    m_ship_target_pos=vec_possible_directions[pick_i];
                                    //cout<<"Flee: New pos: "<<m_ship_target_pos.x<<", "<<m_ship_target_pos.y<<endl;
                                }
                            }
                            else target_out_of_sight=true;//target not in sight

                        }
                    }
                }
                else target_out_of_sight=true;//target outside sight box

                if(target_out_of_sight)
                {
                    //cout<<"Threat out of sight\n";
                    //set final target pos to go to and return to idle state
                    b2Vec2 flee_direction=body_pos-target_pos;
                    flee_direction.Normalize();
                    float flee_angle=atan2f( flee_direction.x , -flee_direction.y )*_Rad2Deg-90.0;
                    vector<b2Vec2> vec_possible_directions;
                    for(float angle=flee_angle-90.0;angle<flee_angle+90.0;angle+=30.0)
                    {
                        b2Vec2 direction( cosf(angle*_Deg2Rad), sinf(angle*_Deg2Rad) );
                        b2Vec2 new_pos=body_pos+(m_ai_flee_step_length*direction);
                        //test if anything in the way
                        MyRayCastCallback callback;
                        m_pWorld->RayCast(&callback,body_pos,new_pos);
                        if(!callback.m_any_hit) vec_possible_directions.push_back(new_pos);
                    }
                    if(vec_possible_directions.empty())//no options
                    {
                        ;//use current target pos
                    }
                    else//pick random pos
                    {
                        int pick_i=rand()%(int)vec_possible_directions.size();
                        m_ship_target_pos=vec_possible_directions[pick_i];
                    }
                    //return to idle state
                    m_plan_phase=pp_idle;
                }


            }break;

            case pp_lift:
            {
                //make sure that the body dosent move, ignored if just above target (the tuch impack from before hooking could move the body)
                b2Vec2 lin_speed=m_pTarget_body->GetLinearVelocity();
                if( (fabs(lin_speed.x)>m_lift_target_movement_sens||
                     fabs(lin_speed.y)>m_lift_target_movement_sens) && !m_above_target_flag )
                {
                    cout<<"Target moved, ignored\n";
                    //ignore target
                    m_ship_target_pos=body_pos;
                    m_plan_phase=pp_idle;
                    //reset flags
                    m_above_target_flag=false;
                }

                //if players in sight, run away
                for(int player_i=0;player_i<(int)m_vec_pPlayer_bodies.size();player_i++)
                {
                    //test if players within search box
                    float search_dist=m_ai_detection_range;
                    b2Vec2 player_pos=m_vec_pPlayer_bodies[player_i]->GetPosition();
                    if( body_pos.x+search_dist>player_pos.x && body_pos.x-search_dist<player_pos.x &&
                        body_pos.y+search_dist>player_pos.y && body_pos.y-search_dist<player_pos.y )
                    {
                        st_body_user_data* player_data=(st_body_user_data*)m_vec_pPlayer_bodies[player_i]->GetUserData();

                        //ignore cloaked and dead players
                        if(player_data->b_cloaked || !player_data->b_alive) continue;

                        //test if target is within view
                        MyRayCastCallback callback;
                        callback.set_ignore_body(m_pBody);
                        m_pWorld->RayCast(&callback,body_pos,player_pos);
                        if(callback.m_any_hit)
                        {
                            b2Body* body_in_view=callback.m_pFixture->GetBody();
                            st_body_user_data* data=(st_body_user_data*)body_in_view->GetUserData();
                            //if hit, make sure that it belongs to the player
                            if(data->s_info=="player" ||
                               data->s_info=="rope" ||
                               data->s_info=="hook" )
                            {//target in view
                                //set new target to attack
                                cout<<"Lifter spotted player, run away\n";
                                m_pTarget_body=m_vec_pPlayer_bodies[player_i];
                                m_ship_target_pos=body_pos;
                                m_plan_phase=pp_run_away;
                                if(body_pos.x<player_pos.x) m_target_on_right_side=true;
                                else                        m_target_on_right_side=false;
                            }
                        }
                        else//no hit, strange...
                        {
                            cout<<"ERROR: Lifter flees from player: No bodies in sight range\n";
                        }
                        break;//dont test for other players
                    }
                }

            }break;

            case pp_go_to:
            {
                //cout<<"Go to\n";
                //test if done
                if(m_vec_checkpoints.empty())
                {
                    //all checkpoints reached
                    m_plan_phase=pp_idle;
                }
                else
                {
                    //test if curret checkpoint is reached
                    b2Vec2 target_pos=m_vec_checkpoints.front();
                    if(body_pos.x-m_offside_limit_ok<target_pos.x &&
                       body_pos.x+m_offside_limit_ok>target_pos.x &&
                       body_pos.y-m_height_limit_ok<target_pos.y &&
                       body_pos.y+m_height_limit_ok>target_pos.y )
                    {
                        //checkpoint reached, erase and go to next checkpoint
                        m_vec_checkpoints.erase( m_vec_checkpoints.begin() );
                        //test if done
                        if(m_vec_checkpoints.empty())
                        {
                            //all checkpoints reached
                            m_plan_phase=pp_idle;

                            //if in convoy mode, leave map
                            if(m_convoy_mode)
                            {
                                return 2;
                            }
                        }
                        else//set next checkpoint target
                        {
                            m_ship_target_pos=m_vec_checkpoints.front();
                        }
                    }
                    //cout<<"Body pos: "<<body_pos.x<<", "<<body_pos.y<<"\tTarget pos: "<<target_pos.x<<", "<<target_pos.y<<endl;
                }

            }break;
        }
    }

    //measure height above ground
    if(m_measure_ground_dist_timer<=0.0)
    {
        m_measure_ground_dist_timer=m_measure_ground_dist_delay;

        b2Vec2 body_pos=m_pBody->GetPosition();
        float search_distance=1000;
        b2Vec2 point_below=body_pos;//must hit ground
        point_below.y+=search_distance;
        MyRayCastCallback callback;
        callback.set_ignore_body(m_pBody);
        callback.set_ignore_body_type("object");//to avoid lifters with fuel to measure wrong
        m_pWorld->RayCast(&callback,body_pos,point_below);
        if(callback.m_any_hit)
         m_height_above_ground=callback.m_fraction*search_distance;
        else m_height_above_ground=1000;
        //cout<<"Distance to ground: "<<m_height_above_ground<<endl;
    }
    else m_measure_ground_dist_timer-=time_dif;

    //ship movement
    switch(m_type)
    {
        case et_default:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.10;
            b2Vec2 motor_pos_left(-1.0,1.0);
            b2Vec2 motor_pos_right(1.0,1.0);

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                }
                else
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                }

            }

            //if tilt is within limit and height is too low, thrust both motors to get up
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y)
            {
                //blast both motors
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                       sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
            }

            //if x_pos is outside limit, test if ship is tilted to go in correct direction
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //is the ship's tilt correct
                    if(angle>m_tilt_limit_max*0.5)
                    {
                        //blast both to go right
                        float sens=200.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                    }
                    else//try to adjust tilt to correct side
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                    }
                }
                else//far to right
                {
                    //is the ship's tilt correct
                    if(angle<-m_tilt_limit_max*0.5)//target is more than half of tilt max limit
                    {
                        //blast both to go left
                        float sens=200.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                    }
                    else//try to adjust tilt to correct side
                    {
                        //blast right motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                    }
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=20.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                }
                else
                {
                    //blast left motor
                    float sens=20.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                }
            }

            //if tilt is within better limit, adjust for better height
            else if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
            {
                //blast both motors
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                       sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
            }
        }break;

        case et_burst_bot:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.3*m_size,0.5*m_size);
            b2Vec2 motor_pos_right(0.3*m_size,0.5*m_size);

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within limit and height is too low, thrust both motors to get up
            //or if too close to ground
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min)
            {
                //blast both motors
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                       sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
            }

            //if x_pos is outside limit, test if ship is tilted to go in correct direction
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //is the ship's tilt correct
                    if(angle>m_tilt_limit_max*0.5)
                    {
                        //blast both to go right
                        float sens=200.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                    else//try to adjust tilt to correct side
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                }
                else//far to right
                {
                    //is the ship's tilt correct
                    if(angle<-m_tilt_limit_max*0.5)//target is more than half of tilt max limit
                    {
                        //blast both to go left
                        float sens=200.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                    else//try to adjust tilt to correct side
                    {
                        //blast right motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    }
                }
            }

            //if in combat, try to aim
            else if(m_plan_phase==pp_attack)
            {
                //get target angle
                b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                //cout<<target_angle<<endl;

                //if target within aim, shoot
                float weapon_1_angle=angle-30.0;
                float weapon_2_angle=angle+30.0;
                if(weapon_1_angle+m_ai_aim_angle_tolerance>target_angle && weapon_1_angle-m_ai_aim_angle_tolerance<target_angle)
                {//left weapon (1) in sight
                    fire_turret(sound_box,1);
                }
                else if(weapon_2_angle+m_ai_aim_angle_tolerance>target_angle && weapon_2_angle-m_ai_aim_angle_tolerance<target_angle)
                {//right weapon (2) in sight
                    fire_turret(sound_box,2);
                }
                else//adjust aim
                {
                    //find which side is closest
                    if(target_angle>0.0)//target on right side, aim with right weapon
                    {
                        if(weapon_1_angle<target_angle)//target to the right
                        {
                            //cout<<"R LEFT\n";
                            //blast left motor
                            float sens=20.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                                   sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                        }
                        else//target to the left
                        {
                            //cout<<"R RIGHT\n";
                            //blast right motor
                            float sens=20.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                                   sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                        }
                    }
                    else//aim with left weapon
                    {
                        if(weapon_2_angle<target_angle)//target to the right
                        {
                            //cout<<"L LEFT\n";
                            //blast left motor
                            float sens=20.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                                   sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                        }
                        else//target to the left
                        {
                            //cout<<"L RIGHT\n";
                            //blast right motor
                            float sens=20.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                                   sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                        }
                    }
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=20.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=20.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                           sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within better limit, adjust for better height
            else if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
            {
                //blast both motors
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                       sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
            }

        }break;

        case et_auto_flat:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.35*m_size,0.20*m_size);
            b2Vec2 motor_pos_right(0.35*m_size,0.20*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-150.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+150.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                }
                else
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                }
            }

            //if tilt is within limit and height is too low, thrust both motors to get up
            //or if too close to ground
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min)
            {
                //blast both motors
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 forc_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( forc_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*forc_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.3*force_left );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast mostly left motor
                    float sens_left=300.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 forc_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( forc_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*forc_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.3*force_left );


                    /*//is the ship's tilt correct
                    if(angle>m_tilt_limit_max*0.5)
                    {
                        //blast both to go right
                        float sens=200.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                    else//try to adjust tilt to correct side
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }*/
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast mostly right motor
                    float sens_left=100.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 forc_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( forc_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*forc_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.3*force_left );


                    /*//is the ship's tilt correct
                    if(angle<-m_tilt_limit_max*0.5)//target is more than half of tilt max limit
                    {
                        //blast both to go left
                        float sens=200.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                    else//try to adjust tilt to correct side
                    {
                        //blast right motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(m_pBody->GetAngle()-_pi*0.5) * sens ,
                                               sin(m_pBody->GetAngle()-_pi*0.5) * sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    }*/
                }
            }

            //if in combat, try to aim
            else if(m_plan_phase==pp_attack)
            {
                //get target angle
                b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                //cout<<target_angle<<endl;

                //if target within aim, shoot
                float weapon_1_angle=angle-90.0;
                float weapon_2_angle=angle+90.0;
                if(weapon_1_angle+m_ai_aim_angle_tolerance>target_angle && weapon_1_angle-m_ai_aim_angle_tolerance<target_angle)
                {//left weapon (1) in sight
                    //temp straight ahead
                    fire_turret(sound_box,1);
                }
                else if(weapon_2_angle+m_ai_aim_angle_tolerance>target_angle && weapon_2_angle-m_ai_aim_angle_tolerance<target_angle)
                {//right weapon (2) in sight
                    fire_turret(sound_box,2);
                }
                else//adjust tilt
                {
                    if(angle>m_height_limit_ok)
                    {
                        //blast right motor
                        float sens=50.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                    }
                    else
                    {
                        //blast left motor
                        float sens=50.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                    }
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=20.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*force );
                }
                else
                {
                    //blast left motor
                    float sens=20.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.3*force );
                }
            }

            //if tilt is within better limit, adjust for better height
            else if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
            {
                //blast both motors
                float sens=100.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 forc_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( forc_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.3*forc_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.3*force_left );
            }

        }break;

        case et_lifter:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.37*m_size,0.3*m_size);
            b2Vec2 motor_pos_right(0.37*m_size,0.3*m_size);
            b2Vec2 motor_pos_side_left(-0.5*m_size,-0.05*m_size);
            b2Vec2 motor_pos_side_right(0.5*m_size,-0.05*m_size);
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg+180.0;
            float right_side_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float left_side_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;

            //lock up speed
            bool engine_up_off=false;
            b2Vec2 linear_speed=m_pBody->GetLinearVelocity();
            //cout<<linear_speed.y<<endl;
            if(linear_speed.y<-m_speed_limit_up)
            {
                engine_up_off=true;
                //cout<<"off\n";
            }
            if(m_thurst_regulator>1.0)
            {
                m_thurst_regulator*=0.999;
                //cout<<m_thurst_regulator<<endl;
            }

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                //if a target is hooked, release is
                if(m_target_hooked)
                {
                    m_target_hooked=false;
                    m_above_target_flag=false;
                    m_pWorld->DestroyJoint(m_hook_joint);
                }

                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if target is hooked, go up
            else if(m_target_hooked && !m_convoy_mode)
            {
                b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                if(lin_speed.y>=0.0) m_thurst_regulator*=1.01;//increase thust if standing still/going down
                //run lower motors
                float sens=400.0*time_dif*_world_gravity*motor_sens*m_thurst_regulator;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );

                //ready if body have reached m_height_above_target
                if(pos.y<m_hooked_pos.y-m_height_above_target)
                {
                    m_pWorld->DestroyJoint(m_hook_joint);
                    m_target_hooked=false;
                    m_above_target_flag=false;
                    m_ship_target_pos=pos;
                }
            }

            //if too close to ground, blast both (ignored if above target)
            else if(m_height_above_ground<m_height_above_ground_min && !m_above_target_flag && !engine_up_off)
            {
                //cout<<"height min "<<m_height_above_ground<<" < "<<m_height_above_ground_min<<endl;

                if(m_convoy_mode)
                {
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    //cout<<"linspeed: "<<lin_speed.y<<endl;
                    if(lin_speed.y>-m_speed_limit_up)
                    {
                        //cout<<"speed down\n";
                        m_thurst_regulator*=1.01;//increase thust if standing still/going down
                    }
                    //else m_thurst_regulator*=0.99;
                    motor_sens*=m_thurst_regulator*5.0;
                }

                //blast both motors
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //adjust angle better
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                //cout<<"angle adjust\n";

                if(m_convoy_mode)
                {
                    motor_sens*=m_thurst_regulator;
                }

                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within limit and height is too low, thrust both motors to get up
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y-m_height_above_target && !engine_up_off)
            {
                //cout<<"height adjust\n";

                if(m_convoy_mode)
                {
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>=0.0) m_thurst_regulator*=1.01;//increase thust if standing still/going down
                    motor_sens*=m_thurst_regulator;
                    //cout<<"reg: "<<m_thurst_regulator<<endl;
                }

                //blast both motors
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );


                //extra side adjust
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else if(pos.x-m_offside_limit_max>m_ship_target_pos.x)//far to right
                {
                    //blast right motor
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                //cout<<"side adjust\n";

                if(m_convoy_mode && !engine_up_off)
                {
                    m_thurst_regulator*=1.0005;
                    motor_sens*=m_thurst_regulator;
                }

                //not above target anymore
                m_above_target_flag=false;

                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=400.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //adjust for x_pos
            else if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=400.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //raise above target flag
            else if(!m_above_target_flag && m_plan_phase==pp_lift)
            {
                m_above_target_flag=true;
            }

            //go lower to find tuch target
            else if(m_above_target_flag && !m_target_hooked)
            {
                //blast both motors, gently
                float sens=300.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right= b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right,m_pBody->GetWorldPoint( motor_pos_right ),true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );

                //test if target is within hook distance
                b2Vec2 point_below=pos;
                point_below.y+=m_hook_distance;
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,pos,point_below);
                if(callback.m_any_hit)
                {
                    //test if it is the target
                    b2Body* found_body=callback.m_pFixture->GetBody();
                    if(found_body==m_pTarget_body)
                    {
                        cout<<"Target hooked\n";
                        m_pBody_connected_to=m_pTarget_body;
                        m_target_hooked=true;
                        m_thurst_regulator=1.0;
                        m_hooked_pos=pos;
                        //create joint (weld)
                        b2RopeJointDef ropeJointDef;
                        ropeJointDef.bodyA = m_pBody;
                        ropeJointDef.bodyB = m_pTarget_body;
                        ropeJointDef.collideConnected = false;
                        //get relative pos of sensor from other body
                        b2Vec2 rel_pos=pos-m_pTarget_body->GetPosition();
                        ropeJointDef.localAnchorA.Set(0,0);
                        ropeJointDef.localAnchorB=rel_pos;
                        m_hook_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );
                        /*//create joint (weld)
                        b2WeldJointDef weldJointDef;
                        weldJointDef.bodyA = m_pBody;
                        weldJointDef.bodyB = m_pTarget_body;
                        weldJointDef.collideConnected = false;
                        //get relative pos of sensor from other body
                        b2Vec2 rel_pos=pos-m_pTarget_body->GetPosition();
                        weldJointDef.localAnchorA.Set(0,0);
                        weldJointDef.localAnchorB=rel_pos;
                        m_hook_joint = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );*/
                    }
                    else//go back up and try again
                    {
                        st_body_user_data* data=(st_body_user_data*)found_body->GetUserData();
                        cout<<"Wrong target nearby: "<<data->s_info<<endl;
                        m_above_target_flag=false;
                    }
                }
            }

        }break;

        case et_flipper:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            if(m_body_flipped) angle+=180.0;//adjust angle for flip
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left_top(-1.4*m_size,-0.15*m_size);
            b2Vec2 motor_pos_left_low(-1.4*m_size,0.15*m_size);
            b2Vec2 motor_pos_center_top(0.0*m_size,-0.5*m_size);
            b2Vec2 motor_pos_center_low(0.0*m_size,0.5*m_size);
            float motor_angle_top=m_pBody->GetAngle()*_Rad2Deg+0.0;
            float motor_angle_low=m_pBody->GetAngle()*_Rad2Deg+180.0;

            //flip progress
            if(m_body_flipped!=m_flip_target_left)
            {
                if(m_movement_phase!=1) m_thrust_smooth=0.0;
                else if(m_thrust_smooth<1.0) m_thrust_smooth+=time_dif;
                if(m_thrust_smooth>1.0) m_thrust_smooth=1.0;
                m_movement_phase=1;
                //cout<<"flip change\n";
                if(m_flip_target_left)//ship facing right, should flip with left low motor
                {
                    //test if target angle have been reached
                    if(angle>100.0)
                    {
                        m_body_flipped=true;//ship is now facing left
                    }
                    else
                    {
                        //blast low motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_low ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_low ), 0.1*force );
                    }
                }
                else//ship facing left, should flip with left top motor
                {
                    //test if target angle have been reached
                    if(angle<-100.0)
                    {
                        m_body_flipped=false;//ship is now facing right
                    }
                    else
                    {
                        //blast top motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin(motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_top ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_top ), 0.1*force );
                    }
                }
            }

            //fix extreme tilt
            else if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(m_movement_phase!=2) m_thrust_smooth=0.0;
                else if(m_thrust_smooth<1.0) m_thrust_smooth+=time_dif;
                if(m_thrust_smooth>1.0) m_thrust_smooth=1.0;
                m_movement_phase=2;
                //cout<<"tilt fix\n";
                if(angle>m_tilt_limit_max)
                {
                    //cout<<"right\n";
                    //blast left top
                    float sens=100.0*time_dif*_world_gravity*motor_sens*m_thrust_smooth;
                    b2Vec2 force = b2Vec2( cos(motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_top ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_top ), 0.1*force );
                }
                else
                {
                    //cout<<"left\n";
                    //blast left low
                    float sens=100.0*time_dif*_world_gravity*motor_sens*m_thrust_smooth;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_low ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_low ), 0.1*force );
                }
            }

            //fix minimum height
            else if(m_height_above_ground<m_height_above_ground_min)
            {
                if(m_movement_phase!=3) m_thrust_smooth=0.0;
                else if(m_thrust_smooth<1.0) m_thrust_smooth+=time_dif;
                if(m_thrust_smooth>1.0) m_thrust_smooth=1.0;
                m_movement_phase=3;
                //cout<<"Too close to ground\n";
                if(m_body_flipped)//ship upside down, blast top motors
                {
                    //blast center motor
                    float sens=1000.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_top), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_top ), 0.1*force );
                }
                else//blast low motors
                {
                    //blast center motor
                    float sens=1000.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_low), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_low ), 0.1*force );
                }

                //counter blast if angular rotation
                float ang_rotation=m_pBody->GetAngularVelocity();
                if(ang_rotation>m_counter_rotation_limit)//blast top
                {
                    float sens=100.0*time_dif*_world_gravity*motor_sens*ang_rotation;
                    b2Vec2 force = b2Vec2( cos( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_top), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_top ), 0.1*force );
                }
                else if(ang_rotation<-m_counter_rotation_limit)//blast low
                {
                    float sens=100.0*time_dif*_world_gravity*motor_sens*-ang_rotation;
                    b2Vec2 force = b2Vec2( cos( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_low), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_low ), 0.1*force );
                }
            }

            //adjust to target height
            else if(pos.y>m_ship_target_pos.y-m_height_above_target)
            {
                if(m_movement_phase!=4) m_thrust_smooth=0.0;
                else if(m_thrust_smooth<1.0) m_thrust_smooth+=time_dif;
                if(m_thrust_smooth>1.0) m_thrust_smooth=1.0;
                m_movement_phase=4;
                //cout<<"adjust height\n";
                if(m_body_flipped)//ship upside down, blast top motors
                {
                    //blast center motor
                    float sens=1000.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_top), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_top ), 0.1*force );
                }
                else//blast low motors
                {
                    //blast center motor
                    float sens=1000.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_low), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_low ), 0.1*force );
                }

                //counter blast if angular rotation
                float ang_rotation=m_pBody->GetAngularVelocity();
                if(ang_rotation>m_counter_rotation_limit)//blast top
                {
                    float sens=100.0*time_dif*_world_gravity*motor_sens*ang_rotation;
                    b2Vec2 force = b2Vec2( cos( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_top), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_top ), 0.1*force );
                }
                else if(ang_rotation<-m_counter_rotation_limit)//blast low
                {
                    float sens=100.0*time_dif*_world_gravity*motor_sens*-ang_rotation;
                    b2Vec2 force = b2Vec2( cos( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_low), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_low ), 0.1*force );
                }
            }

            //if xpos outside limit
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(m_movement_phase!=5) m_thrust_smooth=0.0;
                else if(m_thrust_smooth<1.0) m_thrust_smooth+=time_dif;
                if(m_thrust_smooth>1.0) m_thrust_smooth=1.0;
                m_movement_phase=5;
                //cout<<"Going sideways\n";
                //if right side towards target, lean and blast center
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//left side of target
                {
                    //test if ship is flipped towards target
                    if(!m_body_flipped)
                    {
                        //flip is correct, test if ship is tilted towards target
                        if(angle>m_tilt_limit_max*0.5)
                        {
                            //blast center low motor
                            float sens=500.0*time_dif*_world_gravity*motor_sens*m_thrust_smooth;
                            b2Vec2 force = b2Vec2( cos( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_low), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_low ), 0.1*force );
                        }
                        else//adjust for better tilt
                        {
                            //blast left low
                            float sens=100.0*time_dif*_world_gravity*motor_sens*m_thrust_smooth;
                            b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_low ), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_low ), 0.1*force );
                        }
                    }
                    else//flip is wrong, rotate
                    {
                        m_flip_target_left=false;
                    }
                }
                else//right side of target
                {
                    //test if ship is flipped towards target
                    if(m_body_flipped)
                    {
                        //flip is correct, test if ship is tilted towards target
                        if(angle<-m_tilt_limit_max*0.5)
                        {
                            //blast center top motor
                            float sens=500.0*time_dif*_world_gravity*motor_sens*m_thrust_smooth;
                            b2Vec2 force = b2Vec2( cos( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_top), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_top ), 0.1*force );
                        }
                        else//adjust for better tilt
                        {
                            //blast left top
                            float sens=100.0*time_dif*_world_gravity*motor_sens*m_thrust_smooth;
                            b2Vec2 force = b2Vec2( cos(motor_angle_top*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin(motor_angle_top*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left_top ), true );
                            //add particle
                            m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left_top ), 0.1*force );
                        }
                    }
                    else//flip is wrong, rotate
                    {
                        m_flip_target_left=true;
                    }
                }
            }



            //if in attack, try to fire at target
            if(m_plan_phase==pp_attack)
            {
                //have to be within limit box
                if( pos.x+m_offside_limit_max>m_ship_target_pos.x && pos.x-m_offside_limit_max<m_ship_target_pos.x )
                {
                    //if passed target and standing still, do early flip
                    if( (pos.x<m_ship_target_pos.x &&  m_body_flipped) ||
                        (pos.x>m_ship_target_pos.x && !m_body_flipped) )
                    {
                        //test if ship have stopped
                        b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                        if( fabs(lin_speed.x) < m_speed_limit_min )
                         m_flip_target_left=!m_flip_target_left;
                    }

                    //get target angle
                    b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                    float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                    while(target_angle>=180.0)  target_angle-=360.0;
                    while(target_angle<=-180.0) target_angle+=360.0;
                    //test if within limit
                    float weapon_angle=angle+90.0;
                    if(m_body_flipped) weapon_angle-=180;
                    while(weapon_angle>=180.0)  weapon_angle-=360.0;
                    while(weapon_angle<=-180.0) weapon_angle+=360.0;
                    if(weapon_angle+m_ai_aim_angle_tolerance>target_angle && weapon_angle-m_ai_aim_angle_tolerance<target_angle)
                    {
                        //fire
                        fire_turret(sound_box);
                    }
                }
            }

        }break;

        case et_stand_turret:
        {
            if(m_plan_phase==pp_attack)
            {
                //aim at target, if nearby
                b2Vec2 target_direction=m_pTarget_body->GetPosition() - m_pBody->GetPosition();
                float target_angle=atan2f( target_direction.x , -target_direction.y )*_Rad2Deg;
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
                //adjust turret angle
                if(turret_angle<target_angle)
                {
                    m_turret_angle+=m_turret_turn_speed*time_dif;
                    if(m_turret_angle>m_turret_angle_max) m_turret_angle=m_turret_angle_max;
                }
                else if(turret_angle>target_angle)
                {
                    m_turret_angle-=m_turret_turn_speed*time_dif;
                    if(m_turret_angle<m_turret_angle_min) m_turret_angle=m_turret_angle_min;
                }
                //test if target is within limit angle
                float new_turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
                if( new_turret_angle+m_ai_aim_angle_tolerance>target_angle &&
                    new_turret_angle-m_ai_aim_angle_tolerance<target_angle )
                {
                    //fire
                    fire_turret(sound_box);
                }

                //cout<<"RelAng: "<<m_turret_angle<<"\tTarAng: "<<target_angle<<"\tAim: "<<new_turret_angle<<endl;
            }


        }break;

        case et_rocket_tank:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.5*m_size,0.0*m_size);
            b2Vec2 motor_pos_right(0.5*m_size,0.0*m_size);
            b2Vec2 motor_pos_center(0.0*m_size,0.25*m_size);
            b2Vec2 motor_pos_tilt_top(-0.45*m_size,-0.25*m_size);
            b2Vec2 motor_pos_tilt_low(-0.45*m_size,0.25*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float center_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;
            float tilt_top_motor_angle=m_pBody->GetAngle()*_Rad2Deg+0.0;
            float tilt_low_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                //timer
                if(m_tilt_adjust_timer<=0.0)
                {
                    if(angle>m_tilt_limit_max)
                    {
                        //blast top tilt motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_top), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_top ), 0.1*force );
                    }
                    else
                    {
                        //blast low tilt motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_low), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_low ), 0.1*force );
                    }
                }
                else m_tilt_adjust_timer-=time_dif;
            }
            else//stop tilt rotation
            {
                m_tilt_adjust_timer=m_tilt_adjust_delay;//reset timer if tilt is ok

                //counter blast if angular rotation
                float ang_rotation=m_pBody->GetAngularVelocity();
                if(ang_rotation>m_counter_rotation_limit)//blast top
                {
                    float sens=200.0*time_dif*_world_gravity*motor_sens*ang_rotation;
                    b2Vec2 force = b2Vec2( cos( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_top), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_top ), 0.1*force );
                }
                else if(ang_rotation<-m_counter_rotation_limit)//blast low
                {
                    float sens=200.0*time_dif*_world_gravity*motor_sens*-ang_rotation;
                    b2Vec2 force = b2Vec2( cos( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_low), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_low ), 0.1*force );
                }
            }

            //if height is too low, thrust center motor to get up
            if(pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min)
            {
                //blast center motor
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                        sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }

                //weak blast to maintain height
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                        sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
            }

            //if in combat, try to aim
            else if(m_plan_phase==pp_attack)
            {
                //get target angle
                b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                //cout<<target_angle<<endl;

                //is the turret on the right side
                if(target_angle>0.0 && m_body_flipped)//target to the right but turret to the left
                {
                    m_flip_target_left=false;
                }
                else if(target_angle<0.0 && !m_body_flipped)//target to the left but turret to the right
                {
                    m_flip_target_left=true;
                }
                //adjust turret flip, when timer is 0 turret fully to the left
                if(!m_body_flipped && m_flip_target_left)
                {
                    if(m_turret_flip_timer<=0.0)
                    {
                        m_turret_flip_timer=0.0;
                        //flip complete
                        m_body_flipped=!m_body_flipped;
                    }
                    else m_turret_flip_timer-=time_dif;
                }
                else if(m_body_flipped && !m_flip_target_left)
                {
                    if(m_turret_flip_timer>=m_turret_flip_delay)
                    {
                        m_turret_flip_timer=m_turret_flip_delay;
                        //flip complete
                        m_body_flipped=!m_body_flipped;
                    }
                    else m_turret_flip_timer+=time_dif;
                }

                //if target within aim, shoot
                if(m_body_flipped==m_flip_target_left)
                {
                    float weapon_angle=angle+90.0;//right
                    if(m_body_flipped) weapon_angle-=180;//left
                    if(weapon_angle+m_ai_aim_angle_tolerance>target_angle &&
                       weapon_angle-m_ai_aim_angle_tolerance<target_angle)
                    {//fire weapon
                        fire_turret(sound_box);
                    }
                }
                //else cout<<"Turret flip timer: "<<m_turret_flip_timer<<endl;

                //weak blast to maintain height, if below target
                if(pos.y>m_ship_target_pos.y-m_height_limit_max)
                {
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
                }
            }


            else//if not in attack mode, move closer to target pos
            {
                //cout<<"Pos adjust\n";
                //adjust for better x pos
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
                {
                    if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                    {
                        //cout<<"go right\n";
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                    else//far to right
                    {
                        //cout<<"go left\n";
                        //blast right motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    }

                    //weak blast to maintain height
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
                }

                //adjust for better height
                if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
                {
                    //weak blast to maintain height
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
                }
            }

        }break;

        case et_grenade_ship:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.4*m_size,0.15*m_size);
            b2Vec2 motor_pos_right(0.4*m_size,0.15*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-120.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+120.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within limit and height is too low, thrust both motors to get up
            //or if too close to ground
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min)
            {
                //blast both motors
                float sens=300.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast mostly left motor
                    float sens_left=300.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=261.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast mostly right motor
                    float sens_left=261.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
            }

            //if in combat, try to aim
            else if(m_plan_phase==pp_attack)
            {
                //get target angle
                b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                //cout<<target_angle<<endl;

                //which turret to fire
                if(target_angle<0.0)//left
                {
                    //target have to be outside min range
                    if(pos.x-m_fire_range_min<m_ship_target_pos.x)
                    {
                        //target too close, go right
                        float sens_left=300.0*time_dif*_world_gravity*motor_sens;
                        float sens_right=261.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                    sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                        b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                     sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                        //add thrust to body
                        m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                    }
                    else
                    {
                        fire_turret(sound_box,1);//left
                    }
                }
                else//right
                {
                    //target have to be outside min range
                    if(pos.x+m_fire_range_min>m_ship_target_pos.x)
                    {
                        //target too close, go left
                        float sens_left=261.0*time_dif*_world_gravity*motor_sens;
                        float sens_right=300.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                    sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                        b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                     sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                        //add thrust to body
                        m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                    }
                    else
                    {
                        fire_turret(sound_box,2);//left
                    }
                }

                //adjust for better height
                if(pos.y+m_height_above_target>m_ship_target_pos.y)
                {
                    //blast both motors
                    float sens=280.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
            }

            //get closer to target x pos
            else if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast mostly left motor
                    float sens_left=300.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=261.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast mostly right motor
                    float sens_left=261.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within better limit, adjust for better height
            else if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
            {
                //blast both motors
                float sens=280.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 forc_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( forc_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*forc_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

        }break;

        case et_flying_turret:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.4*m_size,0.15*m_size);
            b2Vec2 motor_pos_right(0.4*m_size,0.15*m_size);
            b2Vec2 motor_pos_side_left(-0.5*m_size,0.0*m_size);
            b2Vec2 motor_pos_side_right(0.5*m_size,0.0*m_size);
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg+180.0;
            float right_side_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float left_side_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                //cout<<"first ang\n";
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if too close to ground, blast both
            else if( pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min )
            {
                //cout<<"height\n";
                //blast both motors
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //adjust angle better
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                //cout<<"ang\n";
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                //cout<<"xadj\n";
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=150.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //if in attack mode, aim
            else if(m_plan_phase==pp_attack)
            {
                //cout<<"attack\n";
                //go to height above target
                if( pos.y+m_height_above_target>m_ship_target_pos.y )
                {
                    //blast both motors
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                //maintain height within height limit
                else if( pos.y+m_height_above_target+m_height_limit_ok>m_ship_target_pos.y )
                {
                    //add some thrust to maintain height, relative to falling speed
                    float sens=150.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>0) sens+=sens*lin_speed.y;

                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                //else to high, fall down (motors off)


            }

            //adjust for x_pos
            else if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
            {
                //cout<<"x adjust\n";
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=150.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }
            //else cout<<"nothing\n";

            //turret adjustment
            if(m_plan_phase==pp_attack)
            {
                //aim at target, if nearby
                b2Vec2 target_direction=m_pTarget_body->GetPosition() - m_pBody->GetPosition();
                float target_angle=atan2f( target_direction.x , -target_direction.y )*_Rad2Deg;
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;

                //turret upside-down
                if(target_angle<0.0)  target_angle+=180;
                else                  target_angle-=180;
                if(turret_angle<0.0)  turret_angle+=180;
                else                  turret_angle-=180;

                //adjust turret angle
                if(turret_angle<target_angle)
                {
                    m_turret_angle+=m_turret_turn_speed*time_dif;
                    if(m_turret_angle>m_turret_angle_max) m_turret_angle=m_turret_angle_max;
                }
                else if(turret_angle>target_angle)
                {
                    m_turret_angle-=m_turret_turn_speed*time_dif;
                    if(m_turret_angle<m_turret_angle_min) m_turret_angle=m_turret_angle_min;
                }
                //test if target is within limit angle
                float new_turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
                if(new_turret_angle<0.0)  new_turret_angle+=180;
                else                      new_turret_angle-=180;
                if( new_turret_angle+m_ai_aim_angle_tolerance>target_angle &&
                    new_turret_angle-m_ai_aim_angle_tolerance<target_angle )
                {
                    //fire
                    fire_turret(sound_box);
                }

                //cout<<"RelAng: "<<m_turret_angle<<"\tTarAng: "<<target_angle<<"\tAim: "<<new_turret_angle<<endl;
            }

        }break;

        case et_cannon_tank:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.5*m_size,0.0*m_size);
            b2Vec2 motor_pos_right(0.5*m_size,0.0*m_size);
            b2Vec2 motor_pos_center(0.0*m_size,0.25*m_size);
            b2Vec2 motor_pos_tilt_top(-0.45*m_size,-0.25*m_size);
            b2Vec2 motor_pos_tilt_low(-0.45*m_size,0.25*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float center_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;
            float tilt_top_motor_angle=m_pBody->GetAngle()*_Rad2Deg+0.0;
            float tilt_low_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                //timer
                if(m_tilt_adjust_timer<=0.0)
                {
                    if(angle>m_tilt_limit_max)
                    {
                        //blast top tilt motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_top), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_top ), 0.1*force );
                    }
                    else
                    {
                        //blast low tilt motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_low), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_low ), 0.1*force );
                    }
                }
                else m_tilt_adjust_timer-=time_dif;
            }
            else//stop tilt rotation
            {
                m_tilt_adjust_timer=m_tilt_adjust_delay;//reset timer if tilt is ok

                //counter blast if angular rotation
                float ang_rotation=m_pBody->GetAngularVelocity();
                if(ang_rotation>m_counter_rotation_limit)//blast top
                {
                    float sens=200.0*time_dif*_world_gravity*motor_sens*ang_rotation;
                    b2Vec2 force = b2Vec2( cos( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( tilt_top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_top), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_top ), 0.1*force );
                }
                else if(ang_rotation<-m_counter_rotation_limit)//blast low
                {
                    float sens=200.0*time_dif*_world_gravity*motor_sens*-ang_rotation;
                    b2Vec2 force = b2Vec2( cos( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( tilt_low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_tilt_low), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_tilt_low ), 0.1*force );
                }
            }

            //if height is too low, thrust center motor to get up
            if(pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min)
            {
                //blast center motor
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                        sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }

                //weak blast to maintain height
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                        sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
            }

            //if in combat, try to aim
            else if(m_plan_phase==pp_attack)
            {
                //get target angle
                b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                //cout<<target_angle<<endl;

                //is the turret on the right side
                if(target_angle>0.0 && m_body_flipped)//target to the right but turret to the left
                {
                    m_flip_target_left=false;
                }
                else if(target_angle<0.0 && !m_body_flipped)//target to the left but turret to the right
                {
                    m_flip_target_left=true;
                }
                //adjust turret flip, when timer is 0 turret fully to the left
                if(!m_body_flipped && m_flip_target_left)
                {
                    if(m_turret_flip_timer<=0.0)
                    {
                        m_turret_flip_timer=0.0;
                        //flip complete
                        m_body_flipped=!m_body_flipped;
                    }
                    else m_turret_flip_timer-=time_dif;
                }
                else if(m_body_flipped && !m_flip_target_left)
                {
                    if(m_turret_flip_timer>=m_turret_flip_delay)
                    {
                        m_turret_flip_timer=m_turret_flip_delay;
                        //flip complete
                        m_body_flipped=!m_body_flipped;
                    }
                    else m_turret_flip_timer+=time_dif;
                }

                //if target within aim, shoot
                if(m_body_flipped==m_flip_target_left)
                {
                    float weapon_angle=angle+90.0;//right
                    if(m_body_flipped) weapon_angle-=180;//left
                    if(weapon_angle+m_ai_aim_angle_tolerance>target_angle &&
                       weapon_angle-m_ai_aim_angle_tolerance<target_angle)
                    {//fire weapon
                        fire_turret(sound_box);
                    }
                }
                //else cout<<"Turret flip timer: "<<m_turret_flip_timer<<endl;

                //weak blast to maintain height, if below target
                if(pos.y>m_ship_target_pos.y-m_height_limit_max)
                {
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
                }
            }


            else//if not in attack mode, move closer to target pos
            {
                //cout<<"Pos adjust\n";
                //adjust for better x pos
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
                {
                    if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                    {
                        //cout<<"go right\n";
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                    }
                    else//far to right
                    {
                        //cout<<"go left\n";
                        //blast right motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    }

                    //weak blast to maintain height
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
                }

                //adjust for better height
                if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
                {
                    //weak blast to maintain height
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( center_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center ), 0.1*force );
                }
            }

        }break;

        case et_miner:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.010*m_size;
            b2Vec2 motor_pos_left(-0.43*m_size,0.29*m_size);
            b2Vec2 motor_pos_right(0.43*m_size,0.29*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-150.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+150.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                //cout<<"Max angle: "<<angle<<endl;

                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within limit and height is too low, thrust both motors to get up
            //or if too close to ground
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min)
            {
                //cout<<"Height fix\n";

                //blast both motors
                float sens=190.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast mostly left motor
                    float sens_left=100.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=90.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast mostly right motor
                    float sens_left=90.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
            }

            //if in combat, get closer, place mine and leave
            else if(m_plan_phase==pp_attack)
            {
                //if x_pos is outside ok limit, blast one of the motors to go sideways
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
                {
                    if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                    {
                        //cout<<"go right closer\n";
                        //blast mostly left motor
                        float sens_left=100.0*time_dif*_world_gravity*motor_sens;
                        float sens_right=90.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                    sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                        b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                     sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                        //add thrust to body
                        m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                    }
                    else//far to right
                    {
                        //cout<<"go left closer\n";
                        //blast mostly right motor
                        float sens_left=90.0*time_dif*_world_gravity*motor_sens;
                        float sens_right=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                    sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                        b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                     sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                        //add thrust to body
                        m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                    }
                }

                //fix better height
                else if(pos.y-m_height_limit_ok>m_ship_target_pos.y || pos.y+m_height_limit_ok<m_ship_target_pos.y )
                {
                    //cout<<"Height fix closer\n";
                    if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
                    {
                        //blast both motors
                        float sens=150.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                    sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                     sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                    }
                    //else fall
                }

                else//get ship to stay in place
                {
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if( fabs(lin_speed.y)>m_speed_limit_min )
                    {
                        //counterblast to stop fall
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                    sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                     sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                        m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                    }
                    else//ship pos stable, place mine and flee
                    {
                        fire_turret(sound_box);
                        m_plan_phase=pp_run_away;
                        //set target to just above
                        m_ship_target_pos.y-=30.0;
                    }
                }
            }

            //get closer to target x pos
            else if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    //blast mostly left motor
                    float sens_left=100.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=90.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                else//far to right
                {
                    //cout<<"go left\n";
                    //blast mostly right motor
                    float sens_left=90.0*time_dif*_world_gravity*motor_sens;
                    float sens_right=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens_left );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens_right );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if tilt is within better limit, adjust for better height
            else if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
            {
                //blast both motors
                float sens=110.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 forc_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( forc_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*forc_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

        }break;

        case et_aim_bot:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.025*m_size;
            b2Vec2 motor_pos_top_left(-0.25*m_size,-0.375*m_size);
            b2Vec2 motor_pos_top_right(0.25*m_size,-0.375*m_size);
            b2Vec2 motor_pos_low_left(-0.25*m_size,0.375*m_size);
            b2Vec2 motor_pos_low_right(0.25*m_size,0.375*m_size);
            float motor_angle_top_left =m_pBody->GetAngle()*_Rad2Deg-27.0;
            float motor_angle_top_right=m_pBody->GetAngle()*_Rad2Deg+27.0;
            float motor_angle_low_left =m_pBody->GetAngle()*_Rad2Deg-153.0;
            float motor_angle_low_right=m_pBody->GetAngle()*_Rad2Deg+153.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_low_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left ), 0.1*force );
                }
            }

            //if too close to ground, blast both
            else if( pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min )
            {
                //blast both motors
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_low_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left ),  0.1*force_left );
            }

            //adjust angle better
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_low_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=150.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left  = b2Vec2( cos(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_low_left  ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left  ), 0.1*force_left );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //blast left motors
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_top = b2Vec2( cos( motor_angle_top_left*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_top_left*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_low = b2Vec2( cos( motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_low, m_pBody->GetWorldPoint( motor_pos_low_left ), true );
                    m_pBody->ApplyForce( force_top, m_pBody->GetWorldPoint( motor_pos_top_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left ), 0.1*force_low );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_top_left ), 0.1*force_top );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_top = b2Vec2( cos( motor_angle_top_right*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_top_right*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_low = b2Vec2( cos( motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_low, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    m_pBody->ApplyForce( force_top, m_pBody->GetWorldPoint( motor_pos_top_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_low );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_top_right ), 0.1*force_top );
                }

                //also run lower motors to keep height
                float sens=150.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left  = b2Vec2( cos(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_low_left  ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left  ), 0.1*force_left );
            }

            //if in attack mode, aim
            else if(m_plan_phase==pp_attack)
            {
                //maintain height within height limit
                if( pos.y<m_ship_target_pos.y-m_height_limit_max )
                {
                    ;//fall
                }
                //maintain height within height limit
                else
                {
                    //add some thrust to maintain height, relative to falling speed
                    float sens=150.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>0) sens+=sens*lin_speed.y;

                    b2Vec2 force_left  = b2Vec2( cos(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_low_left  ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left  ), 0.1*force_left );
                }

                //aim at target
                b2Vec2 target_direction=m_pTarget_body->GetPosition() - m_pBody->GetPosition();
                float target_angle=atan2f( target_direction.x , -target_direction.y )*_Rad2Deg;
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
                while(target_angle>= 180.0) target_angle-=360.0;
                while(target_angle<=-180.0) target_angle+=360.0;
                while(turret_angle>= 180.0) turret_angle-=360.0;
                while(turret_angle<=-180.0) turret_angle+=360.0;

                //adjust turret angle
                if(turret_angle<target_angle)
                {
                    m_turret_angle+=m_turret_turn_speed*time_dif;
                }
                else if(turret_angle>target_angle)
                {
                    m_turret_angle-=m_turret_turn_speed*time_dif;
                }
                //test if target is within limit angle
                float new_turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
                while(new_turret_angle>= 180.0) new_turret_angle-=360.0;
                while(new_turret_angle<=-180.0) new_turret_angle+=360.0;
                if( new_turret_angle+m_ai_aim_angle_tolerance>target_angle &&
                    new_turret_angle-m_ai_aim_angle_tolerance<target_angle )
                {
                    //fire
                    fire_turret(sound_box);
                }

                //cout<<"RelAng: "<<m_turret_angle<<"\tTarAng: "<<target_angle<<"\tAim: "<<new_turret_angle<<endl;
            }

            //adjust for x_pos
            else
            {
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_top = b2Vec2( cos( motor_angle_top_left*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_top_left*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_low = b2Vec2( cos( motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_low_left*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_low, m_pBody->GetWorldPoint( motor_pos_low_left ), true );
                    m_pBody->ApplyForce( force_top, m_pBody->GetWorldPoint( motor_pos_top_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left ), 0.1*force_low );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_top_left ), 0.1*force_top );
                }
                else if(pos.x-m_offside_limit_ok>m_ship_target_pos.x)//far to right
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_top = b2Vec2( cos( motor_angle_top_right*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_top_right*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_low = b2Vec2( cos( motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_low, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    m_pBody->ApplyForce( force_top, m_pBody->GetWorldPoint( motor_pos_top_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_low );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_top_right ), 0.1*force_top );
                }

                //also run lower motors to keep height
                if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
                {
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left  = b2Vec2( cos(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_low_left  ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left  ), 0.1*force_left );
                }
                //maintain height within limit
                else if(pos.y+m_height_limit_ok<m_ship_target_pos.y)
                {
                    //add some thrust to maintain height, relative to falling speed
                    float sens=150.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>0) sens+=sens*lin_speed.y;

                    b2Vec2 force_left  = b2Vec2( cos(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(motor_angle_low_left *_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(motor_angle_low_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_low_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_low_left  ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_low_left  ), 0.1*force_left );
                }
                //else fall
            }

        }break;

        case et_rammer:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_center_left(-0.2*m_size,0.0*m_size);
            b2Vec2 motor_pos_center_right(0.2*m_size,0.0*m_size);
            b2Vec2 motor_pos_left(-0.4*m_size,0.5*m_size);
            b2Vec2 motor_pos_right(0.4*m_size,0.5*m_size);
            float motor_angle_center_left =m_pBody->GetAngle()*_Rad2Deg-90.0;
            float motor_angle_center_right=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float motor_angle_low =m_pBody->GetAngle()*_Rad2Deg-180.0;

            //shield timer update
            if(m_shield_target_on)
            {
                if(m_shield_timer<m_shield_delay) m_shield_timer+=time_dif;
                if(m_shield_timer>m_shield_delay) m_shield_timer=m_shield_delay;
            }
            else
            {
                if(m_shield_timer>0.0) m_shield_timer-=time_dif;
                if(m_shield_timer<0.0) m_shield_timer=0.0;
            }
            if(m_shield_timer==m_shield_delay) m_shield_on=true;
            else m_shield_on=false;
            //turn off shield, will be on if in attack mode
            m_shield_target_on=false;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if too close to ground, blast both
            else if( pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min )
            {
                //blast both motors
                float sens=500.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens ,
                                       sin( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force );
            }

            //adjust angle better
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_low*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=250.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens ,
                                       sin( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force );
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_center_left*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_center_left*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_center_right*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_center_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=250.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force = b2Vec2( cos( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens ,
                                       sin( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force );
            }

            //if in attack mode, aim
            else if(m_plan_phase==pp_attack)
            {
                m_shield_target_on=true;//on for ramming

                //maintain height within height limit
                if( pos.y<m_ship_target_pos.y-m_height_limit_max )
                {
                    ;//fall
                }
                //maintain height within height limit
                else
                {
                    //add some thrust to maintain height, relative to falling speed
                    float sens=250.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>0) sens+=sens*lin_speed.y;

                    b2Vec2 force = b2Vec2( cos( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force );
                }

                //rush against target, if not already passed
                if(!m_target_swap_side)
                {
                    if(m_target_on_right_side)//go right
                    {
                        //blast left motor
                        float sens=800.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(motor_angle_center_left*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin(motor_angle_center_left*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_left ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_left ), 0.1*force );
                    }
                    else//go left
                    {
                        //blast right motor
                        float sens=800.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos(motor_angle_center_right*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin(motor_angle_center_right*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_right ), true );
                        //add particle
                        m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_right ), 0.1*force );
                    }
                }
                else//target passed, go to new target pos
                {
                    m_shield_target_on=false;//ram impact over, turn off shield

                    m_target_swap_side=false;//reset
                    //cout<<"Passed target\n";
                    //find suitable pos to go to
                    if(m_target_on_right_side)
                    {
                        //find place to the left of the target
                        b2Vec2 new_target=m_ship_target_pos;
                        new_target.x-=m_ram_reset_distance;
                        //test new pos
                        MyRayCastCallback callback;
                        callback.set_ignore_body(m_pBody);
                        m_pWorld->RayCast(&callback,m_pBody->GetPosition(),new_target);
                        if(!callback.m_any_hit)
                        {
                            //no hit, set new target
                            m_plan_phase=pp_go_to;
                            m_vec_checkpoints.push_back(new_target);
                            m_ship_target_pos=new_target;
                        }
                        else//not possible to go there
                        {
                            //try right instead
                            new_target.x+=m_ram_reset_distance*2.0;
                            //test new pos
                            MyRayCastCallback callback;
                            callback.set_ignore_body(m_pBody);
                            m_pWorld->RayCast(&callback,m_pBody->GetPosition(),new_target);
                            if(!callback.m_any_hit)
                            {
                                //no hit, set new target
                                m_plan_phase=pp_go_to;
                                m_vec_checkpoints.push_back(new_target);
                                m_ship_target_pos=new_target;
                            }
                            else m_plan_phase=pp_run_away;//not possible either, run away instead
                        }
                    }
                    else
                    {
                        //find place to the right of the target
                        b2Vec2 new_target=m_ship_target_pos;
                        new_target.x+=m_ram_reset_distance;
                        //test new pos
                        MyRayCastCallback callback;
                        callback.set_ignore_body(m_pBody);
                        m_pWorld->RayCast(&callback,m_pBody->GetPosition(),new_target);
                        if(!callback.m_any_hit)
                        {
                            //no hit, set new target
                            m_plan_phase=pp_go_to;
                            m_vec_checkpoints.push_back(new_target);
                            m_ship_target_pos=new_target;
                        }
                        else//not possible to go there
                        {
                            //try right instead
                            new_target.x-=m_ram_reset_distance*2.0;
                            //test new pos
                            MyRayCastCallback callback;
                            callback.set_ignore_body(m_pBody);
                            m_pWorld->RayCast(&callback,m_pBody->GetPosition(),new_target);
                            if(!callback.m_any_hit)
                            {
                                //no hit, set new target
                                m_plan_phase=pp_go_to;
                                m_vec_checkpoints.push_back(new_target);
                                m_ship_target_pos=new_target;
                            }
                            else m_plan_phase=pp_run_away;//not possible either, run away instead
                        }
                    }
                }
            }

            //adjust for x_pos
            else
            {
                if(pos.x<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_center_left*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_center_left*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(motor_angle_center_right*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(motor_angle_center_right*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_center_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_center_right ), 0.1*force );
                }

                //also run lower motors to keep height
                if(pos.y-m_height_limit_ok>m_ship_target_pos.y)
                {
                    float sens=300.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force );
                }
                //maintain height within limit
                else if(pos.y>m_ship_target_pos.y)
                {
                    //add some thrust to maintain height, relative to falling speed
                    float sens=250.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>0) sens+=sens*lin_speed.y;

                    b2Vec2 force = b2Vec2( cos( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( motor_angle_low *_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force );
                }
                //else fall
            }

        }break;

        case et_beamer:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos_left(-0.4*m_size,0.10*m_size);
            b2Vec2 motor_pos_right(0.4*m_size,0.10*m_size);
            b2Vec2 motor_pos_side_left(-0.5*m_size,0.0*m_size);
            b2Vec2 motor_pos_side_right(0.5*m_size,0.0*m_size);
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+180.0;
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg+180.0;
            float right_side_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float left_side_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //blast right motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=200.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if too close to ground, blast both
            else if( pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min )
            {
                //blast both motors
                float sens=200.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //adjust angle better
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force );
                }
                else
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin(left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ), 0.1*force );
                }
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=150.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

            //if in attack mode, aim
            else if(m_plan_phase==pp_attack)
            {
                //go to height above target
                if( pos.y+m_height_above_target>m_ship_target_pos.y )
                {
                    //blast both motors
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                //maintain height within height limit
                else if( pos.y+m_height_above_target+m_height_limit_ok>m_ship_target_pos.y )
                {
                    //add some thrust to maintain height, relative to falling speed
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
                    if(lin_speed.y>0) sens+=sens*lin_speed.y;

                    b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                 sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                    m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
                }
                //else to high, fall down (motors off)

                //move towards taget
                if( pos.x<m_ship_target_pos.x )//target to the right
                {
                    //blast left motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//target to the left
                {
                    //blast right motor
                    float sens=100.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //if within offside limit, and above target, fire
                if( pos.x+m_offside_limit_ok>m_ship_target_pos.x && pos.x-m_offside_limit_ok<m_ship_target_pos.x &&
                    pos.y<m_ship_target_pos.y )
                {
                    fire_turret(sound_box,time_dif);
                }
            }

            //adjust for x_pos
            else if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                {
                    //blast left motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( left_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_left ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_left ), 0.1*force );
                }
                else//far to right
                {
                    //blast right motor
                    float sens=50.0*time_dif*_world_gravity*motor_sens;
                    b2Vec2 force = b2Vec2( cos( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                           sin( right_side_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                    //add thrust to body
                    m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos_side_right ), true );
                    //add particle
                    m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_side_right ), 0.1*force );
                }

                //also run lower motors to keep height
                float sens=400.0*time_dif*_world_gravity*motor_sens;
                b2Vec2 force_left = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                            sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                b2Vec2 force_right = b2Vec2( cos(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                             sin(right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                //add thrust to body
                m_pBody->ApplyForce( force_right, m_pBody->GetWorldPoint( motor_pos_right ), true );
                m_pBody->ApplyForce( force_left,  m_pBody->GetWorldPoint( motor_pos_left ), true );
                //add particle
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_right ), 0.1*force_right );
                m_pParticle_engine->add_particle( m_pBody->GetWorldPoint( motor_pos_left ),  0.1*force_left );
            }

        }break;

        case et_cloaker:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();
            b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
            float  ang_speed=m_pBody->GetAngularVelocity();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos(0.0*m_size,0.0*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float top_motor_angle  =m_pBody->GetAngle()*_Rad2Deg+ 0.0;
            float low_motor_angle  =m_pBody->GetAngle()*_Rad2Deg+180.0;

            float max_angular_speed=1.0;
            float max_linear_speed=10.0;

            //update cloak timer (if timer is less than the delay, cloak ON, if equal to the delay enables ship to fire)
            if(m_cloak_target_off)
            {
                //increase timer
                if(m_cloak_timer<m_cloak_delay) m_cloak_timer+=time_dif;
                if(m_cloak_timer>m_cloak_delay) m_cloak_timer=m_cloak_delay;
            }
            else//decrease timer
            {
                if(m_cloak_timer>0.0) m_cloak_timer-=time_dif;
                if(m_cloak_timer<0.0) m_cloak_timer=0.0;
                //turn on cloak, will be on if ship is in stable attack mode, below
                m_cloak_target_off=true;
            }

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    if( fabs(ang_speed) < max_angular_speed || ang_speed>0.0 )
                    {
                        //rotate left
                        float sens=100.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(-sens,true);
                    }
                }
                else
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    float ang_speed=m_pBody->GetAngularVelocity();
                    if( fabs(ang_speed) < max_angular_speed || ang_speed<0.0 )
                    {
                        //rotate right
                        float sens=100.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(sens,true);
                    }
                }
            }

            //if tilt is within limit and height wrong, adjust
            //or if too close to ground
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y || pos.y+m_height_limit_max<m_ship_target_pos.y ||
                    m_height_above_ground<m_height_above_ground_min )
            {
                //cout<<"lin speed: "<<lin_speed.y<<endl;
                if( pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min )
                {
                    //go up
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y>0.0 )
                    {
                        //blast low motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
                else//target under
                {
                    //go down
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y<0.0 )
                    {
                        //blast low motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x<0.0 )
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }

                }
                else//far to right
                {
                    //cout<<"go left\n";
                    if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x>0.0 )
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
            }

            //if in combat, try to aim
            else if(m_plan_phase==pp_attack)
            {
                m_cloak_target_off=false;//will go to fire mode

                //get target angle
                b2Vec2 direction=m_pTarget_body->GetPosition() - pos;
                float target_angle=atan2f( direction.x , -direction.y )*_Rad2Deg;
                //cout<<target_angle<<endl;

                //if target within aim, shoot
                float weapon_1_angle=angle-90.0;
                float weapon_2_angle=angle+90.0;
                if(weapon_1_angle+m_ai_aim_angle_tolerance>target_angle && weapon_1_angle-m_ai_aim_angle_tolerance<target_angle)
                {//left weapon (1) in sight
                    //can only fire if cloak is OFF
                    if(m_cloak_timer==0.0) fire_turret(sound_box,1);
                }
                else if(weapon_2_angle+m_ai_aim_angle_tolerance>target_angle && weapon_2_angle-m_ai_aim_angle_tolerance<target_angle)
                {//right weapon (2) in sight
                    //can only fire if cloak is OFF
                    if(m_cloak_timer==0.0) fire_turret(sound_box,2);
                }
                else//adjust pos
                {
                    //adjust height
                    if( pos.y-m_height_limit_ok>m_ship_target_pos.y )
                    {
                        //go up
                        if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y>0.0 )
                        {
                            //blast low motor
                            float sens=10.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                        }
                    }
                    else if( pos.y+m_height_limit_ok<m_ship_target_pos.y )//target under
                    {
                        //go down
                        if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y<0.0 )
                        {
                            //blast low motor
                            float sens=10.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                        }
                    }

                    //adjust angle
                    if(angle>m_tilt_limit_ok)
                    {
                        //apply only if max speed have not been reached or rotates at wrong direction
                        if( fabs(ang_speed) < max_angular_speed || ang_speed>0.0 )
                        {
                            //rotate left
                            float sens=10.0*time_dif*motor_sens;
                            //add torque to body
                            m_pBody->ApplyTorque(-sens,true);
                        }
                    }
                    else if(angle<m_tilt_limit_ok)
                    {
                        //apply only if max speed have not been reached or rotates at wrong direction
                        float ang_speed=m_pBody->GetAngularVelocity();
                        if( fabs(ang_speed) < max_angular_speed || ang_speed<0.0 )
                        {
                            //rotate right
                            float sens=10.0*time_dif*motor_sens;
                            //add torque to body
                            m_pBody->ApplyTorque(sens,true);
                        }
                    }
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    if( fabs(ang_speed) < max_angular_speed || ang_speed>0.0 )
                    {
                        //rotate left
                        float sens=50.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(-sens,true);
                    }
                }
                else if(angle<m_tilt_limit_ok)
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    float ang_speed=m_pBody->GetAngularVelocity();
                    if( fabs(ang_speed) < max_angular_speed || ang_speed<0.0 )
                    {
                        //rotate right
                        float sens=50.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(sens,true);
                    }
                }
            }

            //if tilt is within better limit, adjust for better pos
            else
            {
                //height adjust
                if( pos.y-m_height_limit_ok>m_ship_target_pos.y )
                {
                    //go up
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y>0.0 )
                    {
                        //blast low motor
                        float sens=50.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
                else if( pos.y+m_height_limit_ok<m_ship_target_pos.y )//target under
                {
                    //go down
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y<0.0 )
                    {
                        //blast low motor
                        float sens=50.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }

                //offside adjust
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
                {
                    if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                    {
                        //cout<<"go right\n";
                        if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x<0.0 )
                        {
                            //blast left motor
                            float sens=50.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                        }

                    }
                    else//far to right
                    {
                        //cout<<"go left\n";
                        if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x>0.0 )
                        {
                            //blast left motor
                            float sens=50.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                        }
                    }
                }
            }

        }break;

        case et_scanner:
        {
            //balance ship into wanted pos
            float angle=m_pBody->GetAngle()*_Rad2Deg;
            while(angle>=180.0)  angle-=360.0;
            while(angle<=-180.0) angle+=360.0;
            b2Vec2 pos=m_pBody->GetPosition();
            b2Vec2 lin_speed=m_pBody->GetLinearVelocity();
            float  ang_speed=m_pBody->GetAngularVelocity();

            float motor_sens=0.017*m_size;
            b2Vec2 motor_pos(0.0*m_size,0.0*m_size);
            float left_motor_angle =m_pBody->GetAngle()*_Rad2Deg-90.0;
            float right_motor_angle=m_pBody->GetAngle()*_Rad2Deg+90.0;
            float top_motor_angle  =m_pBody->GetAngle()*_Rad2Deg+ 0.0;
            float low_motor_angle  =m_pBody->GetAngle()*_Rad2Deg+180.0;

            float max_angular_speed=1.0;
            float max_linear_speed=5.0;

            //if tilt is above limit, full thurst on one side
            if(angle>m_tilt_limit_max||angle<-m_tilt_limit_max)
            {
                if(angle>m_tilt_limit_max)
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    if( fabs(ang_speed) < max_angular_speed || ang_speed>0.0 )
                    {
                        //rotate left
                        float sens=100.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(-sens,true);
                    }
                }
                else
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    float ang_speed=m_pBody->GetAngularVelocity();
                    if( fabs(ang_speed) < max_angular_speed || ang_speed<0.0 )
                    {
                        //rotate right
                        float sens=100.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(sens,true);
                    }
                }
            }

            //if tilt is within limit and height wrong, adjust
            //or if too close to ground
            else if(pos.y-m_height_limit_max>m_ship_target_pos.y || pos.y+m_height_limit_max<m_ship_target_pos.y ||
                    m_height_above_ground<m_height_above_ground_min )
            {
                //cout<<"lin speed: "<<lin_speed.y<<endl;
                if( pos.y-m_height_limit_max>m_ship_target_pos.y || m_height_above_ground<m_height_above_ground_min )
                {
                    //go up
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y>0.0 )
                    {
                        //blast low motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
                else//target under
                {
                    //go down
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y<0.0 )
                    {
                        //blast low motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
            }

            //if x_pos is outside limit, blast one of the motors to go sideways
            else if(pos.x+m_offside_limit_max<m_ship_target_pos.x||pos.x-m_offside_limit_max>m_ship_target_pos.x)
            {
                if(pos.x+m_offside_limit_max<m_ship_target_pos.x)//far to left
                {
                    //cout<<"go right\n";
                    if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x<0.0 )
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }

                }
                else//far to right
                {
                    //cout<<"go left\n";
                    if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x>0.0 )
                    {
                        //blast left motor
                        float sens=100.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
            }

            //if tilt is within limit and height is within limit, adjust for better tilt
            else if(angle>m_tilt_limit_ok||angle<-m_tilt_limit_ok)
            {
                if(angle>m_tilt_limit_ok)
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    if( fabs(ang_speed) < max_angular_speed || ang_speed>0.0 )
                    {
                        //rotate left
                        float sens=50.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(-sens,true);
                    }
                }
                else if(angle<m_tilt_limit_ok)
                {
                    //apply only if max speed have not been reached or rotates at wrong direction
                    float ang_speed=m_pBody->GetAngularVelocity();
                    if( fabs(ang_speed) < max_angular_speed || ang_speed<0.0 )
                    {
                        //rotate right
                        float sens=50.0*time_dif*motor_sens;
                        //add torque to body
                        m_pBody->ApplyTorque(sens,true);
                    }
                }
            }

            //if tilt is within better limit, adjust for better pos
            else
            {
                //height adjust
                if( pos.y-m_height_limit_ok>m_ship_target_pos.y )
                {
                    //go up
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y>0.0 )
                    {
                        //blast low motor
                        float sens=50.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( low_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }
                else if( pos.y+m_height_limit_ok<m_ship_target_pos.y )//target under
                {
                    //go down
                    if( fabs(lin_speed.y) < max_linear_speed || lin_speed.y<0.0 )
                    {
                        //blast low motor
                        float sens=50.0*time_dif*_world_gravity*motor_sens;
                        b2Vec2 force = b2Vec2( cos( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                               sin( top_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                        //add thrust to body
                        m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                    }
                }

                //offside adjust
                if(pos.x+m_offside_limit_ok<m_ship_target_pos.x||pos.x-m_offside_limit_ok>m_ship_target_pos.x)
                {
                    if(pos.x+m_offside_limit_ok<m_ship_target_pos.x)//far to left
                    {
                        //cout<<"go right\n";
                        if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x<0.0 )
                        {
                            //blast left motor
                            float sens=50.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( left_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                        }

                    }
                    else//far to right
                    {
                        //cout<<"go left\n";
                        if( fabs(lin_speed.x) < max_linear_speed || lin_speed.x>0.0 )
                        {
                            //blast left motor
                            float sens=50.0*time_dif*_world_gravity*motor_sens;
                            b2Vec2 force = b2Vec2( cos( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens ,
                                                   sin( right_motor_angle*_Deg2Rad-_pi*0.5) * -sens );
                            //add thrust to body
                            m_pBody->ApplyForce( force, m_pBody->GetWorldPoint( motor_pos ), true );
                        }
                    }
                }
            }

        }break;
    }


    return 1;
}

bool enemy::draw(void)
{
    switch(m_type)
    {
        case et_burst_bot:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(361.0/1024.0,(1024.0-34.0)/1024.0);
            glVertex2f(-18.0,-12.0);
            glTexCoord2f(361.0/1024.0,(1024.0-32.0-34.0)/1024.0);
            glVertex2f(-18.0, 21.0);
            glTexCoord2f(396.0/1024.0,(1024.0-32.0-34.0)/1024.0);
            glVertex2f( 18.0, 21.0);
            glTexCoord2f(396.0/1024.0,(1024.0-34.0)/1024.0);
            glVertex2f( 18.0,-12.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(361.0/1024.0,(1024.0-0.0)/1024.0);
            glVertex2f(-18.0,-12.0);
            glTexCoord2f(361.0/1024.0,(1024.0-32.0)/1024.0);
            glVertex2f(-18.0, 21.0);
            glTexCoord2f(396.0/1024.0,(1024.0-32.0)/1024.0);
            glVertex2f( 18.0, 21.0);
            glTexCoord2f(396.0/1024.0,(1024.0)/1024.0);
            glVertex2f( 18.0,-12.0);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_auto_flat:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(401.0/1024.0,(1024.0-1.0-21.0)/1024.0);
            glVertex2f(-15.0,-9.0);
            glTexCoord2f(401.0/1024.0,(1024.0-20.0-21.0)/1024.0);
            glVertex2f(-15.0, 11.0);
            glTexCoord2f(430.0/1024.0,(1024.0-20.0-21.0)/1024.0);
            glVertex2f( 15.0, 11.0);
            glTexCoord2f(430.0/1024.0,(1024.0-1.0-21.0)/1024.0);
            glVertex2f( 15.0,-9.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(401.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-15.0,-9.0);
            glTexCoord2f(401.0/1024.0,(1024.0-20.0)/1024.0);
            glVertex2f(-15.0, 11.0);
            glTexCoord2f(430.0/1024.0,(1024.0-20.0)/1024.0);
            glVertex2f( 15.0, 11.0);
            glTexCoord2f(430.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-9.0);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_flipper:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(435.0/1024.0,(1024.0-1.0-31.0)/1024.0);
            glVertex2f(-35.0,-15.0);
            glTexCoord2f(435.0/1024.0,(1024.0-30.0-31.0)/1024.0);
            glVertex2f(-35.0, 15.0);
            glTexCoord2f(484.0/1024.0,(1024.0-30.0-31.0)/1024.0);
            glVertex2f( 15.0, 15.0);
            glTexCoord2f(484.0/1024.0,(1024.0-1.0-31.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(435.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-35.0,-15.0);
            glTexCoord2f(435.0/1024.0,(1024.0-30.0)/1024.0);
            glVertex2f(-35.0, 15.0);
            glTexCoord2f(484.0/1024.0,(1024.0-30.0)/1024.0);
            glVertex2f( 15.0, 15.0);
            glTexCoord2f(484.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            //draw led
            /*glColor3f(0,m_led_timer/m_led_time,0);
            float led_size=4.0;
            glBegin(GL_QUADS);
            glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
            glVertex2f(-led_size-25.0,4.0-led_size);
            glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
            glVertex2f(-led_size-25.0,4.0+led_size);
            glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
            glVertex2f(led_size-25.0,4.0+led_size);
            glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
            glVertex2f(led_size-25.0,4.0-led_size);

            glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
            glVertex2f(-led_size-19.0,4.0-led_size);
            glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
            glVertex2f(-led_size-19.0,4.0+led_size);
            glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
            glVertex2f(led_size-19.0,4.0+led_size);
            glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
            glVertex2f(led_size-19.0,4.0-led_size);

            glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
            glVertex2f(-led_size-13.0,4.0-led_size);
            glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
            glVertex2f(-led_size-13.0,4.0+led_size);
            glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
            glVertex2f(led_size-13.0,4.0+led_size);
            glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
            glVertex2f(led_size-13.0,4.0-led_size);
            glEnd();*/

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_rocket_tank:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(489.0/1024.0,(1024.0-1.0-25.0)/1024.0);
            glVertex2f(-15.0,-15.0);
            glTexCoord2f(489.0/1024.0,(1024.0-24.0-25.0)/1024.0);
            glVertex2f(-15.0, 9.0);
            glTexCoord2f(518.0/1024.0,(1024.0-24.0-25.0)/1024.0);
            glVertex2f( 15.0, 9.0);
            glTexCoord2f(518.0/1024.0,(1024.0-1.0-25.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(489.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-15.0,-15.0);
            glTexCoord2f(489.0/1024.0,(1024.0-24.0)/1024.0);
            glVertex2f(-15.0, 9.0);
            glTexCoord2f(518.0/1024.0,(1024.0-24.0)/1024.0);
            glVertex2f( 15.0, 9.0);
            glTexCoord2f(518.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_cannon_tank:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(523.0/1024.0,(1024.0-1.0-25.0)/1024.0);
            glVertex2f(-15.0,-15.0);
            glTexCoord2f(523.0/1024.0,(1024.0-24.0-25.0)/1024.0);
            glVertex2f(-15.0, 9.0);
            glTexCoord2f(552.0/1024.0,(1024.0-24.0-25.0)/1024.0);
            glVertex2f( 15.0, 9.0);
            glTexCoord2f(552.0/1024.0,(1024.0-1.0-25.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(523.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-15.0,-15.0);
            glTexCoord2f(523.0/1024.0,(1024.0-24.0)/1024.0);
            glVertex2f(-15.0, 9.0);
            glTexCoord2f(552.0/1024.0,(1024.0-24.0)/1024.0);
            glVertex2f( 15.0, 9.0);
            glTexCoord2f(552.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_grenade_ship:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(557.0/1024.0,(1024.0-1.0-23.0)/1024.0);
            glVertex2f(-15.0,-11.0);
            glTexCoord2f(557.0/1024.0,(1024.0-22.0-23.0)/1024.0);
            glVertex2f(-15.0, 11.0);
            glTexCoord2f(586.0/1024.0,(1024.0-22.0-23.0)/1024.0);
            glVertex2f( 15.0, 11.0);
            glTexCoord2f(586.0/1024.0,(1024.0-1.0-23.0)/1024.0);
            glVertex2f( 15.0,-11.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(557.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-15.0,-11.0);
            glTexCoord2f(557.0/1024.0,(1024.0-22.0)/1024.0);
            glVertex2f(-15.0, 11.0);
            glTexCoord2f(586.0/1024.0,(1024.0-22.0)/1024.0);
            glVertex2f( 15.0, 11.0);
            glTexCoord2f(586.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-11.0);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_miner:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(591.0/1024.0,(1024.0-1.0-23.0)/1024.0);
            glVertex2f(-15.0,-11.0);
            glTexCoord2f(591.0/1024.0,(1024.0-22.0-23.0)/1024.0);
            glVertex2f(-15.0, 11.0);
            glTexCoord2f(620.0/1024.0,(1024.0-22.0-23.0)/1024.0);
            glVertex2f( 15.0, 11.0);
            glTexCoord2f(620.0/1024.0,(1024.0-1.0-23.0)/1024.0);
            glVertex2f( 15.0,-11.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(591.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-15.0,-11.0);
            glTexCoord2f(591.0/1024.0,(1024.0-22.0)/1024.0);
            glVertex2f(-15.0, 11.0);
            glTexCoord2f(620.0/1024.0,(1024.0-22.0)/1024.0);
            glVertex2f( 15.0, 11.0);
            glTexCoord2f(620.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-11.0);
            glEnd();

            //draw leds
            if(m_hp_curr>0.0)
            {
                float led_size=6.0;
                if(m_led_flip) glColor3f( (m_led_time-m_led_timer)/m_led_time,0.1,0.1);
                else glColor3f(m_led_timer/m_led_time,0.1,0.1);
                if(!m_pWeapon_curr->is_ready_to_fire()) glColor4f(0.6,0.3,0.0,0.5);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size-4.0,-led_size-4.0);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size-4.0,led_size-4.0);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size-4.0,led_size-4.0);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size-4.0,-led_size-4.0);
                glEnd();

                if(m_led_flip) glColor3f(m_led_timer/m_led_time,0.1,0.1);
                else glColor3f( (m_led_time-m_led_timer)/m_led_time,0.1,0.1);
                if(!m_pWeapon_curr->is_ready_to_fire()) glColor4f(0.6,0.3,0.0,0.5);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size+4.0,-led_size-4.0);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size+4.0,led_size-4.0);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size+4.0,led_size-4.0);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size+4.0,-led_size-4.0);
                glEnd();
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);


            glPopMatrix();

        }break;

        case et_lifter:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(703.0/1024.0,(1024.0-1.0-43.0)/1024.0);
            glVertex2f(-25.0,-21.0);
            glTexCoord2f(703.0/1024.0,(1024.0-42.0-43.0)/1024.0);
            glVertex2f(-25.0, 21.0);
            glTexCoord2f(752.0/1024.0,(1024.0-42.0-43.0)/1024.0);
            glVertex2f( 25.0, 21.0);
            glTexCoord2f(752.0/1024.0,(1024.0-1.0-43.0)/1024.0);
            glVertex2f( 25.0,-21.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(703.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-25.0,-21.0);
            glTexCoord2f(703.0/1024.0,(1024.0-42.0)/1024.0);
            glVertex2f(-25.0, 21.0);
            glTexCoord2f(752.0/1024.0,(1024.0-42.0)/1024.0);
            glVertex2f( 25.0, 21.0);
            glTexCoord2f(752.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 25.0,-21.0);
            glEnd();

            //draw leds
            if(m_target_hooked)
            {
                //float led_time_shift=-1.0;
                //float led_time_curr=m_led_timer;
                //bool led_flip=m_led_flip;
                float intense_color1=(sinf(m_led_timer/m_led_time*2.0*_pi)+1.0)/2.0;
                float intense_color2=(sinf(m_led_timer/m_led_time*2.0*_pi+_pi*0.3)+1.0)/2.0;
                float intense_color3=(sinf(m_led_timer/m_led_time*2.0*_pi+_pi*0.6)+1.0)/2.0;
                float led_size=6.0;
                //if(m_led_timer<m_led_time*0.5) glColor3f(m_led_timer/m_led_time,0.1,0.1);
                //else glColor3f(0.1,0.1,0.1);
                glColor3f( intense_color1,intense_color1*0.3,0.0);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,5.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,5.0-led_size);
                glEnd();

                //led 2
                glColor3f( intense_color2,intense_color2*0.3,0.0);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,-8.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,-8.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,-8.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,-8.0-led_size);
                glEnd();

                //led 3
                glColor3f( intense_color3,intense_color3*0.3,0.0);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,-18.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,-18.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,-18.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,-18.0-led_size);
                glEnd();
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);

            glPopMatrix();

        }break;

        case et_stand_turret:
        {
            //draw turret barrel
            glPushMatrix();
            //glColor3f(0.8,0.2,0.2);
            float turret_size=2.0*m_size;
            b2Vec2 pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,-0.35*m_size));
            float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(turret_angle,0,0,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.3,0.3,0.3,0.8);
            glLineWidth(3);
            glBegin(GL_LINES);
            glVertex2f(0,-3.0*turret_size);
            glVertex2f(0,-1.0*turret_size);
            glEnd();
            glPopMatrix();
            glDisable(GL_BLEND);
            glLineWidth(1);

            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(757.0/1024.0,(1024.0-1.0-41.0)/1024.0);
            glVertex2f(-26.0,-20.0);
            glTexCoord2f(757.0/1024.0,(1024.0-40.0-41.0)/1024.0);
            glVertex2f(-26.0, 20.0);
            glTexCoord2f(808.0/1024.0,(1024.0-40.0-41.0)/1024.0);
            glVertex2f( 26.0, 20.0);
            glTexCoord2f(808.0/1024.0,(1024.0-1.0-41.0)/1024.0);
            glVertex2f( 26.0,-20.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(757.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-26.0,-20.0);
            glTexCoord2f(757.0/1024.0,(1024.0-40.0)/1024.0);
            glVertex2f(-26.0, 20.0);
            glTexCoord2f(808.0/1024.0,(1024.0-40.0)/1024.0);
            glVertex2f( 26.0, 20.0);
            glTexCoord2f(808.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 26.0,-20.0);
            glEnd();

            //draw led
            if(!m_is_dead)
            {
                float led_size=6.0;
                if(m_led_flip) glColor3f( (m_led_time-m_led_timer)/m_led_time,(m_led_time-m_led_timer)/m_led_time*0.5,0.1);
                else glColor3f(m_led_timer/m_led_time,m_led_timer/m_led_time*0.5,0.1);
                if(m_plan_phase==pp_attack) glColor3f(0.8,0.1,0.1);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(11.0-led_size,-5.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(11.0-led_size,-5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(11.0+led_size,-5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(11.0+led_size,-5.0-led_size);
                glEnd();
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            /*//draw turret barrel
            glPushMatrix();
            //glColor3f(0.8,0.2,0.2);
            float turret_size=2.0*m_size;
            b2Vec2 pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,-0.35*m_size));
            float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(turret_angle,0,0,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.3,0.3,0.3,0.8);
            glLineWidth(3);
            glBegin(GL_LINES);
            glVertex2f(0,-3.0*turret_size);
            glVertex2f(0,-1.5*turret_size);
            glEnd();
            glBegin(GL_LINE_STRIP);
            //body
            glVertex2f(-1.0*turret_size,-1.0*turret_size);
            glVertex2f(-1.0*turret_size, 1.0*turret_size);
            glVertex2f( 1.0*turret_size, 1.0*turret_size);
            glVertex2f( 1.0*turret_size,-1.0*turret_size);
            glVertex2f(-1.0*turret_size,-1.0*turret_size);
            glEnd();
            //barrel
            glBegin(GL_LINE_STRIP);
            glVertex2f(-0.6*turret_size,-3.0*turret_size);
            glVertex2f(-0.6*turret_size,-1.0*turret_size);
            glVertex2f( 0.6*turret_size,-1.0*turret_size);
            glVertex2f( 0.6*turret_size,-3.0*turret_size);
            glVertex2f(-0.6*turret_size,-3.0*turret_size);
            glEnd();
            glPopMatrix();
            glDisable(GL_BLEND);*/

        }break;

        case et_flying_turret:
        {
            //draw turret
            glPushMatrix();
            float turret_size=2.0*m_size;
            b2Vec2 pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,0.275*m_size));
            float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(turret_angle,0,0,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.3,0.3,0.3,0.8);
            glLineWidth(3);
            glBegin(GL_LINES);
            glVertex2f(0,-3.0*turret_size);
            glVertex2f(0,-1.0*turret_size);
            glEnd();
            glPopMatrix();
            glDisable(GL_BLEND);
            glLineWidth(1);

            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(813.0/1024.0,(1024.0-1.0-27.0)/1024.0);
            glVertex2f(-20.0,-13.0);
            glTexCoord2f(813.0/1024.0,(1024.0-26.0-27.0)/1024.0);
            glVertex2f(-20.0, 13.0);
            glTexCoord2f(852.0/1024.0,(1024.0-26.0-27.0)/1024.0);
            glVertex2f( 20.0, 13.0);
            glTexCoord2f(852.0/1024.0,(1024.0-1.0-27.0)/1024.0);
            glVertex2f( 20.0,-13.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(813.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-20.0,-13.0);
            glTexCoord2f(813.0/1024.0,(1024.0-26.0)/1024.0);
            glVertex2f(-20.0, 13.0);
            glTexCoord2f(852.0/1024.0,(1024.0-26.0)/1024.0);
            glVertex2f( 20.0, 13.0);
            glTexCoord2f(852.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 20.0,-13.0);
            glEnd();

            //draw led
            if(!m_is_dead)
            {
                float led_size=6.0;
                if(m_led_flip) glColor3f( (m_led_time-m_led_timer)/m_led_time,(m_led_time-m_led_timer)/m_led_time*0.5,0.1);
                else glColor3f(m_led_timer/m_led_time,m_led_timer/m_led_time*0.5,0.1);
                if(m_plan_phase==pp_attack) glColor3f(0.8,0.1,0.1);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(4.0-led_size,-4.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(4.0-led_size,-4.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(4.0+led_size,-4.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(4.0+led_size,-4.0-led_size);
                glEnd();
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            /*//draw turret barrel
            glColor3f(0.8,0.2,0.2);
            glPushMatrix();
            float turret_size=2.0*m_size;
            b2Vec2 pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,0.275*m_size));
            float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(turret_angle,0,0,1);
            glBegin(GL_LINE_STRIP);
            //body
            glVertex2f(-1.0*turret_size,-1.0*turret_size);
            glVertex2f(-1.0*turret_size, 1.0*turret_size);
            glVertex2f( 1.0*turret_size, 1.0*turret_size);
            glVertex2f( 1.0*turret_size,-1.0*turret_size);
            glVertex2f(-1.0*turret_size,-1.0*turret_size);
            glEnd();
            //barrel
            glBegin(GL_LINE_STRIP);
            glVertex2f(-0.6*turret_size,-3.0*turret_size);
            glVertex2f(-0.6*turret_size,-1.0*turret_size);
            glVertex2f( 0.6*turret_size,-1.0*turret_size);
            glVertex2f( 0.6*turret_size,-3.0*turret_size);
            glVertex2f(-0.6*turret_size,-3.0*turret_size);
            glEnd();
            glPopMatrix();*/

        }break;

        case et_aim_bot:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(857.0/1024.0,(1024.0-1.0-31.0)/1024.0);
            glVertex2f(-15.0,-15.0);
            glTexCoord2f(857.0/1024.0,(1024.0-30.0-31.0)/1024.0);
            glVertex2f(-15.0, 15.0);
            glTexCoord2f(886.0/1024.0,(1024.0-30.0-31.0)/1024.0);
            glVertex2f( 15.0, 15.0);
            glTexCoord2f(886.0/1024.0,(1024.0-1.0-31.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(857.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-15.0,-15.0);
            glTexCoord2f(857.0/1024.0,(1024.0-30.0)/1024.0);
            glVertex2f(-15.0, 15.0);
            glTexCoord2f(886.0/1024.0,(1024.0-30.0)/1024.0);
            glVertex2f( 15.0, 15.0);
            glTexCoord2f(886.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 15.0,-15.0);
            glEnd();

            //draw led
            if(!m_is_dead)
            {
                float led_size=6.0;
                if(m_led_flip) glColor3f( (m_led_time-m_led_timer)/m_led_time,(m_led_time-m_led_timer)/m_led_time*0.5,0.1);
                else glColor3f(m_led_timer/m_led_time,m_led_timer/m_led_time*0.5,0.1);
                if(m_plan_phase==pp_attack) glColor3f(0.8,0.1,0.1);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,10.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,10.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,10.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,10.0-led_size);
                glEnd();
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            //draw turret barrel
            //glColor3f(0.8,0.2,0.2);
            glPushMatrix();
            float turret_size=3.0*m_size;
            b2Vec2 pos=m_pBody->GetPosition();
            float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_angle;
            glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
            glRotatef(turret_angle,0,0,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.4,0.4,0.4,0.8);
            glBegin(GL_QUADS);
            //body
            glVertex2f(-1.0*turret_size,-1.0*turret_size);
            glVertex2f(-1.0*turret_size, 1.0*turret_size);
            glVertex2f( 1.0*turret_size, 1.0*turret_size);
            glVertex2f( 1.0*turret_size,-1.0*turret_size);
            //glEnd();
            //barrel
            glColor4f(0.3,0.3,0.3,0.8);
            //glBegin(GL_LINES);
            glVertex2f(-0.6*turret_size,-4.0*turret_size);
            glVertex2f(-0.6*turret_size,-1.0*turret_size);
            glVertex2f( 0.6*turret_size,-1.0*turret_size);
            glVertex2f( 0.6*turret_size,-4.0*turret_size);
            glEnd();
            glPopMatrix();

        }break;

        case et_beamer:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(625.0/1024.0,(1024.0-1.0-23.0)/1024.0);
            glVertex2f(-20.0,-11.0);
            glTexCoord2f(625.0/1024.0,(1024.0-22.0-23.0)/1024.0);
            glVertex2f(-20.0, 11.0);
            glTexCoord2f(664.0/1024.0,(1024.0-22.0-23.0)/1024.0);
            glVertex2f( 20.0, 11.0);
            glTexCoord2f(664.0/1024.0,(1024.0-1.0-23.0)/1024.0);
            glVertex2f( 20.0,-11.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(625.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-20.0,-11.0);
            glTexCoord2f(625.0/1024.0,(1024.0-22.0)/1024.0);
            glVertex2f(-20.0, 11.0);
            glTexCoord2f(664.0/1024.0,(1024.0-22.0)/1024.0);
            glVertex2f( 20.0, 11.0);
            glTexCoord2f(664.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 20.0,-11.0);
            glEnd();

            //draw led
            if(m_led_glow>0.0)
            {
                //if(m_draw_beam) glColor4f(0.6,0,0,0.2);
                //else glColor4f(0.2,0.2,0.2,0.1);
                glColor4f(m_led_glow,0.0,0.0,0.1);
                float led_size=6.0;
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,6.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,6.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,6.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,6.0-led_size);
                glEnd();
            }

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);

            glPopMatrix();

            //draw weapon beam
            if(m_draw_beam)
            {
                /*b2Vec2 pos=m_pBody->GetPosition();
                float angle=m_pBody->GetAngle()*_Rad2Deg;
                glPushMatrix();
                glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
                glRotatef(angle,0,0,1);*/

                glColor3f(0.8,0.2,0.2);
                glBegin(GL_LINES);
                glVertex2f(m_vBeam_start.x*_Met2Pix,m_vBeam_start.y*_Met2Pix);
                glVertex2f(m_vBeam_end.x*_Met2Pix,m_vBeam_end.y*_Met2Pix);
                glEnd();

                //glPopMatrix();

                m_draw_beam=false;
                m_play_beam_sound=true;
            }
        }break;

        case et_cloaker:
        {
            //draw ship if not cloaked
            if(m_cloak_timer<m_cloak_delay*0.7)
            {
                float color=1.0-(m_cloak_timer/m_cloak_delay);

                glPushMatrix();
                b2Vec2 body_pos=m_pBody->GetWorldCenter();
                float angle=m_pBody->GetAngle();
                glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
                glRotatef(angle*_Rad2Deg,0,0,1);

                //draw body mask
                glColor3f(1,1,1);
                glEnable(GL_BLEND);
                glBlendFunc(GL_DST_COLOR,GL_ZERO);
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_texture);
                glBegin(GL_QUADS);
                glTexCoord2f(669.0/1024.0,(1024.0-1.0-19.0)/1024.0);
                glVertex2f(-15.0,-9.0);
                glTexCoord2f(669.0/1024.0,(1024.0-18.0-19.0)/1024.0);
                glVertex2f(-15.0, 9.0);
                glTexCoord2f(698.0/1024.0,(1024.0-18.0-19.0)/1024.0);
                glVertex2f( 15.0, 9.0);
                glTexCoord2f(698.0/1024.0,(1024.0-1.0-19.0)/1024.0);
                glVertex2f( 15.0,-9.0);
                glEnd();

                //draw body color
                glColor3f(color,color,color);
                glBlendFunc(GL_ONE,GL_ONE);
                glBegin(GL_QUADS);
                glTexCoord2f(669.0/1024.0,(1024.0-1.0)/1024.0);
                glVertex2f(-15.0,-9.0);
                glTexCoord2f(669.0/1024.0,(1024.0-18.0)/1024.0);
                glVertex2f(-15.0, 9.0);
                glTexCoord2f(698.0/1024.0,(1024.0-18.0)/1024.0);
                glVertex2f( 15.0, 9.0);
                glTexCoord2f(698.0/1024.0,(1024.0-1.0)/1024.0);
                glVertex2f( 15.0,-9.0);
                glEnd();
                glColor3f(1,1,1);

                glDisable(GL_BLEND);
                glDisable(GL_TEXTURE_2D);
                glPopMatrix();
            }
        }break;

        case et_scanner:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*_Rad2Deg,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(333.0/1024.0,(1024.0-21.0)/1024.0);
            glVertex2f(-10.0,-10.0);
            glTexCoord2f(333.0/1024.0,(1024.0-19.0-21.0)/1024.0);
            glVertex2f(-10.0, 10.0);
            glTexCoord2f(352.0/1024.0,(1024.0-19.0-21.0)/1024.0);
            glVertex2f( 10.0, 10.0);
            glTexCoord2f(352.0/1024.0,(1024.0-21.0)/1024.0);
            glVertex2f( 10.0,-10.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(333.0/1024.0,(1024.0-0.0)/1024.0);
            glVertex2f(-10.0,-10.0);
            glTexCoord2f(333.0/1024.0,(1024.0-19.0)/1024.0);
            glVertex2f(-10.0, 10.0);
            glTexCoord2f(352.0/1024.0,(1024.0-19.0)/1024.0);
            glVertex2f( 10.0, 10.0);
            glTexCoord2f(352.0/1024.0,(1024.0-0.0)/1024.0);
            glVertex2f( 10.0,-10.0);
            glEnd();

            //draw leds
            if(m_plan_phase==pp_idle && m_hp_curr>0.0)
            {
                float led_size=5.0;
                //if(m_led_timer<m_led_time*0.5) glColor3f(m_led_timer/m_led_time,0.1,0.1);
                //else glColor3f(0.1,0.1,0.1);
                if(m_led_flip) glColor3f( (m_led_time-m_led_timer)/m_led_time,0.1,0.1);
                else glColor3f(m_led_timer/m_led_time,0.1,0.1);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,5.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,5.0-led_size);
                glEnd();

                //if(m_led_timer<m_led_time*0.5) glColor3f(0.1,0.1,0.1);
                //else glColor3f(m_led_timer/m_led_time,0.1,0.1);
                if(m_led_flip) glColor3f(m_led_timer/m_led_time,0.1,0.1);
                else glColor3f( (m_led_time-m_led_timer)/m_led_time,0.1,0.1);
                glBegin(GL_QUADS);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(-led_size,-5.0-led_size);
                glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(-led_size,-5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
                glVertex2f(led_size,-5.0+led_size);
                glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
                glVertex2f(led_size,-5.0-led_size);
                glEnd();
            }
            glDisable(GL_TEXTURE_2D);

            //draw scan line
            float line_length=3.0;
            if(m_led_flip2 && m_hp_curr>0.0)
            {
                if(m_led_flip)
                {
                    float scan_start=-4.0+9.0*(m_led_timer/m_led_time)-line_length;
                    float scan_end=scan_start+line_length*2.0;
                    if(scan_start<-4.0) scan_start=-4.0;
                    if(scan_start>5.0) scan_start=5.0;
                    if(scan_end<-4.0) scan_end=-4.0;
                    if(scan_end>5.0) scan_end=5.0;
                    glLineWidth(1);
                    glColor3f(1,1,0);
                    glBegin(GL_LINES);
                    glVertex2f(scan_start,0.0);
                    glVertex2f(scan_end,0.0);
                    glEnd();
                }
                else
                {
                    float scan_start=-4.0+9.0*((m_led_timer-m_led_time)/m_led_time)-line_length;
                    float scan_end=scan_start+line_length*2.0;
                    if(m_led_timer>m_led_time*0.5)//flip side, draw start of incoming line
                    {
                        if(scan_start<-4.0) scan_start=-4.0;
                        if(scan_start>5.0) scan_start=5.0;
                        if(scan_end<-4.0) scan_end=-4.0;
                        if(scan_end>5.0) scan_end=5.0;
                        glLineWidth(1);
                        glColor3f(1,1,0);
                        glBegin(GL_LINES);
                        glVertex2f(scan_start,0.0);
                        glVertex2f(scan_end,0.0);
                        glEnd();
                    }
                }
            }
            else if(m_hp_curr>0.0)
            {
                if(!m_led_flip)
                {
                    float scan_start=-4.0+9.0*((m_led_timer-m_led_time)/m_led_time)-line_length;
                    float scan_end=scan_start+line_length*2.0;
                    //if(m_led_timer<m_led_time*0.5)//flip side, draw start of incoming line
                    {
                        scan_start+=18;
                        scan_end+=18;

                        if(scan_start<-4.0) scan_start=-4.0;
                        if(scan_start>5.0) scan_start=5.0;
                        if(scan_end<-4.0) scan_end=-4.0;
                        if(scan_end>5.0) scan_end=5.0;
                        glLineWidth(1);
                        glColor3f(1,1,0);
                        glBegin(GL_LINES);
                        glVertex2f(scan_start,0.0);
                        glVertex2f(scan_end,0.0);
                        glEnd();
                    }
                }
            }

            glPopMatrix();

            //draw weapon beam
            if(m_draw_beam)
            {
                glColor3f(0.2,0.8,0.2);
                glBegin(GL_LINES);
                glVertex2f(m_vBeam_start.x*_Met2Pix,m_vBeam_start.y*_Met2Pix);
                glVertex2f(m_vBeam_end.x*_Met2Pix,m_vBeam_end.y*_Met2Pix);
                glEnd();

                m_draw_beam=false;
            }
            glDisable(GL_BLEND);

        }break;

        case et_rammer:
        {
            glPushMatrix();
            b2Vec2 body_pos=m_pBody->GetWorldCenter();
            float angle=m_pBody->GetAngle();
            glTranslatef(body_pos.x*_Met2Pix,body_pos.y*_Met2Pix,0);
            glRotatef(angle*180.0/_pi,0,0,1);

            //draw body mask
            glColor3f(1,1,1);
            glEnable(GL_BLEND);
            glBlendFunc(GL_DST_COLOR,GL_ZERO);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glBegin(GL_QUADS);
            glTexCoord2f(891.0/1024.0,(1024.0-1.0-31.0)/1024.0);
            glVertex2f(-21.0,-15.0);
            glTexCoord2f(891.0/1024.0,(1024.0-30.0-31.0)/1024.0);
            glVertex2f(-21.0, 15.0);
            glTexCoord2f(932.0/1024.0,(1024.0-30.0-31.0)/1024.0);
            glVertex2f( 21.0, 15.0);
            glTexCoord2f(932.0/1024.0,(1024.0-1.0-31.0)/1024.0);
            glVertex2f( 21.0,-15.0);
            glEnd();

            //draw body color
            glBlendFunc(GL_ONE,GL_ONE);
            glBegin(GL_QUADS);
            glTexCoord2f(891.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f(-21.0,-15.0);
            glTexCoord2f(891.0/1024.0,(1024.0-30.0)/1024.0);
            glVertex2f(-21.0, 15.0);
            glTexCoord2f(932.0/1024.0,(1024.0-30.0)/1024.0);
            glVertex2f( 21.0, 15.0);
            glTexCoord2f(932.0/1024.0,(1024.0-1.0)/1024.0);
            glVertex2f( 21.0,-15.0);
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glPopMatrix();

            //draw shield (ON or OFF)
            if(m_shield_timer>0.0)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glPushMatrix();
                b2Vec2 pos=m_pBody->GetPosition();
                float angle=m_pBody->GetAngle()*_Rad2Deg;
                float shield_blend=m_shield_timer/m_shield_delay;
                glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
                glRotatef(angle,0,0,1);

                glLineWidth(3);
                float shield_size=1.2;
                glColor4f(0.6,0.6,0.9,shield_blend);
                glBegin(GL_LINE_STRIP);
                glVertex2f(-0.5*m_size*shield_size*_Met2Pix,-0.5*m_size*shield_size*_Met2Pix);
                glVertex2f(-0.8*m_size*shield_size*_Met2Pix,0.0*m_size*shield_size*_Met2Pix);
                glVertex2f(-0.5*m_size*shield_size*_Met2Pix,0.5*m_size*shield_size*_Met2Pix);
                glVertex2f(0.5*m_size*shield_size*_Met2Pix,0.5*m_size*shield_size*_Met2Pix);
                glVertex2f(0.8*m_size*shield_size*_Met2Pix,0.0*m_size*shield_size*_Met2Pix);
                glVertex2f(0.5*m_size*shield_size*_Met2Pix,-0.50*m_size*shield_size*_Met2Pix);
                glVertex2f(-0.5*m_size*shield_size*_Met2Pix,-0.5*m_size*shield_size*_Met2Pix);
                glEnd();
                glLineWidth(1);

                glPopMatrix();

                glDisable(GL_BLEND);
            }

        }break;
    }

    return true;
}

b2Body* enemy::get_body_ptr(void)
{
    return m_pBody;
}

bool enemy::set_target_pos(b2Vec2 target_pos)
{
    m_ship_target_pos=target_pos;

    return true;
}

bool enemy::change_hp(float value_dif)
{
    m_hp_curr+=value_dif;

    switch(m_type)
    {
        case et_cloaker:
        {
            //turn off cloak if damaged
            if(value_dif<0.0)
             m_cloak_timer=0.0;

        }break;

        case et_rammer:
        {
            if(m_shield_on && value_dif<0.0)
             m_hp_curr-=value_dif;//no damage taken

        }break;

        case et_lifter:
        {
            //release hooked body
            if(m_hp_curr<=0.0 && m_target_hooked)
            {
                m_pWorld->DestroyJoint(m_hook_joint);
                m_target_hooked=false;
            }

        }break;
    }

    cout<<"Enemy HP: "<<m_hp_curr<<endl;

    if(m_hp_curr<=0.0 && m_hp_curr-value_dif>0.0)
    {
        //enemy dead
        st_body_user_data* data=(st_body_user_data*)m_pBody->GetUserData();
        data->b_alive=false;
        m_hp_curr=0.0;

        //show explosion
        m_pParticle_engine->add_explosion( m_pBody->GetPosition(),rand()%100+50,rand()%40+80,1.5,float(rand()%360) );

        return false;//death
    }
    else if(m_hp_curr>m_hp_max)
    {
        m_hp_curr=m_hp_max;
    }
    return true;
}

bool enemy::set_current_weapon(weapon* pCurr_weapon)
{
    m_pWeapon_curr=pCurr_weapon;

    return true;
}

bool enemy::fire_turret(int sound_box,int weapon_id,float time_dif)
{
    //test if ready
    if( m_pWeapon_curr->is_ready_to_fire() )
    {
        switch(m_type)
        {
            case et_default:
            {
                //give direction of turret (speed of player?)
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction_w_speed);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

            }break;

            case et_burst_bot:
            {
                //give direction of turret (speed of player?)
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_1_angle=m_pBody->GetAngle()*_Rad2Deg-90.0-30.0;
                float turret_2_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+30.0;
                //normalize direction
                b2Vec2 fire_direction(0,0);
                switch(weapon_id)
                {
                    //left
                    case 1: fire_direction.Set( cosf(turret_1_angle*_Deg2Rad), sinf(turret_1_angle*_Deg2Rad) ); break;
                    //right
                    case 2: fire_direction.Set( cosf(turret_2_angle*_Deg2Rad), sinf(turret_2_angle*_Deg2Rad) ); break;
                }

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction_w_speed);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

                return true;

            }break;

            case et_auto_flat:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_1_angle=m_pBody->GetAngle()*_Rad2Deg-90.0-90.0;
                float turret_2_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+90.0;
                //normalize direction
                b2Vec2 fire_direction(0,0);
                switch(weapon_id)
                {
                    //left
                    case 1: fire_direction.Set( cosf(turret_1_angle*_Deg2Rad), sinf(turret_1_angle*_Deg2Rad) ); break;
                    //right
                    case 2: fire_direction.Set( cosf(turret_2_angle*_Deg2Rad), sinf(turret_2_angle*_Deg2Rad) ); break;
                }

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction_w_speed);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

                return true;

            }break;

            case et_flipper:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+90.0;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_spread,data);
                }

                return true;
            }break;

            case et_stand_turret:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,-0.35*m_size));
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+m_turret_angle;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

                return true;
            }break;

            case et_rocket_tank:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,-0.35*m_size));
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg+0.0;//right side
                if(m_body_flipped) turret_angle+=180;//left side
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_rocket,data);
                }

                return true;
            }break;

            case et_grenade_ship:
            {
                //give direction of turret (speed of player?)
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_1_angle=m_pBody->GetAngle()*_Rad2Deg-90.0-40.0;
                float turret_2_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+40.0;
                //normalize direction
                b2Vec2 fire_direction(0,0);
                switch(weapon_id)
                {
                    //left
                    case 1: fire_direction.Set( cosf(turret_1_angle*_Deg2Rad), sinf(turret_1_angle*_Deg2Rad) ); break;
                    //right
                    case 2: fire_direction.Set( cosf(turret_2_angle*_Deg2Rad), sinf(turret_2_angle*_Deg2Rad) ); break;
                }

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction_w_speed);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_grenade,data);
                }

                return true;
            }break;

            case et_flying_turret:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,0.275*m_size));
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+m_turret_angle;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

                return true;
            }break;

            case et_cannon_tank:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetWorldPoint(b2Vec2(0.0*m_size,-0.35*m_size));
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg+0.0;//right side
                if(m_body_flipped) turret_angle+=180;//left side
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //muzzle flash TEMP, later directed
                m_pParticle_engine->add_explosion(barrel_tip_pos,10,200.0,0.2);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_cannon,data);
                }

                return true;
            }break;

            case et_miner:
            {
                //give direction of turret (speed of player?)
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0-180.0;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_mine,data);
                }

                return true;
            }

            case et_aim_bot:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+m_turret_angle;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

                return true;
            }break;

            case et_beamer:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetWorldPoint( b2Vec2(0.0,0.0) );
                float turret_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+180;
                //normalize direction
                b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

                //test beam pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //fire
                int ret_value=m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction,time_dif);

                //get beam pos
                m_pWeapon_curr->get_beam_pos(m_vBeam_start,m_vBeam_end);
                m_draw_beam=true;

                //emmit particles
                float force=2.0;
                m_pParticle_engine->add_particle(m_vBeam_start,-force*fire_direction,0.5);
                if(ret_value==1)
                {
                    //emmit end particles
                    force=4.0;
                    m_pParticle_engine->add_particle(m_vBeam_end,force*fire_direction,0.5);
                }
            }break;

            case et_cloaker:
            {
                //give direction of turret
                b2Vec2 body_pos=m_pBody->GetPosition();
                float turret_1_angle=m_pBody->GetAngle()*_Rad2Deg-90.0-90.0;
                float turret_2_angle=m_pBody->GetAngle()*_Rad2Deg-90.0+90.0;
                //normalize direction
                b2Vec2 fire_direction(0,0);
                switch(weapon_id)
                {
                    //left
                    case 1: fire_direction.Set( cosf(turret_1_angle*_Deg2Rad), sinf(turret_1_angle*_Deg2Rad) ); break;
                    //right
                    case 2: fire_direction.Set( cosf(turret_2_angle*_Deg2Rad), sinf(turret_2_angle*_Deg2Rad) ); break;
                }

                //test bullet pathway through barrel with raycast
                b2Vec2 barrel_tip_pos=b2Vec2( body_pos.x+fire_direction.x*m_fire_ship_to_barrel_dist*m_size,
                                              body_pos.y+fire_direction.y*m_fire_ship_to_barrel_dist*m_size );
                MyRayCastCallback callback;
                callback.set_ignore_body(m_pBody);
                m_pWorld->RayCast(&callback,body_pos,barrel_tip_pos);
                if(callback.m_any_hit) return false;

                //add ship's speed to fire direction
                //b2Vec2 fire_direction_w_speed=0.1*m_pBody->GetLinearVelocity()+fire_direction;

                //fire
                m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction);

                //play sound
                if(sound_box!=0)
                {
                    float data[]={0,0,0, 0,0,0, 0,0,-1,
                                  0,1,0, 0,0,-1, 0,0,0,
                                  1,  1,  0};
                    switch(sound_box)
                    {
                        case 0: break;//no sound
                        case 1:
                        {
                            data[14]=0;
                        }break;
                        case 2://left
                        {
                            data[12]=-_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 3://right
                        {
                            data[12]=_sound_box_side_shift;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 4:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                        case 5:
                        {
                            data[12]=0;
                            data[19]=_sound_box_level_outside;
                        }break;
                    }
                    m_pSound->playWAVE(wav_weapon_pea,data);
                }

                return true;

            }break;
        }
    }

    return false;//was not ready
}

weapon* enemy::get_weapon_ptr(void)
{
    return m_pWeapon_curr;
}

gear* enemy::get_gear_ptr(void)
{
    return m_pGear_curr;
}

bool enemy::hook_status(void)
{
    return m_target_hooked;
}

bool enemy::hook_disconnect(void)
{
    if(!m_target_hooked) return false;

    m_target_hooked=false;

    //test if other joints are connecting
    b2Joint* tmp_joint=m_pWorld->GetJointList();
    int joint_counter=0;
    bool multiple_carriers=false;
    while(tmp_joint)
    {
        if( m_pBody_connected_to==tmp_joint->GetBodyA() ||
            m_pBody_connected_to==tmp_joint->GetBodyB() )
        {
            joint_counter++;
            if(joint_counter>1)
            {
                multiple_carriers=true;
                break;
            }
        }

        tmp_joint=tmp_joint->GetNext();
    }

    //destroy joint
    m_pWorld->DestroyJoint(m_hook_joint);


    if(!multiple_carriers)
    {
        //notify thet body that it is no longer carried
        st_body_user_data* data=(st_body_user_data*)m_pBody_connected_to->GetUserData();
        data->b_is_carried=false;
    }

    return true;
}

bool enemy::force_hook(b2Body* object_ptr,b2Vec2 shift_pos)
{
    if(m_target_hooked) return false;

    cout<<"Target force hooked\n";
    m_pBody_connected_to=object_ptr;
    m_target_hooked=true;
    m_thurst_regulator=object_ptr->GetMass()*10.0;
    //cout<<"mass: "<<object_ptr->GetMass()<<endl;
    m_hooked_pos=m_pBody->GetPosition();
    //create joint (weld)
    b2RopeJointDef ropeJointDef;
    ropeJointDef.bodyA = m_pBody;
    ropeJointDef.bodyB = object_ptr;
    ropeJointDef.collideConnected = true;
    ropeJointDef.maxLength=0.2;
    //get relative pos of sensor from other body
    b2Vec2 rel_pos=m_pBody->GetPosition() - object_ptr->GetPosition() + shift_pos;
    ropeJointDef.localAnchorA.Set(0,0);
    ropeJointDef.localAnchorB=rel_pos;
    m_hook_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );

    return true;
}

b2Body* enemy::get_connected_body(void)
{
    return m_pBody_connected_to;
}

bool enemy::update_player_bodies_ptr(void)
{
    m_vec_pPlayer_bodies.clear();

    b2Body* tmp=m_pWorld->GetBodyList();
    while(tmp)
    {
        st_body_user_data* data=(st_body_user_data*)tmp->GetUserData();
        if(data->s_info=="player")
         m_vec_pPlayer_bodies.push_back(tmp);

        tmp=tmp->GetNext();
    }

    return true;
}

bool enemy::set_convoy_pos(b2Vec2 end_pos)
{
    //enable convoy mode
    m_convoy_mode=true;

    m_convoy_end_pos=end_pos;
    m_ship_target_pos=m_convoy_end_pos;
    m_vec_checkpoints.push_back(m_convoy_end_pos);

    //lifters ignore other tasks
    if(m_type==et_lifter) m_plan_phase=pp_go_to;

    return true;
}

bool enemy::delete_equipment(void)
{
    delete m_pWeapon_curr;
    delete m_pGear_curr;

    return true;
}

bool enemy::scanner_alarm(b2Vec2 scanner_pos,b2Body* target_body)
{
    //ignore alarm if in attack mode
    if(m_plan_phase==pp_attack) return false;

    m_plan_phase=pp_idle;
    m_ship_target_pos=scanner_pos;
    m_pTarget_body=target_body;

    return true;
}
