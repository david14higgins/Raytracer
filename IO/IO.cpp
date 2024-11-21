#include "IO.h"
#include <fstream>
#include <stdexcept>
#include "../Geometry/Sphere.h"
#include "../Geometry/Cylinder.h"
#include "../Geometry/Triangle.h"
#include "../Lighting/PointLight.h"

Renderer IO::loadRenderer(const std::string& filename, bool useBVH) {
    std::string filepath = "assets/" + filename;
    json j = parseJSONFile(filepath);

    try {
        // Check if required fields exist
        if (!j.contains("rendermode")) {
            throw std::runtime_error("JSON missing 'rendermode' field");
        }
        if (!j.contains("camera")) {
            throw std::runtime_error("JSON missing 'camera' field");
        }
        if (!j.contains("scene")) {
            throw std::runtime_error("JSON missing 'scene' field");
        }

        // Parse number of bounces
        int nbounces = 0;
        if (j.contains("nbounces")) {
            nbounces = j["nbounces"];
        }

        // Parse render mode
        std::string renderModeStr = j["rendermode"];
        Renderer::RenderMode rendermode = parseRenderMode(renderModeStr);

        // Parse camera
        Camera camera = parseCamera(j["camera"]);

        // Parse scene
        Scene scene = loadSceneFromJSON(j["scene"]);

        // Create and return renderer
        return Renderer(nbounces, rendermode, camera, scene, useBVH);
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error in loadRendererDetails: " + std::string(e.what()));
    }
}

json IO::parseJSONFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    try {
        json j;
        file >> j;
        return j;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
    }
}

Material IO::parseMaterial(const json& materialData) {
    bool hasTexture = false;
    std::string textureFilename = "";
    if (materialData.contains("hasTexture")) {
        hasTexture = materialData["hasTexture"];
    }
    if (materialData.contains("textureFilename")) {
        textureFilename = materialData["textureFilename"];
    }

    return Material(
        materialData["ks"],
        materialData["kd"],
        materialData["specularexponent"],
        {materialData["diffusecolor"][0], materialData["diffusecolor"][1], materialData["diffusecolor"][2]},
        {materialData["specularcolor"][0], materialData["specularcolor"][1], materialData["specularcolor"][2]},
        materialData["isreflective"],
        materialData["reflectivity"],
        materialData["isrefractive"],
        materialData["refractiveindex"],
        hasTexture,
        textureFilename
    );
}

Camera IO::parseCamera(const json& cameraData) {
    Camera::CameraType type = Camera::CameraType::PINHOLE;
    return Camera(
        type,
        cameraData["width"],
        cameraData["height"],
        {cameraData["position"][0], cameraData["position"][1], cameraData["position"][2]},
        {cameraData["lookAt"][0], cameraData["lookAt"][1], cameraData["lookAt"][2]},
        {cameraData["upVector"][0], cameraData["upVector"][1], cameraData["upVector"][2]},
        cameraData["fov"],
        cameraData["exposure"]
    );
}

std::vector<std::shared_ptr<Shape>> IO::loadShapesFromJSON(const json& sceneData) {
    std::vector<std::shared_ptr<Shape>> shapes;

    for (const auto& shapeData : sceneData["shapes"]) {
        std::string type = shapeData["type"];
        Material material;
        if (shapeData.contains("material")) {
            material = parseMaterial(shapeData["material"]);
        }

        if (type == "sphere") {
            Vector3 center = {shapeData["center"][0], shapeData["center"][1], shapeData["center"][2]};
            shapes.push_back(std::make_shared<Sphere>(center, shapeData["radius"], material));
        } 
        else if (type == "cylinder") {
            Vector3 center = {shapeData["center"][0], shapeData["center"][1], shapeData["center"][2]};
            Vector3 axis = {shapeData["axis"][0], shapeData["axis"][1], shapeData["axis"][2]};
            shapes.push_back(std::make_shared<Cylinder>(center, axis, shapeData["radius"], shapeData["height"], material));
        } 
        else if (type == "triangle") {
            Vector3 v0 = {shapeData["v0"][0], shapeData["v0"][1], shapeData["v0"][2]};
            Vector3 v1 = {shapeData["v1"][0], shapeData["v1"][1], shapeData["v1"][2]};
            Vector3 v2 = {shapeData["v2"][0], shapeData["v2"][1], shapeData["v2"][2]};
            shapes.push_back(std::make_shared<Triangle>(v0, v1, v2, material));
        }
    }
    return shapes;
}

std::vector<std::shared_ptr<Light>> IO::loadLightsFromJSON(const json& sceneData) {
    std::vector<std::shared_ptr<Light>> lights;

    if (sceneData.contains("lightsources")) {
        for (const auto& lightData : sceneData["lightsources"]) {
            if (lightData["type"] == "pointlight") {
                lights.push_back(std::make_shared<PointLight>(
                    std::array<float, 3>{lightData["position"][0], lightData["position"][1], lightData["position"][2]},
                    std::array<float, 3>{lightData["intensity"][0], lightData["intensity"][1], lightData["intensity"][2]}
                ));
            }
        }
    }
    return lights;
}

Scene IO::loadSceneFromJSON(const json& sceneData) {
    return Scene(
        std::array<float, 3>{sceneData["backgroundcolor"][0], sceneData["backgroundcolor"][1], sceneData["backgroundcolor"][2]},
        loadLightsFromJSON(sceneData),
        loadShapesFromJSON(sceneData)
    );
}

Renderer::RenderMode IO::parseRenderMode(const std::string& renderModeStr) {
    if (renderModeStr == "binary") {
        return Renderer::RenderMode::BINARY;
    } else if (renderModeStr == "phong") {
        return Renderer::RenderMode::PHONG;
    }
    throw std::invalid_argument("Unknown render mode: " + renderModeStr);
}