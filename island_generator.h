//
// Created by Daniel on 2024-01-05.
//

#ifndef ISLAND_GENERATOR_H
#define ISLAND_GENERATOR_H

//#include <vector>
#include "island_gen_effect.h"
#include <vector>

#include "yarnvoxel.h"
#include "yvoxelchunk.h"
#include "core/io/resource.h"
#include "core/variant/typed_array.h"
#include "modules/noise/fastnoise_lite.h"
#include "scene/resources/texture.h"

class IslandGenEffect;

class IslandGenerator : public Resource {
    GDCLASS(IslandGenerator, Resource);

protected:

    static void _bind_methods();
    //private:

public:
    Array gen_effects;

    inline static Vector3 clearDirections[30] = {};

    Ref<FastNoiseLite> fnl;
    Ref<Gradient> color_ramp;
    Ref<Gradient> get_color_ramp(){return color_ramp;}
    void set_color_ramp(Ref<Gradient> value) {
        color_ramp = value;
    }

    float GetTerrainHeight(Vector2 chunkPos, bool doDebug);
    // std::vector<std::vector<float>> use_map;
    // std::vector<std::vector<float>> scratch_map;

    Array get_effects() const;
    void set_effects(const Array &p_effects);

    void do_changed();

    int seed = -1;

    int get_seed();
    void set_seed(int value);

    Vector2i map_size = {256,256};
    Vector2i get_map_size();
    void set_map_size(Vector2i value);

    float water_level = -1.15f;
    float get_water_level();
    void set_water_level(float value);

    float terrain_min = 1.2f;
    float get_terrain_min() const { return terrain_min;}
    void set_terrain_min(float value) {terrain_min = value;}
    float min_cave_height = -4.2f;
    float get_min_cave_height() const { return min_cave_height;}
    void set_min_cave_height(float value) {min_cave_height = value;}
    float cave_smoothness = 0.0178;
    float get_cave_smoothness();
    void set_cave_smoothness(float value);

    float cave_lacunarity = 1.908;
    float get_cave_lacunarity();
    void set_cave_lacunarity(float value);

    float cave_if_above = -0.008;
    float get_cave_if_above();
    void set_cave_if_above(float value);

    float height_multiplier = 20;
    float get_height_multiplier();
    void set_height_multiplier(float value);

    Vector2 slope_for_stone = {-80,58};
    Vector2 get_slope_for_stone();
    void set_slope_for_stone(Vector2 value);

    Vector2 slope_for_grass = {46,0};
    Vector2 get_slope_for_grass();
    void set_slope_for_grass(Vector2 value);

    int adjustedPosition(int centerWorld, int desired, int halfSize);

    void generate_island(Vector3 realWorldPosition);

    int FindCubeConfiguration(int x, int y, int z, YVoxelChunk *ic, bool do_debug);

    Vector3i FindBiggestSurroundingIncidence(Vector3i pointNumber, YVoxelChunk *ic, bool calculateAirDirection);

    Vector2 GetDensity3D(float x, float y, float z, float initialDensity, float desiredHeight) const;

    IslandGenerator();
    ~IslandGenerator();
};



#endif //ISLAND_GENERATOR_H
