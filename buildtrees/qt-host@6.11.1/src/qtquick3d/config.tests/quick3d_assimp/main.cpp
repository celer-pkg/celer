// Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
// SPDX-License-Identifier: BSD-3-Clause

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/anim.h>
#include <assimp/material.h>
#include <assimp/GltfMaterial.h>
#include <assimp/camera.h>

int main(int , char **)
{
  Assimp::Importer importer;
  Assimp::DefaultLogger::create("AssimpLog.txt", Assimp::Logger::VERBOSE);

  // SET THIS TO REMOVE POINTS AND LINES -> HAVE ONLY TRIANGLES
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE|aiPrimitiveType_POINT);

  // Check for new version by setting animation morph channels.
  aiAnimation anim;
  anim.mNumMorphMeshChannels = 0;

  // Check for something that was set in a recent version
  aiMaterial aiM;
  (void)aiM.GetName();

  // Chack that the version includes the new camera member
  aiCamera aiC;
  aiC.mOrthographicWidth = 0.0;

  return 0;
}
