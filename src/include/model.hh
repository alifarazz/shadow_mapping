#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "mesh.hh"
#include "shader.hh"
#include "texture.hh"
#include "vertex.hh"
#include "utils.hh"
#include "texture_repo.hh"

#include <filesystem>
#include <iostream>
#include <vector>
namespace fs = std::filesystem;

class Model {
public:
  Model(const fs::path &file) { loadModel(file); }
  // COPY
  Model(const Model& other) = delete;
  Model& operator=(const Model& other) = delete;
  // MOVE
  Model(Model&& other) noexcept { *this = std::move(other); }
  Model& operator=(Model&& other);

  auto destory() -> void;

  auto draw(Shader &shader, uint offsetTexture, GLenum drawMode) -> void;
  auto drawWithoutVAOBinding(Shader &shader, uint offsetTexture, GLenum drawMode) -> void;
  auto drawWihtoutTextureBinding(GLenum drawMode) -> void ;

  // protected:
  std::vector<Mesh> meshes;
  fs::path directory;

  auto loadModel(const fs::path &file) -> void;
  auto processNode(aiNode *node, const aiScene *scene) -> void;
  auto processMesh(aiMesh *mesh, const aiScene *scene) -> Mesh;
  auto loadMaterialTextures(aiMaterial *mat, aiTextureType type) -> std::vector<Texture>;
};
 
Model& Model::operator=(Model&& other){
  if (this != &other) {
    meshes = std::move(other.meshes);
    directory = std::move(other.directory);
  }
  return *this;
}

auto Model::destory() -> void {
  for (auto &m: meshes)
    m.destory();
}


auto Model::draw(Shader &shader, uint offsetTexture = 0, GLenum drawMode=GL_TRIANGLES) -> void {
  for (uint i = 0; i < meshes.size(); i++)
    meshes[i].bindDraw(shader, offsetTexture, drawMode);
}


auto Model::drawWithoutVAOBinding(Shader &shader, uint offsetTexture = 0, GLenum drawMode=GL_TRIANGLES) -> void {
  for (uint i = 0; i < meshes.size(); i++) {
    auto &m{meshes[i]};
    m.bindTextures(shader, offsetTexture);
    m.draw(drawMode);
  }
}

auto Model::drawWihtoutTextureBinding(GLenum drawMode=GL_TRIANGLES) -> void {
  for (uint i = 0; i < meshes.size(); i++){
    auto& m{meshes[i]};
    m.bindVAO();
    m.draw(drawMode);
  }
}

auto Model::loadModel(const fs::path &file) -> void {
  Assimp::Importer importer;
  const aiScene *scene{importer.ReadFile(
      file.c_str(), aiProcess_Triangulate | aiProcess_GenNormals |
                                // aiProcessPreset_TargetRealtime_Fast|
                        aiProcess_FlipUVs | aiProcess_OptimizeGraph)};
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }
  directory = file.root_path() / file.relative_path(); //
  processNode(scene->mRootNode, scene);
  std::clog << "LOG::Model::\"Loading Model Successful\": "
            << file << std::endl;
}

auto Model::processNode(aiNode *node, const aiScene *scene) -> void {
  // process meshes
  for (uint i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh{scene->mMeshes[node->mMeshes[i]]};
    meshes.push_back(processMesh(mesh, scene));
  }
  // procss my children
  for (uint i = 0; i < node->mNumChildren; i++)
    processNode(node->mChildren[i], scene);
}

auto Model::processMesh(aiMesh *mesh, const aiScene *scene) -> Mesh {
  std::vector<Vertex> vertices(mesh->mNumVertices);
  std::vector<uint> indices;
  std::vector<Texture> textures;

  for (uint i = 0; i < mesh->mNumVertices; i++) {
    auto &u{vertices[i]};
    auto &v{mesh->mVertices[i]};
    auto &n{mesh->mNormals[i]};

    u.position = glm::vec3{v.x, v.y, v.z};
    u.normal = glm::vec3{n.x, n.y, n.z};

    // I'll use only the first texture
    if (mesh->mTextureCoords[0]) { // does mesh have any tex coords
      auto &t{mesh->mTextureCoords[0][i]};
      u.texCoords = glm::vec2{t.x, t.y};
    } else
      u.texCoords = glm::vec2{0, 0};
  }
  for (uint i = 0; i < mesh->mNumFaces; i++) { // HACK: bad allocations
    auto &face{mesh->mFaces[i]};
    for (uint j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }
  if (mesh->mMaterialIndex != 0) {
    auto *material{scene->mMaterials[mesh->mMaterialIndex]};
    std::vector<Texture> diffuseMaps{
        loadMaterialTextures(material, aiTextureType_DIFFUSE)};
    textures.insert(end(textures), begin(diffuseMaps), end(diffuseMaps));
    std::vector<Texture> specularMaps{
        loadMaterialTextures(material, aiTextureType_SPECULAR)};
    textures.insert(end(textures), begin(specularMaps), end(specularMaps));
  }
  // std::clog << "Vertices: " << vertices.size() << std::endl
            // << "Indices: " << indices.size() << std::endl
            // << "Textures: " << textures.size() << std::endl;

  // for (std::size_t i = 0; i < 200; i++) {
  //   auto &x = vertices[i];
  //   std::clog << "(" << x.position.x << ", " << x.position.y << ", "
  //             << x.position.z << ")" << std::endl;
  // }
  // for (std::size_t i = 0; i < 200; i++) {
  //   auto &x = indices[i];
  //   std::clog << x << ", ";
  // }
  // std::clog << std::endl;
  return Mesh{vertices, indices, textures}; // Copy-Ellison
}

auto Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type)
  -> std::vector<Texture> {
  auto n{mat->GetTextureCount(type)};
  std::vector<Texture> texes;
  texes.resize(n);

  // std::clog << "[Model::loadMaterialTextures]Texture Count: " << n << std::endl;
  for (uint i = 0; i < n; i++) {
    aiString str;
    auto &t{texes[i]};
    mat->GetTexture(type, i, &str);
    auto path = fs::path(directory).parent_path() / fs::path(str.C_Str());
    t.id = DefaultTexRepo.get(path);
    switch (type) {
    case aiTextureType_SPECULAR:
      t.type = Texture::Type::Specular;
      break;
    case aiTextureType_DIFFUSE:
      t.type = Texture::Type::Diffuse;
      break;
    case aiTextureType_NORMALS:
      t.type = Texture::Type::Normal;
      break;
    default:
      t.type = Texture::Type::None;
    }
  }
  return texes;
}
