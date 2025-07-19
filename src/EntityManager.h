#pragma once

#include "Common.h"

#include "Entity.h"
#include "Gridmap.h"
#include "RenderManager.h"

namespace NGame {

class TEntityManager {
public:
    TEntityManager(TRenderManager& renderManager, const std::size_t timestep = 1000 / 30);
    DELETE_COPY(TEntityManager)

    void Run();
    std::shared_ptr<TEntity> Entity(TEntity::TId id);
    void UpdateCollision(TEntity::TId id);
    void UpdateCollision(std::shared_ptr<TEntity> entity);
    
    template<typename T>
    void RegisterEntity(const std::string& name) {
        if (EntityFactories_.find(name) != EntityFactories_.end()) {
            throw std::runtime_error("can't register duplicate entity with name " + name);
        }
        EntityFactories_[name] = &MakeEntityFactory<T>;
    }

    template<class T, class... Args>
    std::shared_ptr<TEntity> MakeEntity(Args&&... args) {
        auto entity = std::make_shared<T>(++IdCounter_, std::forward<Args>(args)...);
        Entities_[entity->Id()] = entity;

        // Explicilty ack changes in entity colision boxes
        Gridmap_.Add(entity->Position(), entity->Size(), entity->Id());
        entity->AckChanges();
        return entity;
    }

    std::shared_ptr<TEntity> MakeEntityByName(const std::string& name);
    bool IsPlaceEmpty(const Vec2i& position, const Vec2i& size, TEntity::TCollisionGroup group, TEntity::TId ignoreId = {});
    std::unordered_set<TEntity::TId> CollisionList(const Vec2i& position, const Vec2i& size, TEntity::TCollisionGroup group, TEntity::TId ignoreId = {});
    std::vector<TEntity::TId> Ids() const;
    void AddToDeferred(const std::string& name);
    void Reset();

private:
    using TFactoryMethod = std::shared_ptr<TEntity>(*)(TEntity::TId);

    void Input();
    void Update();
    void Draw();
    
    template<typename T>
    static std::shared_ptr<TEntity> MakeEntityFactory(TEntity::TId id) {
        return std::make_shared<T>(id);
    }

private:
    TRenderManager& RenderManager_;
    std::unordered_map<TEntity::TId, std::shared_ptr<TEntity>> Entities_;
    std::unordered_map<std::string, TFactoryMethod> EntityFactories_;
    std::vector<std::string> DeferredEntities_;
    std::uint32_t LastUpdateTick_;
    TEntity::TId IdCounter_;
    std::size_t Timestep_;
    TGridmap Gridmap_;
    bool PendingReset_ = false;
};

} // namespace NGame