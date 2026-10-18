#pragma once

#define DECLARE_PUSH_CONSTANT( x )                                                                                     \
    [[vk::push_constant]]                                                                                              \
    x pushConstant;

#define DESCRIPTOR_SET( _Binding, _Set, type )                                                                                   \
    [[vk::binding( _Binding, _Set )]]                                                                                            \
    type 