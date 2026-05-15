// Board.cpp: определяет точку входа для приложения.

#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "render/PipeLine.h"
#include "render/Base3DPass.h"
#define STB_IMAGE_IMPLEMENTATION // макрос для включения функций stb_image
#include "helpers/stb_image.h"
#include "render/Material.h"
#include "render/TextureManager.h"
#include "font/FontController.h"

int main() {
    // 1 GLFW & GLAD
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 450, "ImGui+GLM+GLAD+GLFW Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // 2 ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // . GLM: вектор цвета (RGB)
    glm::vec3 clear_color = glm::vec3(0.45f, 0.55f, 0.60f);

    // 4 PipeLine
    PipeLine pipeline(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Base3DPass basepass;
    pipeline.SetRenderPass(&basepass);
    pipeline.Start();
    
    // 5 Load Textures
    
    //stbi_set_flip_vertically_on_load(true);
    //int width, height, channels;
    //unsigned char* data = stbi_load("C:/all/proj/VisualStudio/Board/res/textures/texture.png", &width, &height, &channels, 0);
    //if (!data) {
    //    std::cout << "Не удалось загрузить текстуру!" << std::endl;
    //    return false;
    //}
    //std::cout << "channels: " << channels << std::endl;
    //TextureManager::Init();
    //unsigned int texID = TextureManager::AddAtlas(GL_RGBA); //LoadTexture(height, width, data, GL_RGBA);
    //TextureManager::AddTextureToAtlas(texID, height, width, data);
    //TextureManager::BindTexture(texID, 0);
    //
    //stbi_image_free(data);

    FontController fontController;
    unsigned int arialID = fontController.AddFont("C:/all/proj/VisualStudio/Board/res/fonts/arial.ttf", 48);
    unsigned int pixelID = fontController.AddFont("C:/all/proj/VisualStudio/Board/res/fonts/pixel.ttf", 48);
    unsigned int beerID = fontController.AddFont("C:/all/proj/VisualStudio/Board/res/fonts/beer.ttf", 48 * 3);
    fontController.UseFont(arialID, 0);


    // 6 Meshes and Shaders
    struct PointData
    {
        float x;
        float y;
        //float z;
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };
    // cube
    PointData cubeData[] = {
        {-0.5f, -0.5f, 255, 255, 255, 255 },    // Нижний левый передний - 0
        { 0.5f, -0.5f, 255, 255, 0  , 255 },    // Нижний правый передний - 1
        { 0.5f,  0.5f, 255, 255, 255, 255 },    // Верхний правый передний - 2
        {-0.5f,  0.5f, 255, 255, 0  , 255 }     // Верхний левый передний - 3
    };
    GLuint cubeIndices[] = {
        0, 1, 2,
        0, 2, 3
    };
    Mesh cubeMesh(GL_TRIANGLES, 6);
    cubeMesh.Use();
    cubeMesh.AddData(cubeData, sizeof(cubeData));
    cubeMesh.AddIndices(cubeIndices);
    cubeMesh.Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointData), (void*)0,
        1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointData), (void*)(2 * sizeof(float)));
    
    struct PointTextureData
    {
        float x;
        float y;
        //float z;
        float u, v;
    };
    // cube with texture
    PointTextureData cubeTextureData[] = {
        //{-0.5f, -0.5f, catTex.uv.u0, catTex.uv.v0 },    // Нижний левый - 0
        //{ 0.5f, -0.5f, catTex.uv.u1, catTex.uv.v0 },    // Нижний правый - 1
        //{ 0.5f,  0.5f, catTex.uv.u1, catTex.uv.v1 },    // Верхний правый - 2
        //{-0.5f,  0.5f, catTex.uv.u0, catTex.uv.v1 }     // Верхний левый - 3
        {-0.5f, -0.5f, 0.0f, 0.0f },    // Нижний левый - 0
        { 0.5f, -0.5f, 1.0f, 0.0f },    // Нижний правый - 1
        { 0.5f,  0.5f, 1.0f, 1.0f },    // Верхний правый - 2
        {-0.5f,  0.5f, 0.0f, 1.0f }     // Верхний левый - 3
    };
    Mesh cubeTextureMesh(GL_TRIANGLES, 6);
    cubeTextureMesh.Use();
    cubeTextureMesh.AddData(cubeTextureData, sizeof(cubeTextureData));
    cubeTextureMesh.AddIndices(cubeIndices);
    cubeTextureMesh.Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointTextureData), (void*)0,
        2, 2, GL_FLOAT, GL_FALSE, sizeof(PointTextureData), (void*)(2 * sizeof(float)));

    Shader baseShader;
    std::string pathToBaseVert = "C:/all/proj/VisualStudio/Board/res/shaders/base.vert";
    std::string pathToBaseFrag = "C:/all/proj/VisualStudio/Board/res/shaders/base.frag";
    baseShader.LoadShaderFromFile(GL_VERTEX_SHADER, pathToBaseVert);
    baseShader.LoadShaderFromFile(GL_FRAGMENT_SHADER, pathToBaseFrag);
    baseShader.CompileProgram();

    Shader textureShader;
    std::string pathToTextureVert = "C:/all/proj/VisualStudio/Board/res/shaders/base_with_tex.vert";
    std::string pathToTextureFrag = "C:/all/proj/VisualStudio/Board/res/shaders/texture.frag";
    textureShader.LoadShaderFromFile(GL_VERTEX_SHADER, pathToTextureVert);
    textureShader.LoadShaderFromFile(GL_FRAGMENT_SHADER, pathToTextureFrag);
    textureShader.CompileProgram();

    // 7 Scene and Objects
    Transform2D baseTransform{ {-3, 0, 0} };
    Camera2D camera{ baseTransform, 10.0f };
    Object cube{ baseTransform };
    baseTransform.SetLocalPX(-2);
    Object cube2{ baseTransform };
    baseTransform.SetLocalPosition({-2, 5, 1.0});
    baseTransform.SetLocalScale({ 15, 15 });
    Object cube3{ baseTransform };
    cube2.t.SetParent(&cube.t);
    cube2.t.SetLocalPZ(-1);
    Material mat(&textureShader);
    mat.SetUniformValue("pipi", 2.41f);

    Scene scene;
    scene.AddObj(&camera);
    scene.AddObj(&cube);
    scene.AddObj(&cube2);
    scene.AddObj(&cube3);
    scene.AddRenderUnit(&cube, &cubeMesh, &baseShader);
    scene.AddRenderUnit(&cube2, &cubeMesh, &baseShader);
    scene.AddRenderUnit(&cube3, &cubeTextureMesh, &textureShader);
    std::string uniform_name = "ourTexture";
    textureShader.SetUniform<GLuint>(uniform_name, 0);

    // 8 Test Fiches
    glm::vec2 pos1{ 0.0, 0.0 };
    glm::vec2 pos2{ 0.0, 0.0 };
    glm::vec2 pos3{ 0.0, 0.0 };
    glm::vec2 scale1{ 1.0, 1.0 };
    glm::vec2 scale2{ 1.0, 1.0 };
    glm::vec2 scale3{ 1.0, 1.0 };
    float rot1 = 0;
    float rot2 = 0;
    float rot3 = 0;

    glm::vec2 posline{ 2.0, 0.0 };
    float rotline = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Простенькое окно ImGui
        ImGui::Begin("Settings");
        ImGui::Text("GLM & ImGui Integration");
        ImGui::ColorEdit3("Background", &clear_color.x); // Прямая привязка к glm::vec3
        ImGui::Text("Cube 1");
        if (ImGui::DragFloat2("##pos1", &pos1.x, 0.1f, -5, 5))
            cube.t.SetLocalPXY(pos1);
        if (ImGui::SliderFloat("##rot1", &rot1, -360, 360))
            cube.t.SetLocalRotation(rot1);
        if (ImGui::DragFloat2("##scale1", &scale1.x, 0.1f, -5, 5))
            cube.t.SetLocalScale(scale1);
        ImGui::Text("Cube 2");
        if (ImGui::DragFloat2("##pos2", &pos2.x, 0.1f, -5, 5))
            cube2.t.SetLocalPXY(pos2);
        if (ImGui::SliderFloat("##rot2", &rot2, -360, 360))
            cube2.t.SetLocalRotation(rot2);
        if (ImGui::DragFloat2("##scale2", &scale2.x, 0.1f, -5, 5))
            cube2.t.SetLocalScale(scale2);
        ImGui::Text("Cube 3");
        if (ImGui::DragFloat2("##pos3", &pos3.x, 0.1f, -5, 5))
            cube3.t.SetLocalPXY(pos3);
        if (ImGui::SliderFloat("##rot3", &rot3, -360, 360))
            cube3.t.SetLocalRotation(rot3);
        if (ImGui::DragFloat2("##scale3", &scale3.x, 0.1f, -15, 15))
            cube3.t.SetLocalScale(scale3);
        ImGui::Text("Text Setters");
        if (ImGui::Button("arial", { 100, 35 }))
            fontController.UseFont(arialID, 0);
        if (ImGui::Button("pixel", { 100, 35 }))
            fontController.UseFont(pixelID, 0);
        if (ImGui::Button("beer", { 100, 35 }))
            fontController.UseFont(beerID, 0);
        ImGui::End();

        // Rendering
        ImGui::Render();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        
        pipeline.ExecuteRender(&scene, &camera);


        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    TextureManager::Shutdown();
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
