//
// Created by Daniel on 2024-01-07.
//

#ifndef IGE_FALLOFF_H
#define IGE_FALLOFF_H

#include "island_gen_effect.h"

class IGE_Falloff : public IslandGenEffect {
    GDCLASS(IGE_Falloff, IslandGenEffect);

protected:
    static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

    float get_curve_falloff(int x, int y, int width, int height, float value);

    float Remap(float value, float from1, float to1, float from2, float to2);

    float euclidianFallOff(int x, int y, float innerRadius, float outerRadius, int width, int height, float value);

    float getFalloffValue(float value);

public:
    enum FalloffType
    {
        FalloffType_Squared,
        FalloffType_Euclidian,
        FalloffType_Curve
    };

    static float getEdgeBlend(float value);

    FalloffType falloff_type = FalloffType_Squared;
    FalloffType get_falloff_type() const;
    void set_falloff_type(FalloffType p_falloff_type);

    float bottom_value = -1.0f;
    float get_bottom_value() const {return bottom_value;}
    void set_bottom_value(float val) {bottom_value = val; emit_changed();}

    //Squared Type
    int falloff_spread = 1;
    int get_falloff_spread() const {return falloff_spread;}
    void set_falloff_spread(int val) {falloff_spread = val; emit_changed();}


    //Curve type
    Ref<Curve> falloff_curve;
    Ref<Curve> get_falloff_curve() const {return falloff_curve;}
    void set_falloff_curve(Ref<Curve> val) {
        if (falloff_curve.is_valid()) {
            falloff_curve->disconnect_changed(callable_mp(this, &IGE_Falloff::changed_curve));
        }
        falloff_curve = val;
        emit_changed();
        if (falloff_curve.is_valid()) {
            falloff_curve->connect_changed(callable_mp(this, &IGE_Falloff::changed_curve));
        }
    }

    void changed_curve() {
        emit_changed();
    }
    //Euclidian Type
    float falloff_inner_outer_radius_x = 0.0f;
    float get_falloff_rad_x() const {return falloff_inner_outer_radius_x;}
    void set_falloff_rad_x(float val) {falloff_inner_outer_radius_x = val; emit_changed();}

    float falloff_inner_outer_radius_y = 60.0f;
    float get_falloff_rad_y() const {return falloff_inner_outer_radius_y;}
    void set_falloff_rad_y(float val) {falloff_inner_outer_radius_y = val; emit_changed();}

    //Any type
    float falloff_clamp_min = -1;
    float get_falloff_clamp_min() const {return falloff_clamp_min;}
    void set_falloff_clamp_min(float val) {falloff_clamp_min = val; emit_changed();}
    float falloff_clamp_max = 4;
    float get_falloff_clamp_max() const {return falloff_clamp_max;}
    void set_falloff_clamp_max(float val) {falloff_clamp_max = val; emit_changed();}

    float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) override;

    IGE_Falloff();
    ~IGE_Falloff() override;
};

VARIANT_ENUM_CAST(IGE_Falloff::FalloffType);

#endif //IGE_FALLOFF_H
