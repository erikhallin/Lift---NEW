#ifndef SOUND_H
#define SOUND_H

#include "sound_list.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <vorbis/vorbisfile.h>

#define _numof_sound_buffers 80 //number of sounds
#define _numof_sound_sources 40 //number of sounds that could be play at the same time
//0-9 sound effects, 10-29 loop sounds, 30-39 music
#define BUFFER_SIZE     32768       // 32 KB buffers, for OGG

using namespace std;

class sound
{
    public:
        sound();                           //Open OpenAL
        ~sound();                          //Close OpenAL

        //Variables
        bool m_ready;

        //Functions
        int  get_error(void);
        bool load_WAVE_from_file(int sound_id, const char* file);//give filename and sound id number, returns if source/buffer was created
        bool load_WAVE_from_string(int sound_id, string data_file);
        bool load_OGG_from_file(int sound_id, const char* file);
        bool playWAVE(int sound_id, float data[]);    //data contains: ListenerPos    0:1:2,  ListenerVel  3:4:5,
                                                      //               ListenerOri at-6:7:8,          up-9:10:11,
                                                      //               SourcePos 12:13:14,    SourceVel 15:16:17,
                                                      //               Pitch           18,    Gain            19,  Loop         20
        int  playSimpleSound(int sound_id,float volume,int source_id=-1,bool loop_flag=false); //volume 0.0 - 1.0
        int  getAndLoadFreeSource(int buffer_id);
        void updateSound(int source_id,float data[]);
        bool set_volume(int source_id,float volume);
        bool set_volume_rel(int source_id,float volume_rel);
        float get_volume(int source_id);
        void stopSound(int source_id);
        void enable_sound(bool flag);
        void enable_music(bool flag);
        bool get_source_status(int source_id);
        bool pause_source(int source_id);
        bool resume_source(int source_id);
        bool set_music_source(int source_id);
        bool set_screen_test(int screen_size[2],float cam_pos[2]);
        bool play_sound_w_test(int sound_id,float source_pos[2]);

    private:
        //Variables
        bool        m_sound_on,m_music_on;
        int         m_error,m_music_source;
        ALCdevice*  m_pALCdevice;
        ALCcontext* m_pALCcontext;
        ALuint      m_arrSource[_numof_sound_sources];
        ALuint      m_arrBuffer[_numof_sound_buffers];

        //Game Specific
        float  m_window_width,m_window_height;
        float* m_pCam_pos;

};


#endif

/*  EXAMPLE

Pre-comp
    Set number of sounds (numOfSound)

Initializing

    sound Sound;

Loading
    Sound.loadWAVE(sound_id_test1,"test1.wav");

Playing
    float data[]={0,0,0, 0,0,0, 0,0,-1,
                  0,1,0, 0,0,0, 0,0,0,
                  1,  1,  0};
    Sound.playWAVE(sound_id_test1,data);
*/
