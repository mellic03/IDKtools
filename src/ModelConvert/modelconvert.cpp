#include <IDKGameEngine/IDKGameEngine.hpp>

#include "loader.hpp"
#include <iomanip>


int main( int argc, char **argv )
{
    std::string input_path  = argv[1];

    uint32_t flags  = aiProcess_FlipUVs;
             flags |= aiProcess_OptimizeMeshes;
             flags |= aiProcess_OptimizeGraph;
             flags |= aiProcess_CalcTangentSpace;
             flags |= aiProcess_LimitBoneWeights;
             flags |= aiProcess_PopulateArmatureData;
            //  flags |= aiProcess_GenBoundingBoxes;


    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(input_path, flags);

    // if (scene->HasAnimations())
    // {
    //     Loader<idk::AnimatedVertex> loader;
    //     loader.process(scene);
    //     loader.write(input_path);
    // }

    // else
    // {
        Loader<idk::Vertex> loader;
        loader.process(scene);
        loader.write(input_path);
    // }


    return 0;
}

