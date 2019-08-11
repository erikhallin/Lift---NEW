#ifndef FILES_IN_TEXT_H
#define FILES_IN_TEXT_H

#include <string>
#include <iostream>

using namespace std;

//Data files converted with Base64 to text

enum data_files//list of all files
{
    file_texture_decal,
    file_texture_goal,
    file_texture_info,
    file_texture_loading,
    file_texture_lost,
    file_texture_manual,
    file_texture_terrain0,
    file_texture_terrain1,
    file_texture_terrain2,
    file_texture_terrain3,
    file_texture_terrain4,
    file_texture_text,
    file_texture_texture,
    file_texture_moretext,
    file_texture_mask
};

string load_base64_file(int file_id);//will return encoded text

#endif
