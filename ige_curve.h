//
// Created by Daniel on 2024-01-13.
//

#ifndef IGE_CURVE_H
#define IGE_CURVE_H

#include "island_gen_effect.h"


class IGE_Curve : public IslandGenEffect {
    GDCLASS(IGE_Curve, IslandGenEffect);

protected:
    static void _bind_methods();

public:
    IGE_Curve();

    bool normalize = false;
    bool get_normalize() const {return normalize;}
    void set_normalize(bool val) {normalize = val; emit_changed();}
    Ref<Curve> curve;
    Ref<Curve> get_curve() const {return curve;}
    void set_curve(Ref<Curve> val) {
        if (curve.is_valid()) {
            curve->disconnect_changed(callable_mp(this, &IGE_Curve::changed_curve));
        }
        curve = val;
        emit_changed();
        if (curve.is_valid()) {
            curve->connect_changed(callable_mp(this, &IGE_Curve::changed_curve));
        }
    }

    void changed_curve() {
        emit_changed();
    }

    float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) override;

    ~IGE_Curve() override;
};


#endif //IGE_CURVE_H
