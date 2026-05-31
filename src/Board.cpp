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
#include "render/VideoManager.h"
#include "font/FontManager.h"
#include "font/FontComponent.h"
#include "render/ParticleManager.h"

int main() {
    // 1 GLFW & GLAD
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800*1.5, 450*1.5, "ImGui+GLM+GLAD+GLFW Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // 2 ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // . GLM: вектор цвета (RGB)
    glm::vec3 clear_color = glm::vec3(0.45f, 0.55f, 0.60f);

    // 4 PipeLine
    PipeLine pipeline(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Base3DPass basepass;
    pipeline.SetRenderPass(&basepass);
    pipeline.Start();

    FontManager::Init();
    unsigned int arialID = FontManager::AddFont("C:/all/proj/VisualStudio/Board/res/fonts/arial.ttf", 96);
    unsigned int pixelID = FontManager::AddFont("C:/all/proj/VisualStudio/Board/res/fonts/pixel.ttf", 96);
    unsigned int beerID = FontManager::AddFont("C:/all/proj/VisualStudio/Board/res/fonts/beer.ttf", 96);
    FontManager::UseFont(arialID, 0);

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

    Shader fontShader;
    std::string pathToFontVert = "C:/all/proj/VisualStudio/Board/res/shaders/font.vert";
    std::string pathToFontFrag = "C:/all/proj/VisualStudio/Board/res/shaders/font.frag";
    fontShader.LoadShaderFromFile(GL_VERTEX_SHADER, pathToFontVert);
    fontShader.LoadShaderFromFile(GL_FRAGMENT_SHADER, pathToFontFrag);
    fontShader.CompileProgram();

    Shader particleShader;
    std::string pathToParticleVert = "C:/all/proj/VisualStudio/Board/res/shaders/particle.vert";
    std::string pathToParticleFrag = "C:/all/proj/VisualStudio/Board/res/shaders/base.frag";
    particleShader.LoadShaderFromFile(GL_VERTEX_SHADER, pathToParticleVert);
    particleShader.LoadShaderFromFile(GL_FRAGMENT_SHADER, pathToParticleFrag);
    particleShader.CompileProgram();

    // 8 Scene and Objects
    Object camera;
    camera.AddComponent<Camera2D>(10.0f);
    Object cube;
    Transform2D* Tcube = cube.GetT();
    Object cube2;
    Transform2D* Tcube2 = cube2.GetT();
    Object video;
    Transform2D* Tvideo = video.GetT();
    Object text;
    Transform2D* Ttext = text.GetT();
    Object dtext;
    Transform2D* Tdtext = dtext.GetT();
    Object particle;
    Transform2D* Tparticle = particle.GetT();

    Tcube->SetLocalPX(-1);
    Tcube2->SetParent(Tcube);
    Tcube2->SetLocalPosition({0, 0, -1});
    Tvideo->SetLocalPosition({ 0, 0, 1.0 });
    Tvideo->SetLocalScale({ 17.7777777778, -10 });

    cube.AddComponent<Material>(&baseShader);
    cube2.AddComponent<Material>(&baseShader);
    Material* Matvideo = video.AddComponent<Material>(&textureShader);
    Video* Cvideo = video.AddComponent<Video>();
    Cvideo->LoadVideo("C:\\all\\proj\\VisualStudio\\Board\\res\\videos\\video.mp4");
    StreamData video_data;
    Cvideo->GetCodecAndStream(video_data);
    Cvideo->InitCodecCTX(video_data);
    AVFrame* frame = nullptr;
    Cvideo->NextFrame(video_data, frame);
    unsigned int video_tex_idx = Cvideo->CreateFrameTextureTestFicha(video_data, frame);
    std::string uniColor = "uniColor";
    text.AddComponent<Material>(&fontShader)->SetUniformValue(uniColor, glm::vec4{ 1, 1, 1, 1 });
    text.AddComponent<StaticText>(1);
    dtext.AddComponent<Material>(&fontShader)->SetUniformValue(uniColor, glm::vec4{ 0, 0, 1, 1 });
    DynamicText* dtextc = dtext.AddComponent<DynamicText>(GL_DYNAMIC_DRAW, 1);
    ParticleSystem* ps = particle.AddComponent<ParticleSystem>(10000, 20, glm::vec2{0.f, 3.f}, "C:/all/proj/VisualStudio/Board/res/shaders/particles.comp");
    Material* Mparticle = particle.AddComponent<Material>(&particleShader);

    Mesh* Mcube = cube.AddComponent<Mesh>(GL_TRIANGLES, 6);
    Mcube->Use();
    Mcube->AllocateData(cubeData, sizeof(cubeData));
    Mcube->AllocateIndices(cubeIndices);
    Mcube->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointData), (void*)0,
        1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointData), (void*)(2 * sizeof(float)));

    Mesh* Mcube2 = cube2.AddComponent<Mesh>(GL_TRIANGLES, 6);
    Mcube2->Use();
    Mcube2->AllocateData(cubeData, sizeof(cubeData));
    Mcube2->AllocateIndices(cubeIndices);
    Mcube2->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointData), (void*)0,
        1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointData), (void*)(2 * sizeof(float)));

    Mesh* Mvideo = video.AddComponent<Mesh>(GL_TRIANGLES, 6);
    Mvideo->Use();
    Mvideo->AllocateData(cubeTextureData, sizeof(cubeTextureData));
    Mvideo->AllocateIndices(cubeIndices);
    Mvideo->Compile(0, 2, GL_FLOAT, GL_FALSE, sizeof(PointTextureData), (void*)0,
        2, 2, GL_FLOAT, GL_FALSE, sizeof(PointTextureData), (void*)(2 * sizeof(float)));

    Camera2D* cameraC = camera.GetComponent<Camera2D>();

    Scene scene;
    scene.AddObj(&camera);
    scene.AddObj(&cube);
    scene.AddObj(&cube2);
    scene.AddObj(&video);
    scene.AddObj(&text);
    scene.AddObj(&dtext);
    scene.AddObj(&particle);
    std::string uniform_name = "ourTexture";
    Matvideo->SetTextureValue(uniform_name, video_tex_idx);
    textureShader.SetUniform<GLuint>(uniform_name, 0);

    // 9 Test Fiches
    glm::vec2 pos1{ 0.0, 0.0 };
    glm::vec2 pos2{ 0.0, 0.0 };
    glm::vec2 posv{ 0.0, 0.0 };
    glm::vec2 post{ 0.0, 0.0 };
    glm::vec2 postd{ 0.0, 0.0 };
    glm::vec2 posp{ 0.0, 0.0 };
    glm::vec2 scale1{ 1.0, 1.0 };
    glm::vec2 scale2{ 1.0, 1.0 };
    glm::vec2 scalev{ 5.0, 10.0 };
    glm::vec2 scalet{ 0.01, 0.01 };
    glm::vec2 scaletd{ 0.01, 0.01 };
    glm::vec2 scalep{ 1.f, 1.f };
    float rot1 = 0;
    float rot2 = 0;
    float rotv = 0;
    float rott = 0;
    float rottd = 0;
    float rotp = 0;
    float video_timer = 0;
    double video_time_counter = 0;

    int frame_counter = 0;
    double start_time = Time::GetTime();
    double frames = 0;
    ImVec4 frames_color = {0.f, 0.f, 0.f, 1.f};

    text.GetComponent<Transform2D>()->SetLocalScale({ 0.01, 0.01 });
    text.GetComponent<Transform2D>()->SetLocalPX(-8.1);
    dtext.GetComponent<Transform2D>()->SetLocalScale({ 0.01, 0.01 });
    char dtexts[256] = "dtext";
    scene.Start();
    while (!glfwWindowShouldClose(window)) {
        frame_counter += 1;
        glfwPollEvents();
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("Engine data");
            frames = frame_counter / (Time::GetTime() - start_time);
            if (frames > 120) {
                frames_color.x = 0.0f;
                frames_color.y = 1.0f;
            }
            else if (frames > 60) {
                frames_color.x = 1.0f;
                frames_color.y = 1.0f;
            }
            else {
                frames_color.x = 1.0f;
                frames_color.y = 0.0f;
            }
            ImGui::TextColored(frames_color, std::to_string(frames).c_str());
            ImGui::End();

            ImGui::Begin("Settings");            
            ImGui::Text("Edit dynamic text");
            if (ImGui::InputText("##text_edit", dtexts, 256)) {
                dtextc->SetText(dtexts);
                dtextc->Recalculate();
            }
            ImGui::Text("Cube 1");
            if (ImGui::DragFloat2("##pos1", &pos1.x, 0.1f, -5, 5))
                Tcube->SetLocalPXY(pos1);
            if (ImGui::SliderFloat("##rot1", &rot1, -360, 360))
                Tcube->SetLocalRotation(rot1);
            if (ImGui::DragFloat2("##scale1", &scale1.x, 0.1f, -5, 5))
                Tcube->SetLocalScale(scale1);
            ImGui::Text("Cube 2");
            if (ImGui::DragFloat2("##pos2", &pos2.x, 0.1f, -5, 5))
                Tcube2->SetLocalPXY(pos2);
            if (ImGui::SliderFloat("##rot2", &rot2, -360, 360))
                Tcube2->SetLocalRotation(rot2);
            if (ImGui::DragFloat2("##scale2", &scale2.x, 0.1f, -5, 5))
                Tcube2->SetLocalScale(scale2);
            ImGui::Text("Video");
            if (ImGui::DragFloat2("##posv", &posv.x, 0.1f, -5, 5))
                Tvideo->SetLocalPXY(posv);
            if (ImGui::SliderFloat("##rotv", &rotv, -360, 360))
                Tvideo->SetLocalRotation(rotv);
            if (ImGui::DragFloat2("##scalev", &scalev.x, 0.1f, -15, 15))
                Tvideo->SetLocalScale(scalev);
            if (ImGui::SliderFloat("##rewind", &video_timer, 0, Cvideo->GetVideoDuration())) {
                Cvideo->SetFramePtr(video_timer, video_data);
                video_time_counter = video_timer;
                Cvideo->NextFrame(video_data, frame);
            }
            ImGui::Text("Text Setters");
            if (ImGui::Button("arial", { 100, 35 })) {
                dtextc->SetFont(arialID);
                dtextc->Recalculate();
            }
            if (ImGui::Button("pixel", { 100, 35 })) {
                dtextc->SetFont(pixelID);
                dtextc->Recalculate();
            }
            if (ImGui::Button("beer", { 100, 35 })) {
                dtextc->SetFont(beerID);
                dtextc->Recalculate();
            }
            ImGui::Text("Text");
            if (ImGui::DragFloat2("##post", &post.x, 0.1f, -50, 50))
                Ttext->SetLocalPXY(post);
            if (ImGui::SliderFloat("##rott", &rott, -360, 360))
                Ttext->SetLocalRotation(rott);
            if (ImGui::DragFloat2("##scalet", &scalet.x, 0.001f, -15, 15))
                Ttext->SetLocalScale(scalet);
            ImGui::Text("Dynamic Text");
            if (ImGui::DragFloat2("##postd", &postd.x, 0.1f, -50, 50))
                Tdtext->SetLocalPXY(postd);
            if (ImGui::SliderFloat("##rottd", &rottd, -360, 360))
                Tdtext->SetLocalRotation(rottd);
            if (ImGui::DragFloat2("##scaletd", &scaletd.x, 0.001f, -50, 50))
                Tdtext->SetLocalScale(scaletd);
            ImGui::Text("ParticleSystem");
            if (ImGui::Button("Emit", { 150, 35 })) {
                ps->Emit();
            }
            if (ImGui::DragFloat2("##posp", &posp.x, 0.1f, -50, 50))
                Tparticle->SetLocalPXY(posp);
            if (ImGui::SliderFloat("##rotp", &rotp, -360, 360))
                Tparticle->SetLocalRotation(rotp);
            if (ImGui::DragFloat2("##scalep", &scalep.x, 0.1f, -50, 50))
                Tparticle->SetLocalScale(scalep);
            ImGui::End();
            ImGui::Render();
        }
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        Time::Update();

        scene.Update();
        pipeline.ExecuteRender(&scene, cameraC);

        video_time_counter += Time::GetDeltaTime();
        if (Cvideo->SmartNextFrame(video_data, frame, video_time_counter))
            Cvideo->SetFrameTextureTestFicha(video_tex_idx, frame);
        else {
            Cvideo->SetFramePtr(0, video_data);
            video_time_counter = 0;
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    TextureManager::Shutdown();
    FontManager::Shutdown();
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
