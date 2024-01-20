//
// Created by Daniel on 2024-01-13.
//

#ifndef IGE_TERRACE_H
#define IGE_TERRACE_H
#include "island_gen_effect.h"


class IGE_Terrace : public IslandGenEffect {
    GDCLASS(IGE_Terrace, IslandGenEffect);

protected:
    static void _bind_methods();

public:
    IGE_Terrace();

    int terrace_amount = 10;
    float get_terrace_amount() const {return terrace_amount;}
    void set_terrace_amount(float val) {terrace_amount = val; emit_changed();}

    float min_height = 0.0f;
    float get_min_height() const {return min_height;}
    void set_min_height(float val) {min_height = val; emit_changed();}
    float max_height = 1.0f;
    float get_max_height() const {return max_height;}
    void set_max_height(float val) {max_height = val; emit_changed();}
    float offset = 0.0f;
    float get_offset() const {return offset;}
    void set_offset(float val) {offset = val; emit_changed();}

    float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) override;
};


#endif //IGE_TERRACE_H
