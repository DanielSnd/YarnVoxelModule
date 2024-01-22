//
// Created by Daniel on 2024-01-05.
//

#include "island_generator.h"

#include "ige_fastnoise.h"
#include "yarnvoxel.h"
#include "yvoxelchunk.h"
#include "core/core_string_names.h"
#include "modules/noise/fastnoise_lite.h"

float IslandGenerator::GetTerrainHeight(Vector2 chunkPos, bool doDebug = false) {
    float result = 1;
    for (int i = 0; i < gen_effects.size(); i++) {
        Ref<IslandGenEffect> ies = gen_effects[i];
        if (ies.is_valid()) {
            if (ies->m_index != i) {
                ies->m_index = i;
            }
            result = (ies->ApplyEffectSpot(chunkPos, map_size, result, seed,doDebug));
        }
    }
    return result;
}

void IslandGenerator::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_terrain_height","chunkPos","doDebug"), &IslandGenerator::GetTerrainHeight,DEFVAL(false));
    ClassDB::bind_method(D_METHOD("generate_island","world_pos"), &IslandGenerator::generate_island,DEFVAL(Vector3(0,0,0)));

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &IslandGenerator::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &IslandGenerator::get_seed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

    ADD_GROUP("Sizes","size_");
    ClassDB::bind_method(D_METHOD("get_map_size"), &IslandGenerator::get_map_size);
    ClassDB::bind_method(D_METHOD("set_map_size", "value"), &IslandGenerator::set_map_size);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "size_map"), "set_map_size", "get_map_size");

    //PROPERTY_HINT_RANGE, "0,400,0.01,or_greater"
    ClassDB::bind_method(D_METHOD("get_height_multiplier"), &IslandGenerator::get_height_multiplier);
    ClassDB::bind_method(D_METHOD("set_height_multiplier", "value"), &IslandGenerator::set_height_multiplier);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "size_height_multiplier"), "set_height_multiplier", "get_height_multiplier");

    ClassDB::bind_method(D_METHOD("get_water_level"), &IslandGenerator::get_water_level);
    ClassDB::bind_method(D_METHOD("set_water_level", "value"), &IslandGenerator::set_water_level);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "size_water_level"), "set_water_level", "get_water_level");

    ADD_GROUP("Elements","el_");
    ClassDB::bind_method(D_METHOD("get_terrain_min"), &IslandGenerator::get_terrain_min);
    ClassDB::bind_method(D_METHOD("set_terrain_min", "value"), &IslandGenerator::set_terrain_min);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "el_terrain_min",PROPERTY_HINT_RANGE, "0,1.2,0.001"), "set_terrain_min", "get_terrain_min");

    ClassDB::bind_method(D_METHOD("get_min_cave_height"), &IslandGenerator::get_min_cave_height);
    ClassDB::bind_method(D_METHOD("set_min_cave_height", "value"), &IslandGenerator::set_min_cave_height);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "el_min_cave_height"), "set_min_cave_height", "get_min_cave_height");

    ClassDB::bind_method(D_METHOD("get_cave_smoothness"), &IslandGenerator::get_cave_smoothness);
    ClassDB::bind_method(D_METHOD("set_cave_smoothness", "value"), &IslandGenerator::set_cave_smoothness);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "el_cave_smoothness",PROPERTY_HINT_RANGE, "0,0.1,0.001"), "set_cave_smoothness", "get_cave_smoothness");

    ClassDB::bind_method(D_METHOD("get_cave_lacunarity"), &IslandGenerator::get_cave_lacunarity);
    ClassDB::bind_method(D_METHOD("set_cave_lacunarity", "value"), &IslandGenerator::set_cave_lacunarity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "el_cave_lacunarity",PROPERTY_HINT_RANGE, "0,3.0,0.001"), "set_cave_lacunarity", "get_cave_lacunarity");

    ClassDB::bind_method(D_METHOD("get_cave_if_above"), &IslandGenerator::get_cave_if_above);
    ClassDB::bind_method(D_METHOD("set_cave_if_above", "value"), &IslandGenerator::set_cave_if_above);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "el_cave_if_above",PROPERTY_HINT_RANGE, "-1.0,1.0,0.001"), "set_cave_if_above", "get_cave_if_above");

    //(Vector3i pointNumber, YVoxelChunk* ic, bool calculateAirDirection) {
    ClassDB::bind_method(D_METHOD("get_biggest_surrounding_incidence","point_number","voxel_chunk","calculate_air"), &IslandGenerator::FindBiggestSurroundingIncidence,DEFVAL(false));

    ClassDB::bind_method(D_METHOD("get_slope_for_stone"), &IslandGenerator::get_slope_for_stone);
    ClassDB::bind_method(D_METHOD("set_slope_for_stone", "value"), &IslandGenerator::set_slope_for_stone);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "el_slope_for_stone"), "set_slope_for_stone", "get_slope_for_stone");

    ClassDB::bind_method(D_METHOD("get_slope_for_grass"), &IslandGenerator::get_slope_for_grass);
    ClassDB::bind_method(D_METHOD("set_slope_for_grass", "value"), &IslandGenerator::set_slope_for_grass);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "el_slope_for_grass"), "set_slope_for_grass", "get_slope_for_grass");

    ClassDB::bind_method(D_METHOD("set_effects", "effects"), &IslandGenerator::set_effects);
    ClassDB::bind_method(D_METHOD("get_effects"), &IslandGenerator::get_effects);

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_ARRAY_TYPE, MAKE_RESOURCE_TYPE_HINT("IslandGenEffect")), "set_effects", "get_effects");

    ClassDB::bind_method(D_METHOD("set_color_ramp", "color_ramp"), &IslandGenerator::set_color_ramp);
    ClassDB::bind_method(D_METHOD("get_color_ramp"), &IslandGenerator::get_color_ramp);

    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "color_ramp", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_color_ramp", "get_color_ramp");
}

Array IslandGenerator::get_effects() const {
    return gen_effects;
}

void IslandGenerator::set_effects(const Array &p_effects) {
    const auto callable = callable_mp(this, &IslandGenerator::do_changed);
    for (int i = 0; i < p_effects.size(); i++) {
        Ref<IslandGenEffect> ies = p_effects[i];
        if(ies.is_valid()) {
            ies->m_seed = seed;
            ies->m_index = i;
            if(!(ies->is_connected(CoreStringNames::get_singleton()->changed,callable))) {
                ies->connect(CoreStringNames::get_singleton()->changed,callable);
            }
        }
    }
    gen_effects = p_effects;
    emit_changed();
}

void IslandGenerator::do_changed() {
    emit_changed();
}

int IslandGenerator::get_seed() {
    return seed;
}

void IslandGenerator::set_seed(int value) {
    seed = value;
    for (int i = 0; i < gen_effects.size(); i++) {
        Ref<IslandGenEffect> ies = gen_effects[i];
        if(ies.is_valid()) {
            ies->set_seed(value);
        }
    }
    emit_changed();
}

// Map Size
Vector2i IslandGenerator::get_map_size() {
    return map_size;
}
void IslandGenerator::set_map_size(Vector2i value) {
    map_size = value;
    emit_changed();
    // scratch_map.resize(value.x, std::vector<float>(value.y, 0.0f));
    // use_map.resize(map_size.x, std::vector<float>(map_size.y, 0.0f));
}

float IslandGenerator::get_water_level() {
    return water_level;
}

void IslandGenerator::set_water_level(float value) {
    water_level = value;
}

// Cave Smoothness
float IslandGenerator::get_cave_smoothness() {
    return cave_smoothness;
}
void IslandGenerator::set_cave_smoothness(float value) {
    cave_smoothness = value;
    emit_changed();
}

// Cave Lacunarity
float IslandGenerator::get_cave_lacunarity()  {
    return cave_lacunarity;
}
void IslandGenerator::set_cave_lacunarity(float value) {
    cave_lacunarity = value;
    emit_changed();
}

// Cave If Above
float IslandGenerator::get_cave_if_above()  {
    return cave_if_above;
}
void IslandGenerator::set_cave_if_above(float value) {
    cave_if_above = value;
    emit_changed();
}

// Height Multiplier
float IslandGenerator::get_height_multiplier() {
    return height_multiplier;
}
void IslandGenerator::set_height_multiplier(float value) {
    height_multiplier = value;
    emit_changed();
}

// Slope For Stone
Vector2 IslandGenerator::get_slope_for_stone() {
    return slope_for_stone;
}
void IslandGenerator::set_slope_for_stone(Vector2 value) {
    slope_for_stone = value;
    emit_changed();
}

// Slope For Grass
Vector2 IslandGenerator::get_slope_for_grass() {
    return slope_for_grass;
}
void IslandGenerator::set_slope_for_grass(Vector2 value) {
    slope_for_grass = value;
    emit_changed();
}

int IslandGenerator::adjustedPosition(int centerWorld, int desired,int halfSize) {
    return (centerWorld + desired) - halfSize;
}

void IslandGenerator::generate_island(Vector3 realWorldPosition) {
    fnl->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    fnl->set_frequency(cave_smoothness);
    fnl->set_fractal_lacunarity(cave_lacunarity);
    fnl->set_seed(seed);
    bool hasIslandChunk = false;
    TypedArray<Vector3i> used_chunks;
    Vector3i previousChunkNumberCached {0,0,0};
    YVoxelChunk* currentIslandChunk = nullptr;
    HashMap<Vector2i,int> highestYWithBlockMap;
    auto yarnvoxel_singleton = YarnVoxel::get_singleton();
    const int halfSizeX = map_size.x / 2;
    const int halfSizeZ = map_size.y /2;
    int firstXAboveWater = map_size.x + 1;
    int firstZAboveWater = map_size.y + 1;
    int lastXAboveWater = 0;
    int lastZAboveWater = 0;
    int highestYWithBlock = -99;

    // Assuming map_size is a struct or class with x and y members
    // If it's not, replace it with the appropriate type.

    std::vector<std::vector<float>> map(map_size.x + 1, std::vector<float>(map_size.y + 1));
    yarnvoxel_singleton->water_level = water_level;
    for (int x = 0; x < map_size.x; x++) {
        for (int z = 0; z < map_size.y; z++) {
            Vector3i newPos = {adjustedPosition(static_cast<int>(realWorldPosition.x), x, halfSizeX), 0, adjustedPosition(static_cast<int>(realWorldPosition.z), z, halfSizeZ)};
            const float terrainFloat = IslandGenerator::GetTerrainHeight(Vector2(static_cast<float>(x), static_cast<float>(z)),false);
            float desiredHeight = (terrainFloat * height_multiplier);
            bool firstOfItsY = false;

            const int roundedHeight = static_cast<int>(Math::round(desiredHeight));
            map[x][z] = desiredHeight;
            bool currentIsInTerrain = false;
            bool currentIsReplacedWith3DDensity = false;
            // if (roundedHeight + 2 > height_multiplier + 5) {
            //     float tryAgain = GetTerrainHeight(x, z,true);
            //     Debug.Log($"Something wrong found rounded {roundedHeight} on x{x} z{z} float found is {terrainFloat} trying again {tryAgain} desired height {desiredHeight}");
            // }
            int countOfTerrainBlocksAbove = 0;
            highestYWithBlock = -99;
            float aboveDensity = 1;
            for (int y = roundedHeight+2; y >= -10; y--)
            {
                newPos.y = y;
                const bool yAboveWaterLevel = static_cast<float>(y) >= water_level;
                bool yAboveWaterLevelPlusSand = static_cast<float>(y) >= water_level + 2.0f;
                if (yAboveWaterLevel) {
                    if (x > lastXAboveWater) lastXAboveWater = x;
                    if (z > lastZAboveWater) lastZAboveWater = z;
                    if (x < firstXAboveWater) firstXAboveWater = x;
                    if (z < firstZAboveWater) firstZAboveWater = z;
                }
                Vector3i chunkNumber = YarnVoxel::GetChunkNumberFromPosition(newPos);
                if ((previousChunkNumberCached != chunkNumber || !hasIslandChunk)) {
                    currentIslandChunk = yarnvoxel_singleton->get_chunk(chunkNumber);
                    hasIslandChunk = true;
                    previousChunkNumberCached = chunkNumber;
                    if(!used_chunks.has(chunkNumber)) {
                        used_chunks.append(chunkNumber);
                    }
                }

                uint8_t desiredByte = 2;
                const bool wasAboveInTerrain = currentIsInTerrain;
                //bool wasAboveReplacedWith3DDensity = currentIsReplacedWith3DDensity;
                auto pointsPosition = YarnVoxel::GetPointNumberFromPosition(newPos);

                float density = CLAMP(static_cast<float>(y) - desiredHeight, -1, 1);

                if(yAboveWaterLevel && fnl.is_valid()) {
                    auto density3d =  GetDensity3D(static_cast<float>(newPos.x),static_cast<float>(newPos.y),static_cast<float>(newPos.z),density,desiredHeight);
                    density = density3d.x;
                    currentIsReplacedWith3DDensity = density3d.y>0.1f;
                }

                currentIsInTerrain = density < YARNVOXEL_TERRAIN_SURFACE;
                if (density < terrain_min) {
                    if (yAboveWaterLevel)
                    {
                        if (currentIsInTerrain) { countOfTerrainBlocksAbove++; }
                        else countOfTerrainBlocksAbove = 0;
                        if ((countOfTerrainBlocksAbove < 2 && static_cast<float>(newPos.y) <= water_level+1.0f) || static_cast<float>(newPos.y) <= water_level)
                            desiredByte = YarnVoxel::BlockType::SAND;
                        else if (countOfTerrainBlocksAbove > 5) {
                        }
                    }
                    else { desiredByte = YarnVoxel::BlockType::SAND; }

                    if (y<highestYWithBlock && !currentIsReplacedWith3DDensity && yAboveWaterLevelPlusSand && ((!wasAboveInTerrain && currentIsInTerrain ) || (wasAboveInTerrain && currentIsInTerrain && countOfTerrainBlocksAbove>=1)) && density < -0.99f) {
                        density = Math::lerp(aboveDensity, density, 0.165f);
                    }
                    if(currentIslandChunk != nullptr) {
                        currentIslandChunk->points[pointsPosition.x][pointsPosition.y][pointsPosition.z] = YarnVoxelData::YVPointValue((density >0.55f && yAboveWaterLevelPlusSand ? static_cast<uint8_t>(0) :desiredByte), floatToInt16(density));
                    }
                    if (y > highestYWithBlock) highestYWithBlock = y;
                }

                if (y == -9) {
                    highestYWithBlockMap[Vector2i{x, z}] = highestYWithBlock;
                }
                aboveDensity = density;
            }
        }
    }

    firstXAboveWater = CLAMP(firstXAboveWater, 0, map_size.x);
    firstZAboveWater = CLAMP(firstZAboveWater, 0, map_size.y);
    lastXAboveWater = CLAMP(lastXAboveWater, 0, map_size.x);
    lastZAboveWater = CLAMP(lastZAboveWater, 0, map_size.y);

    hasIslandChunk = false;
    previousChunkNumberCached = Vector3i{-1,-1,-1};
    uint8_t stoneByte = (YarnVoxel::BlockType::STONE);
    currentIslandChunk = nullptr;

    for (int x = firstXAboveWater; x < lastXAboveWater; x++) {
        for (int z = firstZAboveWater; z < lastZAboveWater; z++) {
            Vector3i newPos = Vector3i(adjustedPosition(static_cast<int>(realWorldPosition.x), x, halfSizeX), 0,
                adjustedPosition(static_cast<int>(realWorldPosition.z), z, halfSizeZ));
            int roundedHeight = static_cast<int>(Math::floor(map[x][z]));
            highestYWithBlock = highestYWithBlockMap[Vector2i{x, z}];
            if (highestYWithBlock <= 0) continue;
            //Debug.Log($"{x} and {z} highestY is {highestYWithBlock}");
            int surfaceIshMin = highestYWithBlock - 4;
            for (int y = roundedHeight + 1; static_cast<float>(y) >= water_level; y--) {
                newPos.y = y;
                Vector3i chunkNumber = yarnvoxel_singleton->FindChunkNumberFromPosition(newPos);
                if ((previousChunkNumberCached != chunkNumber || !hasIslandChunk)) {
                    currentIslandChunk = yarnvoxel_singleton->get_chunk(chunkNumber);
                    hasIslandChunk = true;
                    previousChunkNumberCached = chunkNumber;
                    //IslandManager.SetDirtyChunk(chunkNumber);
                    if(!used_chunks.has(chunkNumber)) {
                        used_chunks.append(chunkNumber);
                    }
                }

                Vector3i pointsPosition = YarnVoxel::GetPointNumberFromPosition(newPos);
                int desiredCubeConfig = FindCubeConfiguration(pointsPosition.x,pointsPosition.y,pointsPosition.z,currentIslandChunk, false);
                if (desiredCubeConfig == 0 || desiredCubeConfig == 255) continue;
                auto slopeAverage = static_cast<float>(YarnVoxelData::AverageSlopeTable[desiredCubeConfig]);
                if (slopeAverage <= -990) {
                    continue;
                }

                if ((slopeAverage < slope_for_stone.x ||
                     slopeAverage > slope_for_stone.y)) {
                    currentIslandChunk->SetPointSurrounding(pointsPosition, stoneByte);
                } else if (slopeAverage > slope_for_grass.y &&
                           slopeAverage < slope_for_grass.x && y > surfaceIshMin && static_cast<float>(y) >=water_level+1) {
                    //Debug.Log($"{x},{y},{z} surfaceish is {surfaceIshMin}");
                    currentIslandChunk->SetPointSurrounding(pointsPosition, 1);
                    if (!(static_cast<float>(y) < water_level + 2)) {
                        continue;
                    }

                    Vector3i under_point = newPos + Vector3i(0,-1,0);
                    Vector3i underChunkNumber = yarnvoxel_singleton->FindChunkNumberFromPosition(under_point);
                    if (!used_chunks.has(underChunkNumber))
                        continue;

                    auto underPoint = YarnVoxel::GetPointNumberFromPosition(under_point);
                    auto under_chunk = yarnvoxel_singleton->get_chunk(underChunkNumber);
                    under_chunk->SetPointSurrounding(underPoint, 1);
                }
            }
        }
    }

    for (int x = firstXAboveWater; x < lastXAboveWater; x++) {
        for (int z = firstZAboveWater; z < lastZAboveWater; z++) {
            Vector3i newPos = Vector3i(adjustedPosition(static_cast<int>(realWorldPosition.x), x, halfSizeX), 0, adjustedPosition(static_cast<int>(realWorldPosition.z), z, halfSizeZ));
            int roundedHeight = YarnVoxel::FastFloor(map[x][z]);
            for (int y = roundedHeight -2; y >= water_level; y--) {
                newPos.y = y;
                if (Vector3i chunkNumber = YarnVoxel::GetChunkNumberFromPosition(newPos); (previousChunkNumberCached != chunkNumber || !hasIslandChunk)) {
                    currentIslandChunk = yarnvoxel_singleton->get_chunk(chunkNumber);
                    previousChunkNumberCached = chunkNumber;
                   // IslandManager.SetDirtyChunk(chunkNumber);
                }
                if (currentIslandChunk != nullptr) {
                    auto pointsPosition = YarnVoxel::GetPointNumberFromPosition(newPos);
                    auto yvpoint = currentIslandChunk->points[pointsPosition.x][pointsPosition.y][pointsPosition.z];
                    if (yvpoint.floatValue > ZERO_SHORT) continue;
                    Vector3i amountTypeSurrounding = FindBiggestSurroundingIncidence(pointsPosition, currentIslandChunk, false);
                    if(auto foundBestBlockType = static_cast<uint8_t>(amountTypeSurrounding.y); amountTypeSurrounding.x >= 3 && foundBestBlockType != 0 && static_cast<uint8_t>(foundBestBlockType)!=yvpoint.byteValue) {
                        currentIslandChunk->points[pointsPosition.x][pointsPosition.y][pointsPosition.z] = YarnVoxelData::YVPointValue(foundBestBlockType, yvpoint.floatValue);
                    }
                }
            }
        }
    }

    for (int i = 0; i < used_chunks.size(); i++) {
        yarnvoxel_singleton->set_dirty_chunk(used_chunks[i]);
    }
}

int IslandGenerator::FindCubeConfiguration(int x, int y, int z, YVoxelChunk* ic,bool do_debug) {
    int corner_index = 0;
    int configurationIndex = 0;

    for (const Vector3i corner : YarnVoxelData::CornerTable) {
        const int desired_x = x+corner.x;
        const int desired_y = y+corner.y;
        const int desired_z = z+corner.z;
        if(do_debug) {
            print_line(vformat("Going through find cube configuration %s,%s,%s other %s,%s,%s in range? %s",x,y,z,desired_x,desired_y,desired_z, ic->is_point_position_in_range_without_neighbours(desired_x,desired_y,desired_z)));
        }
        if(ic->is_point_position_in_range_without_neighbours(desired_x,desired_y,desired_z)) {
            const auto point_found = &ic->points[desired_x][desired_y][desired_z];
            if (point_found->floatValue > YARNVOXEL_TERRAIN_SURFACE) {
                configurationIndex |= 1 << corner_index;
            }
            corner_index = corner_index+1;
        } else {
            auto worldPosToTest = ic->get_world_pos_from_point_number({desired_x,desired_y,desired_z});
            auto otherChunkNumber = YarnVoxel::GetChunkNumberFromPosition(worldPosToTest);
            YVoxelChunk* tryChunk = nullptr;
            if(YarnVoxel::try_get_chunk(otherChunkNumber, tryChunk)) {
                const auto otherDesiredPoint = YarnVoxel::GetPointNumberFromPosition(worldPosToTest);
                const auto point_found = &(tryChunk->points[otherDesiredPoint.x][otherDesiredPoint.y][otherDesiredPoint.z]);
                if(do_debug) {
                    print_line("Found test point on other chunk ",otherDesiredPoint,", point found ",point_found->floatValue);
                }
                if (point_found->floatValue > YARNVOXEL_TERRAIN_SURFACE) {
                    configurationIndex |= 1 << corner_index;
                }
                corner_index = corner_index+1;
            } else {
                if(do_debug) {
                    print_line("Other chunk not found");
                }
                configurationIndex |= 1 << corner_index;
                corner_index = corner_index+1;
            }
        }
    }

    if(do_debug) {
        print_line(vformat("Found cube configuration %s",configurationIndex));
    }
    return configurationIndex;
}

Vector3i IslandGenerator::FindBiggestSurroundingIncidence(Vector3i pointNumber, YVoxelChunk* ic, bool calculateAirDirection) {
        uint8_t return_block;
        int stoneCount=0, sandCount=0, grassCount=0, dirtCount=0;
        int airSurroundingCount = 0;
        //int usingClearDirections = 0;
        YVoxelChunk* other_cached_chunk = nullptr;
        //if(pointNumber.x > YARNVOXEL_CHUNK_WIDTH || pointNumber.x <0) Debug.Log("Something wrong with biggest surrounding incidence");
        //Vector3 averageClearDirection;
        for (auto surroundingVectorDir : YarnVoxelData::SurroundingTable) {
            Vector3i pointPos = pointNumber + surroundingVectorDir;
            if( calculateAirDirection) {
                print_line("checking ",pointNumber," point pos to check ",pointPos," direction ",surroundingVectorDir," chunk number ",ic->chunk_number);
            }
            const YarnVoxelData::YVPointValue* foundInfo = nullptr;
            if (YarnVoxel::IsPositionValid(pointPos)) {
                foundInfo = &ic->points[pointPos.x][pointPos.y][pointPos.z];
                if( calculateAirDirection)
                    print_line("valid pos ",pointPos," found info byte",foundInfo->byteValue," found info float ",foundInfo->floatValue);
            } else {
                const auto desired_pos_other = ic->get_world_pos_from_point_number(pointPos);
                auto other_chunk_number = YarnVoxel::GetChunkNumberFromPosition(desired_pos_other);
                if( calculateAirDirection)
                    print_line("invalid pos ",pointPos," other chunk ",other_chunk_number);
                if ((other_cached_chunk != nullptr && other_cached_chunk->chunk_number == other_chunk_number)) {
                    if (const auto other_point_pos = YarnVoxel::GetPointNumberFromPosition(desired_pos_other); YarnVoxel::IsPositionValid(other_point_pos)) {
                        foundInfo = &(other_cached_chunk->points[other_point_pos.x][other_point_pos.y][other_point_pos.z]);
                        if( calculateAirDirection)
                            print_line("found in other chunk cached ",other_point_pos," found info byte",foundInfo->byteValue," found info float ",foundInfo->floatValue);
                    } else {
                        continue;
                    }
                } else if((YarnVoxel::try_get_chunk(other_chunk_number, other_cached_chunk) && other_cached_chunk != nullptr)) {
                    if (const auto other_point_pos = YarnVoxel::GetPointNumberFromPosition(desired_pos_other); YarnVoxel::IsPositionValid(other_point_pos)) {
                        foundInfo = &(other_cached_chunk->points[other_point_pos.x][other_point_pos.y][other_point_pos.z]);
                        if( calculateAirDirection)
                            print_line("found in other chunk noncached ",other_point_pos," found info byte",foundInfo->byteValue," found info float ",foundInfo->floatValue);
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }
            }
            // if( calculateAirDirection)
            //     print_line("point ",pointNumber," has ",(foundInfo != nullptr));
            if (foundInfo != nullptr) {
                if (foundInfo->floatValue < BIT_LESS_THAN_ZERO_SHORT) {
                    const auto foundbType = foundInfo->byteValue;
                    if (foundbType == YarnVoxel::BlockType::STONE) {
                        stoneCount++;

                        if( calculateAirDirection)
                            print_line("point ",pointNumber," stone ",stoneCount);
                    }
                    if (foundbType == YarnVoxel::BlockType::GRASS) {
                        grassCount++;

                        if( calculateAirDirection)
                            print_line("point ",pointNumber," grass ",grassCount);
                    }
                    if (foundbType == YarnVoxel::BlockType::SAND) {
                        sandCount++;

                        if( calculateAirDirection)
                            print_line("point ",pointNumber," sand ",sandCount);
                    }
                    if (foundbType == YarnVoxel::BlockType::DIRT) {
                        dirtCount++;

                        if( calculateAirDirection)
                            print_line("point ",pointNumber," dirt ",dirtCount);
                    }
                }
                // if (calculateAirDirection && (foundInfo->floatValue > 0.0001f || foundInfo->byteValue == 0)) {
                //     if (usingClearDirections < ClearDirectionsArraySize)
                //     {
                //         auto cleardirection = static_cast<Vector3>(pointPos - pointNumber);
                //         cleardirection.normalize();
                //         clearDirections[usingClearDirections] = cleardirection;
                //         usingClearDirections++;
                //     }
                //     airSurroundingCount++;
                // }
            }
        }

        if (!calculateAirDirection) {
            //averageClearDirection = Vector3(-3, -2, -2);
        }
        // else {
        //     if (usingClearDirections <= 0) {
        //         averageClearDirection = Vector3(-3, -2, -2);
        //     }
        //     else
        //     {
        //         Vector3 averageDirection = {0,0,0};
        //         for (int i = 0; i < usingClearDirections; i++) {
        //             if(i < ClearDirectionsArraySize)
        //                 averageDirection += clearDirections[i];
        //         }
        //
        //         auto cleardir = static_cast<Vector3>(averageDirection / static_cast<float>(usingClearDirections));
        //         cleardir.normalize();
        //         averageClearDirection = cleardir;
        //     }
        // }

        if (stoneCount > sandCount && stoneCount > grassCount && stoneCount > dirtCount) {
            return_block = YarnVoxel::BlockType::STONE;
            return Vector3(static_cast<float>(stoneCount),return_block,static_cast<float>(airSurroundingCount));
        }
        if (dirtCount > stoneCount && dirtCount > grassCount && dirtCount > sandCount) {
            return_block = YarnVoxel::BlockType::DIRT;
            return Vector3(static_cast<float>(dirtCount),return_block,static_cast<float>(airSurroundingCount));
        }
        if (grassCount > stoneCount && grassCount > dirtCount && grassCount > sandCount) {
            return_block = YarnVoxel::BlockType::GRASS;
            return Vector3(static_cast<float>(grassCount),return_block,static_cast<float>(airSurroundingCount));
        }
        if (sandCount > stoneCount && sandCount > dirtCount && sandCount > grassCount) {
            return_block = YarnVoxel::BlockType::SAND;
            return Vector3(static_cast<float>(sandCount),return_block,static_cast<float>(airSurroundingCount));
        }
        if (stoneCount > 0 && stoneCount == grassCount) {
            return_block = YarnVoxel::BlockType::STONE;
            return Vector3(static_cast<float>(stoneCount),return_block,static_cast<float>(airSurroundingCount));
        }

        if( calculateAirDirection)
            print_line("something wrong, returning zero. Stone ",stoneCount," dirt ",dirtCount," grass ",grassCount," sand ",sandCount);
        return Vector3(0,0,0);
}

Vector2 IslandGenerator::GetDensity3D(float x, float y, float z, float initialDensity, float desiredHeight) const {
    Vector2 return_density = {0,-1};
    float density = initialDensity;
    if (density < 0.001f && y >= min_cave_height) {
        const float noiseToCheckIfShouldReplaceWith3DDensity = fnl->get_noise_3d(x + 1024.0f, y + 1024.0f, z + 1024.0f);
        if (noiseToCheckIfShouldReplaceWith3DDensity > cave_if_above) {
            const float gotNoise = fnl->get_noise_3d(x, y, z);
            return_density.y = 1.0f;
            if (gotNoise > 0.0f || density < -0.95f)
                density = gotNoise;
        }
        else {
            return_density.y = -1.0;
        }
    } else {
        return_density.y = -1.0;
    }
    return_density.x = density;
    return return_density;
}

IslandGenerator::IslandGenerator() {
    fnl = Ref<FastNoiseLite>(memnew(FastNoiseLite));
    fnl->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
    fnl->set_frequency(cave_smoothness);
    fnl->set_fractal_lacunarity(cave_lacunarity);

    //GenEffects = new Array;
    // scratch_map.resize(map_size.x, std::vector<float>(map_size.y, 0.0f));
    // use_map.resize(map_size.x, std::vector<float>(map_size.y, 0.0f));
}

IslandGenerator::~IslandGenerator() {
    if(fnl.is_valid()) {
        fnl.unref();
    }
    //DESTRUCTOR
    gen_effects.clear();
}
