#include "core/engine.h"

#include <stdio.h>

namespace gl {

Engine::Engine() { printf("Engine constructed!\n"); }

void Engine::say_hello() { printf("Hello World\n"); }

} //namespace gl
