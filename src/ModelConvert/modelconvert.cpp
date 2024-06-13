#include <IDKGameEngine/IDKGameEngine.hpp>

#include "loader.hpp"
#include <iomanip>

#include <filesystem>
namespace fs = std::filesystem;


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

    std::string output_path = fs::path(input_path).replace_extension("idkvi");
    std::ofstream stream(output_path, std::ios::binary);

    if (scene->HasAnimations())
    {
        Loader<idk::Vertex_P_N_T_UV_SKINNED> loader;
        loader.process(scene);
        loader.write(stream);
        loader.writeSkeleton(stream);
        loader.writeAnimations(stream);
    }

    else
    {
        Loader<idk::Vertex_P_N_T_UV> loader;
        loader.process(scene);
        loader.write(stream);
    }

    stream.close();

    return 0;
}

