//
// Created by Daniel on 2024-01-06.
//

#ifndef IGE_FASTNOISE_H
#define IGE_FASTNOISE_H
#include "island_gen_effect.h"
#include "modules/noise/fastnoise_lite.h"

class IGE_FastNoise : public IslandGenEffect {
    GDCLASS(IGE_FastNoise, IslandGenEffect);

    protected:
        static void _bind_methods();


public:
    Ref<FastNoiseLite> noise;
    bool normalize = false;
    bool get_normalize() const {return normalize;}
    void set_normalize(bool val) {normalize = val; emit_changed();}
    float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) override;

    IGE_FastNoise();
    ~IGE_FastNoise() override;

    Ref<FastNoiseLite> get_noise();

    void set_seed(int seed) override;
    void set_noise(Ref<FastNoiseLite> p_noise);

    void noise_changed();

};


#endif //IGE_FASTNOISE_H
