#pragma once

// #include "texture.hh"

// #include <glad/glad.h>

#include "texture.hh"
#include "vertex.hh"

#include "shader.hh"

#include <glad/glad.h>

#include <filesystem>
#include <iostream>
#include <vector>
namespace fs = std::filesystem;

class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<uint> indices;
  std::vector<Texture> textures;

  Mesh(const std::vector<Vertex> &vertices, const std::vector<uint> &indices,
       const std::vector<Texture> &textures);
  Mesh(std::vector<Vertex> &vertices, std::vector<uint> &idices,
       std::vector<Texture> &textures);

  auto draw(const Shader &shader) -> void;

private:
  uint VAO, VBO, EBO;
  auto setupMesh() -> void;
};

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<uint> &indices,
           const std::vector<Texture> &textures)
    : vertices{vertices}, indices{indices}, textures{textures} {
  setupMesh();
}
Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<uint> &indices,
           std::vector<Texture> &textures)
    : vertices{vertices}, indices{indices}, textures{textures} {
  setupMesh();
}

auto Mesh::setupMesh() -> void {
  constexpr auto sizeofVertices{sizeof(decltype(vertices)::value_type)};
  constexpr auto sizeofIndecies{sizeof(decltype(indices)::value_type)};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeofVertices,
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeofIndecies,
               indices.data(), GL_STATIC_DRAW);

  // 0: vertex positions
  // 1: vertex normal
  // 2: vertex texture coords
  for (uint i = 0; i < 3; i++)
    glEnableVertexAttribArray(i);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, position)));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, normal)));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, texCoords)));

  glBindVertexArray(0);
}

auto Mesh::draw(const Shader &shader) -> void {
  uint diffuseN{1}, specularN{1}, normalN{1};

  glBindVertexArray(VAO);
  for (uint i = 0; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);

    std::string name;
    switch (textures[i].type) {
    case Texture::Type::Diffuse:
      name = "texture_diffuse" + std::to_string(diffuseN++);
      // std::clog <<name;
      break;
    case Texture::Type::Specular:
      name = "texture_specular" + std::to_string(specularN++);
      // std::clog <<name;
      break;
    case Texture::Type::Normal:
      name = "texture_normal" + std::to_string(normalN++);
      // std::clog <<name;
      break;
      // case
    default:
      name = "INVALID";
      std::clog << "INVALIND NAME FOR TEXTURE: " << shader.id() << ':' << i
                << ", TYPE: " << textures[i].type << std::endl;
    }

    std::clog << "--> " << "material." + name << ": "
              << glGetUniformLocation(shader.id(), ("material." + name).c_str())
              << std::endl;
    // glUniform1f(glGetUniformLocation(shader.id(), (name).c_str()), i);
    glUniform1f(glGetUniformLocation(shader.id(), ("material." + name).c_str()), i);
    // glUniform1f(shader.getUniform("material." + name), i);
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }
  glActiveTexture(GL_TEXTURE0);

  // std::cout << indices.size() << std::endl;

  // draw mesh
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  // glBindVertexArray(0);
}

// class Mesh {
// public:
//   static constexpr int INVALID_MATERIAL{0xFF'FF'FF'FF};

//   Mesh();
//   ~Mesh();
//   auto loadMesh(const fs::path &file) -> bool;
//   auto render() -> void;

// private:
//   auto initFromScene(const aiScene *pScene, const fs::path &file) -> bool;
//   auto initMaterials(const aiScene *pScene, const fs::path &file) -> bool;
//   auto initMesh(uint idx, const aiMesh *paiMesh) -> void;
//   auto clear() -> void;

//   struct MeshEntry {
//     MeshEntry();
//     ~MeshEntry();

//     auto init(const std::vector &vertices, const std::vector &indices) ->
//     bool; GLuint VB; GLuint IB; uint numIndices; uint materialIndex;
//   };

//   std::vector<MeshEntry> m_Entries;
//   std::vector<Texture&> m_Textures;
// };

// auto Mesh::loadMesh(const fs::path &file) -> bool {
//   // Release previous mesh if loaded
//   clear();

//   Assimp::Importer importer;
//   const aiScene *pScene{importer.ReadFile(
//       file.c_str(), aiProcess_Triangulate | aiProcess_GenNormals |
//                         aiProcess_FlipUVs)};
//   if (pScene)
//     return initFromScene(pScene, file);
//   else
//     std::clog << "Error parsing " << file << ": " <<
//     importer.GetErrorString()
//               << '\n';
//   return false;
// }

// auto Mesh::render() -> void;

// auto Mesh::initFromScene(const aiScene *pScene, const fs::path &file) -> bool
// {
//   m_Entries.resize(pScene->mNumMeshes);
//   m_Textures.resize(pScene->mNumMaterials);
//   // init the meshes in scene
//   for (uint i = 0; i < m_Entries.size(); i++) {
//     const aiMesh *paiMesh{pScene->mMeshes[i]};
//     initMesh(i, paiMesh);
//   }
//   return initMaterials(pScene, file);
// }

// auto Mesh::initMaterials(const aiScene *pScene, const fs::path &file) -> bool
// {} auto Mesh::initMesh(uint idx, const aiMesh *paiMesh) -> void {
//   m_Entries[idx].materialIndex = paiMesh->mMaterialIndex;

//   std::vector vertices;
//   std::vector indices;
//   const aiVector3D zero3D{0, 0, 0};
//   for (uint i=0; i < paiMesh->mNumVertices;i++){
//     const aiVector3D *pPos = paiMesh->mVertices + i;
//     const aiVector3D *pNormal = paiMesh->HasNormals() ? (paiMesh->mVertices +
//     i) : &zero3D; const aiVector3D *pTexCoord =paiMesh->HasTextureCoords(0) ?
//     &(paiMesh->mTextureCoords[0][i]) : &zero3D;

//     Vertex v
// }

// }
// auto Mesh::clear() -> void;
