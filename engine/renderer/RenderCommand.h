#pragma once

// Oyun kodunun OpenGL'i doğrudan çağırmaması için en ince katman.
// İleride (Bölüm 09) 3D için de aynı komutlar kullanılacak.
namespace engine::RenderCommand {

void setClearColor(float r, float g, float b, float a = 1.0f);
void clear(); // renk ve derinlik tamponlarını temizler

} // namespace engine::RenderCommand
