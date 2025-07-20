#pragma once

#include "Common.h"

#include "AudioManager.h"
#include "DigitManager.h"
#include "EntityManager.h"
#include "FileManager.h"
#include "FontManager.h"
#include "RenderManager.h"
#include "SpriteManager.h"
#include "State.h"
#include "SurfaceManager.h"

#include <cstdint>
#include <memory>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL.h>
#include <unordered_map>


namespace NGame {

class TApp {
public:
    static TApp* Instance();
    int Run();
    
    TState& State();
    const TState& State() const;
    
    TFileManager& FileManager();
    TRenderManager& RenderManager();
    TEntityManager& EntityManager();
    TSurfaceManager& SurfaceManager();
    TSpriteManager& SpriteManager();
    TFontManager& FontManager();
    TDigitManager& DigitManager();
    TAudioManager& AudioManager();

    const TFileManager& FileManager() const;
    const TRenderManager& RenderManager() const;
    const TEntityManager& EntityManager() const;
    const TSurfaceManager& SurfaceManager() const;
    const TSpriteManager& SpriteManager() const;
    const TFontManager& FontManager() const;
    const TDigitManager& DigitManager() const;
    const TAudioManager& AudioManager() const;

private:
    TApp();
    ~TApp();
    void Process();

private:
    std::unique_ptr<TFileManager> FileManager_;
    std::unique_ptr<TRenderManager> RenderManager_;
    std::unique_ptr<TEntityManager> EntityManager_;
    std::unique_ptr<TSurfaceManager> SurfaceManager_;
    std::unique_ptr<TSpriteManager> SpriteManager_;
    std::unique_ptr<TFontManager> FontManager_;
    std::unique_ptr<TDigitManager> DigitManager_;
    std::unique_ptr<TAudioManager> AudioManager_;
    TState State_;
};

} // namespace NGame

