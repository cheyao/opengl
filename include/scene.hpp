#pragma once

#include "components.hpp"
#include "managers/componentManager.hpp"
#include "managers/entityManager.hpp"
#include "misc/sparse_set_view.hpp"

#include <SDL3/SDL_assert.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <tuple>

// Simple type-based hashing for a parameter pack of Components
// (We combine the typeid.hash_code() calls in a stable manner.)
namespace detail {

/**
 * Combine hash codes by a simple formula. 
 * You can switch to a more sophisticated approach as needed.
 */
inline std::size_t combine_hash(std::size_t lhs, std::size_t rhs) {
    // This is a typical Fowler–Noll–Vo or Boost combine approach:
    lhs ^= (rhs + 0x9e3779b97f4a7c15ULL + (lhs << 6) + (lhs >> 2));
    return lhs;
}

/**
 * Compute a hash for a pack of types `Components...`.
 * We'll fold each type's hash_code() into a single size_t.
 */
template <typename... Components>
inline std::size_t type_pack_hash() {
    std::size_t seed = 0;
    // Using a braced-init-list fold pattern:
    (void)std::initializer_list<int>{
        (seed = combine_hash(seed, typeid(Components).hash_code()), 0)...
    };
    return seed;
}

} // namespace detail

// Use a base struct to store the "any" of your cached view
struct ViewCacheBase {
    virtual ~ViewCacheBase() = default;
    bool dirty = true; // If dirty, we must rebuild
};

template <typename... Components>
struct ViewCache : public ViewCacheBase {
    // Store the actual cached view
    utils::sparse_set_view<Components...> cachedView;

    ViewCache(utils::sparse_set_view<Components...> view)
        : cachedView(std::move(view)) {
        dirty = false;
    }
};

class Scene {
#ifdef IMGUI
    // Needed for signal list menu
    friend class SystemManager;
#endif

  public:
    Scene()
        : mEntityManager(new EntityManager())
        , mComponentManager(new ComponentManager())
        , mSignals() {
    }

    Scene(Scene&&) = delete;
    Scene(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;
    Scene& operator=(const Scene&) = delete;

    ~Scene() {
        delete mEntityManager;
        delete mComponentManager;
    }

    // This returns a UUID for a new entity
    [[nodiscard]] EntityID newEntity() {
        const EntityID entity = mEntityManager->getEntity();
        return entity;
    }

    // Adds a component to an entity
    template <typename Component, typename... Args>
    void emplace(const EntityID entity, Args&&... args) {
        mComponentManager->getPool<Component>()->emplace(entity, std::forward<Args>(args)...);
        // Mark entire view cache as dirty because we changed the pools
        markAllCachesDirty();
    }

    // Returns the component of the entity
    template <typename Component>
    [[nodiscard]] Component& get(const EntityID entity) const {
        return mComponentManager->getPool<Component>()->get(entity);
    }

    // Returns whether an entity contains a component
    template <typename Component>
    [[nodiscard]] bool contains(const EntityID entity) const {
        return mComponentManager->getPool<Component>()->contains(entity);
    }

    // Returns a view of the given pack of components
    template <typename... Components>
    [[nodiscard]] utils::sparse_set_view<Components...> view() {
        // 1) Compute a type-based hash for (Components...)
        std::size_t key = detail::type_pack_hash<Components...>();

        // 2) Look up in our cache
        auto it = mViewCache.find(key);
        if (it != mViewCache.end()) {
            // We have an existing cache
            auto* basePtr = it->second.get();
            // Check if it’s the correct typed cache
            auto* typedPtr = dynamic_cast<ViewCache<Components...>*>(basePtr);
            if (typedPtr && !typedPtr->dirty) {
                // It's valid and not dirty => just return it
                return typedPtr->cachedView;
            }
            // else we must rebuild (or typedPtr is null -- which shouldn't normally happen)
        }

        // 3) Build a new view
        auto newView = utils::sparse_set_view<Components...>(mComponentManager);
        // 4) Store in the cache
        auto newCache = std::make_unique<ViewCache<Components...>>(std::move(newView));
        utils::sparse_set_view<Components...> result = newCache->cachedView; // copy out
        mViewCache[key] = std::move(newCache);

        return result; // Return the newly built or updated view
    }

    // Remove an entity
    void erase(const EntityID entity) noexcept {
        SDL_assert(entity != 0);
        mComponentManager->erase(entity);
        mEntityManager->releaseEntity(entity);
        markAllCachesDirty();
    }

    [[nodiscard]] bool valid(const EntityID entity) noexcept {
        return mEntityManager->valid(entity);
    }

    [[nodiscard]] std::int64_t& getSignal(const std::uint64_t signal) noexcept {
        if (!mSignals.contains(signal)) {
            mSignals[signal] = false;
        }
        return mSignals[signal];
    }

    void clearSignals() noexcept { mSignals.clear(); }

    // A structure used e.g. for the mouse
    struct {
        Components::Item item = static_cast<Components::Item>(0);
        std::uint64_t count = 0;
    } mMouse;

  private:
    // Mark entire view cache as dirty, because any change in the pools invalidates all cached views
    void markAllCachesDirty() {
        for (auto& [_, cachePtr] : mViewCache) {
            if (cachePtr) {
                cachePtr->dirty = true;
            }
        }
    }

  private:
    class EntityManager* mEntityManager;
    class ComponentManager* mComponentManager;

    std::unordered_map<std::uint64_t, std::int64_t> mSignals;

    // The type-based cache container
    mutable std::unordered_map<std::size_t, std::unique_ptr<ViewCacheBase>> mViewCache;
};
