#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/constants.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <map>

namespace cg {

// Struct for representing a virtual 3D trackball that can be used for
// object or camera rotation
struct Trackball {
    double radius;
    glm::vec2 center;
    bool tracking;
    glm::vec3 vStart;
    glm::quat qStart;
    glm::quat qCurrent;

    Trackball() : radius(1.0),
                  center(glm::vec2(0.0f, 0.0f)),
                  tracking(false),
                  vStart(glm::vec3(0.0f, 0.0f, 1.0f)),
                  qStart(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
                  qCurrent(glm::quat(1.0f, 0.0f, 0.0f, 0.0f))
    {}
};

// Struct for Wavefront (OBJ) triangle meshes that are indexed and has
// per-vertex normals
struct OBJMesh {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<std::uint32_t> indices;
};

// Struct for Wavefront (OBJ) triangle meshes that are indexed and has
// per-vertex normals and UV texture coordinates
struct OBJMeshUV {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> texcoords;
    std::vector<std::uint32_t> indices;
};

// Helper functions
namespace {
glm::vec3 mapMousePointToUnitSphere(glm::vec2 point, double radius, glm::vec2 center)
{
    float x = point[0] - center[0];
    float y = -point[1] + center[1];
    float z = 0.0f;
    if (x * x + y * y < radius * radius / 2.0f) {
        z = std::sqrt(radius * radius - (x * x + y * y));
    }
    else {
        z = (radius * radius / 2.0f) / std::sqrt(x * x + y * y);
    }
    return glm::normalize(glm::vec3(x, y, z));
}

void computeNormals(const std::vector<glm::vec3> &vertices,
                    const std::vector<std::uint32_t> &indices,
                    std::vector<glm::vec3> *normals)
{
    normals->resize(vertices.size(), glm::vec3(0.0f, 0.0f, 0.0f));

    // Compute per-vertex normals by averaging the unnormalized face normals
    std::uint32_t vertexIndex0, vertexIndex1, vertexIndex2;
    glm::vec3 normal;
    int numIndices = indices.size();
    for (int i = 0; i < numIndices; i += 3) {
        vertexIndex0 = indices[i];
        vertexIndex1 = indices[i + 1];
        vertexIndex2 = indices[i + 2];
        normal = glm::cross(vertices[vertexIndex1] - vertices[vertexIndex0],
                            vertices[vertexIndex2] - vertices[vertexIndex0]);
        (*normals)[vertexIndex0] += normal;
        (*normals)[vertexIndex1] += normal;
        (*normals)[vertexIndex2] += normal;
    }

    int numNormals = normals->size();
    for (int i = 0; i < numNormals; i++) {
        (*normals)[i] = glm::normalize((*normals)[i]);
    }
}
} // namespace

// Start trackball tracking
static void trackballStartTracking(Trackball &trackball, glm::vec2 point)
{
    trackball.vStart = mapMousePointToUnitSphere(point, trackball.radius, trackball.center);
    trackball.qStart = glm::quat(trackball.qCurrent);
    trackball.tracking = true;
}

// Stop trackball tracking
static void trackballStopTracking(Trackball &trackball)
{
    trackball.tracking = false;
}

// Rotate trackball from, e.g., mouse movement
static void trackballMove(Trackball &trackball, glm::vec2 point)
{
    glm::vec3 vCurrent = mapMousePointToUnitSphere(point, trackball.radius, trackball.center);
    glm::vec3 rotationAxis = glm::cross(trackball.vStart, vCurrent);
    float dotProduct = std::max(std::min(glm::dot(trackball.vStart, vCurrent), 1.0f), -1.0f);
    float rotationAngle = std::acos(dotProduct);
    float eps = 0.01f;
    if (rotationAngle < eps) {
        trackball.qCurrent = glm::quat(trackball.qStart);
    }
    else {
        // Note: here we provide rotationAngle in radians. Older versions
        // of GLM (0.9.3 or earlier) require the angle in degrees.
        glm::quat q = glm::angleAxis(rotationAngle, rotationAxis);
        q = glm::normalize(q);
        trackball.qCurrent = glm::normalize(glm::cross(q, trackball.qStart));
    }
}

// Get trackball orientation in matrix form
static glm::mat4 trackballGetRotationMatrix(Trackball &trackball)
{
    return glm::mat4_cast(trackball.qCurrent);
}

// Read an OBJMesh from an .obj file
static bool objMeshLoad(OBJMesh &mesh, const std::string &filename)
{
    const std::string VERTEX_LINE("v ");
    const std::string FACE_LINE("f ");

    // Open OBJ file
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        std::cerr << "Could not open " << filename << std::endl;
        return false;
    }

    // Extract vertices and indices
    std::string line;
    glm::vec3 vertex;
    std::uint32_t vertexIndex0, vertexIndex1, vertexIndex2;
    while (!f.eof()) {
        std::getline(f, line);
        if (line.substr(0, 2) == VERTEX_LINE) {
            std::istringstream vertexLine(line.substr(2));
            vertexLine >> vertex.x;
            vertexLine >> vertex.y;
            vertexLine >> vertex.z;
            mesh.vertices.push_back(vertex);
        }
        else if (line.substr(0, 2) == FACE_LINE) {
            std::istringstream faceLine(line.substr(2));
            faceLine >> vertexIndex0;
            faceLine >> vertexIndex1;
            faceLine >> vertexIndex2;
            mesh.indices.push_back(vertexIndex0 - 1);
            mesh.indices.push_back(vertexIndex1 - 1);
            mesh.indices.push_back(vertexIndex2 - 1);
        }
        else {
            // Ignore line
        }
    }

    // Close OBJ file
    f.close();

    // Compute normals
    computeNormals(mesh.vertices, mesh.indices, &mesh.normals);

    // Display log message
    std::cout << "Loaded OBJ file " << filename << std::endl;
    int numTriangles = mesh.indices.size() / 3;
    std::cout << "Number of triangles: " << numTriangles << std::endl;

    return true;
}

struct uvec3Less {
    bool operator() (const glm::uvec3 &a, const glm::uvec3 &b) const
    {
        return (a.x < b.x) |
               ((a.x == b.x) & (a.y < b.y)) |
               ((a.x == b.x) & (a.y == b.y) & (a.z < b.z));
    }
};

// Read an OBJMeshUV from an .obj file. This function can read texture
// coordinates and/or normals, in addition to vertex positions.
static bool objMeshUVLoad(OBJMeshUV &mesh, const std::string &filename)
{
    const std::string VERTEX_LINE("v ");
    const std::string TEXCOORD_LINE("vt ");
    const std::string NORMAL_LINE("vn ");
    const std::string FACE_LINE("f ");

    std::string line;
    glm::vec3 vertex;
    glm::vec3 normal;
    glm::vec3 texcoord;
    std::uint32_t vindex[3];
    std::uint32_t tindex[3];
    std::uint32_t nindex[3];

    // Open OBJ file
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        std::cerr << "Could not open " << filename << std::endl;
        return false;
    }

    // First pass: read vertex data into temporary mesh
    OBJMeshUV tmp_mesh;
    while (!f.eof()) {
        std::getline(f, line);
        if (line.substr(0, 2) == VERTEX_LINE) {
            std::istringstream ss(line.substr(2));
            ss >> vertex.x >> vertex.y >> vertex.z;
            tmp_mesh.vertices.push_back(vertex);
        }
        else if (line.substr(0, 3) == TEXCOORD_LINE) {
            std::istringstream ss(line.substr(3));
            ss >> texcoord.x >> texcoord.y >> texcoord.z;
            tmp_mesh.texcoords.push_back(texcoord);
        }
        else if (line.substr(0, 3) == NORMAL_LINE) {
            std::istringstream ss(line.substr(3));
            ss >> normal.x >> normal.y >> normal.z;
            tmp_mesh.normals.push_back(normal);
        }
        else {
            // Ignore line
        }
    }

    // Rewind file
    f.clear();
    f.seekg(0);

    // Clear old mesh and pre-allocate space for new mesh data
    mesh.vertices.clear();
    mesh.vertices.reserve(tmp_mesh.vertices.size());
    mesh.texcoords.clear();
    mesh.texcoords.reserve(tmp_mesh.texcoords.size());
    mesh.normals.clear();
    mesh.normals.reserve(tmp_mesh.normals.size());
    mesh.indices.clear();

    // Set up dictionary for mapping unique tuples to indices
    std::map<glm::uvec3, unsigned, uvec3Less> visited;
    unsigned next_index = 0;
    glm::uvec3 key;

    // Second pass: read faces and construct per-vertex texcoords/normals.
    // Note: OBJ-indices start at one, so we need to subtract indices by one.
    while (!f.eof()) {
        std::getline(f, line);
        if (line.substr(0, 2) == FACE_LINE) {
            if (std::sscanf(line.c_str(), "f %d %d %d",
                            &vindex[0], &vindex[1], &vindex[2]) == 3) {
                for (unsigned i = 0; i < 3; ++i) {
                    key = glm::uvec3(vindex[i], 0, 0);
                    if (visited.count(key) == 0) {
                        visited[key] = next_index++;
                        mesh.vertices.push_back(tmp_mesh.vertices[vindex[i] - 1]);
                    }
                    mesh.indices.push_back(visited[key]);
                }
            }
            else if (std::sscanf(line.c_str(), "f %d/%d %d/%d %d/%d",
                                 &vindex[0], &tindex[0],
                                 &vindex[1], &tindex[1],
                                 &vindex[2], &tindex[2]) == 6) {
                for (unsigned i = 0; i < 3; ++i) {
                    key = glm::uvec3(vindex[i], tindex[i], 0);
                    if (visited.count(key) == 0) {
                        visited[key] = next_index++;
                        mesh.vertices.push_back(tmp_mesh.vertices[vindex[i] - 1]);
                        mesh.texcoords.push_back(tmp_mesh.texcoords[tindex[i] - 1]);
                    }
                    mesh.indices.push_back(visited[key]);
                }
            }
            else if (std::sscanf(line.c_str(), "f %d//%d %d//%d %d//%d",
                                 &vindex[0], &nindex[0],
                                 &vindex[1], &nindex[1],
                                 &vindex[2], &nindex[2]) == 6) {
                for (unsigned i = 0; i < 3; ++i) {
                    key = glm::uvec3(vindex[i], nindex[i], 0);
                    if (visited.count(key) == 0) {
                        visited[key] = next_index++;
                        mesh.vertices.push_back(tmp_mesh.vertices[vindex[i] - 1]);
                        mesh.normals.push_back(tmp_mesh.normals[nindex[i] - 1]);
                    }
                    mesh.indices.push_back(visited[key]);
                }
            }
            else if (std::sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                 &vindex[0], &tindex[0], &nindex[0],
                                 &vindex[1], &tindex[1], &nindex[1],
                                 &vindex[2], &tindex[2], &nindex[2]) == 9) {
                for (unsigned i = 0; i < 3; ++i) {
                    key = glm::uvec3(vindex[i], tindex[i], nindex[i]);
                    if (visited.count(key) == 0) {
                        visited[key] = next_index++;
                        mesh.vertices.push_back(tmp_mesh.vertices[vindex[i] - 1]);
                        mesh.texcoords.push_back(tmp_mesh.texcoords[tindex[i] - 1]);
                        mesh.normals.push_back(tmp_mesh.normals[nindex[i] - 1]);
                    }
                    mesh.indices.push_back(visited[key]);
                }
            }
        }
    }

    // Compute normals (if OBJ-file did not contain normals)
    if (mesh.normals.size() == 0) {
        computeNormals(mesh.vertices, mesh.indices, &mesh.normals);
    }

    // Display log message
    std::cout << "Loaded OBJ file " << filename << std::endl;
    int numTriangles = mesh.indices.size() / 3;
    std::cout << "Number of triangles: " << numTriangles << std::endl;

    return true;
}

}  // namespace cg
