/*
 * Copyright (c) 2018 Isetta
 */
#include "EngineLoop.h"

#include "Audio/AudioModule.h"
#include "Collisions/CollisionsModule.h"
#include "Core/Memory/MemoryManager.h"
#include "Graphics/GUIModule.h"
#include "Graphics/RenderModule.h"
#include "Graphics/Window.h"
#include "Input/InputModule.h"
#include "Networking/NetworkingModule.h"

#include "Core/Config/Config.h"
#include "Core/Debug/DebugDraw.h"
#include "Core/Debug/Logger.h"
#include "Core/Filesystem.h"
#include "Core/Time/Clock.h"
#include "Events/Events.h"
#include "Input/Input.h"
#include "Input/KeyCode.h"
#include "Networking/NetworkManager.h"
#include "Scene/Entity.h"
#include "Scene/Level.h"
#include "Scene/LevelManager.h"
#include "brofiler/ProfilerCore/Brofiler.h"

namespace Isetta {

EngineLoop& EngineLoop::Instance() {
  static EngineLoop instance;
  return instance;
}

EngineLoop::EngineLoop() {
  memoryManager = new MemoryManager{};
  windowModule = new WindowModule{};
  renderModule = new RenderModule{};
  inputModule = new InputModule{};
  guiModule = new GUIModule{};
  collisionsModule = new CollisionsModule{};
  audioModule = new AudioModule{};
  networkingModule = new NetworkingModule{};
}
EngineLoop::~EngineLoop() {
  delete memoryManager;
  delete windowModule;
  delete renderModule;
  delete inputModule;
  delete guiModule;
  delete collisionsModule;
  delete audioModule;
  delete networkingModule;
}

void EngineLoop::StartUp() {
  BROFILER_EVENT("Start Up");

  Logger::NewSession();
  Config::Instance().Read("config.cfg");
  if (Filesystem::Instance().FileExists("user.cfg")) {
    Config::Instance().Read("user.cfg");
  }

  intervalTime = 1.0 / Config::Instance().loopConfig.maxFps.GetVal();
  maxSimulationCount = Config::Instance().loopConfig.maxSimCount.GetVal();

  isGameRunning = true;

  memoryManager->StartUp();
  windowModule->StartUp();
  renderModule->StartUp(windowModule->winHandle);
  inputModule->StartUp(windowModule->winHandle);
  guiModule->StartUp(windowModule->winHandle);
  DebugDraw::StartUp();
  collisionsModule->StartUp();
  audioModule->StartUp();
  networkingModule->StartUp();

  LevelManager::Instance().LoadStartupLevel();

  StartGameClock();
}

void EngineLoop::Update() {
  BROFILER_FRAME("Main Thread");

  GetGameClock().UpdateTime();

  // TODO(All) Add networking update

  // end Networking update
  // LOG_INFO(Debug::Channel::General,
  // "//////////////UpdateStart//////////////");

  // Client part
  accumulateTime += GetGameClock().GetDeltaTime();

  for (int i = 0; i < maxSimulationCount && accumulateTime > intervalTime;
       i++) {
    FixedUpdate(intervalTime);
    // LOG_INFO(Debug::Channel::General,
    // std::to_string(accumulateTime));
    accumulateTime -= intervalTime;
  }

  VariableUpdate(GetGameClock().GetDeltaTime());
  // LOG_INFO(Debug::Channel::General,
  // "//////////////UpdateEnd//////////////");
}

void EngineLoop::FixedUpdate(float deltaTime) {
  BROFILER_CATEGORY("Fixed Update", Profiler::Color::IndianRed);

  networkingModule->Update(deltaTime);
  collisionsModule->Update(deltaTime);
  LevelManager::Instance().currentLevel->FixedUpdate();
}
void EngineLoop::VariableUpdate(float deltaTime) {
  BROFILER_CATEGORY("Variable Update", Profiler::Color::SteelBlue);

  inputModule->Update(deltaTime);
  LevelManager::Instance().currentLevel->Update();
  Events::Instance().Update();
  LevelManager::Instance().currentLevel->LateUpdate();
  audioModule->Update(deltaTime);
  renderModule->Update(deltaTime);
  DebugDraw::Update();
  guiModule->Update(deltaTime);
  windowModule->Update(deltaTime);
  memoryManager->Update();
}

void EngineLoop::ShutDown() {
  BROFILER_EVENT("Shut Down");

  LevelManager::Instance().UnloadLevel();
  networkingModule->ShutDown();
  audioModule->ShutDown();
  collisionsModule->ShutDown();
  DebugDraw::ShutDown();
  guiModule->ShutDown();
  inputModule->ShutDown();
  renderModule->ShutDown();
  windowModule->ShutDown();
  memoryManager->ShutDown();
  Logger::ShutDown();
}

void EngineLoop::StartGameClock() const { GetGameClock(); }

void EngineLoop::Run() {
  ASSERT(!isGameRunning);
  StartUp();
  while (isGameRunning) {
    Update();
  }
  ShutDown();
}

Clock& EngineLoop::GetGameClock() {
  static Clock gameTime{};
  return gameTime;
}
}  // namespace Isetta
