#pragma once
#include <string>
#include "geometry.h"

bool LoadPLYToMesh(const std::string& path, Mesh& mesh, bool flipUVs = true);