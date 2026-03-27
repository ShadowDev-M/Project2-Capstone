#include "pch.h"
#include "ScreenManager.h"
#include "FBOManager.h"

void ScreenManager::Initialize(SDL_Window* window_, const SettingsConfig& cfg_)
{
    window = window_;
    cfg = cfg_;

    SDL_SetWindowTitle(window, cfg.windowTitle.c_str());
    SDL_GL_SetSwapInterval(cfg.vsync ? 1 : 0);

    // makes sure window doesn't resize in gamebuild
#ifndef ENGINE_EDITOR
    SDL_SetWindowResizable(window, SDL_FALSE);
#endif
}

void ScreenManager::HandleResize(int w, int h, Source source_)
{
    switch (source_) {
    case Source::EditorUI:
        // Game window setting to change the render resolution 
        SetRenderResolution(w, h);
        break;
    
    case Source::Script:
        // checks to see if its the game build, if it is then it changes the render and the display resolution, i.e like a settings menu that changes the resolution
        if (isGameBuild()) {
            SetRenderResolution(w, h);
            SetDisplayResolution(w, h);
        }
        // otherwise its the editor so just change the render resolution
        else {
            SetRenderResolution(w, h);
        }
        break;
    
    case Source::WindowDrag:
        // if editor dragging then just change the display size, dont touch the render or fbos, (render should maintain aspect ratio)
        if (!isGameBuild()) {
            SetDisplayResolution(w, h);
        }
        else {
            SetDisplayResolution(w, h);
            SetRenderResolution(w, h);
        }
        break;
    }
}

void ScreenManager::SetRenderResolution(int w, int h)
{
    // if the width and height is either less then 0 or the same as it already is then return
    if (w <= 0 || h <= 0) return;
    if (cfg.renderWidth == w && cfg.renderHeight == h) return;

    // setting config width and height
    cfg.renderWidth = w; 
    cfg.renderHeight = h;

    // getting all fbos, and resizing them according to the new width and height
    for (auto& [fbo, data] : FBOManager::getInstance().getAllFBOs()) {
        if (data.isShadow || data.isCubeShadow) continue;
        FBOManager::getInstance().OnResize(fbo, w, h);
    }

    renderResolutionNotifier(w, h);
}

void ScreenManager::SetDisplayResolution(int w, int h)
{
    if (!window || w <= 0 || h <= 0) return;

    // setting config width and height
    cfg.displayWidth = w;
    cfg.displayHeight = h;

    SDL_SetWindowSize(window, w, h);

    displayResolutionNotifier(w, h);
}

void ScreenManager::setTargetFPS(int fps_)
{
    cfg.targetFPS = (fps_ <= 0) ? 0 : fps_;
}

void ScreenManager::setVSync(bool enabled_)
{
    cfg.vsync = enabled_;
    SDL_GL_SetSwapInterval(enabled_ ? 1 : 0);
}

void ScreenManager::setWindowTitle(const std::string& title_)
{
    cfg.windowTitle = title_;
    if (window) SDL_SetWindowTitle(window, title_.c_str());
}