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
#include "imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "model_loader/ply_loader.h"
#include "model_loader/gltf_loader.h"
#include "scene.h"

// ======== Camera state ========
float lastX = 400, lastY = 300;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
bool firstMouse = true;

glm::vec3 camPos(0.0f, 0.0f, 3.0f);
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
	// Use Version 3.3 for OpenGL
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

        screenQuad->Draw();

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

        cube->Draw();

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
    std::string roughnessPath = "assets/material/roughness.jpg";
    std::string metalPath = "assets/material/metal.jpg";
    std::string normalPath = "assets/material/normal.jpg";
    std::string albedoPath = "assets/material/color.jpg";
	std::string aoPath = "assets/material/ao.jpg";

    // Create material with textures
    auto texMaterial = std::make_shared<PBRMaterial>();
    texMaterial->LoadAlbedoMap(albedoPath);
    texMaterial->LoadRoughnessMap(roughnessPath);
    texMaterial->LoadNormalMap(normalPath);
	texMaterial->LoadAoMap(aoPath);

    // Pure color material
    auto colorMaterial = std::make_shared<PBRMaterial>();
    colorMaterial->SetBaseColor(glm::vec3(0.0f, 0.0f, 0.0f));
    colorMaterial->SetRoughness(0.2f);
    colorMaterial->SetMetalness(0.2f);

    //=================================================

    // ==========Load HDR equirectangular==============
    // TODO: Combine it iinto skybox class
    std::string envMapPath = "assets/env.hdr";
    unsigned int envSize = 2048;
    auto envMap = std::make_shared<Cubemap>(envSize, 0);
    envMap->LoadEquiToCubemap(envMapPath);

    auto skyShader = std::make_shared<Shader>("shader/debug.vert", "shader/debug.frag");

    // ================Initialize ImGui====================
	/*IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard navigation
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330"); // Use Version 330 for OpenGL
	*/

	//================Load Testing Model/objects=====================
	// Load complex ply model
	Mesh loadedModel;
	std::string modelPath = "assets/models/bun_zipper.ply";
    if (LoadPLYToMesh(modelPath, loadedModel)) {
        std::cout << "PLY model loaded and transformed successfully!" << std::endl;
    } else {
        std::cerr << "Failed to load and transform PLY model." << std::endl;
        return -1;  // Exit if loading fails
    }

    // Create scene manager
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

	// Sphere for simple material map testing
	auto sphere = std::make_shared<Sphere>(0.5f, 50, 50);
	auto plane = std::make_shared<Plane>(2.0f);

    auto sphereNode = std::make_shared<SceneNode>(sphere, texMaterial);
    auto planeNode = std::make_shared<SceneNode>(plane, texMaterial);
    auto loadedModelNode = std::make_shared<SceneNode>(std::make_shared<Mesh>(loadedModel), texMaterial);

    // Add node to scene
    //scene->AddNode(sphereNode);
    scene->AddNode(planeNode);
    //scene->AddNode(sphereNode);
    scene->AddNode(loadedModelNode);

    // ====================Upload env map======================
    auto pbrShader = std::make_shared<Shader>("shader/pbr_tex.vert", "shader/pbr_tex.frag"); // Init prb shader
    pbrShader->Use(); 
    // Upload env mapping
    env.UploadToShader(pbrShader);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // ==================== Main Render Loop ===================
    while (!glfwWindowShouldClose(window)) {
        ProcessInput(window);

        int fbw = 0, fbh = 0;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        if (fbw == 0 || fbh == 0) { glfwPollEvents(); continue; }
        glViewport(0, 0, fbw, fbh);
        const float aspect = (float)fbw / (float)fbh;

        glClearColor(0.02f, 0.05f, 0.03f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Initialize imgui new frame
		/*ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();*/

        // ================== Camera matrix =======================
        glm::mat4 model = glm::mat4(1.0f); 
        const glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        const glm::mat4 proj = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

        // =================== Render Skybox ====================
        glDepthFunc(GL_LEQUAL); // Skybox depth test (depth is set to always be the farthest)
        glDepthMask(GL_FALSE);  // Disable depth writes

        skyShader->Use();
        skyShader->SetUniform("view", glm::mat4(glm::mat3(view)));  // Remove translation for skybox
        skyShader->SetUniform("projection", proj);
        skyShader->SetUniform("cubemap", SKYBOX_TEXTURE_UNIT);

        glActiveTexture(GL_TEXTURE0 + SKYBOX_TEXTURE_UNIT);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envMap->GetTexture());

        cube->Draw();  // Render skybox

        glDepthMask(GL_TRUE);  // Enable depth writes
        glDepthFunc(GL_LESS);  // Restore depth function

        // ==================== Render Scene Objects (Sphere) =====================
        pbrShader->Use();
        // Upload model, view and proj matrix
        //pbrShader->SetUniform("model", model);
        pbrShader->SetUniform("view", view);
        pbrShader->SetUniform("projection", proj);
        pbrShader->SetUniform("camPos", camPos);

        scene->Render(pbrShader);

    	//plane->Draw();

        // ================== Render ImGui UI =====================
		/*ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);
		ImGui::Begin("Material Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
		// Material setting
		ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.3f",
						ImGuiSliderFlags_NoInput);  // disable text input
		ImGui::SliderFloat("Metalness", &metalness, 0.0f, 1.0f, "%.3f",
						ImGuiSliderFlags_NoInput);
		// Color setting
		ImGui::SliderFloat("R", &R, 0.0f, 1.0f, "%.3f",
				ImGuiSliderFlags_NoInput); 
		ImGui::SliderFloat("G", &G, 0.0f, 1.0f, "%.3f",
				ImGuiSliderFlags_NoInput);
		ImGui::SliderFloat("B", &B, 0.0f, 1.0f, "%.3f",
				ImGuiSliderFlags_NoInput);
		// Color settting
		ImGui::End(); 
		
		// Imgui Render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());*/

        // ==================== Swap Buffers =====================
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up ImGui and OpenGL resources
	/*ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();*/

    return 0;
    
}