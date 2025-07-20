#include "App.h"

#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace NGame {

TApp* TApp::Instance() {
    static TApp* instance = nullptr;

    if (!instance) {
        instance = new TApp();
    }

    return instance;
}

TApp::TApp() {
    FileManager_ = std::make_unique<TFileManager>("Data", "data.pak");
    SurfaceManager_ = std::make_unique<TSurfaceManager>(*FileManager_);
    RenderManager_ = std::make_unique<TRenderManager>(*SurfaceManager_, 320, 240, "EULAMadness");
    SpriteManager_ = std::make_unique<TSpriteManager>(*FileManager_, *RenderManager_);
    EntityManager_ = std::make_unique<TEntityManager>(*RenderManager_, 1000 / 60);
    FontManager_ = std::make_unique<TFontManager>(*SpriteManager_);
    DigitManager_ = std::make_unique<TDigitManager>(*SpriteManager_);
    AudioManager_ = std::make_unique<TAudioManager>(*FileManager_);
}

TApp::~TApp() {
}

void TApp::Process() {
    if (!RenderManager_->IsRunning()) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();  /* this should "kill" the app. */
        #else
        exit(0);
        #endif
    }

    RenderManager_->Reset();
    EntityManager_->Run();
    RenderManager_->Run();
    AudioManager_->Run();
}

int TApp::Run() {
    Process();
    return 0;
}

TState& TApp::State() {
    return State_;
}

const TState& TApp::State() const {
    return State_;
}

TFileManager& TApp::FileManager() {
    return *FileManager_;
}

TRenderManager& TApp::RenderManager() {
    return *RenderManager_;
}

TEntityManager& TApp::EntityManager() {
    return *EntityManager_;
}

TSurfaceManager& TApp::SurfaceManager() {
    return *SurfaceManager_;
}

TSpriteManager& TApp::SpriteManager() {
    return *SpriteManager_;
}

TFontManager& TApp::FontManager() {
    return *FontManager_;
}

TDigitManager& TApp::DigitManager() {
    return *DigitManager_;
}

TAudioManager& TApp::AudioManager() {
    return *AudioManager_;
}

const TFileManager& TApp::FileManager() const {
    return *FileManager_;
}

const TRenderManager& TApp::RenderManager() const {
    return *RenderManager_;
}

const TEntityManager& TApp::EntityManager() const {
    return *EntityManager_;
}

const TSurfaceManager& TApp::SurfaceManager() const {
    return *SurfaceManager_;
}

const TSpriteManager& TApp::SpriteManager() const {
    return *SpriteManager_;
}

const TFontManager& TApp::FontManager() const {
    return *FontManager_;
}

const TDigitManager& TApp::DigitManager() const {
    return *DigitManager_;
}

const TAudioManager& TApp::AudioManager() const {
    return *AudioManager_;
}

} // namespace NGame