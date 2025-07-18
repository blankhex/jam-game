#pragma once

#include "Common.h"

#include "LRU.h"
#include "SurfaceManager.h"

#include <array>
#include <memory>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <variant>
#include <vector>

namespace NGame {

class TRenderManager {
public:
    enum ELayer {
        Tile,
        Background,
        BackgroundGlow,
        Foreground,
        ForegroundGlow,
        Effects,
        EffectsGlow,
        Light,
        Interface,
        SentinelMax
    };

    struct TColorCommand {
        std::uint8_t Red;
        std::uint8_t Green;
        std::uint8_t Blue;
        std::uint8_t Alpha;
    };

    struct TRectCommand {
        Vec2i Position;
        Vec2i Size;
        bool ShouldFill;
    };

    struct TLineCommand {
        Vec2i Start;
        Vec2i End;
    };

    struct TTextureCommand {
        std::shared_ptr<SDL_Texture> Texture;
    };

    struct TSpriteCommand {
        Vec2i TexturePosition;
        Vec2i TextureSize;
        Vec2i TargetPosition;
        Vec2i TargetSize;
        SDL_RendererFlip Flip;
    };

    struct TGeometryCommand {
        const SDL_Vertex* Vertices;
        std::size_t VerticesCount;
        const int* Indices;
        std::size_t IndicesCount;
    };

    using TCommand = std::variant<TColorCommand, TRectCommand, TLineCommand, TTextureCommand, TSpriteCommand, TGeometryCommand>;

    TRenderManager(TSurfaceManager& surfaceManager, int width = 640, int height = 480, const std::string& title = "Game", std::size_t cacheSize = 4);
    DELETE_COPY(TRenderManager)

    std::shared_ptr<SDL_Texture> Get(const std::string& path);
    void SetLayer(ELayer layer);
    void SetColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 255);
    void SetTexture(const std::shared_ptr<SDL_Texture>& texture);
    void DrawRect(const Vec2i& position, const Vec2i& size, bool filled = false);
    void DrawLine(const Vec2i& start, const Vec2i& end);
    void DrawSprite(const Vec2i& texturePosition, const Vec2i& textureSize, const Vec2i& targetPosition, const Vec2i& targetSize, SDL_RendererFlip flip = SDL_FLIP_NONE);
    void DrawGeometry(const SDL_Vertex* vertices, std::size_t verticesCount, const int* indices, std::size_t indicesCount);

    void SetCamera(const Vec2i& position);
    const Vec2i& Camera() const;
    const Vec2i& Size() const;

    bool IsLightEnabled() const;
    void EnableLight(bool value);
    void SetDefaultLightColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue);
    std::uint8_t DefaultLightRed() const;
    std::uint8_t DefaultLightGreen() const;
    std::uint8_t DefaultLightBlue() const;

    void Reset();
    void Run();
    bool Input(SDL_Event* event);
    bool IsRunning();
    void Shutdown();

private:
    SDL_Window* Window_;
    SDL_Renderer* Renderer_;
    TSurfaceManager& SurfaceManager_;
    TLRU<std::string, std::shared_ptr<SDL_Texture>> Cache_;
    std::array<std::vector<TCommand>, SentinelMax> Commands_;
    std::array<std::shared_ptr<SDL_Texture>, SentinelMax> ActiveTextures_;
    SDL_Texture* ScreenTexture_;
    SDL_Texture* LightTexture_;
    TColorCommand DefaultLightColor_;
    bool LightEnabled_ = false;
    ELayer Layer_;
    Vec2i Camera_;
    Vec2i Size_;
    std::vector<SDL_Vertex> VerticesBuffer_;
};

} // namespace NGame