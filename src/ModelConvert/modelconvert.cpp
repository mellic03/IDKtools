#include "IDKengine.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"



void thang( const aiScene *scene, aiNode *node )
{
    // std::cout << "meshes: " << node->mNumMeshes << "\n";

    for (int i=0; i<node->mNumMeshes; i++)
    {
        int mesh_idx = node->mMeshes[i];

        aiMesh *mesh = scene->mMeshes[mesh_idx];

        int material_idx = mesh->mMaterialIndex;
        aiMaterial *material = scene->mMaterials[material_idx];

        int num_textures = material->GetTextureCount(aiTextureType_DIFFUSE);

        for (int j=0; j<num_textures; j++)
        {
            aiString name;
            material->GetTexture(aiTextureType_DIFFUSE, i, &name);
            
            std::cout << name.C_Str() << "\n";
        }

        std::cout << "\n";
    }


    for (int i=0; i<node->mNumChildren; i++)
    {
        thang(scene, node->mChildren[i]);
    }
}



int main( int argc, char **argv )
{
    std::string input_path  = argv[1];

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(input_path, aiProcess_FlipUVs);

    aiNode *root = scene->mRootNode;

    thang(scene, root);



    return 0;
}

