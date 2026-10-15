#pragma once

#define DECLARE_PUSH_CONSTANT( x )                                                                                     \
    [[vk::push_constant]]                                                                                              \
    x pushConstant;

#define DESCRIPTOR_SET( x, y, type )                                                                                   \
    [[vk::binding( x, y )]]                                                                                            \
    type 