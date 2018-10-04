/*
 * Copyright (c) 2018 Isetta
 */
#include "Graphics/GUIModule.h"

#include "Core/Debug/Debug.h"
#include "Core/Debug/Logger.h"
#include "Core/Memory/MemoryManager.h"
#include "Graphics/GUI.h"
#include "Input/Input.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

void* MemAlloc(size_t size, void* user_data) {
  if (user_data) {
    // LOG_INFO(Isetta::Debug::Channel::GUI, {(char*)user_data, "alloc"});
  }
  return malloc(size);
  // return Isetta::MemoryManager::AllocSingleFrame(size);
}
void FreeAlloc(void* ptr, void* user_data) {
  if (user_data) {
    // LOG_INFO(Isetta::Debug::Channel::GUI, {(char*)user_data, "free"});
  }
  free(ptr);
}

namespace Isetta {
void GUIModule::StartUp(GLFWwindow* win) {
  GUI::guiModule = this;
  winHandle = win;
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  ImGui::SetAllocatorFunctions(MemAlloc, FreeAlloc, "GUI");

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  // Setup Dear ImGui binding
  ImGui_ImplGlfw_InitForOpenGL(winHandle, false);
  ImGui_ImplOpenGL3_Init();
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard
  // Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable
  // Gamepad Controls

  // Setup style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Load Fonts
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese());
  // IM_ASSERT(font != NULL);

  Input::RegisterMouseButtonCallback(ImGui_ImplGlfw_MouseButtonCallback);
  Input::RegisterScrollCallback(ImGui_ImplGlfw_ScrollCallback);
  Input::RegisterKeyCallback(ImGui_ImplGlfw_KeyCallback);
  Input::RegisterCharCallback(ImGui_ImplGlfw_CharCallback);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Render();
  // TODO(Jacob)
  // ImGui::SetAllocatorFunctions(MemAlloc, FreeAlloc, "GUI");

  // LOG_INFO(Isetta::Debug::Channel::GUI,
  // "-------------GUI START-------------");
}

void GUIModule::Update(float deltaTime) {
  // LOG_INFO(Isetta::Debug::Channel::GUI,
  //         "-------------GUI UPDATE 1-------------");
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // LOG_INFO(Isetta::Debug::Channel::GUI,
  //         "-------------GUI UPDATE 2-------------");
  glfwGetWindowSize(winHandle, &winWidth, &winHeight);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
  ImGui::SetNextWindowBgAlpha(0.0f);
  ImGui::SetNextWindowPos(ImVec2());
  ImGui::Begin(
      "MainWindow", NULL,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
          ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove |
          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
          ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
          ImGuiWindowFlags_NoFocusOnAppearing |
          ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
  ImGui::PopStyleVar(2);

  for (const auto& callback : updateCallbacks) {
    callback();
  }
  // TODO (Jacob) clear callbacks after each frame

  ImGui::End();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIModule::ShutDown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void GUIModule::OnUpdate(const Action<>& callback) {
  updateCallbacks.push_back(callback);
}

}  // namespace Isetta