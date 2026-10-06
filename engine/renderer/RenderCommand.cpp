#include "renderer/RenderCommand.h"

#include <glad/gl.h>

namespace engine::RenderCommand {

void setClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

} // namespace engine::RenderCommand
