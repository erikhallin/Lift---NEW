#include "player.h"

player::player()
{
    m_id=-1;
    m_rel_focus_point[0]=0.0;
    m_rel_focus_point[1]=0.0;
    m_hook_connected=m_mship_lock_on=false;
    m_drone_spawn_time=1.0;
}

bool player::init(b2World* world_ptr,b2Body* mship_ptr,particle_engine* pPart_eng,sound* pSound,int texture,
                  int player_id,bool reinit,b2Vec2 pos)//reinit if variables stored from prev
{
    m_pWorld=world_ptr;
    m_pMain_ship_body=mship_ptr;
    m_pParticle_engine=pPart_eng;
    m_pSound=pSound;
    m_id=player_id;
    m_texture=texture;
    m_hook_connected=m_rope_lock_on=m_mship_lock_on=m_mship_land_adjusting=m_hook_off=m_draw_beam=false;
    m_carrier_player_id=-1;//off
    m_dock_adjust_timer=_player_mship_dock_time;
    m_key_hold_time_back=_player_mship_control_key_timer;
    m_key_hold_time_start=_player_mship_control_key_timer;
    m_drone_spawn_timer=m_drone_spawn_time;
    m_outside_map_timer=_player_outside_map_time;
    m_force_led_on=false;
    m_barrel_length=1.0;
    m_angle_time=rand()%360;
    m_led_glow=0.0;
    m_skip_tutorial_flag=m_takeoff_flag=false;
    m_sound_col_timer=0;
    m_key_trigger_a=m_key_trigger_b=false;
    m_key_trigger_line_swap=false;
    m_line_type_tow=true;
    m_fuel_line_to_player_ind=0;
    m_stuck_timer=_player_stuck_time;

    if(!reinit)//only first time
    {
        m_hp_curr=15.0;
        m_hp_max=100.0;
        m_fuel_curr=15.0;
        m_fuel_max=100.0;
        m_ammo_curr=15.0;
        m_ammo_max=100.0;
        m_motor_thrust_power_max=400.0;
        m_ship_mass_factor=1.0;
        m_upgrade_counter=0;

        //shield values
        m_shield_broken=false;
        m_shield_regen_timer=m_shield_regen_delay=5.0;
        m_shield_hp_curr=m_shield_hp_max=30.0;
        m_shield_regen_speed=0.2;

        m_drone_mode=dm_off;
    }

    if(m_drone_mode!=dm_on) m_drone_mode=dm_off;//if drone is active, dont reset

    m_turret_rotation=0.0;
    m_key_trigger_weapon_swap=m_key_trigger_gear_swap=m_key_trigger_use_gear=m_key_trigger_dpad=m_key_trigger_start=false;
    m_key_trigger_thumbstick_left=m_key_trigger_thumbstick_right=false;
    m_key_trigger_LB=m_key_trigger_RB=m_key_trigger_back=m_is_spawning=m_manual_flag=false;

    m_vec_rope_bodies.clear();
    m_vec_rope_joints.clear();

    m_rel_focus_point[0]=0.0;
    m_rel_focus_point[1]=0.0;

    uint16 player_col_cat=0x0000;
    switch(player_id)
    {
        case 0: player_col_cat=_COLCAT_player1; break;
        case 1: player_col_cat=_COLCAT_player2; break;
        case 2: player_col_cat=_COLCAT_player3; break;
        case 3: player_col_cat=_COLCAT_player4; break;
    }

    //gear values
    m_cloak_target_off=true;
    m_cloak_delay=2.0;
    m_cloak_timer=m_cloak_delay;//OFF
    //boost values
    m_boost_timer=0.0;
    m_boost_delay=0.5;
    m_boost_multiplyer=2.0;
    //turret values
    m_turret_aim_on=false;
    m_turret_rotation_speed_slow=100.0;
    m_turret_rotation_speed_fast=200.0;
    m_turret_range=20.0;
    //gyro values
    m_gyro_on=false;
    m_gyro_tilt_limit=5.0;
    m_gyro_fall_speed_limit=0.1;

    //player body should be added to main ship landing strip and welded there
    b2Vec2 mship_pos=m_pMain_ship_body->GetPosition();
    //shift for landing strip pos
    /*switch(player_id)
    {
        case 0: mship_pos.x+=0.00; mship_pos.y+=-1.0; break;
        case 1: mship_pos.x+=1.25; mship_pos.y+=-1.0; break;
        case 2: mship_pos.x+=2.55; mship_pos.y+=-1.0; break;
        case 3: mship_pos.x+=4.00; mship_pos.y+=-1.0; break;
    }*/
    switch(player_id)
    {
        case 0: mship_pos=m_pMain_ship_body->GetWorldPoint( b2Vec2(0.00,-1.0) ); break;
        case 1: mship_pos=m_pMain_ship_body->GetWorldPoint( b2Vec2(1.25,-1.0) ); break;
        case 2: mship_pos=m_pMain_ship_body->GetWorldPoint( b2Vec2(2.55,-1.0) ); break;
        case 3: mship_pos=m_pMain_ship_body->GetWorldPoint( b2Vec2(4.00,-1.0) ); break;
    }
    pos=mship_pos;

    b2BodyDef bodydef;
    bodydef.position.Set(pos.x,pos.y);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=_player_damping_lin;
    bodydef.angularDamping=_player_damping_ang;
    m_pBody=m_pWorld->CreateBody(&bodydef);
    //create ship fixture
    b2PolygonShape shape2;
    b2Vec2 p1(0.000,-0.585); b2Vec2 p2(0.500,0.305); b2Vec2 p3(-0.500,0.305);
    b2Vec2 arr[]={p1,p2,p3};
    shape2.Set(arr,3);
    b2FixtureDef fixturedef;
    fixturedef.shape=&shape2;
    fixturedef.density=_player_density*m_ship_mass_factor;
    fixturedef.filter.maskBits=~_COLCAT_mship;//TEMP
    fixturedef.filter.categoryBits=player_col_cat;//TEMP
    //fixturedef.filter.groupIndex=-1;
    m_pBody->CreateFixture(&fixturedef);
    //set data
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="player";
    user_data->i_id=m_id;
    m_pBody->SetUserData(user_data);
    //move into pos
    m_pBody->SetTransform( pos, 0 );

    //add rope hook
    b2BodyDef bodydef_rope;
    bodydef_rope.type = b2_dynamicBody;
    bodydef_rope.linearDamping=_object_damping_lin;
    bodydef_rope.angularDamping=_object_damping_ang;
    b2FixtureDef fixturedef_rope;
    fixturedef_rope.density=_rope_density;
    b2PolygonShape boxShape;
    boxShape.SetAsBox(_rope_part_width+0.1,_rope_part_length);
    bodydef_rope.position.Set(pos.x, pos.y);
    fixturedef_rope.shape = &boxShape;
    m_pHook_body = m_pWorld->CreateBody( &bodydef_rope );
    fixturedef_rope.filter.maskBits=~_COLCAT_mship;//TEMP
    fixturedef_rope.filter.categoryBits=player_col_cat;//TEMP
    //fixturedef_rope.filter.groupIndex=-1;
    m_pHook_body->CreateFixture( &fixturedef_rope );
    //set data
    st_body_user_data* user_data1=new st_body_user_data;
    user_data1->s_info="hook";
    user_data1->i_id=m_id;
    m_pHook_body->SetUserData(user_data1);

    //add sensor to hook
    b2PolygonShape shape_sensor;
    b2Vec2 sensor_arr[]={ b2Vec2(-0.1,0.4),
                          b2Vec2(-0.1,0.2),
                          b2Vec2(0.1,0.2),
                          b2Vec2(0.1,0.4) };
    shape_sensor.Set(sensor_arr,4);
    b2FixtureDef fixturedef_sensor;
    fixturedef_sensor.shape=&shape_sensor;
    fixturedef_sensor.isSensor=true;
    fixturedef_sensor.filter.categoryBits=_COLCAT_all;
    fixturedef_sensor.filter.maskBits=-1;
    m_pHook_sensor=m_pHook_body->CreateFixture(&fixturedef_sensor);
    //cout<<"hook sens init: "<<&m_pHook_sensor<<", "<<m_id<<endl;

    //remember top rope body
    m_vec_rope_bodies.push_back(m_pHook_body);

    //prismatic joint, link rope with ship
    b2PrismaticJointDef prismJointDef;
    prismJointDef.bodyA = m_pBody;
    prismJointDef.bodyB = m_pHook_body;
    prismJointDef.localAxisA.Set(0.0,1.0);
    prismJointDef.collideConnected = false;
    prismJointDef.localAnchorA.Set(0,0);
    prismJointDef.localAnchorB.Set(0,0);
    prismJointDef.enableLimit=true;
    prismJointDef.lowerTranslation=_rope_lower_trans_limit;
    prismJointDef.upperTranslation=_rope_upper_trans_limit;
    prismJointDef.enableMotor=false;
    prismJointDef.motorSpeed=0.0;
    prismJointDef.maxMotorForce=10;
    m_rope_motor_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );

    //weld top rope top ship
    b2RopeJointDef ropeJointDef;
    ropeJointDef.bodyA = m_pBody;
    ropeJointDef.bodyB = m_vec_rope_bodies.back();
    ropeJointDef.collideConnected = false;
    ropeJointDef.localAnchorA.Set(0,0);
    ropeJointDef.localAnchorB.Set(0,0);
    m_pRope_lock_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );
    m_rope_lock_on=true;

    /*//weld ship to main ship
    b2WeldJointDef weldJointDef;
    weldJointDef.bodyA = m_pBody;
    weldJointDef.bodyB = m_pMain_ship_body;
    weldJointDef.collideConnected = true;
    weldJointDef.localAnchorA.Set(0,0);
    b2Vec2 rel_pos=m_pBody->GetPosition()-m_pMain_ship_body->GetPosition();
    weldJointDef.localAnchorB=rel_pos;
    m_pMship_lock_joint = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
    m_mship_lock_on=true;*/

    if(!reinit)//only first time
    {
        //arm ship with deafault weapon
        m_pWeapon_default=new weapon(m_pWorld,wt_pea,1.0);
        m_pWeapon_curr=m_pWeapon_default;
        m_using_default_weapon=true;
        m_weapon_index=-1;

        //arm with default gear
        m_pGear_default=new gear(m_pWorld,gt_unarmed,1.0);
        m_pGear_curr=m_pGear_default;
        m_using_default_gear=true;
        m_gear_index=-1;
    }

    //start spawn animation
    spawn_from_mship();

    return true;
}

b2Body* player::get_body_ptr(void)
{
    return m_pBody;
}

b2Body* player::get_player_drone_body_ptr(void)
{
    return m_pBody_drone;
}

bool player::set_player_drone_body_ptr(b2Body* body_ptr)
{
    m_pBody_drone=body_ptr;

    return true;
}

int player::get_drone_mode(void)
{
    return m_drone_mode;
}

int player::set_drone_mode(int mode,bool mode_recall)
{
    if(mode_recall)
    {
        //ignore effects
        m_drone_mode=mode;
        return true;
    }

    if(mode==dm_on && m_drone_mode!=dm_on)
    {
        //spawn drone
        b2Vec2 player_pos=m_pBody->GetPosition();
        float spawn_dist_above=1.0;
        b2Vec2 spawn_pos( player_pos.x,player_pos.y-spawn_dist_above );

        float drone_size=0.05;
        b2Vec2 edge_points[]={b2Vec2(-drone_size,-drone_size),b2Vec2(drone_size,-drone_size),
                              b2Vec2(drone_size,drone_size),b2Vec2(-drone_size,drone_size)};
        //create body
        b2BodyDef bodydef;
        bodydef.position=spawn_pos;
        bodydef.type=b2_dynamicBody;
        bodydef.linearDamping=_player_drone_damping_lin;
        bodydef.angularDamping=_player_drone_damping_ang;
        m_pBody_drone=m_pWorld->CreateBody(&bodydef);
        //create fixture
        b2PolygonShape shape2;
        shape2.Set(edge_points,4);
        b2FixtureDef fixturedef;
        fixturedef.shape=&shape2;
        fixturedef.density=_player_density;
        //fixturedef.restitution=0.0;
        fixturedef.filter.categoryBits=_COLCAT_all;
        fixturedef.filter.maskBits=-1;
        m_pBody_drone->CreateFixture(&fixturedef);

        //set data
        st_body_user_data* user_data=new st_body_user_data;
        user_data->s_info="drone";
        user_data->i_id=m_id;
        m_pBody_drone->SetUserData(user_data);

        //eject force
        float eject_force=5.0;
        b2Vec2 force = b2Vec2( 0.0 , -eject_force );
        m_pBody_drone->ApplyForce(force, m_pBody_drone->GetPosition(), true );

        //add small particle explosion
        b2Vec2 exp_pos( player_pos.x, player_pos.y-spawn_dist_above*0.5 );
        m_pParticle_engine->add_explosion( exp_pos,50,100,1.0,0.0 );

        m_drone_mode=dm_on;

        return true;
    }

    if( (mode==dm_off || mode==dm_destroyed) && m_drone_mode==dm_on )
    {
        //remove drone (now done in update, to avoid crash if this drone had more interactions)
        /*st_body_user_data* data=(st_body_user_data*)m_pBody_drone->GetUserData();
        delete data;
        m_pWorld->DestroyBody(m_pBody_drone);*/

        m_drone_mode=mode;

        return true;
    }

    if(mode==m_drone_mode)
    {
        //confirmation of mode

        return true;
    }

    if(mode==dm_off && m_drone_mode==dm_destroyed)
    {
        //other ship's drone joined
        m_drone_mode=mode;

        return true;
    }

    cout<<"ERROR: Player Drone: Could not handle drone request\n";

    return false;
}

b2Fixture* player::get_rope_hook_sensor(void)
{
    //cout<<"hook sens get: "<<&m_pHook_sensor<<", "<<m_id<<endl;
    //return &m_pHook_sensor;
    return m_pHook_sensor;
}

b2Body* player::get_connected_body_ptr(void)
{
    if(!m_hook_connected) return m_pBody;//hook not connected

    return m_pHook_joint->GetBodyB();
}

bool player::set_focus_point(float _x,float _y)
{
    m_rel_focus_point[0]=_x;
    m_rel_focus_point[1]=_y;

    return true;
}

bool player::get_focus_point(float focus_point[2])
{
    focus_point[0]=m_rel_focus_point[0];
    focus_point[1]=m_rel_focus_point[1];

    return true;
}

bool player::set_rope_motor(float motor_speed)
{
    if(motor_speed==0.0)
    {
        m_rope_motor_joint->EnableMotor(false);
        return true;
    }
    else m_rope_motor_joint->EnableMotor(true);

    m_rope_motor_joint->SetMotorSpeed(motor_speed);

    //test if rope part should be added
    if(motor_speed>0.0)
    {
        //unlock rope lock
        if(m_rope_lock_on)
        {
            m_pWorld->DestroyJoint(m_pRope_lock_joint);
            m_rope_lock_on=false;
        }

        //test if rope is max length
        if( (int)m_vec_rope_bodies.size()>=_player_rope_length_max )
        {
            //do not add more rope
            m_rope_motor_joint->SetMotorSpeed(0.0);
            m_rope_motor_joint->EnableMotor(false);
            return false;
        }

        float translation_length=m_rope_motor_joint->GetJointTranslation();
        if( translation_length>=m_rope_motor_joint->GetUpperLimit() )
        {//add rope part
            //destroy prism joint
            m_pWorld->DestroyJoint(m_rope_motor_joint);

            //create rope body
            b2BodyDef bodyDef;
            bodyDef.type = b2_dynamicBody;
            bodyDef.linearDamping=_object_damping_lin;
            bodyDef.angularDamping=_object_damping_ang;
            b2FixtureDef fixtureDef;
            fixtureDef.density =_rope_density;
            b2PolygonShape boxShape;
            boxShape.SetAsBox(_rope_part_width,_rope_part_length);
            bodyDef.position=m_pBody->GetWorldCenter();
            fixtureDef.shape = &boxShape;
            b2Body* new_rope_top_body = m_pWorld->CreateBody( &bodyDef );
            new_rope_top_body->CreateFixture( &fixtureDef );
            //set data
            st_body_user_data* user_data=new st_body_user_data;
            user_data->s_info="rope";
            user_data->i_id=m_id;
            new_rope_top_body->SetUserData(user_data);
            //link to rest of rope (ropejoint)
            b2RopeJointDef ropeJointDef;
            ropeJointDef.bodyA = new_rope_top_body;
            ropeJointDef.bodyB = m_vec_rope_bodies.back();
            ropeJointDef.collideConnected = false;
            ropeJointDef.localAnchorA.Set(0,_rope_part_length);
            ropeJointDef.localAnchorB.Set(0,-_rope_part_length);
            ropeJointDef.maxLength=_rope_max_length;
            b2RopeJoint* rope_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );
            /*//link to rest of rope (revolutejoint)
            b2RevoluteJointDef revJointDef;
            revJointDef.bodyA = new_rope_top_body;
            revJointDef.bodyB = m_vec_rope_bodies.back();
            revJointDef.collideConnected = false;
            revJointDef.localAnchorA.Set(0,_rope_part_length);
            revJointDef.localAnchorB.Set(0,-_rope_part_length);
            revJointDef.enableLimit=true;
            revJointDef.lowerAngle=-40.0*_Deg2Rad;
            revJointDef.upperAngle=40.0*_Deg2Rad;
            b2RevoluteJoint* rev_joint = (b2RevoluteJoint*)m_pWorld->CreateJoint( &revJointDef );*/
            /*//link to rest of rope (distjoint)
            b2DistanceJointDef distJointDef;
            distJointDef.bodyA = new_rope_top_body;
            distJointDef.bodyB = m_vec_rope_bodies.back();
            distJointDef.collideConnected = false;
            distJointDef.localAnchorA.Set(0,0.2);
            distJointDef.localAnchorB.Set(0,-0.2);
            distJointDef.length=_rope_max_length;
            b2DistanceJoint* dist_joint = (b2DistanceJoint*)m_pWorld->CreateJoint( &distJointDef );*/
            //link to ship with new prism joint
            b2PrismaticJointDef prismJointDef;
            prismJointDef.bodyA = m_pBody;
            prismJointDef.bodyB = new_rope_top_body;
            prismJointDef.localAxisA.Set(0.0,1.0);
            prismJointDef.collideConnected = false;
            prismJointDef.localAnchorA.Set(0,0);
            prismJointDef.localAnchorB.Set(0,0);
            prismJointDef.enableLimit=true;
            prismJointDef.lowerTranslation=_rope_lower_trans_limit;
            prismJointDef.upperTranslation=_rope_upper_trans_limit;
            prismJointDef.enableMotor=false;
            prismJointDef.motorSpeed=0.0;
            prismJointDef.maxMotorForce=10;
            m_rope_motor_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );

            //update top rope part
            m_vec_rope_bodies.push_back(new_rope_top_body);
            m_vec_rope_joints.push_back(rope_joint);
        }
    }
    //test if rope part should be removed
    else if(motor_speed<0.0)
    {
        float translation_length=m_rope_motor_joint->GetJointTranslation();
        if( translation_length<=m_rope_motor_joint->GetLowerLimit() )
        {
            //are there any parts left
            if( (m_vec_rope_bodies.size()>1 && m_vec_rope_joints.size()>=1 && !m_hook_connected) ||//not hooked and more than one
                (m_vec_rope_bodies.size()>2 && m_vec_rope_joints.size()>=2)  )//or more than 2 rope lengths
            {//remove rope part
                //only possible if rope body's angle is similar to ships (to prevent removing rope around the ship)
                float angle_rope=m_vec_rope_bodies.back()->GetAngle();
                float angle_ship=m_pBody->GetAngle();
                if( fabs(angle_rope-angle_ship)>_rope_motor_max_angle ) return false;//not ok

                //destroy prism joint, rope joint and rope body
                m_pWorld->DestroyJoint(m_rope_motor_joint);
                m_pWorld->DestroyJoint(m_vec_rope_joints.back());
                m_vec_rope_joints.pop_back();
                st_body_user_data* data_to_delete=(st_body_user_data*)m_vec_rope_bodies.back()->GetUserData();
                //delete data_to_delete;(OLD)
                //delete m_vec_rope_bodies.back()->GetUserData();
                //m_pWorld->DestroyBody(m_vec_rope_bodies.back());(OLD)
                data_to_delete->b_to_be_deleted=true;//mark for removal

                m_vec_rope_bodies.pop_back();

                //link new top rope part to ship with new prism joint
                b2PrismaticJointDef prismJointDef;
                prismJointDef.bodyA = m_pBody;
                prismJointDef.bodyB = m_vec_rope_bodies.back();
                prismJointDef.localAxisA.Set(0.0,1.0);
                prismJointDef.collideConnected = false;
                prismJointDef.localAnchorA.Set(0,0);
                prismJointDef.localAnchorB.Set(0,0);
                prismJointDef.enableLimit=true;
                prismJointDef.lowerTranslation=_rope_lower_trans_limit;
                prismJointDef.upperTranslation=_rope_upper_trans_limit;
                prismJointDef.enableMotor=false;
                prismJointDef.motorSpeed=0.0;
                prismJointDef.maxMotorForce=10;
                m_rope_motor_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );
            }
            else if(!m_rope_lock_on && !m_hook_connected)//rope fully in, lock rope pos (not if hook connected)
            {
                //weld top rope
                b2RopeJointDef ropeJointDef;
                ropeJointDef.bodyA = m_pBody;
                ropeJointDef.bodyB = m_vec_rope_bodies.back();
                ropeJointDef.collideConnected = false;
                ropeJointDef.localAnchorA.Set(0,0);
                ropeJointDef.localAnchorB.Set(0,0);
                m_pRope_lock_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );
                m_rope_lock_on=true;
            }
            else//stop motor, is hooked or locked
            {
                m_rope_motor_joint->SetMotorSpeed(0.0);
                m_rope_motor_joint->EnableMotor(false);
            }
        }
    }

    return false;
}

bool player::shift_player_pos(float x,float y,bool absolute_pos)//returns true if hook is connected
{
    //move player
    float angle=m_pBody->GetAngle();
    b2Vec2 pos=m_pBody->GetWorldCenter();
    float shift_x=x;
    float shift_y=y;

    if(absolute_pos)//move to specified pos
    {
        //update x and y to get relative movement
        shift_x=x-pos.x;
        shift_y=y-pos.y;
    }

    m_pBody->SetTransform( b2Vec2( pos.x+shift_x,pos.y+shift_y ), angle );

    //move all rope parts
    for(int part_i=0;part_i<(int)m_vec_rope_bodies.size();part_i++)
    {
        float rope_angle=m_vec_rope_bodies[part_i]->GetAngle();
        b2Vec2 rope_pos=m_vec_rope_bodies[part_i]->GetWorldCenter();
        m_vec_rope_bodies[part_i]->SetTransform( b2Vec2( rope_pos.x+shift_x,rope_pos.y+shift_y ), rope_angle );
    }

    //if connected, move that object (unless that object is the terrain)
    if(m_hook_connected)
    {
        return true;
    }

    return false;
}

bool player::update(float time_dif,float view_pos[4])
{
    //update angle time for leds
    m_angle_time+=time_dif*200.0;
    if(m_angle_time>360.0) m_angle_time-=360.0;

    //update sound timer
    if(m_sound_col_timer>0) m_sound_col_timer-=time_dif;

    //play laser sound if drawing laser beam
    if(m_draw_beam)
    {
        //calc sound area
        b2Vec2 pos=m_pBody->GetPosition();
        int sound_box=0;//sound off
        float extra_side_area=(view_pos[2]-view_pos[0])*_sound_box_side_rel_dist;
        if(pos.x*_Met2Pix>view_pos[0] &&
           pos.x*_Met2Pix<view_pos[2] &&
           pos.y*_Met2Pix>view_pos[1] &&
           pos.y*_Met2Pix<view_pos[3] )
        {
            sound_box=1;//on screen
        }
        else if(pos.x*_Met2Pix>view_pos[0]-extra_side_area &&
                pos.x*_Met2Pix<view_pos[0] &&
                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
        {
            sound_box=2;//left side
        }
        else if(pos.x*_Met2Pix>view_pos[2] &&
                pos.x*_Met2Pix<view_pos[2]+extra_side_area &&
                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
        {
            sound_box=3;//right side
        }
        else if(pos.x*_Met2Pix>view_pos[0] &&
                pos.x*_Met2Pix<view_pos[2] &&
                pos.y*_Met2Pix>view_pos[1]-extra_side_area &&
                pos.y*_Met2Pix<view_pos[1] )
        {
            sound_box=4;//top side
        }
        else if(pos.x*_Met2Pix>view_pos[0] &&
                pos.x*_Met2Pix<view_pos[2] &&
                pos.y*_Met2Pix>view_pos[3] &&
                pos.y*_Met2Pix<view_pos[3]+extra_side_area )
        {
            sound_box=5;//top side
        }

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

            switch(m_id)
            {
                case 0: m_pSound->updateSound(_sound_chan_laser_p1,sound_data); break;
                case 1: m_pSound->updateSound(_sound_chan_laser_p2,sound_data); break;
                case 2: m_pSound->updateSound(_sound_chan_laser_p3,sound_data); break;
                case 3: m_pSound->updateSound(_sound_chan_laser_p4,sound_data); break;
            }
        }
    }

    //update beam led glow
    if(m_led_glow>0.0)
    {
        m_led_glow-=time_dif*20.0;
        if(m_led_glow<0.0) m_led_glow=0.0;
    }
    if(m_draw_beam) m_led_glow=1.0;

    //update barrel length for recoil
    if(m_barrel_length<1.0)
    {
        m_barrel_length+=time_dif;
        if(m_barrel_length>1.0) m_barrel_length=1.0;
    }

    //if in docking progress
    if(m_mship_land_adjusting)
    {
        //cout<<m_pLand_adjust_joint->GetJointTranslation()<<endl;
        //test if done
        //if( m_pLand_adjust_joint->GetJointTranslation()>=m_pLand_adjust_joint->GetUpperLimit())
        if( m_pLand_adjust_joint->GetJointTranslation()<=m_pLand_adjust_joint->GetLowerLimit())
        {
            cout<<"Docking: lock ship pos\n";
            //destroy adjust joint
            m_pWorld->DestroyJoint(m_pLand_adjust_joint);
            //lock ship in place
            m_mship_land_adjusting=false;
            //create joint
            b2WeldJointDef weldJointDef;
            weldJointDef.bodyA = m_pBody;
            weldJointDef.bodyB = m_pMain_ship_body;
            weldJointDef.collideConnected = true;
            weldJointDef.localAnchorA.Set(0,0);
            //b2Vec2 rel_pos=m_pBody->GetPosition()-m_pMain_ship_body->GetPosition();
            b2Vec2 rel_pos=m_pMain_ship_body->GetLocalPoint(m_pBody->GetPosition());
            weldJointDef.localAnchorB=rel_pos;
            m_pMship_lock_joint = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
            m_mship_lock_on=true;
        }
        else
        {
            //timeout
            m_dock_adjust_timer-=time_dif;
            if(m_dock_adjust_timer<=0)//eject
            {
                cout<<"Docking: Timeout, player released\n";
                m_dock_adjust_timer=_player_mship_dock_time;
                disconnect_from_mship();
            }
        }
    }

    //if ship is emerges from mship
    if(m_is_spawning)
    {
        if( m_pLand_raise_joint->GetJointTranslation()>=m_pLand_raise_joint->GetUpperLimit())
        {
            //done, lock ship in pos
            cout<<"Spawning: Ship raised to surface\n";
            //destroy adjust joint
            m_pWorld->DestroyJoint(m_pLand_raise_joint);
            //lock ship in place
            m_is_spawning=false;
            //create joint
            b2WeldJointDef weldJointDef;
            weldJointDef.bodyA = m_pBody;
            weldJointDef.bodyB = m_pMain_ship_body;
            weldJointDef.collideConnected = true;
            weldJointDef.localAnchorA.Set(0,0);
            //b2Vec2 rel_pos=m_pBody->GetPosition()-m_pMain_ship_body->GetPosition();
            b2Vec2 rel_pos=m_pMain_ship_body->GetLocalPoint(m_pBody->GetPosition());
            weldJointDef.localAnchorB=rel_pos;
            m_pMship_lock_joint = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
            m_mship_lock_on=true;

            //turn on collision
            b2Filter filter;
            //filter.maskBits=0x0000;
            b2Fixture* fix_ship=m_pBody->GetFixtureList();
            filter=fix_ship->GetFilterData();
            //filter.maskBits=0x0000;
            filter.categoryBits=_COLCAT_all;
            filter.maskBits=-1;
            //filter.groupIndex=-1;
            fix_ship->SetFilterData(filter);
            //make rope hook free from collision, should be no rope parts

            b2Fixture* fix_hook=m_vec_rope_bodies.back()->GetFixtureList();
            if(fix_hook->IsSensor()) fix_hook=fix_hook->GetNext();//first fixture is the sensor
            if(fix_hook)
            {
                filter=fix_hook->GetFilterData();
                filter.categoryBits=_COLCAT_all;
                filter.maskBits=-1;
                //filter.groupIndex =-1;
                fix_hook->SetFilterData(filter);
            }
            else cout<<"ERROR: Could not find Hook for collision reset\n";
        }
        //else cout<<m_pLand_raise_joint->GetJointTranslation()<<endl;
    }

    //check for rope fail
    if(!m_rope_lock_on)//rope is out
    {
        float force_limit=_rope_force_limit;
        if(!m_line_type_tow) force_limit=_rope_force_limit_fuel;

        for(int joint_i=0;joint_i<(int)m_vec_rope_joints.size();joint_i++)
        {
            b2Vec2 force=m_vec_rope_joints[joint_i]->GetReactionForce(1.0/_world_step_time);
            //cout<<"FORCE: "<<force.x<<", "<<force.y<<endl;
            if( fabs(force.x)>force_limit || fabs(force.y)>force_limit )//break
            {
                //joint_i=m_vec_rope_joints.size()*0.5;//temp midpoint
                //joint_i=rand()%(int)m_vec_rope_joints.size();//temp random point

                cout<<"Hook broken\n";
                m_hook_off=true;

                //create explosion at rope part below
                b2Vec2 pos=m_vec_rope_bodies[joint_i]->GetPosition();
                m_pParticle_engine->add_explosion(pos,20,50.0,1.0);

                //destroy this joint
                m_pWorld->DestroyJoint(m_vec_rope_joints[joint_i]);

                //if body connected to hook, detatch
                if(m_hook_connected) hook_disconnect();

                //store disconnected rope parts in another vector
                m_vec_vec_disconnected_ropes.push_back( vector<b2Body*>() );
                for(int part_i=0;part_i<=joint_i;part_i++)
                {
                    m_vec_vec_disconnected_ropes.back().push_back(m_vec_rope_bodies[part_i]);

                    //mark disconected rope parts and hook as disconnected
                    st_body_user_data* data=(st_body_user_data*)m_vec_rope_bodies[part_i]->GetUserData();
                    data->b_disconnected_part=true;
                }

                //remove joints and bodies that does not belong to the ship anymore from vectors
                vector<b2Body*> vec_bodies;
                vector<b2RopeJoint*> vec_joints;
                for(int part_i=joint_i+1;part_i<(int)m_vec_rope_bodies.size();part_i++)
                {
                    //store rest of bodies/joints in a new vector
                    vec_bodies.push_back(m_vec_rope_bodies[part_i]);
                    if(part_i==(int)m_vec_rope_bodies.size()-1)//one more body than joint in vec due to top joint is prismatic
                    {
                        ;//leave prismatic joint be
                    }
                    else vec_joints.push_back(m_vec_rope_joints[part_i]);
                }
                //empty old vectors and add new elements
                m_vec_rope_bodies.clear();
                m_vec_rope_bodies=vec_bodies;
                m_vec_rope_joints.clear();
                m_vec_rope_joints=vec_joints;

                //play sound
                m_pSound->playSimpleSound(wav_hook_break,0.5);

                break;//no more rope test
            }
        }
    }

    return true;
}

bool player::draw(void)
{
    //set cloak transparancy
    glEnable(GL_BLEND);
    float blend_val=m_cloak_timer/m_cloak_delay;
    //cap value
    if(blend_val<0.1) blend_val=0.1;

    //draw disconnected rope/hooks
    glLineWidth(2);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.7,0.7,0.7,1.0);
    for(int rope_chain_i=0;rope_chain_i<(int)m_vec_vec_disconnected_ropes.size();rope_chain_i++)
    {
        glBegin(GL_LINE_STRIP);
        for(int rope_i=0;rope_i<(int)m_vec_vec_disconnected_ropes[rope_chain_i].size();rope_i++)
        {
            glVertex2f( m_vec_vec_disconnected_ropes[rope_chain_i][rope_i]->GetWorldCenter().x*_Met2Pix,
                        m_vec_vec_disconnected_ropes[rope_chain_i][rope_i]->GetWorldCenter().y*_Met2Pix );
        }
        glEnd();

        //draw disconnected hook
        glPushMatrix();
        b2Vec2 hook_pos=m_vec_vec_disconnected_ropes[rope_chain_i].front()->GetPosition();
        float hook_angle=m_vec_vec_disconnected_ropes[rope_chain_i].front()->GetAngle()*_Rad2Deg;
        glTranslatef( hook_pos.x*_Met2Pix, hook_pos.y*_Met2Pix, 0.0);
        glRotatef(hook_angle,0,0,1);
        glTranslatef(0.0,3.0,0.0);
        glColor4f(0.7,0.7,0.7,1.0);
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0,-3.0);
        glVertex2f(-3.0,0.0);
        glVertex2f(3.0,0.0);
        glVertex2f(-3.0,0.0);
        glVertex2f(-3.0,2.0);
        glVertex2f(3.0,0.0);
        glVertex2f(-3.0,2.0);
        glVertex2f(3.0,2.0);
        glVertex2f(3.0,0.0);
        glEnd();
        glPopMatrix();
    }


    //draw current rope
    glColor4f(0.7,0.7,0.7,blend_val);
    if(!m_line_type_tow)
    {
        //fuel line
        glColor4f(0.3,0.3,0.3,blend_val);
    }
    glBegin(GL_LINE_STRIP);
    for(int rope_i=0;rope_i<(int)m_vec_rope_bodies.size();rope_i++)
    {
        glVertex2f( m_vec_rope_bodies[rope_i]->GetWorldCenter().x*_Met2Pix,
                    m_vec_rope_bodies[rope_i]->GetWorldCenter().y*_Met2Pix );
    }
    glVertex2f( m_pBody->GetWorldCenter().x*_Met2Pix,m_pBody->GetWorldCenter().y*_Met2Pix );
    glEnd();
    glLineWidth(1);

    //draw hook
    if(!m_hook_off)
    {
        glPushMatrix();
        b2Vec2 hook_pos=m_pHook_body->GetPosition();
        float hook_angle=m_pHook_body->GetAngle()*_Rad2Deg;
        glTranslatef( hook_pos.x*_Met2Pix, hook_pos.y*_Met2Pix, 0.0);
        glRotatef(hook_angle,0,0,1);
        glTranslatef(0.0,3.0,0.0);
        glColor4f(0.7,0.7,0.7,blend_val);
        if(!m_line_type_tow)
        {
            //fuel line
            glColor4f(0.3,0.3,0.3,blend_val);
        }
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0,-3.0);
        glVertex2f(-3.0,0.0);
        glVertex2f(3.0,0.0);
        glVertex2f(-3.0,0.0);
        glVertex2f(-3.0,2.0);
        glVertex2f(3.0,0.0);
        glVertex2f(-3.0,2.0);
        glVertex2f(3.0,2.0);
        glVertex2f(3.0,0.0);
        glEnd();
        //draw led
        b2Vec2 led_pos(-0.5,3.0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,m_texture);
        glBlendFunc(GL_ONE,GL_ONE);
        if(!m_hook_connected) glColor4f(0.3*blend_val,0.3*blend_val,0.3*blend_val,blend_val);
        else glColor4f(0.7*blend_val,0.7*blend_val,0.7*blend_val,blend_val);
        if(m_force_led_on)
        {
            glColor4f(0.7*blend_val,0.7*blend_val,0.7*blend_val,blend_val);
            m_force_led_on=false;
        }
        glBegin(GL_QUADS);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(led_pos.x-6.0,led_pos.y-6.0);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(led_pos.x-6.0,led_pos.y+6.0);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(led_pos.x+6.0,led_pos.y+6.0);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(led_pos.x+6.0,led_pos.y-6.0);
        glEnd();
        glPopMatrix();
    }
    else//enable texture
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,m_texture);
    }

    //draw weapon beam
    if(m_draw_beam)
    {
        glDisable(GL_TEXTURE_2D);
        //glPushMatrix();
        glColor3f(0.2,0.8,0.2);
        glBegin(GL_LINES);
        glVertex2f(m_vBeam_start.x*_Met2Pix,m_vBeam_start.y*_Met2Pix);
        glVertex2f(m_vBeam_end.x*_Met2Pix,m_vBeam_end.y*_Met2Pix);
        glEnd();
        glEnable(GL_TEXTURE_2D);

        //glPopMatrix();

        m_draw_beam=false;

    }

    //draw ship
    glPushMatrix();
    b2Vec2 pos=m_pBody->GetPosition();
    float angle=m_pBody->GetAngle()*_Rad2Deg;
    glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
    glRotatef(angle,0,0,1);
    //glEnable(GL_TEXTURE_2D);
    //glBindTexture(GL_TEXTURE_2D,m_texture);
    glColor3f(1,1,1);
    //mask
    glBlendFunc(GL_DST_COLOR,GL_ZERO);
    glBegin(GL_QUADS);
    glTexCoord2f(937.0/1024.0,(1024.0-1.0-29.0)/1024.0);
    glVertex2f(-15.0,-14.0);
    glTexCoord2f(937.0/1024.0,(1024.0-28.0-29.0)/1024.0);
    glVertex2f(-15.0, 14.0);
    glTexCoord2f(967.0/1024.0,(1024.0-28.0-29.0)/1024.0);
    glVertex2f( 15.0, 14.0);
    glTexCoord2f(967.0/1024.0,(1024.0-1.0-29.0)/1024.0);
    glVertex2f( 15.0,-14.0);
    glEnd();
    //color
    glColor3f(blend_val,blend_val,blend_val);
    glBlendFunc(GL_ONE,GL_ONE);
    glBegin(GL_QUADS);
    glTexCoord2f(937.0/1024.0,(1024.0-1.0)/1024.0);
    glVertex2f(-15.0,-14.0);
    glTexCoord2f(937.0/1024.0,(1024.0-28.0)/1024.0);
    glVertex2f(-15.0, 14.0);
    glTexCoord2f(967.0/1024.0,(1024.0-28.0)/1024.0);
    glVertex2f( 15.0, 14.0);
    glTexCoord2f(967.0/1024.0,(1024.0-1.0)/1024.0);
    glVertex2f( 15.0,-14.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    //draw led glow
    if(m_led_glow>0.0)
    {
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_ONE,GL_ONE);
        glColor4f(0.0,m_led_glow,0.0,0.1*blend_val);
        float led_size=6.0;
        glBegin(GL_QUADS);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(m_vBeam_start.x*_Met2Pix-led_size,m_vBeam_start.y*_Met2Pix-led_size);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(m_vBeam_start.x*_Met2Pix-led_size,m_vBeam_start.y*_Met2Pix+led_size);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(m_vBeam_start.x*_Met2Pix+led_size,m_vBeam_start.y*_Met2Pix+led_size);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(m_vBeam_start.x*_Met2Pix+led_size,m_vBeam_start.y*_Met2Pix-led_size);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        //restore ship pos
        glPushMatrix();
        glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
        glRotatef(angle,0,0,1);
    }

    //draw player color led
    //if(true)
    {
        //glPopMatrix();
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_ONE,GL_ONE);
        switch(m_id)
        {
            case 0: glColor4f(0.9,0.0,0.0,0.7*blend_val); break;
            case 1: glColor4f(0.0,0.9,0.0,0.7*blend_val); break;
            case 2: glColor4f(0.0,0.0,0.9,0.7*blend_val); break;
            case 3: glColor4f(0.9,0.9,0.0,0.7*blend_val); break;
        }
        float led_size=6.0;
        glBegin(GL_QUADS);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(0.0-led_size,5.0-led_size);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(0.0-led_size,5.0+led_size);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(0.0+led_size,5.0+led_size);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(0.0+led_size,5.0-led_size);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        //restore ship pos
        //glPushMatrix();
        //glTranslatef( pos.x*_Met2Pix, pos.y*_Met2Pix, 0.0);
        //glRotatef(angle,0,0,1);
    }

    //draw shield
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(m_pGear_curr->get_type()==gt_shield && !m_shield_broken)
    {
        glLineWidth(3);
        float shield_blend=m_shield_hp_curr/m_shield_hp_max;
        glColor4f(0.6,0.6,0.9,shield_blend);
        glBegin(GL_LINE_STRIP);
        glVertex2f(0.000*_Met2Pix,-0.9*_Met2Pix);
        glVertex2f(0.70*_Met2Pix,0.450*_Met2Pix);
        glVertex2f(-0.70*_Met2Pix,0.450*_Met2Pix);
        glVertex2f(0.000*_Met2Pix,-0.9*_Met2Pix);
        glEnd();
        glLineWidth(1);
    }

    //draw turret
    glColor4f(0.5,0.5,0.5,blend_val);
    float turret_size=0.13*_Met2Pix;
    float barrel_length=m_barrel_length*3.5;
    glRotatef(m_turret_rotation,0,0,1);//extra turret rotation
    glBegin(GL_QUADS);
    //body
    glVertex2f(-1.0*turret_size,-1.0*turret_size);
    glVertex2f(-1.0*turret_size, 1.0*turret_size);
    glVertex2f( 1.0*turret_size, 1.0*turret_size);
    glVertex2f( 1.0*turret_size,-1.0*turret_size);
    glEnd();
    //barrel
    glColor4f(0.4,0.4,0.4,blend_val);
    glBegin(GL_QUADS);
    glVertex2f(-0.6*turret_size,-barrel_length*turret_size);
    glVertex2f(-0.6*turret_size,-1.0*turret_size);
    glVertex2f( 0.6*turret_size,-1.0*turret_size);
    glVertex2f( 0.6*turret_size,-barrel_length*turret_size);
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);

    //draw drone
    if(m_drone_mode==dm_on)
    {
        glPushMatrix();

        b2Vec2 drone_pos=m_pBody_drone->GetPosition();
        float drone_angle=m_pBody_drone->GetAngle()*_Rad2Deg;
        glTranslatef( drone_pos.x*_Met2Pix, drone_pos.y*_Met2Pix, 0.0);
        glRotatef(drone_angle,0,0,1);
        float drone_size=0.05*_Met2Pix;
        glColor3f(1.0,0.5,0);
        glBegin(GL_QUADS);
        glVertex2f(-drone_size,-drone_size);
        glVertex2f(-drone_size,drone_size);
        glVertex2f(drone_size,drone_size);
        glVertex2f(drone_size,-drone_size);
        glEnd();

        //led flash
        b2Vec2 led_pos(0.0,0.0);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D,m_texture);
        glBlendFunc(GL_ONE,GL_ONE);
        float brightness=0.0;
        if(m_angle_time>300.0)
        {
            brightness=(sinf((m_angle_time-260.0)/100.0*360.0*_Deg2Rad)+1.0)*0.5;
            //brightness*=brightness;
        }
        glColor3f(brightness,brightness,brightness);
        glBegin(GL_QUADS);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(led_pos.x-6.0,led_pos.y-6.0);
        glTexCoord2f( (269.5-8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(led_pos.x-6.0,led_pos.y+6.0);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0-8.0)/1024.0 );
        glVertex2f(led_pos.x+6.0,led_pos.y+6.0);
        glTexCoord2f( (269.5+8.0)/1024.0, (1024.0-156.0+8.0)/1024.0 );
        glVertex2f(led_pos.x+6.0,led_pos.y-6.0);
        glEnd();
        glDisable(GL_BLEND);
        glDisable(GL_TEXTURE_2D);

        glPopMatrix();
    }

    //cout<<"draw ship done\n";

    return true;
}

bool player::player_hook_connected(void)
{
    return m_hook_connected;
}

bool player::hook_connect(b2Body* pBody_to_connect)
{
    //get body info
    st_body_user_data* data=(st_body_user_data*)pBody_to_connect->GetUserData();
    //cout<<"Body type: "<<data->s_info<<endl;

    if(m_hook_off) return false;//hook not connected to ship
    if(m_hook_connected) return false;//already connected
    if(m_rope_lock_on) return false;//hook disabled

    //test that the body is not part of the ships rope
    for(int rope_i=0;rope_i<(int)m_vec_rope_bodies.size();rope_i++)
     if(m_vec_rope_bodies[rope_i]==pBody_to_connect) return false;
    //or the ship itself
    if(m_pBody==pBody_to_connect) return false;

    //test if fuel line and body is not a player
    if( !m_line_type_tow && data->s_info!="player") return false;
    //check connection to player
    if( !m_line_type_tow )
    {
        m_fuel_line_to_player_ind=data->i_id;
    }

    m_hook_connected=true;
    m_pBody_connected_to=pBody_to_connect;
    b2Vec2 pos=pBody_to_connect->GetPosition();
    //cout<<"POS: "<<pos.x<<", "<<pos.y<<endl;

    //create weld joint
    /*b2RopeJointDef ropeJointDef;
    ropeJointDef.bodyA = m_pHook_sensor->GetBody();
    //ropeJointDef.bodyA = m_pBody;
    ropeJointDef.bodyB = pBody_to_connect;
    //ropeJointDef.bodyB = m_pBody;
    ropeJointDef.collideConnected = false;
    //get relative pos of sensor from other body
    b2Vec2 rel_pos=m_pHook_sensor->GetBody()->GetPosition()-pBody_to_connect->GetPosition();
    ropeJointDef.localAnchorA.Set(0,0);
    //ropeJointDef.localAnchorB.Set(0,0);
    ropeJointDef.localAnchorB=rel_pos;
    m_pHook_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );*/

    b2WeldJointDef weldJointDef;
    weldJointDef.bodyA = m_pHook_sensor->GetBody();
    weldJointDef.bodyB = pBody_to_connect;
    //weldJointDef.bodyB = m_pHook_sensor->GetBody();
    //weldJointDef.bodyA = pBody_to_connect;
    weldJointDef.collideConnected = false;
    //get relative pos of sensor from other body

    //FIX, relative coordinates do not take angle into account
    //b2Vec2 rel_pos=m_pHook_sensor->GetBody()->GetPosition()-pBody_to_connect->GetPosition();//ok if angle = 0
    b2Vec2 rel_pos=pBody_to_connect->GetLocalPoint( m_pHook_sensor->GetBody()->GetWorldPoint( b2Vec2(0.0,0.0) ) );//ok if angle = 0
    //b2Vec2 rel_pos=pBody_to_connect->GetLocalPoint( pBody_to_connect->GetWorldPoint( b2Vec2(0.0,-2.0) ) );//ok if angle = 0
    //cout<<"rel_pos: "<<rel_pos.x<<", "<<rel_pos.y<<endl;
    float angle_body=pBody_to_connect->GetAngle();
    //float angle_sensor=m_pHook_sensor->GetBody()->GetAngle();
    //b2Vec2 rotated_pos;
    //rotated_pos.x=cosf(-angle_body)*rel_pos.x-sinf(-angle_body)*rel_pos.y;
    //rotated_pos.y=sinf(-angle_body)*rel_pos.x+cosf(-angle_body)*rel_pos.y;
    //cout<<"rot_pos: "<<rotated_pos.x<<", "<<rotated_pos.y<<"\t"<<angle_body<<endl;


    weldJointDef.localAnchorA.Set(0,0);
    //weldJointDef.localAnchorB.Set(0,0);
    weldJointDef.localAnchorB=rel_pos;
    weldJointDef.referenceAngle=angle_body;
    //cout<<"Rel point: "<<rel_pos.x<<", "<<rel_pos.y<<endl;
    m_pHook_joint = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );

    return true;
}

bool player::hook_disconnect(void)
{
    if(!m_hook_connected) return false;

    m_hook_connected=false;

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
    m_pWorld->DestroyJoint(m_pHook_joint);

    if(!multiple_carriers)
    {
        //notify thet body that it is no longer carried
        st_body_user_data* data=(st_body_user_data*)m_pBody_connected_to->GetUserData();
        data->b_is_carried=false;
    }

    return true;
}

bool player::is_hook_off(void)
{
    return m_hook_off;
}

bool player::connected_to_mship(void)
{
    //cout<<" ship_lock: "<<m_mship_lock_on<<", spawning: "<<m_is_spawning<<endl;

    return (m_mship_lock_on || m_is_spawning);
}

bool player::is_spawning(void)
{
    return m_is_spawning;
}

bool player::disconnect_from_mship(void)
{
    //destroy joint
    if(m_mship_lock_on)
    {
        m_pWorld->DestroyJoint(m_pMship_lock_joint);
        m_mship_lock_on=false;
    }
    //if adjusting
    if(m_mship_land_adjusting)
    {
        m_pWorld->DestroyJoint(m_pLand_adjust_joint);
        m_mship_land_adjusting=false;
    }
    //if spawning (allowed only in menu)
    if(m_is_spawning)
    {
        m_pWorld->DestroyJoint(m_pLand_raise_joint);
        m_is_spawning=false;

        //turn on collision
        b2Filter filter;
        //filter.maskBits=0x0000;
        b2Fixture* fix_ship=m_pBody->GetFixtureList();
        filter=fix_ship->GetFilterData();
        //filter.maskBits=0x0000;
        filter.categoryBits=_COLCAT_all;
        filter.maskBits=-1;
        //filter.groupIndex=-1;
        fix_ship->SetFilterData(filter);
        //make rope hook free from collision, should be no rope parts

        b2Fixture* fix_hook=m_vec_rope_bodies.back()->GetFixtureList();
        if(fix_hook->IsSensor()) fix_hook=fix_hook->GetNext();//first fixture is the sensor
        if(fix_hook)
        {
            filter=fix_hook->GetFilterData();
            filter.categoryBits=_COLCAT_all;
            filter.maskBits=-1;
            //filter.groupIndex =-1;
            fix_hook->SetFilterData(filter);
        }
        else cout<<"ERROR: Could not find Hook for collision reset\n";
    }

    return true;
}

bool player::change_ship_mass(float mass_change,bool reset_mass)
{
    //update mass factor
    m_ship_mass_factor+=mass_change;

    //reset mass to default
    if(reset_mass) m_ship_mass_factor=1.0;

    cout<<"Ship mass updated: "<<m_ship_mass_factor<<endl;

    //get ship pos, angle and data
    b2Vec2 pos=m_pBody->GetPosition();
    float angle=m_pBody->GetAngle();
    st_body_user_data* user_data_old=(st_body_user_data*)m_pBody->GetUserData();

    //erase rope/hook data(OLD)
    /*for(int body_i=0;body_i<(int)m_vec_rope_bodies.size();body_i++)
    {
        st_body_user_data* rope_user_data=(st_body_user_data*)m_vec_rope_bodies[body_i]->GetUserData();
        delete rope_user_data;
    }*/

    //destroy old joints
    m_pWorld->DestroyJoint(m_rope_motor_joint);
    if(m_mship_lock_on) m_pWorld->DestroyJoint(m_pMship_lock_joint);
    if(m_mship_land_adjusting) m_pWorld->DestroyJoint(m_pLand_adjust_joint);
    if(m_hook_connected) m_pWorld->DestroyJoint(m_pHook_joint);//never on if docked
    if(m_rope_lock_on) m_pWorld->DestroyJoint(m_pRope_lock_joint);//always on if docked
    for(int joint_i=0;joint_i<(int)m_vec_rope_joints.size();joint_i++)
     m_pWorld->DestroyJoint(m_vec_rope_joints[joint_i]);
    m_vec_rope_joints.clear();

    //destroy old bodies mark
    //m_pWorld->DestroyBody(m_pBody);(OLD)
    user_data_old->b_to_be_deleted=true;
    for(int body_i=0;body_i<(int)m_vec_rope_bodies.size();body_i++)
    {
        //m_pWorld->DestroyBody(m_vec_rope_bodies[body_i]);

        st_body_user_data* rope_user_data=(st_body_user_data*)m_vec_rope_bodies[body_i]->GetUserData();
        rope_user_data->b_to_be_deleted=true;
    }
    m_vec_rope_bodies.clear();


    //recreate ship with new mass
    b2BodyDef bodydef;
    bodydef.position.Set(pos.x,pos.y);
    bodydef.type=b2_dynamicBody;
    bodydef.linearDamping=_player_damping_lin;
    bodydef.angularDamping=_player_damping_ang;
    m_pBody=m_pWorld->CreateBody(&bodydef);
    //create ship fixture
    b2PolygonShape shape2;
    b2Vec2 p1(0.000,-0.585); b2Vec2 p2(0.500,0.305); b2Vec2 p3(-0.500,0.305);
    b2Vec2 arr[]={p1,p2,p3};
    shape2.Set(arr,3);
    b2FixtureDef fixturedef;
    fixturedef.shape=&shape2;
    fixturedef.density=_player_density*m_ship_mass_factor;
    fixturedef.filter.categoryBits=_COLCAT_all;
    fixturedef.filter.maskBits=-1;
    m_pBody->CreateFixture(&fixturedef);
    //set data (reuse old data, no)
    st_body_user_data* user_data=new st_body_user_data;
    user_data->s_info="player";
    user_data->i_id=m_id;
    m_pBody->SetUserData(user_data);
    //move into pos
    m_pBody->SetTransform( pos, angle );

    //add rope hook
    b2BodyDef bodydef_rope;
    bodydef_rope.type = b2_dynamicBody;
    bodydef_rope.linearDamping=_object_damping_lin;
    bodydef_rope.angularDamping=_object_damping_ang;
    b2FixtureDef fixturedef_rope;
    fixturedef_rope.density=_rope_density;
    b2PolygonShape boxShape;
    boxShape.SetAsBox(_rope_part_width+0.1,_rope_part_length);
    bodydef_rope.position.Set(pos.x, pos.y);
    fixturedef_rope.shape = &boxShape;
    fixturedef_rope.filter.categoryBits=_COLCAT_all;
    fixturedef_rope.filter.maskBits=-1;
    m_pHook_body = m_pWorld->CreateBody( &bodydef_rope );
    m_pHook_body->CreateFixture( &fixturedef_rope );
    //set data
    st_body_user_data* user_data1=new st_body_user_data;
    user_data1->s_info="hook";
    user_data1->i_id=m_id;
    m_pHook_body->SetUserData(user_data1);

    //add sensor to hook
    b2PolygonShape shape_sensor;
    b2Vec2 sensor_arr[]={ b2Vec2(-0.2,0.4),
                          b2Vec2(-0.2,0.2),
                          b2Vec2(0.2,0.2),
                          b2Vec2(0.2,0.4) };
    shape_sensor.Set(sensor_arr,4);
    b2FixtureDef fixturedef_sensor;
    fixturedef_sensor.shape=&shape_sensor;
    fixturedef_sensor.isSensor=true;
    fixturedef_sensor.filter.categoryBits=_COLCAT_all;
    fixturedef_sensor.filter.maskBits=-1;
    m_pHook_sensor=m_pHook_body->CreateFixture(&fixturedef_sensor);
    //cout<<"hook sens init: "<<&m_pHook_sensor<<", "<<m_id<<endl;

    //remember top rope body
    m_vec_rope_bodies.push_back(m_pHook_body);

    //prismatic joint, link rope with ship
    b2PrismaticJointDef prismJointDef;
    prismJointDef.bodyA = m_pBody;
    prismJointDef.bodyB = m_pHook_body;
    prismJointDef.localAxisA.Set(0.0,1.0);
    prismJointDef.collideConnected = false;
    prismJointDef.localAnchorA.Set(0,0);
    prismJointDef.localAnchorB.Set(0,0);
    prismJointDef.enableLimit=true;
    prismJointDef.lowerTranslation=_rope_lower_trans_limit;
    prismJointDef.upperTranslation=_rope_upper_trans_limit;
    prismJointDef.enableMotor=false;
    prismJointDef.motorSpeed=0.0;
    prismJointDef.maxMotorForce=10;
    m_rope_motor_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );

    //weld top rope top ship
    b2RopeJointDef ropeJointDef;
    ropeJointDef.bodyA = m_pBody;
    ropeJointDef.bodyB = m_vec_rope_bodies.back();
    ropeJointDef.collideConnected = false;
    ropeJointDef.localAnchorA.Set(0,0);
    ropeJointDef.localAnchorB.Set(0,0);
    m_pRope_lock_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );
    m_rope_lock_on=true;

    //weld ship to main ship
    b2WeldJointDef weldJointDef;
    weldJointDef.bodyA = m_pBody;
    weldJointDef.bodyB = m_pMain_ship_body;
    weldJointDef.collideConnected = true;
    weldJointDef.localAnchorA.Set(0,0);
    //b2Vec2 rel_pos=m_pBody->GetPosition()-m_pMain_ship_body->GetPosition();
    b2Vec2 rel_pos=m_pMain_ship_body->GetLocalPoint(m_pBody->GetPosition());
    weldJointDef.localAnchorB=rel_pos;
    m_pMship_lock_joint = (b2WeldJoint*)m_pWorld->CreateJoint( &weldJointDef );
    m_mship_lock_on=true;

    return true;
}

bool player::reset_motion(void)
{
    m_pBody->SetAngularVelocity(0.0);
    m_pBody->SetLinearVelocity(b2Vec2(0.0,0.0));

    return true;
}

float player::get_rel_hp(void)
{
    return m_hp_curr/m_hp_max;
}

float player::get_rel_fuel(void)
{
    return m_fuel_curr/m_fuel_max;
}

float player::get_rel_ammo(void)
{
    return m_ammo_curr/m_ammo_max;
}

bool player::change_hp(float value_dif)
{
    if(value_dif<0.0)//damage taken
    {
        //reset shield regen delay
        m_shield_regen_timer=m_shield_regen_delay;

        //shield absorb damage
        if(m_pGear_curr->get_type()==gt_shield && m_shield_hp_curr>0.0)
        {
            m_shield_hp_curr+=value_dif;
            if(m_shield_hp_curr<0.0)
            {
                cout<<"Shield broken\n";
                //shield broken
                m_shield_broken=true;
                //rest of damage also taken
                m_hp_curr+=m_shield_hp_curr;
                m_shield_hp_curr=0.0;
            }
            else cout<<"Shield HP: "<<m_shield_hp_curr<<endl;
        }
        else m_hp_curr+=value_dif;//no shield, normal damage
    }
    else m_hp_curr+=value_dif;//heal

    if(m_hp_curr<0.0)
    {
        m_hp_curr=0.0;

        //player dead
        st_body_user_data* data=(st_body_user_data*)m_pBody->GetUserData();
        if(data->b_alive)
        {
            data->b_alive=false;

            //create explosion
            m_pParticle_engine->add_explosion( m_pBody->GetPosition(),100,100,1.5,float(rand()%360) );

            return false;
        }
        //else already dead
    }
    else if(m_hp_curr>m_hp_max)
    {
        m_hp_curr=m_hp_max;
    }
    return true;
}

bool player::change_fuel(float value_dif)
{
    m_fuel_curr+=value_dif;
    if(m_fuel_curr<0.0)
    {
        m_fuel_curr=0.0;
        return false;
    }
    else if(m_fuel_curr>m_fuel_max)
    {
        m_fuel_curr=m_fuel_max;
    }
    return true;
}

bool player::change_ammo(float value_dif)
{
    m_ammo_curr+=value_dif;
    if(m_ammo_curr<0.0)
    {
        m_ammo_curr=0.0;
        return false;
    }
    else if(m_ammo_curr>m_ammo_max)
    {
        m_ammo_curr=m_ammo_max;
    }
    return true;
}

int player::dock_player_to_mship(void)
{
    if(!m_rope_lock_on) return 0;//rope lock have to be on

    m_mship_land_adjusting=true;

    //find closest landing slot
    b2Vec2 pos_landstrip[4];//the pos of the 4 landing alternatives
    pos_landstrip[0]=m_pMain_ship_body->GetWorldPoint( b2Vec2(0.00,-2.5) );
    pos_landstrip[1]=m_pMain_ship_body->GetWorldPoint( b2Vec2(1.25,-2.5) );
    pos_landstrip[2]=m_pMain_ship_body->GetWorldPoint( b2Vec2(2.55,-2.5) );
    pos_landstrip[3]=m_pMain_ship_body->GetWorldPoint( b2Vec2(4.00,-2.5) );
    b2Vec2 pos_player=m_pBody->GetPosition();
    int near_land_i=0;
    float land_dist_nearest=sqrt( (pos_player.x-pos_landstrip[0].x)*(pos_player.x-pos_landstrip[0].x)+
                                  (pos_player.y-pos_landstrip[0].y)*(pos_player.y-pos_landstrip[0].y) );
    for(int land_i=1;land_i<4;land_i++)
    {
        float dist=sqrt( (pos_player.x-pos_landstrip[land_i].x)*(pos_player.x-pos_landstrip[land_i].x)+
                         (pos_player.y-pos_landstrip[land_i].y)*(pos_player.y-pos_landstrip[land_i].y) );
        if(dist<land_dist_nearest)
        {
            //cout<<"Docking: dist to "<<land_i<<": "<<dist<<endl;
            near_land_i=land_i;
            land_dist_nearest=dist;
        }
    }
    cout<<"Docking: Nearest land pos: "<<near_land_i+1<<endl;
    b2Vec2 landing_pos=pos_landstrip[near_land_i];

    //create land adjusting joint
    float mship_rot=m_pMain_ship_body->GetAngle();//+_pi*0.5;
    //cout<<"mship angle: "<<mship_rot<<endl;
    b2PrismaticJointDef prismJointDef;
    //b2WeldJointDef prismJointDef;
    prismJointDef.bodyA = m_pMain_ship_body;
    prismJointDef.bodyB = m_pBody;
    //prismJointDef.localAxisA.Set(0.0,1.0);
    //prismJointDef.localAxisA.Set( cosf(mship_rot),-sinf(mship_rot) );
    //b2Vec2 rel_point=m_pMain_ship_body->GetLocalPoint( m_pBody->GetPosition() );//to center of mship
    b2Vec2 rel_point;//=m_pMain_ship_body->GetWorldPoint( m_pBody->GetPosition() );//to landpos
    switch(near_land_i)
    {
        case 0: rel_point=m_pBody->GetPosition()-m_pMain_ship_body->GetWorldPoint(b2Vec2(0.00,-2.50)); break;
        case 1: rel_point=m_pBody->GetPosition()-m_pMain_ship_body->GetWorldPoint(b2Vec2(1.25,-2.50)); break;
        case 2: rel_point=m_pBody->GetPosition()-m_pMain_ship_body->GetWorldPoint(b2Vec2(2.55,-2.50)); break;
        case 3: rel_point=m_pBody->GetPosition()-m_pMain_ship_body->GetWorldPoint(b2Vec2(4.00,-2.50)); break;
    }
    //axis is now related to the world x/y axis, needs to be shifted to match mship axis
    b2Vec2 adjusted_axis=b2Vec2( rel_point.x*cosf(-mship_rot)-rel_point.y*sinf(-mship_rot),
                                 rel_point.x*sinf(-mship_rot)+rel_point.y*cosf(-mship_rot) );
    prismJointDef.localAxisA=adjusted_axis;
    //cout<<"axis: "<<rel_point.x<<", "<<rel_point.y<<endl;
    prismJointDef.collideConnected = false;
    //b2Vec2 shifted_pos=m_pMain_ship_body->GetLocalPoint( m_pBody->GetPosition() );
    prismJointDef.localAnchorA=m_pMain_ship_body->GetLocalPoint( m_pBody->GetPosition() );
    prismJointDef.localAnchorB.Set(0,0);
    prismJointDef.enableLimit=true;
    prismJointDef.lowerTranslation=-land_dist_nearest;
    prismJointDef.upperTranslation=0.0;
    prismJointDef.enableMotor=true;
    prismJointDef.motorSpeed=-100.0;
    prismJointDef.maxMotorForce=100;
    m_pLand_adjust_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );

    //float angle_mship=m_pMain_ship_body->GetAngle();

    //mount new hook if old is broken
    if(m_hook_off)
    {
        //disarm old hook sensor?

        m_hook_off=false;

        //remove final rope part on ship
        m_pWorld->DestroyJoint(m_rope_motor_joint);
        st_body_user_data* user_data=(st_body_user_data*)m_vec_rope_bodies.front()->GetUserData();
        //delete user_data;(OLD)
        //m_pWorld->DestroyBody(m_vec_rope_bodies.front());
        user_data->b_to_be_deleted=true;//marked for deletion
        m_vec_rope_bodies.clear();
        //create new hook body
        b2Vec2 pos=m_pBody->GetPosition();
        b2BodyDef bodydef_rope;
        bodydef_rope.type = b2_dynamicBody;
        bodydef_rope.linearDamping=_object_damping_lin;
        bodydef_rope.angularDamping=_object_damping_ang;
        b2FixtureDef fixturedef_rope;
        fixturedef_rope.density=_rope_density;
        b2PolygonShape boxShape;
        boxShape.SetAsBox(_rope_part_width+0.1,_rope_part_length);
        bodydef_rope.position.Set(pos.x, pos.y);
        fixturedef_rope.shape = &boxShape;
        m_pHook_body = m_pWorld->CreateBody( &bodydef_rope );
        m_pHook_body->CreateFixture( &fixturedef_rope );
        //set data
        st_body_user_data* user_data1=new st_body_user_data;
        user_data1->s_info="hook";
        user_data1->i_id=m_id;
        m_pHook_body->SetUserData(user_data1);

        //add sensor to hook
        b2PolygonShape shape_sensor;
        b2Vec2 sensor_arr[]={ b2Vec2(-0.2,0.4),
                              b2Vec2(-0.2,0.2),
                              b2Vec2(0.2,0.2),
                              b2Vec2(0.2,0.4) };
        shape_sensor.Set(sensor_arr,4);
        b2FixtureDef fixturedef_sensor;
        fixturedef_sensor.shape=&shape_sensor;
        fixturedef_sensor.isSensor=true;
        m_pHook_sensor=m_pHook_body->CreateFixture(&fixturedef_sensor);
        //cout<<"hook sens new: "<<&m_pHook_sensor<<", "<<m_id<<endl;

        //remember top rope body
        m_vec_rope_bodies.push_back(m_pHook_body);

        //prismatic joint, link rope with ship
        b2PrismaticJointDef prismJointDef;
        prismJointDef.bodyA = m_pBody;
        prismJointDef.bodyB = m_pHook_body;
        prismJointDef.localAxisA.Set(0.0,1.0);
        prismJointDef.collideConnected = false;
        prismJointDef.localAnchorA.Set(0,0);
        prismJointDef.localAnchorB.Set(0,0);
        prismJointDef.enableLimit=true;
        prismJointDef.lowerTranslation=_rope_lower_trans_limit;
        prismJointDef.upperTranslation=_rope_upper_trans_limit;
        prismJointDef.enableMotor=false;
        prismJointDef.motorSpeed=0.0;
        prismJointDef.maxMotorForce=10;
        m_rope_motor_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );

        //weld top rope top ship
        b2RopeJointDef ropeJointDef;
        ropeJointDef.bodyA = m_pBody;
        ropeJointDef.bodyB = m_vec_rope_bodies.back();
        ropeJointDef.collideConnected = false;
        ropeJointDef.localAnchorA.Set(0,0);
        ropeJointDef.localAnchorB.Set(0,0);
        m_pRope_lock_joint = (b2RopeJoint*)m_pWorld->CreateJoint( &ropeJointDef );
        m_rope_lock_on=true;

        return 2;//update sensor pointer in game
    }

    return 1;
}

int player::get_mship_dock_status(void)
{
    if(m_mship_land_adjusting) return 1;//dock process
    else if(m_mship_lock_on) return 2;//docked
    else return 0;//not docked
}

bool player::change_turret_rotation(float change)
{
    m_turret_rotation+=change;
    //m_turret_joint->SetMotorSpeed(-change*1.0);

    return true;
}

float player::get_turret_angle(void)
{
    return m_turret_rotation;
}

bool player::set_turret_angle(float angle)
{
    m_turret_rotation=angle;

    return true;
}

bool player::destroy_body_and_joints(void)
{
    //destroy joints
    for(int i=0;i<(int)m_vec_rope_joints.size();i++)
    {
        m_pWorld->DestroyJoint(m_vec_rope_joints[i]);
    }
    m_vec_rope_joints.clear();
    if(m_hook_connected)
     m_pWorld->DestroyJoint(m_pHook_joint);//should never be
    if(m_mship_lock_on)
     m_pWorld->DestroyJoint(m_pMship_lock_joint);//should never be
    if(m_rope_lock_on)
     m_pWorld->DestroyJoint(m_pRope_lock_joint);
    if(m_mship_land_adjusting)
     m_pWorld->DestroyJoint(m_pLand_adjust_joint);//should never be

    //destroy bodies (mark)
    //m_pWorld->DestroyBody(m_pBody);
    st_body_user_data* player_data=(st_body_user_data*)m_pBody->GetUserData();
    player_data->b_to_be_deleted=true;

    //mark rope bodies
    for(int i=0;i<(int)m_vec_rope_bodies.size();i++)
    {
        //mark for removal
        st_body_user_data* data=(st_body_user_data*)m_vec_rope_bodies[i]->GetUserData();
        data->b_to_be_deleted=true;

        //m_pWorld->DestroyBody(m_vec_rope_bodies[i]);
    }
    m_vec_rope_bodies.clear();

    //disarm weapon and gear
    if(m_pWeapon_curr!=m_pWeapon_default)
    {
        delete m_pWeapon_curr;
        delete m_pWeapon_default;
        m_pWeapon_curr=m_pWeapon_default;
    }
    else delete m_pWeapon_curr;
    if(m_pGear_curr!=m_pGear_default)
    {
        delete m_pGear_curr;
        delete m_pGear_default;
        m_pGear_curr=m_pGear_default;
    }
    else delete m_pGear_curr;
    m_using_default_weapon=true;
    m_using_default_gear=true;

    return true;
}

/*bool player::move_and_reset_player(void)
{
    //cut rope

    //disarm weapon and gear
    if(m_pWeapon_curr!=m_pWeapon_default)
    {
        delete m_pWeapon_curr;
        m_pWeapon_curr=m_pWeapon_default;
        m_using_default_weapon=true;
    }
    if(m_pGear_curr!=m_pGear_default && m_pGear_curr->get_type()!=gt_cam_control)
    {
        delete m_pGear_curr;
        m_pGear_curr=m_pGear_default;
        m_using_default_gear=true;
    }

    return true;
}*/

bool player::spawn_from_mship(void)
{
    if(m_is_spawning) return false;//cannot spawn while spawning

    //full hp
    m_hp_curr=m_hp_max;

    m_is_spawning=true;

    /*//set non collision filter
    b2Filter filter;
    //filter.maskBits=0x0000;
    //make ship free from collision
    b2Fixture* fix_ship=m_pBody->GetFixtureList();
    filter=fix_ship->GetFilterData();
    //filter.maskBits=0x0000;
    filter.groupIndex =-1;
    fix_ship->SetFilterData(filter);
    //make rope hook free from collision, should be no rope parts
    b2Fixture* fix_hook=m_vec_rope_bodies.back()->GetFixtureList();
    filter=fix_hook->GetFilterData();
    //filter.maskBits=0x0000;
    filter.groupIndex =-1;
    fix_hook->SetFilterData(filter);*/

    //create joint for raising the ship
    b2PrismaticJointDef prismJointDef;
    prismJointDef.bodyA = m_pMain_ship_body;
    prismJointDef.bodyB = m_pBody;
    prismJointDef.localAxisA.Set(0.0,-1.0);
    prismJointDef.collideConnected = false;
    //prismJointDef.localAnchorA.Set(0,0);
    switch(m_id)
    {
        case 0: prismJointDef.localAnchorA.Set(0.00,-1.0); break;
        case 1: prismJointDef.localAnchorA.Set(1.25,-1.0); break;
        case 2: prismJointDef.localAnchorA.Set(2.55,-1.0); break;
        case 3: prismJointDef.localAnchorA.Set(4.00,-1.0); break;
    }
    prismJointDef.localAnchorB.Set(0,0);
    prismJointDef.enableLimit=true;
    prismJointDef.lowerTranslation=-0.2;
    prismJointDef.upperTranslation=1.5;
    prismJointDef.enableMotor=true;
    prismJointDef.motorSpeed=0.0;
    prismJointDef.maxMotorForce=100;
    m_pLand_raise_joint = (b2PrismaticJoint*)m_pWorld->CreateJoint( &prismJointDef );

    return true;
}

bool player::raise_from_mship(void)
{
    m_pLand_raise_joint->SetMotorSpeed(1.0);

    return true;
}

bool player::get_rope_lock_status(void)
{
    return m_rope_lock_on;
}

bool player::lifting_a_player_ship(void)//test if this player is currently lifting another player
{
    if(m_hook_connected)
    {
        //test if other player hooked
        st_body_user_data* data=(st_body_user_data*)m_pBody_connected_to->GetUserData();
        if(data->s_info=="player") return true;
    }

    return false;
}

bool player::erase_lost_ropes(void)
{
    for(int chain_i=0;chain_i<(int)m_vec_vec_disconnected_ropes.size();chain_i++)
    {
        m_vec_vec_disconnected_ropes[chain_i].clear();
    }
    m_vec_vec_disconnected_ropes.clear();

    return true;
}

int player::get_rope_length(void)
{
    return (int)m_vec_rope_bodies.size();
}

bool player::set_pos(b2Vec2 pos)
{
    //disconnect hook
    hook_disconnect();

    //move body
    m_pBody->SetTransform( pos, _pi*0.0 );

    //move rope
    for(unsigned int rope_i=0;rope_i<m_vec_rope_bodies.size();rope_i++)
    {
        m_vec_rope_bodies[rope_i]->SetTransform( pos, _pi*0.0 );
    }

    return true;
}

bool player::fire_turret(float time_dif,bool ammo_test)
{
    //cout<<"Weapon fire request... ";
    //test if cooldown is ready
    if( m_pWeapon_curr->is_ready_to_fire() )
    {
        //give direction of turret (speed of player?)
        b2Vec2 player_pos=m_pBody->GetPosition();
        float turret_angle=m_pBody->GetAngle()*_Rad2Deg+m_turret_rotation-90.0;
        //normalize direction
        b2Vec2 fire_direction( cosf(turret_angle*_Deg2Rad), sinf(turret_angle*_Deg2Rad) );

        //test bullet pathway through barrel with raycast
        float barrel_length=_player_ship_to_barrel_dist;
        if(m_pWeapon_curr->get_type()==wt_rocket) barrel_length+=0.3;
        b2Vec2 barrel_tip_pos=b2Vec2( player_pos.x+fire_direction.x*barrel_length,
                                      player_pos.y+fire_direction.y*barrel_length );
        MyRayCastCallback callback;
        m_pWorld->RayCast(&callback,player_pos,barrel_tip_pos);
        if(callback.m_any_hit) return false;


        //TEMP
        /*//test if point is inside player ship (only one fixture?)
        if(m_pBody->GetFixtureList()->TestPoint(b2Vec2(player_pos.x+fire_direction.x*barrel_length,
                                                       player_pos.y+fire_direction.y*barrel_length)))
        cout<<"Inslide\n";
        else cout<<"Not inside..";*/


        //add ship's speed to fire direction
        //b2Vec2 fire_direction_w_speed=0.05*m_pBody->GetLinearVelocity()+fire_direction;

        //ammo test
        if(ammo_test)
        {
            float ammo_cost=m_pWeapon_curr->get_ammo_cost();
            if( m_pWeapon_curr->get_type()==wt_beam ) ammo_cost*=time_dif;
            if(ammo_cost>m_ammo_curr) return false;
            //consume ammo
            m_ammo_curr-=ammo_cost;
        }

        //fire
        int ret_value=m_pWeapon_curr->fire_weapon(barrel_tip_pos,fire_direction,time_dif);

        //move barrel back
        m_barrel_length=0.5;

        switch(m_pWeapon_curr->get_type())
        {
            case wt_cannon:
            {
                //muzzle flash, particles
                m_pParticle_engine->add_explosion(barrel_tip_pos,10,200.0,0.2);
            }break;

            case wt_beam:
            {
                //cancel barrel recoil
                m_barrel_length=1.0;

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

                //cout<<"START: "<<m_vBeam_start.x<<", "<<m_vBeam_start.y<<endl;
                //cout<<"END  : "<<m_vBeam_end.x<<", "<<m_vBeam_end.y<<endl;
            }break;

            default:
            {
                //particles from barrel
                m_pParticle_engine->add_explosion(barrel_tip_pos,5,100.0,0.2,turret_angle+90.0);
            }
        }

        //cout<<"done\n";

        return true;
    }
    //else cout<<"Weapon cooldown\n";
    //cout<<"done\n";

    return false;
}

bool player::set_current_weapon(weapon* pCurr_weapon)
{
    if(pCurr_weapon==m_pWeapon_default) m_using_default_weapon=true;
    else m_using_default_weapon=false;

    m_pWeapon_curr=pCurr_weapon;

    return true;
}

bool player::use_default_weapon(void)
{
    m_pWeapon_curr=m_pWeapon_default;
    m_using_default_weapon=true;

    return true;
}

weapon* player::get_weapon_ptr(void)
{
    return m_pWeapon_curr;
}

weapon* player::get_default_weapon_ptr(void)
{
    return m_pWeapon_default;
}

int player::get_weapon_index(void)
{
    return m_weapon_index;
}

bool player::set_weapon_index(int value)
{
    m_weapon_index=value;

    return true;
}

bool player::set_current_gear(gear* pCurr_gear)
{
    if(pCurr_gear==m_pGear_default) m_using_default_gear=true;
    else m_using_default_gear=false;

    m_pGear_curr=pCurr_gear;

    return true;
}

bool player::use_default_gear(void)
{
    m_pGear_curr=m_pGear_default;
    m_using_default_gear=true;

    return true;
}

gear* player::get_gear_ptr(void)
{
    return m_pGear_curr;
}

gear* player::get_default_gear_ptr(void)
{
    return m_pGear_default;
}

int player::get_gear_index(void)
{
    return m_gear_index;
}

bool player::set_gear_index(int value)
{
    m_gear_index=value;

    return true;
}
