#include "EntityManager.h"

#include <algorithm>
#include <SDL2/SDL_timer.h>

namespace NGame {

TEntityManager::TEntityManager(TRenderManager& renderManager, std::size_t timestep)
    : RenderManager_(renderManager), Timestep_(timestep) {
    LastUpdateTick_ = SDL_GetTicks();
}

void TEntityManager::Run() {
    if (PendingReset_) {
        PendingReset_ = false;
        Entities_.clear();
        Gridmap_.Reset();
        for (const auto& name : DeferredEntities_) {
            MakeEntityByName(name);
        }
        DeferredEntities_.clear();
    }

    Input();
    Update();
    Draw();
}

std::shared_ptr<TEntity> TEntityManager::Entity(TEntity::TId id) {
    auto it = Entities_.find(id);
    if (it != Entities_.end()) {
        return it->second;
    }
    throw std::runtime_error("Could not find entity with id: " + std::to_string(id));
}

void TEntityManager::UpdateCollision(TEntity::TId id) {
    auto entity = Entity(id);
    UpdateCollision(entity);
}

void TEntityManager::UpdateCollision(std::shared_ptr<TEntity> entity) {
    if (entity->HasChanges()) {
        Gridmap_.Remove(entity->AckPosition(), entity->AckSize(), entity->Id());
        Gridmap_.Add(entity->Position(), entity->Size(), entity->Id());
        entity->AckChanges();
    }
}

std::shared_ptr<TEntity> TEntityManager::MakeEntityByName(const std::string& name) {
    if (EntityFactories_.find(name) == EntityFactories_.end()) {
        throw std::runtime_error("can't create entity with name " + name);
    }

    auto entity = EntityFactories_[name](++IdCounter_);
    Entities_[entity->Id()] = entity;
    
    // Explicilty ack changes in entity colision boxes
    Gridmap_.Add(entity->Position(), entity->Size(), entity->Id());
    entity->AckChanges();
    return entity;
}

bool TEntityManager::IsPlaceEmpty(const Vec2i& position, const Vec2i& size, TEntity::TCollisionGroup group, TEntity::TId ignoreId) {
    auto query = Gridmap_.Query(position, size);

    return !std::any_of(query.begin(), query.end(), [&](auto id) {
        auto entity = Entity(id);
        if (id == ignoreId)
            return false;
        
        if (!(entity->CollisionGroup() & group).any())
            return false;
        
        return RectOverlaps(position, size, entity->Position(), entity->Size());
    });
}

std::unordered_set<TEntity::TId> TEntityManager::CollisionList(const Vec2i& position, const Vec2i& size, TEntity::TCollisionGroup group, TEntity::TId ignoreId) {
    auto query = Gridmap_.Query(position, size);

    for (auto it = query.begin(); it != query.end(); ) {
        auto entity = Entity(*it);

        if (entity->Id() == ignoreId || !(entity->CollisionGroup() & group).any()) {
            it = query.erase(it);
        } else {
            if (RectOverlaps(position, size, entity->Position(), entity->Size())) {
                ++it;
            } else {
                it = query.erase(it);
            }
        }
    }
    return query;
}

std::vector<TEntity::TId> TEntityManager::Ids() const {
    std::vector<TEntity::TId> result;

    result.reserve(Entities_.size());
    for (const auto& entity : Entities_) {
        result.push_back(entity.first);
    }

    return result;
}

void TEntityManager::AddToDeferred(const std::string& name) {
    DeferredEntities_.push_back(name);
}

void TEntityManager::Reset() {
    PendingReset_ = true;
}

void TEntityManager::Input() {
    SDL_Event event;
    while (RenderManager_.Input(&event)) {
        std::for_each(Entities_.begin(), Entities_.end(), [&](auto& entity) {
            entity.second->Input(&event);
        });
    }
}

void TEntityManager::Update() {
    std::uint32_t currentTick = SDL_GetTicks();
    std::size_t attempts = 5;

    while (--attempts && (currentTick - LastUpdateTick_ > Timestep_)) {
        LastUpdateTick_ += Timestep_;

        std::queue<TEntity::TId> idForRemoval;

        // Update each enitity
        std::for_each(Entities_.begin(), Entities_.end(), [&](auto& pair) {
            auto& entity = pair.second;

            if (!entity->IsRemoved()) {
                entity->Tick(Timestep_);
            }

            if (entity->IsRemoved()) {
                // Explicitly remove collision
                Gridmap_.Remove(entity->AckPosition(), entity->AckSize(), entity->Id());
                idForRemoval.emplace(pair.first);
            } else {
                UpdateCollision(entity);
            }
        });

        // Remove removed entities
        while (!idForRemoval.empty()) {
            Entities_.erase(idForRemoval.front());
            idForRemoval.pop();
        }
    }

    // Failsafe when we run out of attempts
    if (!attempts) {
        LastUpdateTick_ = currentTick;
    }
}

void TEntityManager::Draw() {
    std::for_each(Entities_.begin(), Entities_.end(), [&](auto& pair) {
        auto& entity = pair.second;
        entity->Draw();
    });
}


} // namespace NGame
