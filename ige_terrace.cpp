//
// Created by Daniel on 2024-01-13.
//

#include "ige_terrace.h"

void IGE_Terrace::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_terrace_amount", "val"), &IGE_Terrace::set_terrace_amount);
    ClassDB::bind_method(D_METHOD("get_terrace_amount"), &IGE_Terrace::get_terrace_amount);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "terrace_amount", PROPERTY_HINT_RANGE, "1,10,1"), "set_terrace_amount", "get_terrace_amount");

    ClassDB::bind_method(D_METHOD("set_min_height", "val"), &IGE_Terrace::set_min_height);
    ClassDB::bind_method(D_METHOD("get_min_height"), &IGE_Terrace::get_min_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_height", PROPERTY_HINT_RANGE, "-1.0,1.0,0.01"), "set_min_height", "get_min_height");

    ClassDB::bind_method(D_METHOD("set_max_height", "val"), &IGE_Terrace::set_max_height);
    ClassDB::bind_method(D_METHOD("get_max_height"), &IGE_Terrace::get_max_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_height", PROPERTY_HINT_RANGE, "-1.0,1.0,0.01"), "set_max_height", "get_max_height");

    ClassDB::bind_method(D_METHOD("set_offset", "val"), &IGE_Terrace::set_offset);
    ClassDB::bind_method(D_METHOD("get_offset"), &IGE_Terrace::get_offset);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "offset"), "set_offset", "get_offset");

}

IGE_Terrace::IGE_Terrace() {
}

float IGE_Terrace::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    if (input > min_height && input < max_height) {
        return (Math::round(input * static_cast<float>(terrace_amount)) / static_cast<float>(terrace_amount)) - offset; //Mathf.Clamp01(nMap - FOval)
    }
    return input;
}
