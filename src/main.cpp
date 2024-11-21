#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <chrono>
#include "../Geometry/Shape.h"
#include "../Geometry/Cylinder.h"
#include "../Geometry/Sphere.h"
#include "../Geometry/Triangle.h"
#include "../Geometry/Vector3.h"
#include "../Materials/Material.h"
#include "../Core/Camera.h"
#include "../Lighting/Light.h"
#include "../Lighting/PointLight.h"
#include "../Core/Scene.h"
#include "../Core/Renderer.h"

using json = nlohmann::json;

Material parseMaterial(const json& materialData) {
    bool hasTexture = false;
    std::string textureFilename = "";
    if (materialData.contains("hasTexture")) {
        hasTexture = materialData["hasTexture"];
    }
    if (materialData.contains("textureFilename")) {
        textureFilename = materialData["textureFilename"];
    }
    //std::cout << "Renderer details:\n" << renderer.toString() << std::endl;
    Material material(
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
    
    //std::cout << "Created material:\n" << material.toString() << std::endl;
    return material;
}

Camera parseCamera(const json& cameraData) {
    // Parse camera data from JSON
    Camera::CameraType type = Camera::CameraType::PINHOLE; // Only pinhole camera supported for now
    int width = cameraData["width"];
    int height = cameraData["height"];
    std::array<float, 3> position = {cameraData["position"][0], cameraData["position"][1], cameraData["position"][2]};
    std::array<float, 3> lookAt = {cameraData["lookAt"][0], cameraData["lookAt"][1], cameraData["lookAt"][2]};
    std::array<float, 3> upVector = {cameraData["upVector"][0], cameraData["upVector"][1], cameraData["upVector"][2]};
    float fov = cameraData["fov"];
    float exposure = cameraData["exposure"];

    return Camera(type, width, height, position, lookAt, upVector, fov, exposure);
}

std::vector<std::shared_ptr<Shape>> loadShapesFromJSON(const json& sceneData) {
    std::vector<std::shared_ptr<Shape>> shapes;

    // Parse the shapes from the JSON and create corresponding objects
    for (const auto& shapeData : sceneData["shapes"]) {
        std::string type = shapeData["type"];

        // Check the type and create the corresponding shape
        if (type == "sphere") {
            Vector3 center = {shapeData["center"][0], shapeData["center"][1], shapeData["center"][2]};
            float radius = shapeData["radius"];
            Material material;
            if (shapeData.contains("material")) {
                material = parseMaterial(shapeData["material"]);
            }
            shapes.push_back(std::make_shared<Sphere>(center, radius, material));

        } else if (type == "cylinder") {
            Vector3 center = {shapeData["center"][0], shapeData["center"][1], shapeData["center"][2]};
            Vector3 axis = {shapeData["axis"][0], shapeData["axis"][1], shapeData["axis"][2]};
            float radius = shapeData["radius"];
            float height = shapeData["height"];
            Material material;
            if (shapeData.contains("material")) {
                material = parseMaterial(shapeData["material"]);
            }
            shapes.push_back(std::make_shared<Cylinder>(center, axis, radius, height, material));

        } else if (type == "triangle") {
            Vector3 v0 = {shapeData["v0"][0], shapeData["v0"][1], shapeData["v0"][2]};
            Vector3 v1 = {shapeData["v1"][0], shapeData["v1"][1], shapeData["v1"][2]};
            Vector3 v2 = {shapeData["v2"][0], shapeData["v2"][1], shapeData["v2"][2]};
            Material material;
            if (shapeData.contains("material")) {
                material = parseMaterial(shapeData["material"]);
            }
            shapes.push_back(std::make_shared<Triangle>(v0, v1, v2, material));
        }
    } 
    return shapes;
}



std::vector<std::shared_ptr<Light>> loadLightsFromJSON(const json& sceneData) {
    std::vector<std::shared_ptr<Light>> lights;

    if (sceneData.contains("lightsources")) {
        // Parse the lights from the JSON
        for (const auto& lightData : sceneData["lightsources"]) {
            std::string type = lightData["type"];
            
            if (type == "pointlight") {
                std::array<float, 3> position = {lightData["position"][0], lightData["position"][1], lightData["position"][2]};
                std::array<float, 3> intensity = {lightData["intensity"][0], lightData["intensity"][1], lightData["intensity"][2]};
                lights.push_back(std::make_shared<PointLight>(position, intensity));
            }
        }
    }

    return lights;
}

Scene loadSceneFromJSON(const json& sceneData) {
    std::array<float, 3> backgroundColor = {sceneData["backgroundcolor"][0], sceneData["backgroundcolor"][1], sceneData["backgroundcolor"][2]};
    std::vector<std::shared_ptr<Shape>> shapes = loadShapesFromJSON(sceneData);
    std::vector<std::shared_ptr<Light>> lights = loadLightsFromJSON(sceneData);

    return Scene(backgroundColor, lights, shapes);
}

Renderer::RenderMode parseRenderMode(const std::string& renderModeStr) {
    if (renderModeStr == "binary") {
        return Renderer::RenderMode::BINARY;
    } else if (renderModeStr == "phong") {
        return Renderer::RenderMode::PHONG;
    } else {
        throw std::invalid_argument("Unknown render mode: " + renderModeStr);
    }
}

Renderer loadRendererDetails(const json& j) {
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
        return Renderer(nbounces, rendermode, camera, scene);
    }
    catch (const std::exception& e) {
        std::cerr << "Error in loadRendererDetails: " << e.what() << std::endl;
        throw; // Re-throw the exception
    }
}

int main(int argc, char* argv[]) {
   try {
        if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <scene_file.json>" << std::endl;
            std::cout << "Please provide a path to a scene file." << std::endl;
            return 0;  // Exit gracefully
        }

        std::string filename = argv[1];
        std::string filepath = "assets/" + filename;
        std::ifstream file(filepath);

        // Check if the file is open
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return 1;
        }
        
        json j;
        try {
            file >> j;
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return 1;
        }

        Renderer renderer = loadRendererDetails(j);

        auto start = std::chrono::high_resolution_clock::now();
        renderer.renderScene();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Render time: " << duration.count() << " milliseconds" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 1;
    }
}