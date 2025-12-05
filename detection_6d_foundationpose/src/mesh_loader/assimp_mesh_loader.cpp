#include "detection_6d_foundationpose/mesh_loader.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <filesystem>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <glog/logging.h>
#include <glog/log_severity.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
namespace utils {
namespace algorithm {
class Ritter_bounding_sphere {
public:
  Ritter_bounding_sphere(aiVector3D *array_of_vertexes, uint16_t no_of_vertexes):bounding_sphere_radius(0.0f)
  {
    att_object_vertexes.resize(no_of_vertexes);
    for (uint32_t vertex_count = 0; vertex_count < no_of_vertexes; vertex_count++)
    {
      att_object_vertexes[vertex_count]=array_of_vertexes[vertex_count];
    }
  }

  std::pair<aiVector3D, float> get_approximation_bounding_sphere()
  {
    initial_assumption();
    update_bounding_sphere();
    return {bounding_sphere_center, bounding_sphere_radius};
  }

private:
  std::vector<aiVector3D> att_object_vertexes;
  aiVector3D              bounding_sphere_center;
  float                   bounding_sphere_radius;
  void                    initial_assumption()
  {
    // Step 1: Pick an initial point (first vertex)
    aiVector3D p0 = att_object_vertexes[0];
    // Step 2: Find the farthest point from p0
    aiVector3D p1       = p0;
    float      maxDist2 = 0.0f;
    for (const auto &vertices : att_object_vertexes)
    {
      aiVector3D diff  = vertices - p0;
      float      dist2 = diff.SquareLength();
      if (dist2 > maxDist2)
      {
        maxDist2 = dist2;
        p1       = vertices;
      }
    }
    // Step 3: Find the farthest point from p1
    aiVector3D p2 = p1;
    maxDist2      = 0.0f;
    for (const auto &vertices : att_object_vertexes)
    {
      aiVector3D diff  = vertices - p1;
      float      dist2 = diff.SquareLength();
      if (dist2 > maxDist2)
      {
        maxDist2 = dist2;
        p2       = vertices;
      }
    }

    // Step 4: Initialize sphere center and radius
    bounding_sphere_center = (p1 + p2) * 0.5f;
    bounding_sphere_radius = (p2 - p1).Length() * 0.5f;
  }

  void update_bounding_sphere()
  {
    // Step 5: Adjust sphere to include all vertices
    for (const auto &vertices : att_object_vertexes)
    {
      aiVector3D diff = vertices - bounding_sphere_center;
      float      dist = diff.Length();
      if (dist > bounding_sphere_radius)
      {
        // Move center toward vertex
        float shift = (dist - bounding_sphere_radius) / (2.0f * dist);
        bounding_sphere_center += diff * shift;
        bounding_sphere_radius = (bounding_sphere_radius + dist) * 0.5f;
      }
    }
  }
};

}; // namespace algorithm
double roundToDecimals(double value, int decimals)
{
  double factor = std::pow(10.0, decimals);
  return std::round(value * factor) / factor;
}
}; // namespace utils

namespace detection_6d {

static std::pair<Eigen::Vector3f, Eigen::Vector3f> FindMinMaxVertex(const aiMesh *mesh)
{
  Eigen::Vector3f min_vertex = {0, 0, 0};
  Eigen::Vector3f max_vertex = {0, 0, 0};

  if (mesh->mNumVertices == 0)
  {
    return std::pair{min_vertex, max_vertex};
  }

  min_vertex << mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z;
  max_vertex << mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z;

  // Iterate over all vertices to find the bounding box
  for (size_t v = 0; v < mesh->mNumVertices; v++)
  {
    float vx = mesh->mVertices[v].x;
    float vy = mesh->mVertices[v].y;
    float vz = mesh->mVertices[v].z;

    min_vertex[0] = std::min(min_vertex[0], vx);
    min_vertex[1] = std::min(min_vertex[1], vy);
    min_vertex[2] = std::min(min_vertex[2], vz);

    max_vertex[0] = std::max(max_vertex[0], vx);
    max_vertex[1] = std::max(max_vertex[1], vy);
    max_vertex[2] = std::max(max_vertex[2], vz);
  }
  return std::pair{min_vertex, max_vertex};
}

static float CalcMeshDiameter(const aiMesh *mesh)
{
  if (!mesh || mesh->mNumVertices == 0)
    return 0.0f;
  const auto& [c,r]=utils::algorithm::Ritter_bounding_sphere(mesh->mVertices,mesh->mNumVertices).get_approximation_bounding_sphere();
  // Approximate diameter = 2 * radius
  return (2.0f * r);
}

static void ComputeOBB(const aiMesh    *mesh,
                       Eigen::Matrix4f &out_orient_bbox,
                       Eigen::Vector3f &out_dimension)
{
  std::vector<Eigen::Vector3f> vertices;
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
  {
    vertices.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
  }

  // 计算质心
  Eigen::Vector3f mean = Eigen::Vector3f::Zero();
  for (const auto &v : vertices)
  {
    mean += v;
  }
  mean /= vertices.size();

  // 计算协方差矩阵
  Eigen::Matrix3f cov = Eigen::Matrix3f::Zero();
  for (const auto &v : vertices)
  {
    Eigen::Vector3f diff = v - mean;
    cov += diff * diff.transpose();
  }
  cov /= vertices.size();

  // 特征分解
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(cov);
  Eigen::Matrix3f rotation = solver.eigenvectors();
  Eigen::Vector3f extents  = solver.eigenvalues().cwiseSqrt();

  // 将顶点变换到旋转后的坐标系
  Eigen::MatrixXf transformed(vertices.size(), 3);
  for (int i = 0; i < vertices.size(); ++i)
  {
    Eigen::Vector3f proj = rotation.transpose() * (vertices[i] - mean);
    transformed(i, 0) = proj(0);
    transformed(i, 1) = proj(1);
    transformed(i, 2) = proj(2);
  }

  // 计算边界框
  Eigen::Vector3f minBound = transformed.colwise().minCoeff();
  Eigen::Vector3f maxBound = transformed.colwise().maxCoeff();
  Eigen::Vector3f dimension = maxBound - minBound;

  // 计算OBB中心（在旋转后的坐标系中）
  Eigen::Vector3f center = (minBound + maxBound) / 2.0f;

  // 构建变换矩阵：从OBB坐标系到模型坐标系
  Eigen::Matrix4f to_origin = Eigen::Matrix4f::Identity();
  to_origin.block<3, 3>(0, 0) = rotation;
  to_origin.block<3, 1>(0, 3) = rotation * center + mean;

  out_orient_bbox = to_origin;
  out_dimension   = dimension;
}

class AssimpMeshLoader : public BaseMeshLoader {
public:
  AssimpMeshLoader(const std::string &name, const std::string &mesh_file_path);

  ~AssimpMeshLoader() = default;

  std::string GetName() const noexcept override;

  float GetMeshDiameter() const noexcept override;

  size_t GetMeshNumVertices() const noexcept override;

  size_t GetMeshNumFaces() const noexcept override;

  const std::vector<Eigen::Vector3f> &GetMeshVertices() const noexcept override;

  const std::vector<Eigen::Vector3f> &GetMeshVertexNormals() const noexcept override;

  const std::vector<Eigen::Vector3f> &GetMeshTextureCoords() const noexcept override;

  const std::vector<Vector3ui> &GetMeshTriangleFaces() const noexcept override;

  const Eigen::Vector3f &GetMeshModelCenter() const noexcept override;

  const Eigen::Matrix4f &GetOrientBounds() const noexcept override;

  const Eigen::Vector3f &GetObjectDimension() const noexcept override;

  const cv::Mat &GetTextureMap() const noexcept override;

private:
  std::string                  name_;
  float                        mesh_diamter_;
  Eigen::Vector3f              mesh_center_;
  std::vector<Eigen::Vector3f> vertices_;
  std::vector<Eigen::Vector3f> vertex_normals_;
  std::vector<Eigen::Vector3f> texcoords_;
  std::vector<Vector3ui>       faces_;
  Eigen::Matrix4f              obb_;
  Eigen::Vector3f              dim_;
  cv::Mat                      texture_map_;
};

AssimpMeshLoader::AssimpMeshLoader(const std::string &name, const std::string &mesh_file_path)
    : name_(name)
{
  if (mesh_file_path.empty())
  {
    throw std::invalid_argument("[AssimpMeshLoader] Got empty mesh_file_path !");
  }

  Assimp::Importer importer;
  const aiScene   *scene =
      importer.ReadFile(mesh_file_path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                                            aiProcess_SortByPType);
  if (scene == nullptr)
  {
    throw std::runtime_error("[AssimpMeshLoader] Failed to read mesh file: " + mesh_file_path);
  }

  const aiMesh *mesh = scene->mMeshes[0];
  mesh_diamter_      = CalcMeshDiameter(mesh);
  ComputeOBB(mesh, obb_, dim_);
  auto min_max_vertex = FindMinMaxVertex(mesh);
  mesh_center_        = (min_max_vertex.second + min_max_vertex.first) / 2.0;

  // Walk through each of the mesh's vertices
  for (unsigned int v = 0; v < mesh->mNumVertices; v++)
  {
    Eigen::Vector3f vertice{mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z};
    vertices_.push_back(vertice);

    Eigen::Vector3f normal{mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z};
    vertex_normals_.push_back(normal);

    Eigen::Vector3f tex_coord{0.0, 0.0, 0.0};
    if (mesh->mTextureCoords[0] != nullptr)
    {
      tex_coord = Eigen::Vector3f{mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y,mesh->mTextureCoords[0][v].z};
    }
    texcoords_.push_back(tex_coord);
  }

  for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
  {
    Vector3ui face{mesh->mFaces[f].mIndices[0], mesh->mFaces[f].mIndices[1],
                   mesh->mFaces[f].mIndices[2]};
    faces_.push_back(face);
  }

  std::string texture_map_path;

  auto material = scene->mMaterials[mesh->mMaterialIndex];
  aiString ai_texture_map_path;
  if (material != nullptr && material->GetTexture(aiTextureType_DIFFUSE, 0, &ai_texture_map_path) == AI_SUCCESS)
  {
    texture_map_path = (std::filesystem::path(mesh_file_path).parent_path() / ai_texture_map_path.C_Str()).string();
    LOG(INFO) << "[AssimpMeshLoader] Using texture map filepath from mesh : " << texture_map_path;
  }
  texture_map_ = cv::imread(texture_map_path);
  if (texture_map_.empty())
  {
    LOG(WARNING) << "[AssimpMeshLoader] Got invalid texture_map_path: " << texture_map_path
                 << ", using default texture map!";
    texture_map_ = cv::Mat(2, 2, CV_8UC3, {100, 100, 100});
  }
  cv::cvtColor(texture_map_, texture_map_, cv::COLOR_BGR2RGB);

  LOG(INFO) << "Successfully Loaded textured mesh file!!!";
  LOG(INFO) << "Mesh has vertices_num: " << vertices_.size() << ", diameter: " << mesh_diamter_
            << ", faces_num: " << faces_.size() << ", center: " << mesh_center_;
}

std::string AssimpMeshLoader::GetName() const noexcept
{
  return name_;
}

float AssimpMeshLoader::GetMeshDiameter() const noexcept
{
  return mesh_diamter_;
}

size_t AssimpMeshLoader::GetMeshNumVertices() const noexcept
{
  return vertices_.size();
}

size_t AssimpMeshLoader::GetMeshNumFaces() const noexcept
{
  return faces_.size();
}

const std::vector<Eigen::Vector3f> &AssimpMeshLoader::GetMeshVertices() const noexcept
{
  return vertices_;
}

const std::vector<Eigen::Vector3f> &AssimpMeshLoader::GetMeshVertexNormals() const noexcept
{
  return vertex_normals_;
}

const std::vector<Eigen::Vector3f> &AssimpMeshLoader::GetMeshTextureCoords() const noexcept
{
  return texcoords_;
}

const std::vector<BaseMeshLoader::Vector3ui> &AssimpMeshLoader::GetMeshTriangleFaces()
    const noexcept
{
  return faces_;
}

const Eigen::Vector3f &AssimpMeshLoader::GetMeshModelCenter() const noexcept
{
  return mesh_center_;
}

const Eigen::Matrix4f &AssimpMeshLoader::GetOrientBounds() const noexcept
{
  return obb_;
}

const Eigen::Vector3f &AssimpMeshLoader::GetObjectDimension() const noexcept
{
  return dim_;
}

const cv::Mat &AssimpMeshLoader::GetTextureMap() const noexcept
{
  return texture_map_;
}

std::shared_ptr<BaseMeshLoader> CreateAssimpMeshLoader(const std::string &name,
                                                       const std::string &mesh_file_path)
{
  return std::make_shared<AssimpMeshLoader>(name, mesh_file_path);
}

} // namespace detection_6d
