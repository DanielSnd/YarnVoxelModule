//
// Created by Daniel on 2024-03-26.
//

#ifndef IGE_SAMPLE_IMAGE_H
#define IGE_SAMPLE_IMAGE_H

#include "island_gen_effect.h"
#include "scene/resources/image_texture.h"

class IGE_SampleImage : public IslandGenEffect {
    GDCLASS(IGE_SampleImage, IslandGenEffect);

protected:
    static void _bind_methods();
    float Remap(float value, float from1, float to1, float from2, float to2);

public:
    Ref<Texture2D> image_to_use;
    Ref<Image> actual_image;
    void set_image_to_use(const Ref<Texture2D> &p_img);
    Ref<Texture2D> get_image_to_use() const {return image_to_use;}
    float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) override;
    IGE_SampleImage();
    ~IGE_SampleImage();

};



#endif //IGE_SAMPLE_IMAGE_H
