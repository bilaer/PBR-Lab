#pragma once
#include <string>
#include "geometry.h"
#include <assimp/scene.h>

// Load ply model into mesh object
bool LoadPLYToMesh(const std::string& path, Mesh& mesh, bool flipUVs = true);