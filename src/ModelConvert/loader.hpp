#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <IDKGameEngine/IDKGameEngine.hpp>
#include <libidk/idk_idkvi_file.hpp>


#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "skeleton.hpp"

#define IDK_ALBEDO_TEXTURE 0
#define IDK_NORMAL_TEXTURE 1
#define IDK_RM_TEXTURE 2
#define IDK_AO_TEXTURE 3
#define IDK_NUM_TEXTURES 4


glm::mat4 assimp_matrix_compose( aiVector3D position, aiQuaternion rotation, aiVector3D scale )
{
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::scale(transform, toGLM(scale));
    transform = glm::mat4_cast(toGLM(rotation)) * transform;
    transform = glm::translate(transform, toGLM(position));

    return transform;
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


struct idk_Material
{
    std::string paths[4] = { "dummy", "dummy", "dummy", "dummy" };
};


template <typename vertex_t>
class Loader
{
private:
    struct idk_Mesh
    {
        idk_Material          m_material;
        std::vector<vertex_t> m_vertices;
        std::vector<uint32_t> m_indices;
    };


    idk_Skeleton            m_skeleton;
    std::vector<idk_Mesh>   m_meshes;

    uint32_t textureBitmask( idk_Material &idkmaterial )
    {
        uint32_t bitmask = 0;

        for (uint32_t i=0; i<IDK_NUM_TEXTURES; i++)
        {
            uint32_t bit = (idkmaterial.paths[i] == "dummy") ? 0 : 1;
    
            bitmask |= (bit << i);
        }

        return bitmask;
    }



public:
    bool animated = false;


    void load_material( const aiScene *scene, aiMesh *mesh, idk_Material &idkmaterial )
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        aiString paths[4];
    
        material->GetTexture(aiTextureType_BASE_COLOR, 0, &paths[0]);
        material->GetTexture(aiTextureType_NORMALS,    0, &paths[1]);
        material->GetTexture(aiTextureType_METALNESS,  0, &paths[2]);
        material->GetTexture(aiTextureType_LIGHTMAP,   0, &paths[3]);

        for (int i=0; i<IDK_NUM_TEXTURES; i++)
        {
            if (paths[i].length > 0)
            {
                idkmaterial.paths[i] = paths[i].C_Str();
            }
        }
    }


    void load_vertices( const aiScene *scene, aiNode *node, aiMesh *mesh, idk_Mesh &idkmesh )
    {
        std::vector<vertex_t> &vertices = idkmesh.m_vertices;
        std::vector<uint32_t> &indices = idkmesh.m_indices;

        vertices.resize(mesh->mNumVertices);
        indices.resize(3*mesh->mNumFaces);

        for (int i=0; i<mesh->mNumVertices; i++)
        {
            vertex_t &vert = vertices[i];

            vert.position.x = mesh->mVertices[i].x;
            vert.position.y = mesh->mVertices[i].y;
            vert.position.z = mesh->mVertices[i].z;

            vert.normal.x = mesh->mNormals[i].x;
            vert.normal.y = mesh->mNormals[i].y;
            vert.normal.z = mesh->mNormals[i].z;

            vert.tangent.x = mesh->mTangents[i].x;
            vert.tangent.y = mesh->mTangents[i].y;
            vert.tangent.z = mesh->mTangents[i].z;

            vert.texcoords.x = mesh->mTextureCoords[0][i].x;
            vert.texcoords.y = mesh->mTextureCoords[0][i].y;
        }

        for (int i=0; i<mesh->mNumFaces; i++)
        {
            indices[3*i + 0] = mesh->mFaces[i].mIndices[0];
            indices[3*i + 1] = mesh->mFaces[i].mIndices[1];
            indices[3*i + 2] = mesh->mFaces[i].mIndices[2];
        }
    }


    void load_weights( aiMesh *mesh, idk_Mesh &idkmesh, idk::Vertex v )
    {

    }

    void load_weights( aiMesh *mesh, idk_Mesh &idkmesh, idk::AnimatedVertex v )
    {
        std::vector<vertex_t> &vertices = idkmesh.m_vertices;

        std::map<int, int> weightmap;

        for (int i=0; i<mesh->mNumBones; i++)
        {
            aiBone *aibone     = mesh->mBones[i];
            aiNode *bone_node  = aibone->mNode;

            int aibone_idx = m_skeleton.m_aiBoneNodeIndices[bone_node->mName.C_Str()];

            for (int j=0; j<aibone->mNumWeights; j++)
            {
                aiVertexWeight vWeight = aibone->mWeights[j];

                int   idx    = vWeight.mVertexId;
                float weight = vWeight.mWeight;

                int &n = weightmap[idx];
                vertices[idx].bone_ids[n]     = aibone_idx;
                vertices[idx].bone_weights[n] = weight;
                n += 1;
            }
        }
    }


    void load_mesh( const aiScene *scene, aiNode *node, aiMesh *mesh )
    {
        m_meshes.push_back(idk_Mesh());
        idk_Mesh &idkmesh = m_meshes.back();

        load_material(scene, mesh, idkmesh.m_material);
        load_vertices(scene, node, mesh, idkmesh);
        load_weights(mesh, idkmesh, vertex_t());
    }


    void load_meshes( const aiScene *scene, aiNode *node )
    {
        for (int i=0; i<node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            load_mesh(scene, node, mesh);
        }

        for (int i=0; i<node->mNumChildren; i++)
        {
            load_meshes(scene, node->mChildren[i]);
        }
    }


    void process( const aiScene *scene )
    {
        aiNode *root = scene->mRootNode;

        // Load skeleton
        // -------------------------------------------------------------------------------------
        if (scene->HasAnimations())
        {
            this->animated = true;
            m_skeleton.load(scene);
        }
        // -------------------------------------------------------------------------------------

        load_meshes(scene, root);
    }


    void write_header( std::ofstream &stream )
    {
        if (this->animated)
        {
            stream << "animated" << "\n";
        }

        for (idk_Mesh &mesh: m_meshes)
        {
            stream << mesh.m_indices.size() << "\n";
            stream << textureBitmask(mesh.m_material) << "\n";

            for (int i=0; i<IDK_NUM_TEXTURES; i++)
            {
                stream << mesh.m_material.paths[i] << "\n";
            }
        }
    }


    idk::idkvi_material gen_idkvi_material( idk_Material m )
    {
        idk::idkvi_material material;
        // material.bitmask = textureBitmask(m);

        // for (int i=0; i<IDK_NUM_TEXTURES; i++)
        // {
        //     std::string &path = m.paths[i];
        //     std::memcpy(material.textures[i], path.c_str(), path.size()+1);
        //     material.textures[i][path.size()+2] = '\0';
        // }
    
        return material;
    }


    void write_idkvi( std::ofstream &stream )
    {
        std::vector<vertex_t> all_vertices;
        std::vector<uint32_t> all_indices;
        idk::Buffer<idk::idkvi_mesh> meshes;

        uint32_t basevertex = 0;
        uint32_t baseindex  = 0;
        uint32_t vertex_offset = 0;

        for (idk_Mesh &mesh: m_meshes)
        {
            for (auto &v: mesh.m_vertices)
            {
                all_vertices.push_back(v);
            }

            for (uint32_t idx: mesh.m_indices)
            {
                all_indices.push_back(idx + basevertex);
            }

            basevertex += mesh.m_vertices.size();

            // idk::idkvi_mesh meshfile;
            // meshfile.basevertex = basevertex;
            // meshfile.baseindex  = 0;
            // meshfile.material   = gen_idkvi_material(mesh.m_material);
            // meshes.push_back(meshfile);
        }


        uint32_t vertexformat = idk::VertexFormat::VERTEX_POSITION3F_NORMAL3F_TANGENT3F_UV2F;
        uint32_t num_vertices = all_vertices.size();
        uint32_t num_indices  = all_indices.size();
        uint32_t num_meshes   = meshes.size();

        stream.write(reinterpret_cast<const char *>(&vertexformat), sizeof(uint32_t));
        stream.write(reinterpret_cast<const char *>(&num_vertices), sizeof(uint32_t));
        stream.write(reinterpret_cast<const char *>(&num_indices),  sizeof(uint32_t));
        stream.write(reinterpret_cast<const char *>(&num_meshes),   sizeof(uint32_t));

        stream.write(reinterpret_cast<const char *>(&all_vertices[0]), num_vertices*sizeof(vertex_t));
        stream.write(reinterpret_cast<const char *>(&all_indices[0]),  num_indices*sizeof(uint32_t));
        // stream.write(reinterpret_cast<const char *>(meshes.data()),    meshes.nbytes());
    }


    void write_idka( std::ofstream &stream )
    {
        m_skeleton.write_animations(stream);
    }


    void write( const std::string &filepath )
    {
        std::ofstream stream;
    
        // stream = std::ofstream(file_directory(filepath) + ".txt");
        // write_header(stream);
        // stream.close();

        stream = std::ofstream(file_directory(filepath) + ".idkvi", std::ios::binary);
        write_idkvi(stream);

        // if (this->animated)
        // {
        //     write_idka(stream);
        // }

        stream.close();
    }

};

