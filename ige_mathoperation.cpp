//
// Created by Daniel on 2024-01-12.
//

#include "ige_mathoperation.h"

void IGE_MathOperation::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_math_type", "type"), &IGE_MathOperation::set_math_type);
    ClassDB::bind_method(D_METHOD("get_math_type"), &IGE_MathOperation::get_math_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "math_type", PROPERTY_HINT_ENUM, "Add,Multiply,Divide,Power,Square,Remap,Replace"), "set_math_type", "get_math_type");

    ClassDB::bind_method(D_METHOD("set_number", "val"), &IGE_MathOperation::set_number);
    ClassDB::bind_method(D_METHOD("get_number"), &IGE_MathOperation::get_number);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "number"), "set_number", "get_number");

    ClassDB::bind_method(D_METHOD("set_before_min", "val"), &IGE_MathOperation::set_before_min);
    ClassDB::bind_method(D_METHOD("get_before_min"), &IGE_MathOperation::get_before_min);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "remap_before_min"), "set_before_min", "get_before_min");

    ClassDB::bind_method(D_METHOD("set_before_max", "val"), &IGE_MathOperation::set_before_max);
    ClassDB::bind_method(D_METHOD("get_before_max"), &IGE_MathOperation::get_before_max);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "remap_before_max"), "set_before_max", "get_before_max");
    
    ClassDB::bind_method(D_METHOD("set_after_min", "val"), &IGE_MathOperation::set_after_min);
    ClassDB::bind_method(D_METHOD("get_after_min"), &IGE_MathOperation::get_after_min);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "remap_after_min"), "set_after_min", "get_after_min");

    ClassDB::bind_method(D_METHOD("set_after_max", "val"), &IGE_MathOperation::set_after_max);
    ClassDB::bind_method(D_METHOD("get_after_max"), &IGE_MathOperation::get_after_max);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "remap_after_max"), "set_after_max", "get_after_max");
    

    BIND_ENUM_CONSTANT(MathType_Add);
    BIND_ENUM_CONSTANT(MathType_Multiply);
    BIND_ENUM_CONSTANT(MathType_Divide);
    BIND_ENUM_CONSTANT(MathType_Power);
    BIND_ENUM_CONSTANT(MathType_Square);
    BIND_ENUM_CONSTANT(MathType_Remap);
    BIND_ENUM_CONSTANT(MathType_Replace);
}

void IGE_MathOperation::_validate_property(PropertyInfo &p_property) const {
    if (get_math_type() == MathType_Remap) {
        if (p_property.name.begins_with("number")) {
            p_property.usage = PROPERTY_USAGE_NO_EDITOR;
            return;
        }
    } else {
        if (p_property.name.begins_with("remap")) {
            p_property.usage = PROPERTY_USAGE_NO_EDITOR;
            return;
        }
    }
}

float IGE_MathOperation::Remap(float value, float from1, float to1, float from2, float to2) {
    return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}
float IGE_MathOperation::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    float doResult = input;
    float originalResult = input;
    switch (math_type) {
        case MathType_Add:
            doResult = input+get_number();
            break;
        case MathType_Multiply:
            doResult = input*get_number();
            break;
        case MathType_Divide:
            doResult = input / (get_number() != 0 ? get_number() : 0.00001f);
            break;
        case MathType_Power:
            doResult = Math::pow(input,get_number());
            break;
        case MathType_Square:
            for (int i = 0; i < Number; i++) {
                float aamount = doResult;
                doResult = aamount * aamount;
            }
            break;
        case MathType_Remap:
            doResult = Remap(doResult, get_before_min(),get_before_max(),get_after_min(),get_after_max());
            break;
        case MathType_Replace:
            doResult = get_number();
            break;
    }
    return doResult;
}

IGE_MathOperation::IGE_MathOperation() {
}
