//
// Created by Daniel on 2024-01-06.
//

#include "ige_fastnoise.h"

void IGE_FastNoise::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_noise", "noise"), &IGE_FastNoise::set_noise);
    ClassDB::bind_method(D_METHOD("get_noise"), &IGE_FastNoise::get_noise);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "FastNoiseLite"), "set_noise", "get_noise");

    ClassDB::bind_method(D_METHOD("set_normalize", "normalize"), &IGE_FastNoise::set_normalize);
    ClassDB::bind_method(D_METHOD("get_normalize"), &IGE_FastNoise::get_normalize);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "normalize"), "set_normalize", "get_normalize");
}

float IGE_FastNoise::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    //print_line("Boop ",noise->get_noise_2dv(spot));
    if(noise.is_valid()) {
        if (seed != m_seed) {
            m_seed = seed;
        }
        if (noise->get_seed() != m_seed) {
            noise->set_seed(m_seed);
        }
        if(normalize) {
            return Math::inverse_lerp(-1.0f,1.0f,noise->get_noise_2dv(spot));
        }
        return noise->get_noise_2dv(spot);
    }
    return input;
}

IGE_FastNoise::IGE_FastNoise() {
    noise = Ref<Noise>();
}

IGE_FastNoise::~IGE_FastNoise() {
    if (noise.is_valid()) {
        noise.unref();
    }
}

Ref<FastNoiseLite> IGE_FastNoise::get_noise() {
    return noise;
}

void IGE_FastNoise::set_seed(int seed) {
    IslandGenEffect::set_seed(seed);
    if (noise.is_valid()) {
        noise->set_seed(seed);
        //print_line("Setting seed ",seed," on noise ",noise->get_seed());
    }
}

void IGE_FastNoise::set_noise(Ref<FastNoiseLite> p_noise) {
    if (p_noise == noise) {
        return;
    }
    if (noise.is_valid()) {
        noise->disconnect_changed(callable_mp(this, &IGE_FastNoise::noise_changed));
    }
    noise = p_noise;
    if (noise.is_valid()) {
        noise->connect_changed(callable_mp(this, &IGE_FastNoise::noise_changed));
        if (noise->get_seed() != m_seed) {
            noise->set_seed(m_seed);
        }
    }


    emit_changed();
}

void IGE_FastNoise::noise_changed() {
    emit_changed();
}