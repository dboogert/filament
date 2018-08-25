/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_SAMPLES_WEBAPP_H
#define TNT_FILAMENT_SAMPLES_WEBAPP_H

#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/Viewport.h>

#include <chrono>
#include <functional>

#include <emscripten.h>

extern "C" {
extern void launch();
extern void render();
}

class WebApp {
public:
    using Engine = filament::Engine;
    using View = filament::View;
    using Scene = filament::Scene;
    using SetupCallback = std::function<void(Engine*, View*, Scene*)>;
    using CleanupCallback = std::function<void(Engine*, View*, Scene*)>;
    using ImGuiCallback = std::function<void(Engine*, View*)>;
    using AnimCallback = std::function<void(Engine*, View*, double now)>;

    static WebApp* get() {
        static WebApp app;
        return &app;
    }

    void run(SetupCallback setup, CleanupCallback cleanup,
            ImGuiCallback imgui, AnimCallback animation) {
        mAnimation = animation;
        mEngine = Engine::create(Engine::Backend::OPENGL);
        mSwapChain = mEngine->createSwapChain(nullptr);
        mScene = mEngine->createScene();
        mRenderer = mEngine->createRenderer();
        mCamera = mEngine->createCamera();
        mView = mEngine->createView();
        mView->setScene(mScene);
        mView->setCamera(mCamera);
        setup(mEngine, mView, mScene);
    }

    void resize(uint32_t width, uint32_t height) {
        mView->setViewport({0, 0, width, height});
    }

    void render() {
        auto milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);
        mAnimation(mEngine, mView, milliseconds_since_epoch / 1000.0);
        if (mRenderer->beginFrame(mSwapChain)) {
            mRenderer->render(mView);
            mRenderer->endFrame();
        }
        mEngine->execute();
    }

private:
    WebApp() { }
    Engine* mEngine = nullptr;
    Scene* mScene = nullptr;
    View* mView = nullptr;
    filament::Renderer* mRenderer = nullptr;
    filament::Camera* mCamera = nullptr;
    filament::SwapChain* mSwapChain = nullptr;
    AnimCallback mAnimation;
};

extern "C" void render() {
    WebApp::get()->render();
}

extern "C" void resize(uint32_t width, uint32_t height) {
    WebApp::get()->resize(width, height);
}

#endif // TNT_FILAMENT_SAMPLES_WEBAPP_H
