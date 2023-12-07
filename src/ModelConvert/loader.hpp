#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>


#define IDK_ALBEDO_TEXTURE 0
#define IDK_NORMAL_TEXTURE 1
#define IDK_RM_TEXTURE 2
#define IDK_AO_TEXTURE 3
#define IDK_NUM_TEXTURES 4


template <typename T>
class Loader
{
private:
    int current_mesh = 0;
    std::vector<std::string> texture_paths;
    std::map<std::string, std::set<int>> texture_mappings;

    std::map<std::string, std::map<int, std::string>> texture_mappings2;

    std::map<std::string, idk::idkvi_material_t> idkvi_materials;
    std::map<int, std::vector<idk::Vertex>> all_vertices;
    std::map<int, std::vector<uint32_t>>    all_indices;

public:


};


