#include "physics/physics_system.h"

#include "debug/log.h"
#include "physics/rigidbody.h"

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <cstdarg>

JPH_SUPPRESS_WARNINGS

namespace gl {

// --- Jolt Global Settings ---
static const JPH::uint TEMP_ALLOCATOR_SIZE = 20 * 1024 * 1024;
static const JPH::uint MAX_PHYSICS_JOBS = 1024;
static const JPH::uint MAX_PHYSICS_BARIERS = 8;
static const int NUM_THREADS = std::thread::hardware_concurrency() - 1;

static const JPH::uint MAX_BODIES = 65536;
static const JPH::uint NUM_BODY_MUTEXES =
		0; // 0 means Jolt will use a sensible default (e.g., 65536 / 1024)
static const JPH::uint MAX_BODY_PAIRS = 65536;
static const JPH::uint MAX_CONTACT_CONSTRAINTS = 10240;

static void trace_impl(const char* in_fmt, ...) {
	va_list list;
	va_start(list, in_fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), in_fmt, list);
	va_end(list);

	GL_LOG_TRACE("[JOLT] {}", buffer);
}

#ifdef JPH_ENABLE_ASSERTS
static bool assert_failed_impl(
		const char* expression, const char* message, const char* file, uint line) {
	GL_LOG_ERROR("{}:{}: ({}) {}", file, line, expression, (message != nullptr ? message : ""));
	return true;
};
#endif // JPH_ENABLE_ASSERTS

namespace Layers {
static constexpr JPH::ObjectLayer NON_MOVING = 0;
static constexpr JPH::ObjectLayer MOVING = 1;
static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
}; //namespace Layers

// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
public:
	virtual bool ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const override {
		switch (object1) {
			case Layers::NON_MOVING:
				return object2 == Layers::MOVING; // Non moving only collides with moving
			case Layers::MOVING:
				return true; // Moving collides with everything
			default:
				JPH_ASSERT(false);
				return false;
		}
	}
};

namespace BroadPhaseLayers {
static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
static constexpr JPH::BroadPhaseLayer MOVING(1);
static constexpr uint NUM_LAYERS(2);
}; //namespace BroadPhaseLayers

class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
	BPLayerInterfaceImpl() {
		// Create a mapping table from object to broad phase layer
		_object_to_broad_phase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		_object_to_broad_phase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return _object_to_broad_phase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
		switch ((JPH::BroadPhaseLayer::Type)inLayer) {
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
				return "NON_MOVING";
			case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
				return "MOVING";
			default:
				JPH_ASSERT(false);
				return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer _object_to_broad_phase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
	virtual bool ShouldCollide(
			JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override {
		switch (layer1) {
			case Layers::NON_MOVING:
				return layer2 == BroadPhaseLayers::MOVING;
			case Layers::MOVING:
				return true;
			default:
				JPH_ASSERT(false);
				return false;
		}
	}
};

class MyContactListener : public JPH::ContactListener {
public:
	virtual JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2,
			JPH::RVec3Arg base_offset, const JPH::CollideShapeResult& collision_res) override {
		GL_LOG_INFO("Contact validate callback");

		// Allows you to ignore a contact before it is created (usg layers to not make objects
		// collide is cheaper!)
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2,
			const JPH::ContactManifold& manifold, JPH::ContactSettings& io_settingsm) override {
		GL_LOG_INFO("A contact was added");
	}

	virtual void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2,
			const JPH::ContactManifold& manifold, JPH::ContactSettings& io_settings) override {
		GL_LOG_INFO("A contact was persisted");
	}

	virtual void OnContactRemoved(const JPH::SubShapeIDPair& sub_shape_pair) override {
		GL_LOG_INFO("A contact was removed");
	}
};

class MyBodyActivationListener : public JPH::BodyActivationListener {
public:
	virtual void OnBodyActivated(const JPH::BodyID& bodyID, uint64_t body_user_data) override {
		GL_LOG_INFO("A body got activated");
	}

	virtual void OnBodyDeactivated(const JPH::BodyID& bodyID, uint64_t body_user_data) override {
		GL_LOG_INFO("A body went to sleep");
	}
};

struct JoltBodyComponent {
	JPH::BodyID body_id;
};

struct PhysicsState {
	JPH::TempAllocatorImpl* temp_allocator;
	JPH::JobSystemThreadPool* job_system;
	JPH::PhysicsSystem physics_system;

	BPLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	MyBodyActivationListener body_activation_listener;
	MyContactListener contact_listener;
};

static PhysicsState* s_state = nullptr;

void PhysicsSystem::init() {
	if (s_state) {
		delete s_state;
	}
	s_state = new PhysicsState();

	JPH::RegisterDefaultAllocator();

	JPH::Trace = trace_impl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assert_failed_impl;)

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	s_state->temp_allocator = new JPH::TempAllocatorImpl(TEMP_ALLOCATOR_SIZE);
	s_state->job_system =
			new JPH::JobSystemThreadPool(MAX_PHYSICS_JOBS, MAX_PHYSICS_BARIERS, NUM_THREADS);

	s_state->physics_system.Init(MAX_BODIES, NUM_BODY_MUTEXES, MAX_BODY_PAIRS,
			MAX_CONTACT_CONSTRAINTS, s_state->broad_phase_layer_interface,
			s_state->object_vs_broadphase_layer_filter, s_state->object_vs_object_layer_filter);

	s_state->physics_system.SetBodyActivationListener(&s_state->body_activation_listener);

	s_state->physics_system.SetContactListener(&s_state->contact_listener);

	s_state->physics_system.SetGravity(JPH::Vec3(0, -9.8f, 0));
}

void PhysicsSystem::shutdown() {
	if (!s_state) {
		return;
	}

	JPH::UnregisterTypes();

	// Destroy the factory
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	delete s_state->temp_allocator;
	delete s_state->job_system;

	delete s_state;
	s_state = nullptr;
}

static void sync_bodies(Registry& registry) {
	JPH::BodyInterface& body_interface = s_state->physics_system.GetBodyInterface();

	// Add bodies if not exists
	for (auto entity : registry.view<Position, Velocity>()) {
		auto jolt_body = registry.get<JoltBodyComponent>(entity);
		if (jolt_body) {
			continue;
		}

		auto [pos, vel] = registry.get_many<Position, Velocity>(entity);

		// Half extent = 0.5 for a 1 meter cube (1x1x1)
		JPH::Vec3 half_extent(0.5f, 0.5f, 0.5f);
		JPH::BoxShapeSettings box_shape_settings(half_extent);

		JPH::Shape* shape;

		JPH::ShapeSettings::ShapeResult shape_result = box_shape_settings.Create();
		if (shape_result.HasError()) {
			GL_LOG_ERROR("[JOLT] {}", shape_result.GetError().c_str());
			continue;
		}
		shape = shape_result.Get();

		const auto motion_type = JPH::EMotionType::Dynamic; // Changed to Dynamic for movement test

		JPH::BodyCreationSettings settings(
				shape, JPH::RVec3(pos->x, pos->y, pos->z), // Start position
				JPH::Quat::sIdentity(), motion_type,
				motion_type == JPH::EMotionType::Dynamic ? Layers::MOVING
														 : Layers::NON_MOVING // Object Layer
		);
		settings.mUserData = entity; // Store the ECS Entity ID in the Jolt body

		// Create the body
		JPH::Body* body = body_interface.CreateBody(settings);
		if (!body) {
			GL_LOG_ERROR("[JOLT] Failed to create Jolt body!");
			continue;
		}

		// Add it to the world and activate it
		body_interface.AddBody(body->GetID(), JPH::EActivation::Activate);

		// Register it to the ECS
		auto jbc = registry.assign<JoltBodyComponent>(entity);
		jbc->body_id = body->GetID();
	}
}
void PhysicsSystem::update(Registry& registry) {
	// Add non-existing bodies
	sync_bodies(registry);

	JPH::BodyInterface& body_interface = s_state->physics_system.GetBodyInterface();

	// Apply external changes/forces to Jolt
	for (auto entity : registry.view<JoltBodyComponent, Velocity>()) {
		auto [jolt_body, vel] = registry.get_many<JoltBodyComponent, Velocity>(entity);

		// Set ECS velocity
		if (body_interface.IsActive(jolt_body->body_id)) {
			body_interface.SetLinearVelocity(jolt_body->body_id, JPH::Vec3(vel->x, vel->y, vel->z));
		}
	}

	// Simulate stage
	{
		// 60Hz
		constexpr float TIME_STEP = 1.0f / 60.0f;
		// One collision check in every 60 frames.
		constexpr int COLLISION_STEPS = 1;

		// Update physics engine
		s_state->physics_system.Update(
				TIME_STEP, COLLISION_STEPS, s_state->temp_allocator, s_state->job_system);
	}

	// Copy Jolt results back to ECS
	for (auto entity : registry.view<JoltBodyComponent, Position, Velocity>()) {
		auto [jolt_body, pos, vel] =
				registry.get_many<JoltBodyComponent, Position, Velocity>(entity);

		JPH::RVec3 jolt_pos;
		JPH::Quat jolt_rot;
		body_interface.GetPositionAndRotation(jolt_body->body_id, jolt_pos, jolt_rot);
		JPH::Vec3 jolt_vel = body_interface.GetLinearVelocity(jolt_body->body_id);

		pos->x = jolt_pos.GetX();
		pos->y = jolt_pos.GetY();
		pos->z = jolt_pos.GetZ();

		vel->x = jolt_vel.GetX();
		vel->y = jolt_vel.GetY();
		vel->z = jolt_vel.GetZ();
	}
}

} //namespace gl
