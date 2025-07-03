//
// Created by Daniel on 2024-01-13.
//

#include "ige_curve.h"

void IGE_Curve::_bind_methods() {

    ClassDB::bind_method(D_METHOD("set_normalize", "normalize"), &IGE_Curve::set_normalize);
    ClassDB::bind_method(D_METHOD("get_normalize"), &IGE_Curve::get_normalize);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "normalize"), "set_normalize", "get_normalize");

    ClassDB::bind_method(D_METHOD("set_curve", "curve"), &IGE_Curve::set_curve);
    ClassDB::bind_method(D_METHOD("get_curve"), &IGE_Curve::get_curve);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_curve", "get_curve");
}

IGE_Curve::IGE_Curve() {
}

IGE_Curve::~IGE_Curve() {
    if (curve.is_valid()) {
        curve = nullptr;
    }
}

float IGE_Curve::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    // const int x = static_cast<int>(spot.x);
    // const int z = static_cast<int>(spot.y);

    if (!curve.is_valid()) return input;
    if (normalize) {
        return curve->sample_baked(Math::remap(input,-1.0f,1.0f,0.0f,1.0f));
    }
    return curve->sample_baked(input);
}
