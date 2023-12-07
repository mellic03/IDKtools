#include <IDKengine/IDKengine.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "animation.hpp"
#include <iomanip>


glm::mat4 assimp_to_glm( aiMatrix4x4 from )
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}


std::string file_directory( std::string filepath )
{
    size_t start = 0;
    size_t end   = filepath.length();

    size_t last_slash = end;
    size_t last_dot   = end;

    for (size_t i=end; i>start; i--)
    {
        if (filepath[i] == '.')
            last_dot = i;

        else if (filepath[i] == '/')
        {
            last_slash = i;
            break;
        }
    }

    return filepath.substr(0, last_dot);
}



int current_mesh = 0;
std::vector<std::string> texture_paths;
std::map<std::string, std::set<int>> texture_mappings;

#define IDK_ALBEDO_TEXTURE 0
#define IDK_NORMAL_TEXTURE 1
#define IDK_RM_TEXTURE 2
#define IDK_AO_TEXTURE 3
#define IDK_NUM_TEXTURES 4
std::map<std::string, std::map<int, std::string>> texture_mappings2;

std::map<std::string, idk::idkvi_material_t> idkvi_materials;
std::map<int, std::vector<idk::Vertex>> all_vertices;
std::map<int, std::vector<uint32_t>>    all_indices;



void add_texture( aiString albedo_name, aiMaterial *material, aiTextureType type, int idktype )
{
    int num_textures = material->GetTextureCount(type);

    for (int j=0; j<num_textures; j++)
    {
        aiString str;
        material->GetTexture(type, j, &str);
        texture_mappings2[albedo_name.C_Str()][idktype] = str.C_Str();
    }
}


glm::mat4 getParentedTransform( aiNode *node )
{
    aiNode *p = node;
    glm::mat4 transform = assimp_to_glm(p->mTransformation);

    while (p->mParent != nullptr)
    {
        p = p->mParent;
        transform = assimp_to_glm(p->mTransformation) * transform;
    }

    return transform;
}


glm::vec3 idk_get_specular( aiMaterial *material )
{
    float *data;

    for (int i=0; i<material->mNumProperties; i++)
    {
        aiMaterialProperty *property = material->mProperties[i];

        if (std::string(property->mKey.C_Str()) == "$clr.specular")
        {
            data = (float *)(property->mData);
            return glm::min(glm::vec3(data[0], data[1], data[2]), 1.0f);
        }
    }

    return glm::vec3(0.04f);
}


void idk_readmodel( const aiScene *scene, aiNode *node )
{
    aiMetadata *metadata = node->mMetaData;

    if (metadata != nullptr)
    {
        std::cout << "P: " << metadata->mNumProperties << "\n";
    }

    for (int i=0; i<node->mNumMeshes; i++)
    {
        glm::mat4 transform = getParentedTransform(node);
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        int material_idx = mesh->mMaterialIndex;
        aiMaterial *material = scene->mMaterials[material_idx];

        aiString albedo_name;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &albedo_name);
        texture_mappings[albedo_name.C_Str()].insert(current_mesh);
        texture_mappings2[albedo_name.C_Str()][IDK_ALBEDO_TEXTURE] = albedo_name.C_Str();

        idkvi_materials[albedo_name.C_Str()].reflectance = idk_get_specular(material);


        add_texture(albedo_name, material, aiTextureType_METALNESS, IDK_RM_TEXTURE);
        add_texture(albedo_name, material, aiTextureType_LIGHTMAP, IDK_AO_TEXTURE);
        add_texture(albedo_name, material, aiTextureType_NORMALS, IDK_NORMAL_TEXTURE);

        for (int j=0; j<mesh->mNumVertices; j++)
        {
            idk::Vertex vert;
            vert.texcoords.x = 0.0f;
            vert.texcoords.y = 0.0f;

            vert.position.x = mesh->mVertices[j].x;
            vert.position.y = mesh->mVertices[j].y;
            vert.position.z = mesh->mVertices[j].z;
            vert.position   = transform * glm::vec4(vert.position, 1.0f);

            vert.normal.x = mesh->mNormals[j].x;
            vert.normal.y = mesh->mNormals[j].y;
            vert.normal.z = mesh->mNormals[j].z;
            vert.normal   = transform * glm::vec4(vert.normal, 0.0f);

            if (mesh->HasTangentsAndBitangents())
            {
                vert.tangent.x = mesh->mTangents[j].x;
                vert.tangent.y = mesh->mTangents[j].y;
                vert.tangent.z = mesh->mTangents[j].z;
                vert.tangent   = transform * glm::vec4(vert.tangent, 0.0f);
            }

            if (mesh->HasTextureCoords(0))
            {
                vert.texcoords.x = mesh->mTextureCoords[0][j].x;
                vert.texcoords.y = mesh->mTextureCoords[0][j].y;
            }

            all_vertices[current_mesh].push_back(vert);
        }


        for (int j=0; j<mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];

            for (int k=0; k<face.mNumIndices; k++)
            {
                all_indices[current_mesh].push_back(face.mIndices[k]);
            }
        }

        current_mesh += 1;
    }


    for (int i=0; i<node->mNumChildren; i++)
    {
        idk_readmodel(scene, node->mChildren[i]);
    }

}



uint32_t idk_texturebitmask( std::string albedo_name )
{
    uint32_t bitmask = 0;

    for (int i=0; i<IDK_NUM_TEXTURES; i++)
    {
        if (texture_mappings2[albedo_name][i] != "") bitmask |= 1 << i;
    }

    return bitmask;
}


std::string idk_texturebitmask_str( std::string albedo_name )
{
    std::string bitmask = "0000";

    for (int i=0; i<IDK_NUM_TEXTURES; i++)
    {
        bitmask += (texture_mappings2[albedo_name][i] == "") ? "0" : "1";
    }

    return bitmask;
}



int main( int argc, char **argv )
{
    std::string input_path  = argv[1];

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(input_path, aiProcess_FlipUVs);

    idk_readmodel(scene, scene->mRootNode);
    // idk_loadAnimation(scene, input_path);

    std::ofstream text_stream(file_directory(input_path) + ".txt");

    size_t vertex_offset = 0;

    // std::vector<idk::idkvi_material_t> materials;
    std::vector<idk::Vertex> vertices;
    std::vector<uint32_t>    indices;

    for (auto &[texture_path, mesh_ids]: texture_mappings)
    {
        // size_t num_vertices = 0;
        size_t num_indices  = 0;

        for (int id: mesh_ids)
        {
            for (auto &vert: all_vertices[id])
            {
                vertices.push_back(vert);
            }
    
            for (auto &idx: all_indices[id])
            {
                indices.push_back(idx + vertex_offset);
            }

            // num_vertices  += all_vertices[id].size();
            num_indices   += all_indices[id].size();
            vertex_offset += all_vertices[id].size();
        }

        // materials.push_back(idkvi_materials[texture_path]);
        // text_stream << num_vertices << "\n";
        text_stream << num_indices << "\n";

        auto &textures = texture_mappings2[texture_path];
        text_stream << idk_texturebitmask(texture_path) << "\n";

        for (int i=0; i<IDK_NUM_TEXTURES; i++)
        {
            text_stream << (textures[i] == "" ? "dummy" : textures[i]) << "\n";
        }
    }
    text_stream.close();


    std::ofstream vert_stream(file_directory(input_path) + ".idkvi", std::ios::binary);

    // size_t num_materials = materials.size();
    size_t num_vertices = vertices.size();
    size_t num_indices  = indices.size();
    // vert_stream.write(reinterpret_cast<const char *>(&num_materials), sizeof(size_t));
    vert_stream.write(reinterpret_cast<const char *>(&num_vertices),  sizeof(size_t));
    vert_stream.write(reinterpret_cast<const char *>(&num_indices),   sizeof(size_t));

    // vert_stream.write(reinterpret_cast<const char *>(&materials[0]), sizeof(materials[0])*materials.size());
    vert_stream.write(reinterpret_cast<const char *>(&vertices[0]), sizeof(idk::Vertex)*vertices.size());
    vert_stream.write(reinterpret_cast<const char *>(&indices[0]),  sizeof(uint32_t)*indices.size());

    vert_stream.close();

    return 0;
}

