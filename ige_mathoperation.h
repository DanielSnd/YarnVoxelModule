//
// Created by Daniel on 2024-01-12.
//

#ifndef IGE_MATHOPERATION_H
#define IGE_MATHOPERATION_H
#include "island_gen_effect.h"


class IGE_MathOperation : public IslandGenEffect {
    GDCLASS(IGE_MathOperation, IslandGenEffect);

protected:
    static void _bind_methods();
    void _validate_property(PropertyInfo &p_property) const;

    float Remap(float value, float from1, float to1, float from2, float to2);

public:
    enum MathType {
        MathType_Add,
        MathType_Multiply,
        MathType_Divide,
        MathType_Power,
        MathType_Square,
        MathType_Remap,
        MathType_Replace
    };

    MathType math_type = MathType_Add;
    MathType get_math_type() const {return math_type;}
    void set_math_type(MathType p_math) {math_type = p_math;emit_changed();
    notify_property_list_changed();};

    float BeforeMin = 0.0f;
    float get_before_min() const {return BeforeMin;}
    void set_before_min(float val) {BeforeMin = val; emit_changed();}
    float BeforeMax = 0.0f;
    float get_before_max() const {return BeforeMax;}
    void set_before_max(float val) {BeforeMax = val; emit_changed();}
    float AfterMin = 0.0f;
    float get_after_min() const {return AfterMin;}
    void set_after_min(float val) {AfterMin = val; emit_changed();}
    float AfterMax = 0.0f;
    float get_after_max() const {return AfterMax;}
    void set_after_max(float val) {AfterMax = val; emit_changed();}

    float Number = 0.0f;
    float get_number() const {return Number;}
    void set_number(float val) {Number = val; emit_changed();}

    float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug) override;
    IGE_MathOperation();


};

VARIANT_ENUM_CAST(IGE_MathOperation::MathType);
#endif //IGE_MATHOPERATION_H
