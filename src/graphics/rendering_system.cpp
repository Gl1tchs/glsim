#include "graphics/rendering_system.h"

#include "debug/log.h"

namespace gl {

void RenderingSystem::on_init(Registry& p_registry) { GL_LOG_TRACE("RenderingSystem::on_init"); }

void RenderingSystem::on_destroy(Registry& p_registry) {
	GL_LOG_TRACE("RenderingSystem::on_destroy");
}

void RenderingSystem::on_update(Registry& p_registry, float p_dt) {
	GL_LOG_TRACE("RenderingSystem::on_update");
}

} //namespace gl