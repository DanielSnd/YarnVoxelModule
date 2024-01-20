//
// Created by Daniel on 2024-01-05.
//

#include "island_gen_effect.h"

void IslandGenEffect::_bind_methods() {

    ClassDB::bind_method(D_METHOD("get_enabled"), &IslandGenEffect::get_enabled);
    ClassDB::bind_method(D_METHOD("set_enabled", "value"), &IslandGenEffect::set_enabled);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

    ClassDB::bind_method(D_METHOD("get_amount"), &IslandGenEffect::get_amount);
    ClassDB::bind_method(D_METHOD("set_amount", "value"), &IslandGenEffect::set_amount);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "amount",PROPERTY_HINT_RANGE, "0,1.0,0.01"), "set_amount", "get_amount");

    ClassDB::bind_method(D_METHOD("get_use_mask"), &IslandGenEffect::get_use_mask);
    ClassDB::bind_method(D_METHOD("set_use_mask", "value"), &IslandGenEffect::set_use_mask);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_mask"), "set_use_mask", "get_use_mask");

    ClassDB::bind_method(D_METHOD("get_invert_mask"), &IslandGenEffect::get_invert_mask);
    ClassDB::bind_method(D_METHOD("set_invert_mask", "value"), &IslandGenEffect::set_invert_mask);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "mask_invert"), "set_invert_mask", "get_invert_mask");

    ClassDB::bind_method(D_METHOD("get_mask_frequency"), &IslandGenEffect::get_mask_frequency);
    ClassDB::bind_method(D_METHOD("set_mask_frequency", "value"), &IslandGenEffect::set_mask_frequency);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mask_frequency",PROPERTY_HINT_RANGE, "0.015,1.0,0.005"), "set_mask_frequency", "get_mask_frequency");

    ClassDB::bind_method(D_METHOD("get_mask_minimum"), &IslandGenEffect::get_mask_minimum);
    ClassDB::bind_method(D_METHOD("set_mask_minimum", "value"), &IslandGenEffect::set_mask_minimum);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mask_minimum",PROPERTY_HINT_RANGE, "-0.1,1.0,0.01"), "set_mask_minimum", "get_mask_minimum");

    ClassDB::bind_method(D_METHOD("get_mask_multiplier"), &IslandGenEffect::get_mask_multiplier);
    ClassDB::bind_method(D_METHOD("set_mask_multiplier", "value"), &IslandGenEffect::set_mask_multiplier);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mask_multiplier",PROPERTY_HINT_RANGE, "0.01,5.0,0.01"), "set_mask_multiplier", "get_mask_multiplier");
}

void IslandGenEffect::_validate_property(PropertyInfo &p_property) const {
    if (p_property.name.begins_with("mask") && !get_use_mask()) {
        p_property.usage = PROPERTY_USAGE_NO_EDITOR;
        return;
    }
    // if (p_property.name.begins_with("euclidian") && get_falloff_type() != FalloffType_Euclidian) {
    //     p_property.usage = PROPERTY_USAGE_NO_EDITOR;
    //     return;
    // }
    // if (p_property.name.begins_with("curve") && get_falloff_type() != FalloffType_Curve) {
    //     p_property.usage = PROPERTY_USAGE_NO_EDITOR;
    //     return;
    // }
}

float IslandGenEffect::GetMask(float x, float y) {
    float mask = YarnVoxel::static_perlin_noise(static_cast<float
                    >(x) / 16.0f*get_mask_frequency() + 0.001f + Math::fmod(static_cast<float>(Math::abs(m_seed)) + (2.0f*static_cast<float>(m_index)),225.0f), static_cast<float>(y) / 16.0f * get_mask_frequency() + 0.001f + Math::fmod(static_cast<float>(Math::abs(m_seed)) + (2.0f*static_cast<float>(m_index)),225.0f)) * get_mask_multiplier();
    mask = CLAMP(mask, 0.0f, 1.0f);
    if (get_invert_mask()) mask = 1 - mask;
    return mask;
}
float IslandGenEffect::ApplyEffectSpot(Vector2 spot, Vector2 size, float input, int seed, bool doDebug) {
    float newResult = input;
    if (enabled) {
        newResult = DoApplyEffectSpot(spot,size,input,seed,doDebug);
        if (use_mask) {
            auto maskval = GetMask(spot.x,spot.y);
            if (maskval > mask_minimum ) {
                newResult = Math::lerp(newResult,input,maskval);
                return Math::lerp(newResult,input,1.0f - amount);
            }
        }
        newResult = Math::lerp(newResult,input,1.0f - amount);
    }
    return newResult;
}

float IslandGenEffect::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    //print_line("BAAP");
    return 0.0;
}


void IslandGenEffect::set_seed(int value) {
        m_seed = value;
        emit_changed();
}

IslandGenEffect::IslandGenEffect() {
}

IslandGenEffect::~IslandGenEffect() {
}
