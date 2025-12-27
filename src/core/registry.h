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

constexpr inline Entity create_entity_id(uint32_t p_index, uint32_t p_version) {
	return ((Entity)p_index << 32) | p_version;
}

constexpr inline uint32_t get_entity_index(Entity p_entity) { return p_entity >> 32; }

constexpr inline uint32_t get_entity_version(Entity p_entity) {
	// this conversation will loose the top 32 bits
	return (uint32_t)p_entity;
}

constexpr inline bool is_entity_valid(Entity p_entity) { return (p_entity >> 32) != UINT32_MAX; }

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

	ComponentPool(size_t p_element_size);
	~ComponentPool();

	ComponentPool(const ComponentPool& p_other);

	size_t get_size() const;

	void* get(size_t p_idx);

	template <std::default_initializable T> T* add(uint32_t p_idx);

private:
	std::vector<std::unique_ptr<uint8_t[]>> pages;
	size_t element_size = 0;
};

template <typename... TComponents> class SceneView {
public:
	SceneView(EntityContainer* p_entities);

	class Iterator {
	public:
		Iterator(EntityContainer* p_entities, uint32_t p_index, ComponentMask p_mask, bool p_all);

		Entity operator*() const;

		bool operator==(const Iterator& p_other) const;

		bool operator!=(const Iterator& p_other) const;

		Iterator operator++();

	private:
		bool _is_index_valid();

	private:
		EntityContainer* entities;
		uint32_t index;
		ComponentMask mask;
		bool all = false;
	};

	const Iterator begin() const;

	const Iterator end() const;

private:
	EntityContainer* entities = nullptr;
	ComponentMask component_mask;
	bool all = false;
};

/**
 * Container of entities and components assigned to them.
 */
class Registry {
public:
	virtual ~Registry();

	void clear();

	void copy_to(Registry& p_dest);

	/**
	 * Create new entity instance on the scene
	 */
	Entity spawn();

	/**
	 * Find out wether the entity is valid or not
	 */
	bool is_valid(Entity p_entity);

	/**
	 * Removes entity from the scene and increments
	 * version
	 */
	void despawn(Entity p_entity);

	/**
	 * Sets component mask of the p_component_id
	 *
	 */
	bool assign_id(Entity p_entity, uint32_t p_component_id);

	bool remove(Entity p_entity, uint32_t p_component_id);

	bool has(Entity p_entity, uint32_t p_component_id);

	/**
	 * Assigns specified component to the entity
	 */
	template <typename T> T* assign(Entity p_entity);

	/**
	 * Assigns given components to the entity
	 */
	template <typename... TComponents> std::tuple<TComponents*...> assign_many(Entity p_entity);

	/**
	 * Remove specified component from the entity
	 */
	template <typename T> bool remove(Entity p_entity);

	/**
	 * Remove specified components from the entity
	 */
	template <typename... TComponents> void remove_many(Entity p_entity);

	/**
	 * Get specified component from the entity
	 */
	template <typename T> T* get(Entity entity);

	/**
	 * Get specified components from the entity
	 */
	template <typename... TComponents> std::tuple<TComponents*...> get_many(Entity p_entity);

	/**
	 * Find out wether an entity has the specified components
	 */
	template <typename... TComponents> bool has(Entity p_entity);

	/**
	 * Get entities with specified components,
	 * if no component provided it will return all
	 * of the entities
	 */
	template <typename... TComponents> SceneView<TComponents...> view();

private:
	uint32_t entity_counter = 0;
	EntityContainer entities;
	std::queue<Entity> free_indices;
	std::vector<std::shared_ptr<ComponentPool>> component_pools;
};

} //namespace gl

#include "core/registry.inl"
