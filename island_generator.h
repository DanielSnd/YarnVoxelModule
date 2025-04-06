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

private:
    // Room generation helper methods
    Vector2 GetRoomNoise(float x, float z) const;
    Vector2 GetRoomDensityModifier(const Vector3& world_pos, float base_density) const;

public:
    Array gen_effects;

    inline static Vector3 clearDirections[30] = {};

    Ref<FastNoiseLite> fnl;
    Ref<Gradient> color_ramp;
    Ref<Gradient> get_color_ramp(){return color_ramp;}
    void set_color_ramp(Ref<Gradient> value) {
        color_ramp = value;
        emit_changed();
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

    Vector2i map_size = {128,128};
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

    void generate_chunk(Vector3i chunk_number, bool force_cave = false, bool smooth_points = false);

    IslandGenerator();
    ~IslandGenerator();

    // Room generation properties
    Ref<FastNoiseLite> room_noise;
    float room_noise_scale = 0.005f;
    float room_threshold = 0.6f;
    float floor_smoothness = 0.3f;
    int room_height = 8;

    float get_room_noise_scale() const { return room_noise_scale; }
    void set_room_noise_scale(float value) { room_noise_scale = value; emit_changed(); }
    
    float get_room_threshold() const { return room_threshold; }
    void set_room_threshold(float value) { room_threshold = value; emit_changed(); }
    
    float get_floor_smoothness() const { return floor_smoothness; }
    void set_floor_smoothness(float value) { floor_smoothness = value; emit_changed(); }
    
    int get_room_height() const { return room_height; }
    void set_room_height(int value) { room_height = value; emit_changed(); }
};

#endif //ISLAND_GENERATOR_H
