#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include <IDKGameEngine/IDKGameEngine.hpp>
#include <IDKGraphics/animation/IDKanimation.hpp>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"



void print_mat4( const glm::mat4 &m )
{
    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {
            std::cout << m[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


glm::mat4 toGLM( const aiMatrix4x4 &from )
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

glm::vec3 toGLM( aiVector3D v )
{
    return glm::vec3(v.x, v.y, v.z);
}

glm::quat toGLM( aiQuaternion q )
{
    return glm::quat(q.w, q.x, q.y, q.z);
}


bool is_nullMatrix( const glm::mat4 &m )
{
    return m == glm::mat4(0.0f);
}


struct idk_Skeleton
{
    aiNode *m_armature = nullptr;

    std::map<std::string, aiNode *>      m_aiNodeMap;
    std::map<std::string, aiNode *>      m_aiBoneNodes;
    std::map<std::string, glm::mat4>     m_aiBoneTransforms;
    std::vector<aiNode *>                m_aiBoneNodeArray;
    std::map<std::string, int>           m_aiBoneNodeIndices;
    std::map<std::string, std::set<int>> m_aiBoneNodeChildIndices;
    std::vector<idk::Animation>          m_animations;


    void load_nodes( const aiScene *scene, aiNode *node )
    {
        m_aiNodeMap[node->mName.C_Str()] = node;

        for (int i=0; i<node->mNumChildren; i++)
        {
            load_nodes(scene, node->mChildren[i]);
        }
    }


    int load_armature( aiNode *node )
    {
        std::string node_name = node->mName.C_Str();

        // if (node_name.substr(0, 3) != "idk")
        // {
        //     return -1;
        // }

        int idx = m_aiBoneNodeArray.size();
        m_aiBoneNodeArray.push_back(node);

        m_aiBoneNodeIndices[node_name] = idx;
        m_aiBoneNodes[node_name] = node;

        for (int i=0; i<node->mNumChildren; i++)
        {
            int child_idx = load_armature(node->mChildren[i]);
            
            // if (child_idx != -1)
            // {
                m_aiBoneNodeChildIndices[node_name].insert(child_idx);
            // }
        }

        return idx;
    }


    aiNode *find_armature_root( const aiScene *scene)
    {
        for (int i=0; i<scene->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[i];

            for (int j=0; j<mesh->mNumBones; j++)
            {
                aiBone *bone = mesh->mBones[j];
                return bone->mArmature;
            }
        }

        return nullptr;
    }


    void load_animation( std::string animation_name, int num_channels, aiNodeAnim **channels, float duration )
    {
        m_animations.push_back(idk::Animation());

        idk::Animation &animation = m_animations.back();
        animation.m_duration = duration;
        animation.m_bones.resize(m_aiBoneNodeArray.size());

        for (int i=0; i<num_channels; i++)
        {
            aiNodeAnim *node_anim = channels[i];
            std::string node_name = node_anim->mNodeName.C_Str();
            aiNode     *bone_node = m_aiBoneNodes[node_name];

            int array_idx = m_aiBoneNodeIndices[node_name];

            idk::AnimBone &animbone  = animation.m_bones[array_idx];
            animbone.local_transform = toGLM(bone_node->mTransformation);
            animbone.inverse_bind    = m_aiBoneTransforms[node_name];

            if (is_nullMatrix(animbone.inverse_bind))
            {
                animbone.inverse_bind = glm::mat4(1.0f);
            }

            for (int child_idx: m_aiBoneNodeChildIndices[node_name])
            {
                animbone.children.push_back(child_idx);
            }

            for (int j=0; j<node_anim->mNumPositionKeys; j++)
            {
                animbone.positions.push_back(toGLM(node_anim->mPositionKeys[j].mValue));
                animbone.position_timings.push_back(node_anim->mPositionKeys[j].mTime);
            }

            for (int j=0; j<node_anim->mNumRotationKeys; j++)
            {
                animbone.rotations.push_back(toGLM(node_anim->mRotationKeys[j].mValue));
                animbone.rotation_timings.push_back(node_anim->mRotationKeys[j].mTime);
            }


            for (int j=0; j<node_anim->mNumScalingKeys; j++)
            {
                animbone.scales.push_back(toGLM(node_anim->mScalingKeys[j].mValue));
                animbone.scale_timings.push_back(node_anim->mScalingKeys[j].mTime);
            }
        }
    }


    void load_animations( const aiScene *scene )
    {
        for (int i=0; i<scene->mNumAnimations; i++)
        {
            aiAnimation *animation = scene->mAnimations[i];
            std::string  animation_name = animation->mName.C_Str();
    
            std::cout << "TPS:  " << animation->mTicksPerSecond << "\n";

            std::cout << "Animation: " << animation_name << "\n";
            std::cout << "Bones:     " << animation->mNumChannels << "\n";
            std::cout << "Duration:  " << animation->mDuration << "\n";

            load_animation(animation_name, animation->mNumChannels, animation->mChannels, animation->mDuration);
        }
    }


    void print( int idx )
    {
        aiNode *node = m_aiBoneNodeArray[idx];

        std::cout << node->mName.C_Str() << "-" << idx << "-( ";

        for (int child_idx: m_aiBoneNodeChildIndices[node->mName.C_Str()])
        {
            std::cout << child_idx << " ";
        }
        std::cout << ")  <--  ";

        if (node->mParent != nullptr && node != m_armature)
        {
            print(m_aiBoneNodeIndices[node->mParent->mName.C_Str()]);
        }
    }


    void print()
    {
        for (int i=0; i<m_aiBoneNodeArray.size(); i++)
        {
            print(i);
            std::cout << "\n\n";
        }
    }


    void load( const aiScene *scene )
    {
        aiNode *root = scene->mRootNode;

        load_nodes(scene, root);

        for (int i=0; i<scene->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[i];

            for (int j=0; j<mesh->mNumBones; j++)
            {
                aiBone *bone = mesh->mBones[j];
                m_aiBoneTransforms[bone->mName.C_Str()] = toGLM(bone->mOffsetMatrix);
            }
        }

        m_armature = find_armature_root(scene);
        load_armature(m_armature);
        load_animations(scene);
        print();
    }


    void write_animbone( std::ofstream &stream, idk::AnimBone &bone )
    {
        uint32_t num_children = bone.children.size();
        stream.write(reinterpret_cast<const char *>(&num_children), sizeof(uint32_t));
        stream.write(reinterpret_cast<const char *>(&bone.children[0]), num_children*sizeof(int));

        uint32_t num_positions = bone.positions.size();
        uint32_t num_rotations = bone.rotations.size();
        uint32_t num_scales    = bone.scales.size();

        stream.write(reinterpret_cast<const char *>(&num_positions), sizeof(uint32_t));
        if (num_positions > 0)
        {
            stream.write(reinterpret_cast<const char *>(&bone.positions[0]), num_positions*sizeof(glm::vec3));
            stream.write(reinterpret_cast<const char *>(&bone.position_timings[0]), num_positions*sizeof(float));
        }

        stream.write(reinterpret_cast<const char *>(&num_rotations), sizeof(uint32_t));
        if (num_rotations > 0)
        {
            stream.write(reinterpret_cast<const char *>(&bone.rotations[0]), num_rotations*sizeof(glm::quat));
            stream.write(reinterpret_cast<const char *>(&bone.rotation_timings[0]), num_rotations*sizeof(float));
        }

        stream.write(reinterpret_cast<const char *>(&num_scales), sizeof(uint32_t));
        if (num_scales > 0)
        {
            stream.write(reinterpret_cast<const char *>(&bone.scales[0]), num_scales*sizeof(glm::vec3));
            stream.write(reinterpret_cast<const char *>(&bone.scale_timings[0]), num_scales*sizeof(float));
        }

        stream.write(reinterpret_cast<const char *>(&bone.local_transform), sizeof(glm::mat4));
        stream.write(reinterpret_cast<const char *>(&bone.inverse_bind),    sizeof(glm::mat4));
    }


    void write_animation( std::ofstream &stream, idk::Animation &animation )
    {
        float  duration  = animation.m_duration;
        uint32_t num_bones = animation.m_bones.size();
        
        stream.write(reinterpret_cast<const char *>(&duration),  sizeof(float));
        stream.write(reinterpret_cast<const char *>(&num_bones), sizeof(uint32_t));

        for (idk::AnimBone &bone: animation.m_bones)
        {
            write_animbone(stream, bone);
        }
    }


    void write_animations( std::ofstream &stream )
    {
        uint32_t num_animations = m_animations.size();
        stream.write(reinterpret_cast<const char *>(&num_animations), sizeof(uint32_t));

        for (idk::Animation &animation: m_animations)
        {
            write_animation(stream, animation);
        }
    }

};