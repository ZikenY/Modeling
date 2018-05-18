//
//  RGObjLoader.h
//  obj-loader
//
//  Created by sbx_fc on 14-10-20.
//  Copyright (c) 2014å¹´ rungame. All rights reserved.
//

#ifndef __OBJ_LOADER__RGOBJLOADER__
#define __OBJ_LOADER__RGOBJLOADER__

#pragma warning(disable: 4786)
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

extern bool mtl_existed;
extern float mesh_bound_x_max;
extern float mesh_bound_x_min;
extern float mesh_bound_y_max;
extern float mesh_bound_y_min;
extern float mesh_bound_z_max;
extern float mesh_bound_z_min;

// ¶¨ÒåÃ¿¸ögroupµÄ¸ñÊ½:
typedef struct
{
    std::vector<unsigned int>   indices;        //Õâ¸ö×éËùÓĞ¶¥µãĞÅÏ¢µÄË÷Òı
    std::vector<float>          positions;      //¶¥µã×ø±ê
    std::vector<float>          normals;        //¶¥µã·¨Ïß
    std::vector<float>          texcoords;      //¶¥µãµÄUV×ø±ê
    std::vector<int>            material_ids;   //²ÄÖÊIDµÄË÷Òı¶ÔÓ¦µÄ²ÄÖÊÊı×ématerials,Ëü°üº¬ÁËËùÓĞ¶¥µãµÄ²ÄÖÊĞÅÏ¢£¬Ã¿¸ö¶¥µãÍ¨¹ıIDÈ¥¶ÔÓ¦ÏàÓ¦µÄ²ÄÖÊĞÅÏ¢
} mesh_r;

typedef struct
{
    std::string     name;
    mesh_r          mesh;
} shape_r;

typedef struct
{
    std::string name;
    
    float ambient[4];       // »·¾³·´Éä Ka
    float diffuse[4];       // Âş·´Éä Kd
    float specular[4];      // ¾µÃæ·´Éä Ks
    float transmittance[4]; // Í¸Ã÷¶È£¨ÂË¹âÍ¸ÉäÂÊ£© Tf
    float emission[4];
    float shininess;        // ¾µÃæÖ¸Êı
    float ior;
    float dissolve;
    int illum;
    
    std::string ambient_texname;
    std::string diffuse_texname;
    std::string specular_texname;
    std::string normal_texname;
    std::map<std::string, std::string> unknown_parameter;
} material_r;


class MaterialReader
{
public:
    MaterialReader(){}
    virtual ~MaterialReader(){}
    virtual std::string operator() (const std::string& matId,
        std::vector<material_r>& materials,
        std::map<std::string, int>& matMap
        ) = 0;
};


class MaterialFileReader: public MaterialReader
{
public:
    MaterialFileReader(const std::string& mtl_basepath): m_mtlBasePath(mtl_basepath) {}
    virtual ~MaterialFileReader() {}
    virtual std::string operator()(
        const std::string& matId,
        std::vector<material_r>& materials,
        std::map<std::string,
        int>& matMap
        );

private:
    std::string m_mtlBasePath;
};

std::string LoadObj(
    std::vector<shape_r>& shapes,
    std::vector<material_r>& materials,
    const char* filename
    );

std::string LoadObj(
    std::vector<shape_r>& shapes,
    std::vector<material_r>& materials,
    std::istream& inStream,
    MaterialReader& readMatFn
    );

#endif // defined(__OBJ_LOADER__RGOBJLOADER__)
