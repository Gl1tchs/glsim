#pragma once

namespace gl {

// TODO: dynamically allocate
inline constexpr uint32_t MAX_ENTITIES = 1000;
inline constexpr uint32_t MAX_COMPONENTS = 32;

// first 32 bits is index and last 32 bits are version
typedef uint64_t Entity;

typedef std::bitset<MAX_COMPONENTS> ComponentMask;

struct EntityDescriptor {
	Entity id;
	ComponentMask mask;
};

typedef std::vector<EntityDescriptor> EntityContainer;

constexpr inline Entity create_entity_id(uint32_t index, uint32_t version) {
	return ((Entity)index << 32) | version;
}

constexpr inline uint32_t get_entity_index(Entity entity) { return entity >> 32; }

constexpr inline uint32_t get_entity_version(Entity entity) {
	// this conversation will loose the top 32 bits
	return (uint32_t)entity;
}

constexpr inline bool is_entity_valid(Entity entity) { return (entity >> 32) != UINT32_MAX; }

inline uint32_t s_component_counter = 0;

// returns different id for different component types
template <class T> inline uint32_t get_component_id() {
	static uint32_t s_component_id = s_component_counter++;
	return s_component_id;
}

inline constexpr Entity INVALID_ENTITY_ID = create_entity_id(UINT32_MAX, 0);

/**
 * Paged component pool for blazingly fast lookups
 */
class ComponentPool {
public:
	typedef std::function<void()> DeletorFunc;

	static constexpr size_t PAGE_SIZE = 1024;

	ComponentPool(size_t element_size);
	~ComponentPool();

	ComponentPool(const ComponentPool& other);

	size_t get_size() const;

	void* get(size_t idx);

	template <std::default_initializable T> T* add(uint32_t idx);

private:
	std::vector<std::unique_ptr<uint8_t[]>> _pages;
	size_t _element_size = 0;
};

template <typename... TComponents> class SceneView {
public:
	SceneView(EntityContainer* entities);

	class Iterator {
	public:
		Iterator(EntityContainer* entities, uint32_t index, ComponentMask mask, bool all);

		Entity operator*() const;

		bool operator==(const Iterator& other) const;

		bool operator!=(const Iterator& other) const;

		Iterator operator++();

	private:
		bool _is_index_valid();

	private:
		EntityContainer* _entities;
		uint32_t _index;
		ComponentMask _mask;
		bool _all = false;
	};

	const Iterator begin() const;

	const Iterator end() const;

private:
	EntityContainer* _entities = nullptr;
	ComponentMask _component_mask;
	bool _all = false;
};

/**
 * Container of entities and components assigned to them.
 */
class Registry {
public:
	virtual ~Registry();

	void clear();

	void copy_to(Registry& dest);

	/**
	 * Create new entity instance on the scene
	 */
	Entity spawn();

	/**
	 * Find out wether the entity is valid or not
	 */
	bool is_valid(Entity entity);

	/**
	 * Removes entity from the scene and increments
	 * version
	 */
	void despawn(Entity entity);

	/**
	 * Sets component mask of the component_id
	 *
	 */
	bool assign_id(Entity entity, uint32_t component_id);

	bool remove(Entity entity, uint32_t component_id);

	bool has(Entity entity, uint32_t component_id);

	/**
	 * Assigns specified component to the entity
	 */
	template <typename T> T* assign(Entity entity);

	/**
	 * Assigns given components to the entity
	 */
	template <typename... TComponents> std::tuple<TComponents*...> assign_many(Entity entity);

	/**
	 * Remove specified component from the entity
	 */
	template <typename T> bool remove(Entity entity);

	/**
	 * Remove specified components from the entity
	 */
	template <typename... TComponents> void remove_many(Entity entity);

	/**
	 * Get specified component from the entity
	 */
	template <typename T> T* get(Entity entity);

	/**
	 * Get specified components from the entity
	 */
	template <typename... TComponents> std::tuple<TComponents*...> get_many(Entity entity);

	/**
	 * Find out wether an entity has the specified components
	 */
	template <typename... TComponents> bool has(Entity entity);

	/**
	 * Get entities with specified components,
	 * if no component provided it will return all
	 * of the entities
	 */
	template <typename... TComponents> SceneView<TComponents...> view();

private:
	uint32_t _entity_counter = 0;
	EntityContainer _entities;
	std::queue<Entity> _free_indices;
	std::vector<std::shared_ptr<ComponentPool>> _component_pools;
};

} //namespace gl

#include "core/registry.inl"
