//
// Created by Daniel on 2024-01-05.
//

#ifndef ISLAND_GEN_EFFECT_H
#define ISLAND_GEN_EFFECT_H

#include "island_generator.h"
#include "core/io/resource.h"
#include "island_generator.h"

class IslandGenerator;

class IslandGenEffect : public Resource {
    GDCLASS(IslandGenEffect, Resource);

protected:
    static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;


private:

public:
    bool enabled = true;
    int m_seed = -1;
    int m_index = 0;
    float GetMask(float x, float y);
    float ApplyEffectSpot(Vector2 spot, Vector2 size, float input, int seed, bool doDebug = false);
    virtual float DoApplyEffectSpot(Vector2 &spot, Vector2 &size, float input, int seed, bool doDebug = false);

    bool use_mask  = false;
    bool get_use_mask() const { return use_mask;}
    void set_use_mask(bool value) {use_mask = value; emit_changed();}

    bool invert_mask  = false;
    bool get_invert_mask() const { return invert_mask;}
    void set_invert_mask(bool value) {invert_mask = value; emit_changed();}

    float mask_frequency = 0.5f;
    float get_mask_frequency() { return mask_frequency; }
    void set_mask_frequency(float value) { mask_frequency = value; emit_changed(); }

    float mask_minimum = 0.5f;
    float get_mask_minimum() { return mask_minimum; }
    void set_mask_minimum(float value) { mask_minimum = value; emit_changed(); }

    float mask_multiplier = 0.5f;
    float get_mask_multiplier() { return mask_multiplier; }
    void set_mask_multiplier(float value) { mask_multiplier = value; emit_changed(); }

    // Getter and setter for 'enabled'
    bool get_enabled() {
        return enabled;
    }
    void set_enabled(bool value) {
        enabled = value;
        emit_changed();
    }

    // Getter and setter for 'amount'
    float amount = 1;
    float get_amount() { return amount; }
    void set_amount(float value) { amount = value; emit_changed(); }

    // Getter and setter for 'amount'
    float get_seed() {
        return m_seed;
    }
    virtual void set_seed(int value);
    IslandGenEffect();
    ~IslandGenEffect();
};



#endif //ISLAND_GEN_EFFECT_H

