#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <string>


void idk_readbones( const aiScene *scene );
void idk_loadAnimation( const aiScene *scene, std::string input_path );


