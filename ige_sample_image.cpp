//
// Created by Daniel on 2024-03-26.
//

#include "ige_sample_image.h"

void IGE_SampleImage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_image_to_use", "img"), &IGE_SampleImage::set_image_to_use);
    ClassDB::bind_method(D_METHOD("get_image_to_use"), &IGE_SampleImage::get_image_to_use);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_image_to_use", "get_image_to_use");

}

float IGE_SampleImage::Remap(float value, float from1, float to1, float from2, float to2) {
    return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

void IGE_SampleImage::set_image_to_use(const Ref<Texture2D> &p_img) {
    image_to_use = p_img;
    actual_image = nullptr;
    if (!image_to_use.is_null() && image_to_use.is_valid()) {
        actual_image = image_to_use->get_image();
    }
    emit_changed();
}

float IGE_SampleImage::DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) {
    //print_line("Boop ",noise->get_noise_2dv(spot));
    if (actual_image.is_null() || !actual_image.is_valid())
        return input;

    const Vector2 image_size = actual_image->get_size();
    const Vector2i rounded_new_spot = Vector2i(static_cast<int>(Remap(spot.x,0.0,size.x,0.0,image_size.x)),static_cast<int>(Remap(spot.y,0.0,size.y,0.0,image_size.y)));
    const Color col =  actual_image->get_pixel(rounded_new_spot.x,rounded_new_spot.y);
    return MAX(col.r,MAX(col.b,col.g));
}

IGE_SampleImage::IGE_SampleImage() {

}

IGE_SampleImage::~IGE_SampleImage() {
    if (image_to_use.is_valid()) image_to_use.unref();
}
