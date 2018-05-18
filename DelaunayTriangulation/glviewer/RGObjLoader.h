//
//  RGObjLoader.h
//  obj-loader
//
//  Created by sbx_fc on 14-10-20.
//  Copyright (c) 2014年 rungame. All rights reserved.
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

// ����ÿ��group�ĸ�ʽ:
typedef struct
{
    std::vector<unsigned int>   indices;        //��������ж�����Ϣ������
    std::vector<float>          positions;      //��������
    std::vector<float>          normals;        //���㷨��
    std::vector<float>          texcoords;      //�����UV����
    std::vector<int>            material_ids;   //����ID��������Ӧ�Ĳ�������materials,�����������ж���Ĳ�����Ϣ��ÿ������ͨ��IDȥ��Ӧ��Ӧ�Ĳ�����Ϣ
} mesh_r;

typedef struct
{
    std::string     name;
    mesh_r          mesh;
} shape_r;

typedef struct
{
    std::string name;
    
    float ambient[4];       // �������� Ka
    float diffuse[4];       // ������ Kd
    float specular[4];      // ���淴�� Ks
    float transmittance[4]; // ͸���ȣ��˹�͸���ʣ� Tf
    float emission[4];
    float shininess;        // ����ָ��
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
