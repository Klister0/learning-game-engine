# glad: OpenGL fonksiyonlarının adreslerini çalışma anında ekran kartı sürücüsünden yükler.
# Bu dosyalar glad2 2.0.8 ile üretildi:
#   glad --api gl:core=3.3 --extensions="" --out-path external/glad c
add_library(glad STATIC src/gl.c)
target_include_directories(glad PUBLIC include)
