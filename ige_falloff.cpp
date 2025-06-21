//
// Created by Daniel on 2024-01-07.
//

#include "ige_falloff.h"

void IGE_Falloff::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_falloff_type", "type"), &IGE_Falloff::set_falloff_type);
    ClassDB::bind_method(D_METHOD("get_falloff_type"), &IGE_Falloff::get_falloff_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "falloff_type", PROPERTY_HINT_ENUM, "Squared,Euclidian,Curve"), "set_falloff_type", "get_falloff_type");

    ClassDB::bind_method(D_METHOD("set_bottom_value", "val"), &IGE_Falloff::set_bottom_value);
    ClassDB::bind_method(D_METHOD("get_bottom_value"), &IGE_Falloff::get_bottom_value);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bottom_value"), "set_bottom_value", "get_bottom_value");

    ClassDB::bind_method(D_METHOD("set_falloff_spread", "spread"), &IGE_Falloff::set_falloff_spread);
    ClassDB::bind_method(D_METHOD("get_falloff_spread"), &IGE_Falloff::get_falloff_spread);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "squared_falloff_spread", PROPERTY_HINT_RANGE, "1,10,1"), "set_falloff_spread", "get_falloff_spread");

    ClassDB::bind_method(D_METHOD("set_falloff_curve", "curve"), &IGE_Falloff::set_falloff_curve);
    ClassDB::bind_method(D_METHOD("get_falloff_curve"), &IGE_Falloff::get_falloff_curve);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "curve_falloff_spread", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_falloff_curve", "get_falloff_curve");

    ClassDB::bind_method(D_METHOD("set_falloff_rad_x", "value"), &IGE_Falloff::set_falloff_rad_x);
    ClassDB::bind_method(D_METHOD("get_falloff_rad_x"), &IGE_Falloff::get_falloff_rad_x);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "euclidian_falloff_radius_x", PROPERTY_HINT_RANGE, "0.0,100.0,0.01"), "set_falloff_rad_x", "get_falloff_rad_x");

    ClassDB::bind_method(D_METHOD("set_falloff_rad_y", "value"), &IGE_Falloff::set_falloff_rad_y);
    ClassDB::bind_method(D_METHOD("get_falloff_rad_y"), &IGE_Falloff::get_falloff_rad_y);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "euclidian_falloff_radius_y", PROPERTY_HINT_RANGE, "0.0,100.0,0.01"), "set_falloff_rad_y", "get_falloff_rad_y");

    ClassDB::bind_method(D_METHOD("set_falloff_clamp_min", "value"), &IGE_Falloff::set_falloff_clamp_min);
    ClassDB::bind_method(D_METHOD("get_falloff_clamp_min"), &IGE_Falloff::get_falloff_clamp_min);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "falloff_clamp_min", PROPERTY_HINT_RANGE, "-1.0,4.0,0.01"), "set_falloff_clamp_min", "get_falloff_clamp_min");

    ClassDB::bind_method(D_METHOD("set_falloff_clamp_max", "value"), &IGE_Falloff::set_falloff_clamp_max);
    ClassDB::bind_method(D_METHOD("get_falloff_clamp_max"), &IGE_Falloff::get_falloff_clamp_max);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "falloff_clamp_max", PROPERTY_HINT_RANGE, "-1.0,4.0,0.01"), "set_falloff_clamp_max", "get_falloff_clamp_max");

    BIND_ENUM_CONSTANT(FalloffType_Squared);
    BIND_ENUM_CONSTANT(FalloffType_Euclidian);
    BIND_ENUM_CONSTANT(FalloffType_Curve);
}

void IGE_Falloff::_validate_property(PropertyInfo &p_property) const {
    if (p_property.name.begins_with("squared") && get_falloff_type() != FalloffType_Squared) {
        p_property.usage = PROPERTY_USAGE_NO_EDITOR;
        return;
    }
    if (p_property.name.begins_with("euclidian") && get_falloff_type() != FalloffType_Euclidian) {
        p_property.usage = PROPERTY_USAGE_NO_EDITOR;
        return;
    }
    if (p_property.name.begins_with("curve") && get_falloff_type() != FalloffType_Curve) {
        p_property.usage = PROPERTY_USAGE_NO_EDITOR;
        return;
    }
}

float IGE_Falloff::get_curve_falloff(int x, int y,int width, int height,float value) {
    float innerRadius = 0;
    float outerRadius = width;
    float dx = (Math::ceil(width*0.5f)) - x;
    float dy = (Math::ceil(height*0.5f)) - y;
    float distSqr = dx * dx + dy * dy;
    float iRadSqr = innerRadius * innerRadius;
    float oRadSqr = outerRadius * outerRadius;

    if (distSqr >= oRadSqr) return 0.0f;
    if (distSqr <= iRadSqr) return value;

    float dist = Math::sqrt(distSqr);
    float t = 1-Math::inverse_lerp(innerRadius, outerRadius, dist);
    t = Math::inverse_lerp(0.5f, 1.0f,t);
    if (!falloff_curve.is_valid()) return value;
    return falloff_curve->sample_baked(t);
}

float IGE_Falloff::Remap(float value, float from1, float to1, float from2, float to2) {
    return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

float IGE_Falloff::euclidianFallOff(int x, int y, float innerRadius,float outerRadius,int width, int height,float value) {
    innerRadius = Remap(innerRadius, 0, 100, 0, static_cast<float>(width));
    outerRadius = Remap(outerRadius, 0, 100, 0, static_cast<float>(height));
    const float dx = (Math::ceil(static_cast<float>(width)*0.5f)) - x;
    const float dy = (Math::ceil(static_cast<float>(height)*0.5f)) - y;
    const float distSqr = dx * dx + dy * dy;
    const float iRadSqr = innerRadius * innerRadius;
    const float oRadSqr = outerRadius * outerRadius;
    // if (x == y && x < width/2) {
    //     const float t = 1-Math::inverse_lerp(innerRadius, outerRadius, Math::sqrt(distSqr));
    //     print_line("Falloff euclidian distsqr ",distSqr," oradsqr ", oRadSqr," iradsqr ",iRadSqr," value ",value," t ",t);
    // }
    if (distSqr >= oRadSqr) return bottom_value;
    if (distSqr <= iRadSqr) return value;

    const float t = 1-Math::inverse_lerp(innerRadius, outerRadius, Math::sqrt(distSqr));

    return Math::lerp(bottom_value,value,t);
}

float IGE_Falloff::getFalloffValue(float value) {
    float a = 3;
    float b = falloff_spread;

    return Math::pow(value, a) / (Math::pow(value, a) + Math::pow(b - b * value, a));
}

void IGE_Falloff::set_falloff_type(FalloffType p_falloff_type) {
    falloff_type = p_falloff_type;
    emit_changed();
    notify_property_list_changed();
}

IGE_Falloff::FalloffType IGE_Falloff::get_falloff_type() const {
    return falloff_type;
}

float IGE_Falloff::getEdgeBlend(float value) {
    // You can adjust the blend function here if needed
    return 1.0 - value;
}

float IGE_Falloff::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    if (falloff_clamp_min == 0.0 && falloff_clamp_max == 0.0) {
        falloff_clamp_min = 0.0;
        falloff_clamp_max = 1.0;
    }
    const int length = static_cast<int>(size.y);
    const int width = static_cast<int>(size.x);
    const int x = static_cast<int>(spot.x);
    const int z = static_cast<int>(spot.y);
    switch (falloff_type) {
        case FalloffType_Squared: {
            const float i = 1 - (static_cast<float>(2 * x) / static_cast<float>(width));
            const float j = 1 - (static_cast<float>(2 * z) / static_cast<float>(length));
            const float absIJ = MAX(Math::abs(i), Math::abs(j));
            input = CLAMP(input - getFalloffValue(absIJ),falloff_clamp_min,falloff_clamp_max);
            input = Math::lerp(bottom_value, input, getEdgeBlend(absIJ));
            //if(doDebug) Debug.Log($"Fall off - Received value {originalValue}, after curve falloff got {nMap}, non clamped would be {originalValue - FOval}");
        }
        break;
        case FalloffType_Euclidian: {
            const float FOvall = euclidianFallOff(x,z,falloff_inner_outer_radius_x,falloff_inner_outer_radius_y,width,length,input);
            input = CLAMP(FOvall,falloff_clamp_min,falloff_clamp_max);
            // if (x == z && x < size.x/2) {
            //     print_line( x,",",z," Falloff received ",originalValue," after ", input," non clamped ",FOvall);
            // }
            //if(doDebug) Debug.Log($"Fall off - Received value {originalValue}, after curve falloff got {nMap}, non clamped would be {FOvall}");
        }
        break;
        case FalloffType_Curve: {
            input = CLAMP(input - (1-get_curve_falloff(x,z,width,length,input)),falloff_clamp_min,falloff_clamp_max);
            //if(doDebug) Debug.Log($"Fall off - Received value {originalValue}, after curve falloff got {nMap}, non clamped would be {originalValue-FOvalll}");
        }
        break;
    }

    return input;
}

IGE_Falloff::IGE_Falloff() = default;

IGE_Falloff::~IGE_Falloff() {
    if (falloff_curve.is_valid()) {
        falloff_curve = nullptr;
    }
}
