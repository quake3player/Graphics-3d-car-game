#include "Engine.h"
#include "ShaderProgram.h"
#include "MeshNode.h"
#include "Camera.h"
#include "assets.h"
#include "Geometry.h"
#include "game_runtime.h"
#include "game_logic.h"
#include "input_handler.h"
#include "physics_system.h"
#include "ui_system.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <filesystem>
#include <algorithm>

int main(int argc, char** argv) {
    Engine& engine = Engine::get();
    if (!engine.init(konst::kWindowWidth, konst::kWindowHeight, "Scene Hierarchical Modeling")) {
        return -1;
    }

    GameRuntime rt;
    resetWorld(rt);
    
    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--extras") rt.enableExtras = true;
        if (arg == "--shadows") rt.enableShadows = true;
    }

    {
        initImGui(engine.getWindow());

        ShaderProgram shader;
    if (!shader.loadFiles("SceneGame/shaders/vertex.glsl", "SceneGame/shaders/fragment.glsl")) {
        engine.shutdown();
        return -1;
    }

    // Shadow depth shader
    ShaderProgram shadowShader;
    if (!shadowShader.loadFiles("SceneGame/shaders/shadow_vertex.glsl", "SceneGame/shaders/shadow_fragment.glsl")) {
        engine.shutdown();
        return -1;
    }

    // Shadow framebuffer (depth-only)
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLuint shadowFBO, shadowDepthTex;
    glGenFramebuffers(1, &shadowFBO);
    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Sun shadow FBO incomplete!" << std::endl;
        engine.shutdown();
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Spotlight shadow maps
    GLuint spotShadowFBO, spotShadowMapArray;
    glGenFramebuffers(1, &spotShadowFBO);
    glGenTextures(1, &spotShadowMapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, spotShadowMapArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, 1024, 1024, 8, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, spotShadowFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, spotShadowMapArray, 0, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Spot shadow FBO incomplete!" << std::endl;
        engine.shutdown();
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint texBrick    = loadTexture("assets/textures/brick.png");
    GLuint texWood     = loadTexture("assets/textures/wood.jpg");
    GLuint texRoad     = loadTexture("assets/textures/road.png");
    GLuint texConcrete = loadTexture("assets/textures/concrete.png");
    GLuint texStone    = loadTexture("assets/textures/stone.png");

    Mesh cubeMesh  = Geometry::createCube();
    Mesh cylMesh   = Geometry::createCylinder();
    Mesh trackMesh = Geometry::createOvalTrack(
        64, konst::kTrackOuterA, konst::kTrackOuterB,
            konst::kTrackInnerA, konst::kTrackInnerB);

    // Using outer rt which has the parsed CLI arguments
    rt.shaderProgram = shader.getID();
    rt.texBrick    = texBrick;
    rt.texWood     = texWood;
    rt.texRoad     = texRoad;
    rt.texConcrete = texConcrete;
    rt.texStone    = texStone;
    rt.lastTime = static_cast<float>(glfwGetTime());
    initializeGameRuntime(rt);

    CameraManager camMan;
    bool bulletTime = false;

    // ───── Scene Graph ─────
    SceneNode rootNode;

    // Ground plane
    auto* groundNode = new MeshNode(cubeMesh, glm::vec3(0.16f, 0.28f, 0.16f));
    groundNode->setLocalTransform(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.06f, 0.0f)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(konst::kArenaSize, 0.12f, konst::kArenaSize))
    );
    rootNode.addChild(groundNode);

    // Oval track (the road surface)
    auto* trackNode = new MeshNode(trackMesh, glm::vec3(0.35f, 0.35f, 0.38f));
    trackNode->setUseTexture(true);
    trackNode->setTextureID(texRoad);
    trackNode->setLocalTransform(
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.02f, 0.0f))
    );
    rootNode.addChild(trackNode);

    // ───── Boundary Walls ─────
    const float halfArena = konst::kArenaSize * 0.5f;
    const float wallThickness = 1.6f;
    {
        auto* northWall = new MeshNode(cubeMesh, glm::vec3(0.70f, 0.70f, 0.74f));
        northWall->setUseTexture(true); northWall->setTextureID(texBrick);
        northWall->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, konst::kWallHeight * 0.5f, -halfArena + wallThickness * 0.5f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(konst::kArenaSize, konst::kWallHeight, wallThickness))
        );
        rootNode.addChild(northWall);

        auto* southWall = new MeshNode(cubeMesh, glm::vec3(0.70f, 0.70f, 0.74f));
        southWall->setUseTexture(true); southWall->setTextureID(texBrick);
        southWall->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, konst::kWallHeight * 0.5f, halfArena - wallThickness * 0.5f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(konst::kArenaSize, konst::kWallHeight, wallThickness))
        );
        rootNode.addChild(southWall);

        auto* westWall = new MeshNode(cubeMesh, glm::vec3(0.70f, 0.70f, 0.74f));
        westWall->setUseTexture(true); westWall->setTextureID(texBrick);
        westWall->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(-halfArena + wallThickness * 0.5f, konst::kWallHeight * 0.5f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(wallThickness, konst::kWallHeight, konst::kArenaSize))
        );
        rootNode.addChild(westWall);

        auto* eastWall = new MeshNode(cubeMesh, glm::vec3(0.70f, 0.70f, 0.74f));
        eastWall->setUseTexture(true); eastWall->setTextureID(texBrick);
        eastWall->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(halfArena - wallThickness * 0.5f, konst::kWallHeight * 0.5f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(wallThickness, konst::kWallHeight, konst::kArenaSize))
        );
        rootNode.addChild(eastWall);
    }

    // ───── Track-Side Barrier Walls ─────
    // Short walls along segments of the oval track for visual flair
    const float barrierHeight = konst::kBarrierHeight;
    const float barrierThick = konst::kBarrierThick;
    const int barrierSegs = konst::kBarrierSegments;
    for (const auto& tw : rt.trackWalls) {
        float radius_a = tw.outer ? konst::kTrackOuterA + barrierThick : konst::kTrackInnerA - barrierThick;
        float radius_b = tw.outer ? konst::kTrackOuterB + barrierThick : konst::kTrackInnerB - barrierThick;
        float angleRange = tw.endAngle - tw.startAngle;
        if (angleRange < 0) angleRange += 2.0f * konst::kPi;
        float dAngle = angleRange / barrierSegs;
        for (int s = 0; s < barrierSegs; s++) {
            float a = tw.startAngle + s * dAngle + dAngle * 0.5f;
            float bx = std::cos(a) * radius_a;
            float bz = std::sin(a) * radius_b;
            // Length of this arc segment for scaling (approx)
            float cx1 = std::cos(a - dAngle * 0.5f) * radius_a;
            float cz1 = std::sin(a - dAngle * 0.5f) * radius_b;
            float cx2 = std::cos(a + dAngle * 0.5f) * radius_a;
            float cz2 = std::sin(a + dAngle * 0.5f) * radius_b;
            float segLen = std::sqrt((cx2-cx1)*(cx2-cx1) + (cz2-cz1)*(cz2-cz1));
            
            // Rotation to align barrier along the tangent
            float tangentAngle = std::atan2(
                -std::sin(a) * radius_b,
                -std::cos(a) * radius_a
            ) + konst::kPi * 0.5f;
            
            auto* barrier = new MeshNode(cubeMesh, glm::vec3(0.65f, 0.60f, 0.55f));
            barrier->setUseTexture(true);
            barrier->setTextureID(texBrick);
            glm::mat4 bt = glm::translate(glm::mat4(1.0f), glm::vec3(bx, barrierHeight * 0.5f, bz));
            bt = glm::rotate(bt, tangentAngle, glm::vec3(0, 1, 0));
            // Add a slight overlap by multiplying segLen by 1.1 so they join seamlessly
            bt = glm::scale(bt, glm::vec3(segLen * 1.1f, barrierHeight, barrierThick));
            barrier->setLocalTransform(bt);
            rootNode.addChild(barrier);
        }
    }

    // ───── Car Hierarchy (carRoot -> chassis, cabin, wheels, accessories) ─────
    SceneNode* carRoot = new SceneNode();
    rootNode.addChild(carRoot);

    auto* carChassis = new MeshNode(cubeMesh, glm::vec3(0.85f, 0.10f, 0.15f)); // Sporty Red Chassis
    carChassis->setMetallic(0.8f);
    carRoot->addChild(carChassis);

    auto* carCabin = new MeshNode(cubeMesh, glm::vec3(0.12f, 0.12f, 0.12f)); // Tinted Windows
    carCabin->setMetallic(0.9f);
    carRoot->addChild(carCabin);

    // Headlights (two small bright cubes on the front)
    auto* headlightL = new MeshNode(cubeMesh, glm::vec3(1.0f, 1.0f, 0.9f));
    auto* headlightR = new MeshNode(cubeMesh, glm::vec3(1.0f, 1.0f, 0.9f));
    carRoot->addChild(headlightL);
    carRoot->addChild(headlightR);

    // Front Grille & Splitter
    auto* grille = new MeshNode(cubeMesh, glm::vec3(0.05f, 0.05f, 0.05f));
    auto* splitter = new MeshNode(cubeMesh, glm::vec3(0.10f, 0.10f, 0.10f));
    carRoot->addChild(grille);
    carRoot->addChild(splitter);

    // Taillights & Exhaust
    auto* tailLightL = new MeshNode(cubeMesh, glm::vec3(1.0f, 0.1f, 0.05f));
    auto* tailLightR = new MeshNode(cubeMesh, glm::vec3(1.0f, 0.1f, 0.05f));
    auto* exhaustL = new MeshNode(cylMesh, glm::vec3(0.3f, 0.3f, 0.3f));
    auto* exhaustR = new MeshNode(cylMesh, glm::vec3(0.3f, 0.3f, 0.3f));
    exhaustL->setMetallic(0.8f);
    exhaustR->setMetallic(0.8f);
    carRoot->addChild(tailLightL);
    carRoot->addChild(tailLightR);
    carRoot->addChild(exhaustL);
    carRoot->addChild(exhaustR);

    // Spoiler
    auto* spoilerStrutL = new MeshNode(cubeMesh, glm::vec3(0.1f, 0.1f, 0.1f));
    auto* spoilerStrutR = new MeshNode(cubeMesh, glm::vec3(0.1f, 0.1f, 0.1f));
    auto* spoilerWing = new MeshNode(cubeMesh, glm::vec3(0.85f, 0.10f, 0.15f));
    spoilerWing->setMetallic(0.8f);
    carRoot->addChild(spoilerStrutL);
    carRoot->addChild(spoilerStrutR);
    carRoot->addChild(spoilerWing);

    auto* w_fl = new MeshNode(cylMesh, glm::vec3(0.08f, 0.08f, 0.08f)); // Dark tire rubber
    auto* w_fr = new MeshNode(cylMesh, glm::vec3(0.08f, 0.08f, 0.08f));
    auto* w_bl = new MeshNode(cylMesh, glm::vec3(0.08f, 0.08f, 0.08f));
    auto* w_br = new MeshNode(cylMesh, glm::vec3(0.08f, 0.08f, 0.08f));
    carRoot->addChild(w_fl);
    carRoot->addChild(w_fr);
    carRoot->addChild(w_bl);
    carRoot->addChild(w_br);

    // Collect all hardcoded car child nodes for visibility toggling
    std::vector<SceneNode*> hardcodedCarNodes = {
        carChassis, carCabin, headlightL, headlightR,
        grille, splitter, tailLightL, tailLightR,
        exhaustL, exhaustR, spoilerStrutL, spoilerStrutR, spoilerWing,
        w_fl, w_fr, w_bl, w_br
    };

    // ───── OBJ Car Models ─────
    rt.carModelNames.push_back("Original (Hardcoded)");
    std::vector<SceneNode*> objCarGroups; // one group node per OBJ model
    
    std::string modelsDir = "assets/models";
    if (std::filesystem::exists(modelsDir)) {
        std::vector<std::string> objPaths;
        for (const auto& entry : std::filesystem::directory_iterator(modelsDir)) {
            if (entry.path().extension() == ".obj") {
                objPaths.push_back(entry.path().string());
            }
        }
        std::sort(objPaths.begin(), objPaths.end());
        
        for (const auto& objPath : objPaths) {
            std::string name = std::filesystem::path(objPath).stem().string();
            std::cout << "Loading car model: " << name << " ..." << std::endl;
            auto parts = Geometry::loadOBJParts(objPath);
            if (!parts.empty()) {
                auto* group = new SceneNode();
                carRoot->addChild(group);
                for (auto& part : parts) {
                    auto* node = new MeshNode(part.mesh, part.color);
                    node->setMetallic(part.metallic);
                    group->addChild(node);
                }
                objCarGroups.push_back(group);
                rt.carModelNames.push_back(name);
            }
        }
    }
    std::cout << "Available car models: " << rt.carModelNames.size() << std::endl;
    
    // Per-model Y rotation offsets (radians). Default 0 for most models.
    std::vector<float> objCarRotations(objCarGroups.size(), 0.0f);
    // Shelby is exported sideways, rotate +90° ACW
    for (size_t i = 0; i < objCarGroups.size(); i++) {
        if (rt.carModelNames[i + 1].find("Shelby") != std::string::npos) {
            objCarRotations[i] = 1.5708f;
        }
    }
    
    // Store saved transforms for hardcoded car nodes (set after first frame)
    bool hardcodedTransformsInitialized = false;
    std::vector<glm::mat4> hardcodedSavedTransforms(hardcodedCarNodes.size());

    // ───── Buildings — Multi-Story with Windmills & Gimbles ─────
    struct BuildingSceneNodes {
        std::vector<MeshNode*> storyNodes; // one cube per story
        MeshNode* windmillHub;
        MeshNode* gimbleNode;
    };
    std::vector<BuildingSceneNodes> buildingScene;

    for (size_t bi = 0; bi < rt.buildings.size(); bi++) {
        const auto& b = rt.buildings[bi];
        BuildingSceneNodes bsn;

        // Create a parent empty node for the whole building
        SceneNode* buildingParent = new SceneNode();
        rootNode.addChild(buildingParent);

        float storyHeight = b.height / b.stories;
        GLuint tex = texBrick;
        switch (b.textureType) {
            case 0: tex = texBrick;    break;
            case 1: tex = texWood;     break;
            case 2: tex = texConcrete; break;
            case 3: tex = texStone;    break;
        }

        for (int s = 0; s < b.stories; s++) {
            float yCenter = storyHeight * s + storyHeight * 0.5f;
            // Slight color variation per story
            float tint = 1.0f - s * 0.08f;
            auto* storyMesh = new MeshNode(cubeMesh, glm::vec3(b.r * tint, b.g * tint, b.b * tint));
            storyMesh->setUseTexture(true);
            storyMesh->setTextureID(tex);
            storyMesh->setLocalTransform(
                glm::translate(glm::mat4(1.0f), glm::vec3(b.x, yCenter, b.z)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(b.width, storyHeight - 0.3f, b.depth))
            );
            buildingParent->addChild(storyMesh);
            bsn.storyNodes.push_back(storyMesh);

            // Floor separator (thin dark slab between stories)
            if (s > 0) {
                float sepY = storyHeight * s;
                auto* sep = new MeshNode(cubeMesh, glm::vec3(0.25f, 0.22f, 0.20f));
                sep->setLocalTransform(
                    glm::translate(glm::mat4(1.0f), glm::vec3(b.x, sepY, b.z)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(b.width + 0.4f, 0.3f, b.depth + 0.4f))
                );
                buildingParent->addChild(sep);
            }
        }

        // Windmill: hub with 4 long blades + 4 short inner blades (alternating)
        MeshNode* wHub = new MeshNode(cylMesh, glm::vec3(0.85f, 0.85f, 0.85f));
        for (int i = 0; i < 4; i++) {
            // Long outer blade
            MeshNode* blade = new MeshNode(cubeMesh, glm::vec3(0.9f));
            glm::mat4 bt = glm::rotate(glm::mat4(1.0f), i * konst::kPi / 2.0f, glm::vec3(0, 0, 1));
            bt = glm::translate(bt, glm::vec3(0.0f, 1.0f, 0.0f));
            bt = glm::scale(bt, glm::vec3(0.2f, 2.0f, 0.1f));
            blade->setLocalTransform(bt);
            wHub->addChild(blade);
            
            // Short inner blade (offset 45 degrees, half length)
            MeshNode* innerBlade = new MeshNode(cubeMesh, glm::vec3(0.7f, 0.75f, 0.8f));
            glm::mat4 ibt = glm::rotate(glm::mat4(1.0f), (i + 0.5f) * konst::kPi / 2.0f, glm::vec3(0, 0, 1));
            ibt = glm::translate(ibt, glm::vec3(0.0f, 0.5f, 0.0f));
            ibt = glm::scale(ibt, glm::vec3(0.15f, 1.0f, 0.08f));
            innerBlade->setLocalTransform(ibt);
            wHub->addChild(innerBlade);
        }
        buildingParent->addChild(wHub);
        bsn.windmillHub = wHub;

        // Wall-bracket light: arm (cube) + lantern (cube) child
        MeshNode* arm = new MeshNode(cubeMesh, glm::vec3(0.3f, 0.3f, 0.3f));
        MeshNode* lantern = new MeshNode(cubeMesh, glm::vec3(
            rt.spotlights[bi].r, rt.spotlights[bi].g, rt.spotlights[bi].b));
        // Lantern hangs at the end of the arm, below it
        lantern->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -0.45f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(2.5f, 2.0f, 2.5f))
        );
        arm->addChild(lantern);
        buildingParent->addChild(arm);
        bsn.gimbleNode = arm;

        buildingScene.push_back(bsn);
    }

    // ───── Building Number Markers (colored cubes on rooftops) ─────
    // Each building gets N small cubes on its roof (N = building index + 1)
    for (size_t bi = 0; bi < rt.buildings.size(); bi++) {
        const auto& b = rt.buildings[bi];
        int num = (int)bi + 1;
        for (int n = 0; n < num; n++) {
            auto* marker = new MeshNode(cubeMesh, glm::vec3(1.0f, 0.2f, 0.2f)); // red markers
            float xOff = (n - (num - 1) * 0.5f) * 1.5f; // spread cubes
            marker->setLocalTransform(
                glm::translate(glm::mat4(1.0f), glm::vec3(b.x + xOff, b.height + 1.0f, b.z)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.8f))
            );
            rootNode.addChild(marker);
        }
    }

    // ───── Street Lamps around the track ─────
    std::vector<MeshNode*> lampPosts;
    std::vector<MeshNode*> lampBulbs;
    for (int i = 0; i < rt.lampPosts.size(); i++) {
        float lx = rt.lampPosts[i].x;
        float lz = rt.lampPosts[i].z;

        // Pole (skinny cylinder)
        auto* pole = new MeshNode(cylMesh, glm::vec3(0.3f, 0.3f, 0.32f));
        pole->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(lx, konst::kLampHeight * 0.5f, lz)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, konst::kLampHeight, 0.3f))
        );
        rootNode.addChild(pole);
        lampPosts.push_back(pole);

        // Bulb (bigger bright cube on top for road visibility)
        auto* bulb = new MeshNode(cubeMesh, glm::vec3(1.0f, 0.95f, 0.7f));
        bulb->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(lx, konst::kLampHeight + 0.5f, lz)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.4f, 1.0f, 1.4f))
        );
        rootNode.addChild(bulb);
        lampBulbs.push_back(bulb);
    }

    // ───── Ground Camera Marker (visible cube showing where ground cam is) ─────
    MeshNode* groundCamMarker = new MeshNode(cubeMesh, glm::vec3(1.0f, 1.0f, 0.0f)); // bright yellow
    rootNode.addChild(groundCamMarker);

    // ───── Parking Box Visual (Lighthouse Challenge) ─────
    MeshNode* parkingBox = new MeshNode(cubeMesh, glm::vec3(0.0f, 1.0f, 0.2f)); // green box
    rootNode.addChild(parkingBox);

    // ───── Lighthouse Mesh ─────
    MeshNode* lhGroup = new MeshNode(cubeMesh, glm::vec3(0.0f));
    glm::vec3 lhColor(0.9f, 0.95f, 1.0f);
    MeshNode* lhBase = new MeshNode(cylMesh, lhColor);
    lhBase->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 5.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 10.0f, 4.0f)));
    lhGroup->addChild(lhBase);
    MeshNode* lhMid = new MeshNode(cylMesh, lhColor);
    lhMid->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 15.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 10.0f, 3.0f)));
    lhGroup->addChild(lhMid);
    MeshNode* lhTop = new MeshNode(cylMesh, lhColor);
    lhTop->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 23.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 6.0f, 2.0f)));
    lhGroup->addChild(lhTop);
    MeshNode* lhRoom = new MeshNode(cylMesh, glm::vec3(1.0f, 1.0f, 0.2f)); // Yellow room
    lhRoom->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 26.5f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.0f, 1.5f)));
    lhGroup->addChild(lhRoom);

    // Gimbal armature for the Lighthouse
    MeshNode* lhArm = new MeshNode(cubeMesh, glm::vec3(0.2f, 0.2f, 0.2f));
    lhGroup->addChild(lhArm);
    MeshNode* lhBulb = new MeshNode(cubeMesh, glm::vec3(1.0f, 1.0f, 0.2f)); // yellow bulb
    lhArm->addChild(lhBulb);

    rootNode.addChild(lhGroup);

    // ───── Render Loop ─────
    while (!engine.shouldClose()) {
        engine.beginFrame();
        
        float currentTime = glfwGetTime();
        float dt = currentTime - rt.lastTime;
        rt.lastTime = currentTime;
        if (dt > 0.1f) dt = 0.1f;
        float simDt = bulletTime ? dt * konst::kBulletTimeFactor : dt;

        handleInput(engine.getWindow(), rt, simDt, bulletTime);
        updatePhysics(rt, simDt);

        // ── Update CarRoot Transform (Global pos & rot) ──
        glm::mat4 rtMat = glm::translate(glm::mat4(1.0f), glm::vec3(rt.carX, rt.carY, rt.carZ));
        rtMat = glm::rotate(rtMat, rt.carAngle, glm::vec3(0, 1, 0));
        carRoot->setLocalTransform(rtMat);

        // All internal logic is now in parent space relative to scale(1.0, 1.0, 1.0)
        // We explicitly multiply constants by World boundaries!
        
        // Chassis Box
        carChassis->setLocalTransform(glm::scale(glm::mat4(1.0f), glm::vec3(konst::kCarWidth, konst::kCarHeight, konst::kCarLength)));

        // Cabin Extends
        carCabin->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.70f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(konst::kCarWidth * 0.70f, konst::kCarHeight * 0.45f, konst::kCarLength * 0.60f))
        );

        // Headlights (front of car)
        headlightL->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(-1.33f, 0.1f, -3.43f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.68f, 0.20f, 0.28f))
        );
        headlightR->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(1.33f, 0.1f, -3.43f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.68f, 0.20f, 0.28f))
        );
        
        // Front Grille & Splitter
        grille->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, -3.57f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(2.28f, 0.50f, 0.14f))
        );
        splitter->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.8f, -3.78f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(4.18f, 0.10f, 1.4f))
        );

        // Taillights
        tailLightL->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(-1.33f, 0.2f, 3.57f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.76f, 0.20f, 0.14f))
        );
        tailLightR->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(1.33f, 0.2f, 3.57f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.76f, 0.20f, 0.14f))
        );

        // Exhaust
        glm::mat4 exhaustRot = glm::rotate(glm::mat4(1.0f), 1.5708f, glm::vec3(1, 0, 0));
        exhaustL->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(-0.95f, -0.7f, 3.64f)) *
            exhaustRot * glm::scale(glm::mat4(1.0f), glm::vec3(0.30f, 0.35f, 0.30f))
        );
        exhaustR->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.95f, -0.7f, 3.64f)) *
            exhaustRot * glm::scale(glm::mat4(1.0f), glm::vec3(0.30f, 0.35f, 0.30f))
        );

        // Spoiler
        spoilerStrutL->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(-1.33f, 1.2f, 3.15f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.19f, 0.6f, 1.05f))
        );
        spoilerStrutR->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(1.33f, 1.2f, 3.15f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(0.19f, 0.6f, 1.05f))
        );
        spoilerWing->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, 3.36f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(4.18f, 0.10f, 2.1f))
        );

        // Wheel animation
        static float wheelRot = 0;
        wheelRot -= rt.carSpeed * simDt * 0.5f;
        glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), 1.5708f, glm::vec3(0, 0, 1));
        rotZ = glm::rotate(rotZ, wheelRot, glm::vec3(0, 1, 0));
        
        // Pushing wheels OUT and DOWN. Using normal uniform scaling now!
        float ww = 2.15f; // absolute x-width
        float wh = 1.05f;  // absolute y-depth
        float wl = 2.45f;  // z length offset
        glm::vec3 uniScale(0.70f); // 0.70 radius circle base!
        
        w_fl->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-ww, -wh, -wl)) * rotZ * glm::scale(glm::mat4(1.0f), glm::vec3(uniScale.x, 0.40f /*tire width*/, uniScale.z)));
        w_fr->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3( ww, -wh, -wl)) * rotZ * glm::scale(glm::mat4(1.0f), glm::vec3(uniScale.x, 0.40f, uniScale.z)));
        w_bl->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-ww, -wh,  wl)) * rotZ * glm::scale(glm::mat4(1.0f), glm::vec3(uniScale.x, 0.40f, uniScale.z)));
        w_br->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3( ww, -wh,  wl)) * rotZ * glm::scale(glm::mat4(1.0f), glm::vec3(uniScale.x, 0.40f, uniScale.z)));

        // ── Car Model Switching ──
        glm::mat4 hideTransform = glm::scale(glm::mat4(1.0f), glm::vec3(0.0f));
        
        if (rt.selectedCarModel == 0) {
            // Show hardcoded car (transforms already set above, nothing to do)
            // Hide all OBJ models
            for (auto* grp : objCarGroups) {
                grp->setLocalTransform(hideTransform);
            }
        } else {
            // Hide all hardcoded car parts
            for (auto* node : hardcodedCarNodes) {
                node->setLocalTransform(hideTransform);
            }
            // Show the selected OBJ model, hide others
            int objIdx = rt.selectedCarModel - 1;
            for (int i = 0; i < (int)objCarGroups.size(); i++) {
                if (i == objIdx) {
                    // Scale OBJ to match car dimensions (normalized to unit box, scale to car size)
                    float carScale = std::max({konst::kCarWidth, konst::kCarHeight, konst::kCarLength});
                    objCarGroups[i]->setLocalTransform(
                        glm::rotate(glm::mat4(1.0f), objCarRotations[objIdx], glm::vec3(0, 1, 0)) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(carScale))
                    );
                } else {
                    objCarGroups[i]->setLocalTransform(hideTransform);
                }
            }
        }

        // ── Ground Camera Marker (yellow cube at ground cam position) ──
        groundCamMarker->setLocalTransform(
            glm::translate(glm::mat4(1.0f), glm::vec3(rt.groundCamX, rt.groundCamY, rt.groundCamZ)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f))
        );

        // ── Parking Box & Lighthouse Placement ──
        if (rt.lighthouseMode) {
            // Draw a flat thin green pad at 45,45
            parkingBox->setLocalTransform(
                glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, 0.05f, 45.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(6.0f, 0.1f, 6.0f))
            );
            lhGroup->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(38.0f, 0.0f, 38.0f)));
        } else {
            // Hide them underground
            parkingBox->setLocalTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.0f)));
            lhGroup->setLocalTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.0f)));
        }

        // ── Update Buildings: windmill hubs + gimble swing ──
        for (size_t i = 0; i < rt.buildings.size(); i++) {
            const auto& b = rt.buildings[i];
            
            bool isTopBottom = (std::abs(b.x) < 5.0f);
            float sideSignX = isTopBottom ? 0.0f : (b.x < 0.0f ? 1.0f : -1.0f); // inward facing X
            float sideSignZ = isTopBottom ? (b.z > 0.0f ? -1.0f : 1.0f) : 0.0f; // inward facing Z (if z=44, face -Z)
            
            // Windmill hub — on the side of the building facing center (road)
            float hubX = b.x + sideSignX * (b.width * 0.5f + 0.5f);
            float hubZ = b.z + sideSignZ * (b.depth * 0.5f + 0.5f);
            
            glm::mat4 hubt = glm::translate(glm::mat4(1.0f), glm::vec3(hubX, b.height * 0.85f, hubZ));
            
            // Spin and mount rotation
            // We want the blades to be parallel to the wall they are attached to
            if (isTopBottom) {
                // If top/bottom, wall normal is Z. Blades should spin in XY plane.
                hubt = glm::rotate(hubt, konst::kPi * 0.5f, glm::vec3(0, 1, 0)); // Rotate cylinder so its cap faces Z
                hubt = glm::rotate(hubt, b.windmillAngle, glm::vec3(1, 0, 0)); // Spin blades
            } else {
                // If left/right, wall normal is X. Blades should spin in YZ plane.
                hubt = glm::rotate(hubt, b.windmillAngle, glm::vec3(1, 0, 0)); // Spin blades
            }
            hubt = glm::scale(hubt, glm::vec3(0.6f, 1.2f, 1.2f));
            buildingScene[i].windmillHub->setLocalTransform(hubt);

            // Gimbal light — perpendicular to building edge, swings ±30° around it
            // Mount: on road-facing edge, offset from midpoint
            float edgeOffsetX = isTopBottom ? (b.width * 0.15f) : 0.0f;
            float edgeOffsetZ = isTopBottom ? 0.0f : (b.depth * 0.15f);
            float gimbleX = b.x + sideSignX * (b.width * 0.5f) + edgeOffsetX;
            float gimbleZ = b.z + sideSignZ * (b.depth * 0.5f) + edgeOffsetZ;
            float gimbleY = b.height + 0.3f;
            glm::vec3 mountPos(gimbleX, gimbleY, gimbleZ);
            
            // Perpendicular direction: straight out from building edge toward road center
            glm::vec3 perpDir(sideSignX, 0.0f, sideSignZ);
            if (glm::length(perpDir) < 0.01f) perpDir = glm::vec3(0, 0, -1); // fallback
            perpDir = glm::normalize(perpDir);
            
            // Base direction: perpendicular + angled downward at ~60° to illuminate road
            glm::vec3 baseDir = glm::normalize(glm::vec3(perpDir.x, -1.2f, perpDir.z));
            
            // Swing ±30° around Y axis, centered on perpendicular
            float swingAngle = std::sin(rt.spotlights[i].swingPhase)
                             * konst::kGimbalSwingDeg * (konst::kPi / 180.0f);
            glm::mat3 swingRot = glm::rotate(glm::mat4(1.0f), swingAngle, glm::vec3(0,1,0));
            glm::vec3 lightDir = glm::normalize(swingRot * baseDir);
            
            rt.spotlights[i].x = mountPos.x;
            rt.spotlights[i].y = mountPos.y;
            rt.spotlights[i].z = mountPos.z;
            rt.spotlights[i].dirX = lightDir.x;
            rt.spotlights[i].dirY = lightDir.y;
            rt.spotlights[i].dirZ = lightDir.z;

            // Arm visual: short bracket angling downward from mount point
            // Yaw = perpendicular direction + swing
            float baseYaw = std::atan2(perpDir.x, perpDir.z);
            float armPitch = 1.1f; // ~63° downward — clear visible bend toward road
            
            glm::mat4 gimt = glm::translate(glm::mat4(1.0f), mountPos);
            gimt = glm::rotate(gimt, baseYaw + swingAngle, glm::vec3(0,1,0));
            gimt = glm::rotate(gimt, armPitch, glm::vec3(1,0,0)); // tilt downward
            gimt = glm::translate(gimt, glm::vec3(0.0f, 0.0f, -konst::kGimbalArmLength * 0.5f));
            gimt = glm::scale(gimt, glm::vec3(konst::kGimbalArmThick, konst::kGimbalArmThick, konst::kGimbalArmLength));
            buildingScene[i].gimbleNode->setLocalTransform(gimt);
        }

        // ── Lighthouse Spotlight Logic ──
        if (rt.lighthouseMode && rt.spotlights.size() > 4) {
            int i = 4;
            glm::vec3 towerCenter(38.0f, 26.5f, 38.0f);
            float sweepTargetVal = std::sin(rt.spotlights[i].swingPhase) * 48.0f;
            glm::vec3 sweepTarget(sweepTargetVal, 0.0f, sweepTargetVal);
            glm::vec3 lightDir = glm::normalize(sweepTarget - towerCenter);
            
            // To prevent large occluding shadows from its own base, we MUST originate the true
            // geometric light source OUTSIDE the tower! The Arm orbits the tower.
            glm::vec3 horizDir = glm::normalize(glm::vec3(lightDir.x, 0.0f, lightDir.z));
            glm::vec3 bulbOrbitPos = towerCenter + horizDir * 2.5f;
            
            rt.spotlights[i].x = bulbOrbitPos.x;
            rt.spotlights[i].y = bulbOrbitPos.y;
            rt.spotlights[i].z = bulbOrbitPos.z;
            rt.spotlights[i].dirX = lightDir.x;
            rt.spotlights[i].dirY = lightDir.y;
            rt.spotlights[i].dirZ = lightDir.z;

            // Animate lhArm identical to the gimbal arms but from its local coordinates
            float baseYaw = std::atan2(lightDir.x, lightDir.z); // yaw angle
            float armPitch = std::asin(-lightDir.y); // pitch angle (negative because lightDir is pointing down)
            
            // local pos of arm is (0, 26.5, 0)
            glm::mat4 lht = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 26.5f, 0.0f));
            lht = glm::rotate(lht, baseYaw, glm::vec3(0, 1, 0));
            // Push arm geometry outward over the railing!
            lht = glm::translate(lht, glm::vec3(0.0f, 0.0f, 2.0f));
            lht = glm::rotate(lht, armPitch, glm::vec3(1, 0, 0));
            lhArm->setLocalTransform(lht);
            
            lhBulb->setLocalTransform(
                glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.8f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f))
            );
        }

        // ── Shadow Pass: render scene from sun's perspective ──
        glm::vec3 sunDir = glm::normalize(glm::vec3(
            std::cos(rt.sunPhi) * std::sin(rt.sunTheta),
            -std::sin(rt.sunPhi),
            std::cos(rt.sunPhi) * std::cos(rt.sunTheta)
        ));
        if (std::abs(sunDir.x) < 0.001f && std::abs(sunDir.z) < 0.001f) {
            sunDir.x = 0.001f; // Prevent perfectly vertical crash
        }
        glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
        glm::mat4 spotLightSpaceMatrices[8];
        for(int i = 0; i < 8; i++) spotLightSpaceMatrices[i] = glm::mat4(1.0f);
        
        if (rt.enableShadows) {
            glm::vec3 lightPos = -sunDir * 80.0f; // sun position (far away, opposite direction)
            glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0,1,0));
            float orthoSize = 60.0f;
            glm::mat4 lightProj = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 200.0f);
            lightSpaceMatrix = lightProj * lightView;

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            shadowShader.use();
            shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
            rootNode.draw(shadowShader); // render depth only
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ── Spotlights Shadow Pass ──
            glViewport(0, 0, 1024, 1024);
            glBindFramebuffer(GL_FRAMEBUFFER, spotShadowFBO);
            shadowShader.use();
            glCullFace(GL_BACK); // MUST be back-face culling, otherwise flat-roofed cars disappear from top-down perspective!
            for (int i = 0; i < rt.spotlights.size() && i < 8; i++) {
                if (rt.spotlights[i].r + rt.spotlights[i].g + rt.spotlights[i].b < 0.1f) continue;
                glm::vec3 pos(rt.spotlights[i].x, rt.spotlights[i].y, rt.spotlights[i].z);
                glm::vec3 dir(rt.spotlights[i].dirX, rt.spotlights[i].dirY, rt.spotlights[i].dirZ);
                // Push FBO view slightly forward along light vector to prevent parent geometry occluding its own shadow
                glm::vec3 shadowPos = pos + glm::normalize(dir) * 1.5f;

                float outerRad = (rt.lighthouseMode && i == 4 ? 30.0f : konst::kSpotOuterCutoffDeg) * konst::kPi / 180.0f;
                // Field of view for the spotlight frustum MUST be large enough to contain the spot cone!
                glm::mat4 spotProj = glm::perspective(outerRad * 2.0f, 1.0f, 0.1f, 100.0f);
                glm::mat4 spotView = glm::lookAt(shadowPos, shadowPos + dir, glm::vec3(0,1,0));
                spotLightSpaceMatrices[i] = spotProj * spotView;
                
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, spotShadowMapArray, 0, i);
                glClear(GL_DEPTH_BUFFER_BIT);
                shadowShader.setMat4("lightSpaceMatrix", spotLightSpaceMatrices[i]);
                rootNode.draw(shadowShader);
            }
            glCullFace(GL_BACK);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // Restore viewport
        int winW = engine.getWidth();
        int winH = engine.getHeight();
        if (winH == 0) winH = 1;
        glViewport(0, 0, winW, winH);

        // ── Main Render Pass ──

        // Sync zoom FOV from runtime to camera
        camMan.zoomFov = rt.zoomFov;
        
        camMan.mode = static_cast<CameraManager::ViewMode>(rt.cameraIndex);
        camMan.groundPos = glm::vec3(rt.groundCamX, rt.groundCamY, rt.groundCamZ);
        camMan.groundPitch = rt.groundCamPitch;
        camMan.groundYaw = rt.groundCamYaw;
        camMan.skyOffsetX = rt.skyOffsetX;
        camMan.skyOffsetZ = rt.skyOffsetZ;
        camMan.carCamYaw = rt.carCamYaw;
        camMan.carCamPitch = rt.carCamPitch;
        camMan.selectedLight = rt.selectedLight;
        camMan.heliHeight = rt.heliHeight;
        camMan.heliDist = rt.heliDist;

        glm::vec3 cPos(rt.carX, rt.carY, rt.carZ);
        int li = rt.selectedLight;
        if (li < 0 || li >= (int)rt.spotlights.size()) li = 0;
        glm::vec3 lPos(rt.spotlights[li].x, rt.spotlights[li].y, rt.spotlights[li].z);
        glm::vec3 lDir(rt.spotlights[li].dirX, rt.spotlights[li].dirY, rt.spotlights[li].dirZ);

        glm::mat4 view = camMan.getViewMatrix(cPos, rt.carAngle, lPos, lDir);
        glm::mat4 proj = camMan.getProjectionMatrix(float(winW)/float(winH));

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        shader.setVec3("viewPos", glm::vec3(glm::inverse(view)[3]));
        
        // Bind shadow map to texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
        shader.setInt("shadowMap", 1);
        shader.setInt("enableShadows", rt.enableShadows ? 1 : 0);

        // Bind spot shadow array
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_ARRAY, spotShadowMapArray);
        shader.setInt("spotShadowMaps", 2);
        for (int i = 0; i < rt.spotlights.size() && i < 8; i++) {
            shader.setMat4("spotLightSpaceMatrices[" + std::to_string(i) + "]", spotLightSpaceMatrices[i]);
        }

        // Sun light — scaled by daylight level
        shader.setVec3("sunDirection", sunDir);
        shader.setVec3("sunColor", glm::vec3(1.0f, 0.95f, 0.85f) * rt.daylightLevel);
        
        // Ambient strength — daylight controls this (near-black at night)
        float ambient = 0.005f + 0.25f * rt.daylightLevel; // range: 0.005 (night) to 0.255 (day)
        shader.setFloat("ambientStrength", ambient);

        // Spotlights
        shader.setInt("numSpotlights", rt.spotlights.size());
        for (size_t i = 0; i < rt.spotlights.size(); i++) {
            std::string pre = "spotlights[" + std::to_string(i) + "].";
            shader.setVec3(pre + "position", glm::vec3(rt.spotlights[i].x, rt.spotlights[i].y, rt.spotlights[i].z));
            shader.setVec3(pre + "direction", glm::vec3(rt.spotlights[i].dirX, rt.spotlights[i].dirY, rt.spotlights[i].dirZ));
            shader.setVec3(pre + "color", glm::vec3(rt.spotlights[i].r, rt.spotlights[i].g, rt.spotlights[i].b) * 2.0f);
            
            float cutOff = konst::kSpotCutoffDeg;
            float outerCutOff = konst::kSpotOuterCutoffDeg;
            if (rt.lighthouseMode && i == 4) {
                cutOff = 25.0f; // wider span
                outerCutOff = 30.0f;
            }
            shader.setFloat(pre + "cutOff", glm::cos(cutOff * konst::kPi / 180.0f));
            shader.setFloat(pre + "outerCutOff", glm::cos(outerCutOff * konst::kPi / 180.0f));
            shader.setFloat(pre + "constant", 1.0f);
            shader.setFloat(pre + "linear", konst::kSpotLinear);
            shader.setFloat(pre + "quadratic", konst::kSpotQuadratic);
        }

        // Point lights — one for each street lamp (makes road lighting visible)
        shader.setInt("numPointLights", rt.lampPosts.size());
        for (int i = 0; i < rt.lampPosts.size(); i++) {
            float lx = rt.lampPosts[i].x;
            float lz = rt.lampPosts[i].z;
            std::string pre = "pointLights[" + std::to_string(i) + "].";
            shader.setVec3(pre + "position", glm::vec3(lx, konst::kLampHeight + 0.5f, lz));
            shader.setVec3(pre + "color", glm::vec3(1.0f, 0.92f, 0.65f) * 1.5f);
            shader.setFloat(pre + "constant", 1.0f);
            shader.setFloat(pre + "linear", konst::kSpotLinear);
            shader.setFloat(pre + "quadratic", konst::kSpotQuadratic);
        }

        rootNode.draw(shader);
        renderUI(rt, dt > 0.0001f ? 1.0f / dt : 0.0f, bulletTime);
        engine.endFrame();
    }

    cleanupImGui();
    } // End of scope, will cleanly destruct shader before glfwTerminate
    engine.shutdown();
    return 0;
}
