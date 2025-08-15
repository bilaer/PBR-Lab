#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <vector>

#include "cubemap/cubemap.h"
#include "texture/texture.h"
#include "cubemap/skybox.h"
#include "geometry.h"
#include "env.h"
#include "material.h"
#include "config.h"
#include <glm/glm.hpp>

// ======== Camera state ========
float lastX = 400, lastY = 300;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
bool firstMouse = true;

glm::vec3 camPos(0.0f, 0.0f, 0.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

// ======== Input callbacks ========
static void KeyCallback(GLFWwindow* w, int key, int sc, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(w, GLFW_TRUE);
}

static void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = (float)xpos; lastY = (float)ypos; firstMouse = false; }
    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camFront = glm::normalize(front);
}

static void ProcessInput(GLFWwindow* window) {
    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camPos += cameraSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camPos -= cameraSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camPos -= glm::normalize(glm::cross(camFront, camUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camPos += glm::normalize(glm::cross(camFront, camUp)) * cameraSpeed;
}

// ======== Window + GL init (one place) ========
GLFWwindow* CreateWindowAndContext(int winW, int winH, const char* title) {
    if (!glfwInit()) { std::cerr << "Failed to initialize GLFW\n"; return nullptr; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(winW, winH, title, nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return nullptr; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window); glfwTerminate(); return nullptr;
    }

    // enable seamless to prevent seams between faces in cubemap
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    return window;
}

// Show brdf as texture to check if it is correct
void ShowBRDFLUTDebugWindow(GLuint brdfLUT, GLFWwindow* sharedContext, const std::shared_ptr<ScreenQuad>& screenQuad) {
    // ✅ 显示窗口
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ✅ 创建共享上下文窗口
    GLFWwindow* debugWindow = glfwCreateWindow(512, 512, "BRDF LUT Viewer", nullptr, sharedContext);
    if (!debugWindow) {
        std::cerr << "❌ Failed to create BRDF LUT Debug window!" << std::endl;
        return;
    }

    glfwMakeContextCurrent(debugWindow);
    glfwSwapInterval(1);  // V-Sync

    auto debugShader = std::make_shared<Shader>("shader/show_brdf.vert", "shader/show_brdf.frag");

    // ✅ Main Loop
    while (!glfwWindowShouldClose(debugWindow)) {
        if (glfwGetKey(debugWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(debugWindow, true);
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        debugShader->Use();
        debugShader->SetUniform("brdfLUT", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brdfLUT);

        screenQuad->Draw(debugShader);

        glfwSwapBuffers(debugWindow);
        glfwPollEvents();
    }

    glfwDestroyWindow(debugWindow);
}

// ======== Render loop (single window, no black bars) ========
// --------------TODO: will be replaced with Skybox class and camera class
void RunDebugLoop(GLFWwindow* window, GLuint cubemap, const std::shared_ptr<UnitCube>& cube) {
    auto shader = std::make_shared<Shader>("shader/debug.vert", "shader/debug.frag");

    while (!glfwWindowShouldClose(window)) {
        ProcessInput(window);

        int fbw = 0, fbh = 0;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        if (fbw == 0 || fbh == 0) { glfwPollEvents(); continue; }
        glViewport(0, 0, fbw, fbh);
        const float aspect = (float)fbw / (float)fbh;

        glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // camera matrix
        const glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        const glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

        // remove translation for skybox
        const glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        shader->Use();
        shader->SetUniform("cubemap", 0);
        shader->SetUniform("view", viewNoTrans);
        shader->SetUniform("projection", proj);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        cube->Draw(shader);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

// ======== main ========
int main() {
    // initialize the window
    GLFWwindow* window = CreateWindowAndContext(800, 600, "Cubemap Debug");
    if (!window) return -1;

    // Initialize shared resources
    auto cube = std::make_shared<UnitCube>();
    auto screenQuad = std::make_shared<ScreenQuad>();
    Environment env; // Create env object to load irradiance, prefilter and brdf lut

    //================Env maps=======================
    // load prefilter map
    const unsigned int prefilterSize = 128;
    const unsigned int mipLevels = 8;
    std::string prefilterPath = "debug/prefilter.ktx";
    env.LoadPrefilterMap(prefilterPath, prefilterSize, mipLevels);

    // load irradiance map
    const unsigned int irradianceSize = 32;
    std::string irradiancePath = "debug/irradiance.ktx";
    env.LoadIrradianceMap(irradiancePath, irradianceSize);

    // load brdf lut
    std::string brdflutPath = "debug/brdflut.hdr";
    std::string brdflutktxPath = "debug/brdflut.ktx";
    unsigned int brdfSize = 512;
    env.LoadBRDFLut(brdflutktxPath, brdfSize);

    // Debug
    //ShowBRDFLUTDebugWindow(env.GetBRDFLUT(), window, screenQuad);
    //RunDebugLoop(window, env.GetPrefilter(), cube);

    // ==============Load texture for PBR material===================
    std::string roughnessPath = "assets/test/roughness.jpg";
    std::string metalPath = "assets/test/metal.jpg";
    std::string normalPath = "assets/test/normal.png";
    std::string albedoPath = "assets/test/color.jpg";
    
    // Create material with texture
    auto texMaterial = std::make_shared<PBRMaterial>();
    texMaterial->LoadAlbedoMap(albedoPath);
    texMaterial->LoadMetalnessMap(metalPath);
    texMaterial->LoadNormalMap(normalPath);
    texMaterial->LoadRoughnessMap(roughnessPath);

    // Create material without texture
    auto pureMaterial = std::make_shared<PBRMaterial>();
    pureMaterial->SetBaseColor(glm::vec3(1.0f, 0.0f, 0.0f));   // 红色
    pureMaterial->SetRoughness(0.2f);                           // 光滑表面
    pureMaterial->SetMetalness(1.0f);                           // 纯金属
    pureMaterial->SetAO(1.0f);                                  // 全环境光，无遮挡

    // ==========Load HDR equirectangular==============
    // TODO: Combine it iinto skybox class
    std::string envMapPath = "assets/env.hdr";
    unsigned int envSize = 2048;
    auto envMap = std::make_shared<Cubemap>(envSize, 0);
    envMap->LoadEquiToCubemap(envMapPath);

    // Dispay generated cubemap
    envMap->Bind(SKYBOX_TEXTURE_UNIT);
    RunDebugLoop(window, envMap->GetTexture(), cube);
    envMap->Unbind();

    // Clean
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
