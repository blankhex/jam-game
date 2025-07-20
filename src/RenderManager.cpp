#include "RenderManager.h"

#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <stdexcept>

namespace NGame {

TRenderManager::TRenderManager(TSurfaceManager& surfaceManager, int width, int height, const std::string& title, std::size_t cacheSize)
    : SurfaceManager_(surfaceManager), Cache_(cacheSize), Size_(width, height) {
    Window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, 
                               SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_RESIZABLE);
    if (!Window_) {
        throw std::runtime_error("Can't create window");
    }

    Renderer_ = SDL_CreateRenderer(Window_, -1, SDL_RENDERER_ACCELERATED);
    if (!Renderer_) {
        SDL_DestroyWindow(Window_);
        throw std::runtime_error("Can't create renderer");
    }

    ScreenTexture_ = SDL_CreateTexture(Renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!ScreenTexture_) {
        SDL_DestroyRenderer(Renderer_);
        SDL_DestroyWindow(Window_);
        throw std::runtime_error("Can't create screen texture");
    }

    LightTexture_ = SDL_CreateTexture(Renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!LightTexture_) {
        SDL_DestroyTexture(ScreenTexture_);
        SDL_DestroyRenderer(Renderer_);
        SDL_DestroyWindow(Window_);
        throw std::runtime_error("Can't create light texture");
    }
    SDL_SetTextureBlendMode(LightTexture_, SDL_BLENDMODE_MOD);

    // Make window bigger
    if (width < 640 && height < 480) {
        SDL_SetWindowSize(Window_, 640, 480);
    }

    Reset();
}

std::shared_ptr<SDL_Texture> TRenderManager::Get(const std::string& path) {
    if (Cache_.Contains(path)) {
        return Cache_.Get(path);
    }

    std::shared_ptr<SDL_Surface> surface = SurfaceManager_.Get(path);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(Renderer_, surface.get());
    if (!texture) {
        std::runtime_error("Can't create texture from surface " + path);
    }

    std::shared_ptr<SDL_Texture> result(texture, [](SDL_Texture* texture) { SDL_DestroyTexture(texture); });
    Cache_.Set(path, result);
    return result;
}

void TRenderManager::SetLayer(ELayer layer) {
    Layer_ = layer;
}

void TRenderManager::SetColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    Commands_[Layer_].push_back(TColorCommand{red, green, blue, alpha});
}

void TRenderManager::SetTexture(const std::shared_ptr<SDL_Texture>& texture) {
    if (ActiveTextures_[Layer_] != texture) {
        Commands_[Layer_].push_back(TTextureCommand{texture});
        ActiveTextures_[Layer_] = texture;
    }
}

void TRenderManager::DrawRect(const Vec2i& position, const Vec2i& size, bool filled) {
    Commands_[Layer_].push_back(TRectCommand{position, size, filled});
}

void TRenderManager::DrawLine(const Vec2i& start, const Vec2i& end) {
    Commands_[Layer_].push_back(TLineCommand{start, end});
}

void TRenderManager::DrawSprite(const Vec2i& texturePosition, const Vec2i& textureSize, const Vec2i& targetPosition, const Vec2i& targetSize, SDL_RendererFlip flip) {
    if (!ActiveTextures_[Layer_]) {
        throw std::runtime_error("layer has not active texture");
    }
    Commands_[Layer_].push_back(TSpriteCommand{texturePosition, textureSize, targetPosition, targetSize, flip});
}

void TRenderManager::DrawGeometry(const SDL_Vertex* vertices, std::size_t verticesCount, const int* indices, std::size_t indicesCount) {
    if (!ActiveTextures_[Layer_]) {
        throw std::runtime_error("layer has not active texture");
    }

    Commands_[Layer_].push_back(TGeometryCommand{vertices, verticesCount, indices, indicesCount});
}

void TRenderManager::SetCamera(const Vec2i& position) {
    Camera_ = position;
}

const Vec2i& TRenderManager::Camera() const {
    return Camera_;
}

const Vec2i& TRenderManager::Size() const {
    return Size_;
}

bool TRenderManager::IsLightEnabled() const {
    return LightEnabled_;
}

void TRenderManager::EnableLight(bool value) {
    LightEnabled_ = value;
}

void TRenderManager::SetDefaultLightColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
    DefaultLightColor_ = {red, green, blue, 255};
}

std::uint8_t TRenderManager::DefaultLightRed() const {
    return DefaultLightColor_.Red;
}

std::uint8_t TRenderManager::DefaultLightGreen() const {
    return DefaultLightColor_.Green;
}

std::uint8_t TRenderManager::DefaultLightBlue() const {
    return DefaultLightColor_.Blue;
}


void TRenderManager::Run() {
    if (!IsRunning()) {
        return;
    }

    for (size_t layer = 0; layer < SentinelMax; ++layer) {
        switch (layer) {
        case ELayer::BackgroundGlow:
        case ELayer::ForegroundGlow:
        case ELayer::EffectsGlow:
            if (SDL_SetRenderDrawBlendMode(Renderer_, SDL_BlendMode::SDL_BLENDMODE_ADD)) {
                std::runtime_error("can't set blendmode");
            }
            if (SDL_SetRenderTarget(Renderer_, ScreenTexture_)) {
                std::runtime_error("can't set render target");
            }
            break;

        case ELayer::Light:
            if (!IsLightEnabled()) {
                continue;
            }
            if (SDL_SetRenderDrawBlendMode(Renderer_, SDL_BlendMode::SDL_BLENDMODE_BLEND)) {
                std::runtime_error("can't set blendmode");
            }
            if (SDL_SetRenderTarget(Renderer_, LightTexture_)) {
                std::runtime_error("can't set render target");
            }
            break;
        
        default:
            if (SDL_SetRenderDrawBlendMode(Renderer_, SDL_BlendMode::SDL_BLENDMODE_BLEND)) {
                std::runtime_error("can't set blendmode");
            }
            if (SDL_SetRenderTarget(Renderer_, ScreenTexture_)) {
                std::runtime_error("can't set render target");
            }
            break;
        }

        SDL_Texture* activeTexture = nullptr;
        
        for (const auto& command : Commands_[layer]) {
            auto drawOperation = TOverload {
                [&](const TColorCommand& value) {
                    SDL_SetRenderDrawColor(Renderer_, value.Red, value.Green, value.Blue, value.Alpha);
                },
                [&](const TRectCommand& value) {
                    SDL_Rect rect;
                    rect.x = value.Position.X;
                    rect.y = value.Position.Y;
                    rect.w = value.Size.X;
                    rect.h = value.Size.Y;

                    if (layer != ELayer::Interface) {
                        rect.x -= Camera_.X;
                        rect.y -= Camera_.Y;
                    }

                    if (value.ShouldFill) {
                        SDL_RenderFillRect(Renderer_, &rect);
                    } else {
                        SDL_RenderDrawRect(Renderer_, &rect);
                    }
                },
                [&](const TLineCommand& value) {
                    if (layer != ELayer::Interface) {
                        SDL_RenderDrawLine(Renderer_, value.Start.X - Camera_.X,  value.Start.Y - Camera_.Y,  value.End.X - Camera_.X,  value.End.Y - Camera_.Y);
                    } else {
                        SDL_RenderDrawLine(Renderer_, value.Start.X, value.Start.Y, value.End.X, value.End.Y);
                    }
                },
                [&](const TTextureCommand& value) {
                    activeTexture = value.Texture.get();

                    switch (layer) {
                    case ELayer::BackgroundGlow:
                    case ELayer::ForegroundGlow:
                    case ELayer::EffectsGlow:
                        if (SDL_SetTextureBlendMode(activeTexture, SDL_BlendMode::SDL_BLENDMODE_ADD)) {
                            std::runtime_error("can't set texture blend mode");
                        }
                        break;

                    default:
                        if (SDL_SetTextureBlendMode(activeTexture, SDL_BlendMode::SDL_BLENDMODE_BLEND)) {
                            std::runtime_error("can't set texture blend mode");
                        }
                        break;
                    }
                },
                [&](const TSpriteCommand& value) {
                    SDL_Rect sourceRect;
                    sourceRect.x = value.TexturePosition.X;
                    sourceRect.y = value.TexturePosition.Y;
                    sourceRect.w = value.TextureSize.X;
                    sourceRect.h = value.TextureSize.Y;

                    SDL_Rect destinationRect;
                    destinationRect.x = value.TargetPosition.X;
                    destinationRect.y = value.TargetPosition.Y;
                    destinationRect.w = value.TargetSize.X;
                    destinationRect.h = value.TargetSize.Y;

                    if (layer != ELayer::Interface) {
                        destinationRect.x -= Camera_.X;
                        destinationRect.y -= Camera_.Y;
                    }

                    SDL_RenderCopyEx(Renderer_, activeTexture, &sourceRect, &destinationRect, 0, NULL, value.Flip);
                },
                [&](const TGeometryCommand& value) {
                    if (layer == ELayer::Interface) {
                        SDL_RenderGeometry(Renderer_, activeTexture, value.Vertices, value.VerticesCount, value.Indices, value.IndicesCount);
                    } else {
                        VerticesBuffer_.clear();
                        for (std::size_t i = 0; i < value.VerticesCount; ++i) {
                            SDL_Vertex convertedVertex = value.Vertices[i];
                            convertedVertex.position.x -= Camera_.X;
                            convertedVertex.position.y -= Camera_.Y;
                            VerticesBuffer_.push_back(convertedVertex);
                        }
                        
                        SDL_RenderGeometry(Renderer_, activeTexture, VerticesBuffer_.data(), VerticesBuffer_.size(), value.Indices, value.IndicesCount);
                    }
                    
                }
            };

            std::visit(drawOperation, command);
        }

        // Apply light
        if (layer == ELayer::Light) {
            SDL_SetRenderTarget(Renderer_, ScreenTexture_);
            SDL_RenderCopy(Renderer_, LightTexture_, NULL, NULL);
        }
    }

    // Keep logical resolution the same
    int windowWidth, windowHeight;
    SDL_GetWindowSize(Window_, &windowWidth, &windowHeight);
    Vec2f scaleFactors = {float(windowWidth) / Size_.X, float(windowHeight) / Size_.Y};
    float targetFactor = std::min(scaleFactors.X, scaleFactors.Y);
    
    SDL_Rect destinationRect;
    destinationRect.w = targetFactor * Size_.X;
    destinationRect.h = targetFactor * Size_.Y;
    destinationRect.x = (windowWidth - destinationRect.w) / 2;
    destinationRect.y = (windowHeight - destinationRect.h) / 2;

    SDL_SetRenderTarget(Renderer_, NULL);
    SDL_RenderCopy(Renderer_, ScreenTexture_, NULL, &destinationRect);
    SDL_RenderPresent(Renderer_);
}

bool TRenderManager::Input(SDL_Event* event) {
    bool result = SDL_PollEvent(event);

    if (result && event->type == SDL_QUIT) {
        Shutdown();
    }

    return result;
}

bool TRenderManager::IsRunning() {
    return Window_ != nullptr;
}

void TRenderManager::Shutdown() {
    if (IsRunning()) {
        Reset();
        SDL_DestroyTexture(LightTexture_);
        SDL_DestroyTexture(ScreenTexture_);
        SDL_DestroyRenderer(Renderer_);
        SDL_DestroyWindow(Window_);
        ScreenTexture_ = nullptr;
        Renderer_ = nullptr;
        Window_ = nullptr;
    }
}

void TRenderManager::Reset() {
    Layer_ = Tile;
    ActiveTextures_.fill(nullptr);
    for (auto& queue : Commands_) {
        queue.clear();
    }
    
    SDL_Rect a;
    SDL_RenderGetViewport(Renderer_, &a);

    SDL_SetRenderDrawColor(Renderer_, 0, 0, 0, 0);
    SDL_SetRenderTarget(Renderer_, NULL);
    SDL_RenderClear(Renderer_);

    SDL_SetRenderDrawColor(Renderer_, 0, 0, 0, 0);
    SDL_SetRenderTarget(Renderer_, ScreenTexture_);
    SDL_RenderClear(Renderer_);

    if (IsLightEnabled()) {
        SDL_SetRenderDrawColor(Renderer_, DefaultLightColor_.Red, DefaultLightColor_.Green, DefaultLightColor_.Blue, DefaultLightColor_.Alpha);
        SDL_SetRenderTarget(Renderer_, LightTexture_);
        SDL_RenderClear(Renderer_);
    }
}

} // namespace NGame