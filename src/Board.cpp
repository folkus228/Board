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

int main() {
    // 1. GLFW & GLAD
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 450, "ImGui+GLM+GLAD+GLFW Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // 2. ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // 3. GLM: вектор цвета (RGB)
    glm::vec3 clear_color = glm::vec3(0.45f, 0.55f, 0.60f);

    // 4. PipeLine
    PipeLine pipeline(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Base3DPass basepass;
    pipeline.SetRenderPass(&basepass);
    pipeline.Start();

    // 5 Meshes and Shaders
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

    PointData pointData[] = {
        {-0.5f, -0.5f, /*-0.5f,*/ 255, 255, 255, 255 },    // Нижний левый передний - 0
        { 0.5f, -0.5f, /*-0.5f,*/ 255, 255, 0  , 255 },    // Нижний правый передний - 1
        { 0.5f,  0.5f, /*-0.5f,*/ 255, 255, 255, 255 },    // Верхний правый передний - 2
        {-0.5f,  0.5f, /*-0.5f,*/ 255, 255, 0  , 255 }     // Верхний левый передний - 3
    };

    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    Mesh cubeMesh(GL_TRIANGLES, 6); // в нинциализаторе и так USE прописывается
    cubeMesh.Use();
    cubeMesh.AddData(pointData, sizeof(pointData));
    cubeMesh.AddIndices(indices);
    cubeMesh.Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointData), (void*)0,
        1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointData), (void*)(2 * sizeof(float)));

    Shader baseShader;
    std::string pathToBaseVert = "C:/all/proj/VisualStudio/Board/res/shaders/base.vert";
    std::string pathToBaseFrag = "C:/all/proj/VisualStudio/Board/res/shaders/base.frag";
    baseShader.LoadShaderFromFile(GL_VERTEX_SHADER, pathToBaseVert);
    baseShader.LoadShaderFromFile(GL_FRAGMENT_SHADER, pathToBaseFrag);
    baseShader.CompileProgram();

    // 6. Scene and Objects
    Transform2D baseTransform;
    Camera2D camera{ baseTransform, 10.0f };
    Object cube{ baseTransform };
    Object cube2{ baseTransform };
    cube2.t.SetParent(&cube.t);

    Scene scene;
    scene.AddObj(&camera);
    scene.AddObj(&cube);
    scene.AddRenderUnit(&cube, &cubeMesh, &baseShader);
    scene.AddRenderUnit(&cube2, &cubeMesh, &baseShader);

    // 7 Test Fiches
    glm::vec2 pos1{ 0.0, 0.0 };
    glm::vec2 pos2{ 0.0, 0.0 };
    float rot1 = 0;
    float rot2 = 0;
    glm::vec2 scale1{ 1.0, 1.0 };
    glm::vec2 scale2{ 1.0, 1.0 };

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
        if (ImGui::DragFloat2("##pos1", &pos1.x, 0.1, -5, 5))
            cube.t.SetLocalPXY(pos1);
        if (ImGui::DragFloat("##rot1", &rot1, 0.1, -360, 360))
            cube.t.SetLocalRotation(rot1);
        if (ImGui::DragFloat2("##scale1", &scale1.x, 0.1, -5, 5))
            cube.t.SetLocalScale(scale1);
        ImGui::Text("Cube 2");
        if (ImGui::DragFloat2("##pos2", &pos2.x, 0.1, -5, 5))
            cube2.t.SetLocalPXY(pos2);
        if (ImGui::DragFloat("##rot2", &rot2, 0.1, -360, 360))
            cube2.t.SetLocalRotation(rot2);
        if (ImGui::DragFloat2("##scale2", &scale2.x, 0.1, -5, 5))
            cube2.t.SetLocalScale(scale2);
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

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
