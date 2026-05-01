@echo off
glslangValidator -V --target-env vulkan1.4 vulkan_fragment.frag -o vulkan_fragment.spv
glslangValidator -V --target-env vulkan1.4 vulkan_vertex.vert -o vulkan_vertex.spv
@echo on 
