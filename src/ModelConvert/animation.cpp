#include "animation.hpp"
#include <IDKengine/IDKengine.hpp>




static std::vector<glm::mat4> all_bones;


void
idk_readbones( const aiScene *scene )
{
    for (int i=0; i<scene->mNumMeshes; i++)
    {
        int current_mesh = i;
        aiMesh *mesh = scene->mMeshes[i];

        std::cout << "bones: " << mesh->mNumBones << "\n";

        for (int j=0; j<mesh->mNumBones; j++)
        {
            aiBone *bone = mesh->mBones[j];

            for (int w=0; w<bone->mNumWeights; w++)
            {
                aiVertexWeight weight = bone->mWeights[w];
            }

            std::cout << bone->mName.C_Str() << "\n";
        }

        std::cout << "\n";
    }
}


void
idk_loadAnimation( const aiScene *scene, std::string input_path )
{
    idk_readbones(scene);

}


