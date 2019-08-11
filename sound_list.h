#ifndef SOUND_LIST_H
#define SOUND_LIST_H

//List of all in game sounds

enum sounds
{
    wav_hook_break=0,
    wav_hook_connect,
    wav_hook_disconnect,

    wav_mship_connect,
    wav_mship_disconnect,
    wav_mship_input_fuel,
    wav_mship_input_ship,
    wav_mship_motor,
    wav_mship_gear_motor,
    wav_mship_recycle,

    wav_drone_join_ship,
    wav_drone_crash,
    wav_drone_eject,

    wav_starmap_land,//start
    wav_starmap_travel,
    wav_starmap_select,
    wav_starmap_startup,//start of travel
    wav_starmap_startdown,//end of travel

    wav_weapon_pea,
    wav_weapon_spread,
    wav_weapon_rocket,
    wav_weapon_grenade,
    wav_weapon_mine,
    wav_weapon_cannon,
    wav_weapon_laser,

    wav_ship_explosion,
    wav_ship_col,
    wav_ship_upgrade,

    wav_player_ship_raising,
    wav_player_motor,
    wav_player_motor_boost,
    wav_player_rope_motor,

    wav_bullet_hit,
    wav_bullet_explosion,

    wav_enemy_ship_detected,
    wav_enemy_ship_lost,

    wav_gear_enable,
    wav_gear_disable,

    wav_drone_motor,

    wav_turret_rotation,

    wav_alarm,

    ogg_starmap_noise,
    ogg_music0_intro,
    ogg_music0_loop,
    ogg_music1_intro,
    ogg_music1_loop,
    ogg_music2_intro,
    ogg_music2_loop,
    ogg_music3_intro,
    ogg_music3_loop,
    ogg_music4_intro,
    ogg_music4_loop,
    ogg_music5_intro,
    ogg_music5_loop,
    ogg_music6_intro,
    ogg_music6_loop,
    ogg_music7_intro,
    ogg_music7_loop,
    ogg_music8_intro,
    ogg_music8_loop,
    ogg_music9_intro,
    ogg_music9_loop

};


#endif
