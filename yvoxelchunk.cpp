//
// Created by daniel on 12/25/23.
//
#include <iostream>
#include <chrono>
#include "yvoxelchunk.h"

#include "yarnvoxel.h"
#include "core/variant/variant_utility.h"
//
// void YVoxelChunk::add(int p_value) {
//     count += p_value;
// }
//
// void YarnVoxel::multiply(int p_value) {
//     count *= p_value;
// }
//
// void YarnVoxel::reset() {
//     count = 0;
// }
//

void YVoxelChunk::AttemptSetDirtyNeighbour(Vector3i pointHit) const {
    constexpr int getArrayLength = std::size(YarnVoxelData::SurroundingAndUpDownTable);
    for (auto i : YarnVoxelData::SurroundingAndUpDownTable) {
        auto cchunkNumber = YarnVoxel::GetChunkNumberFromPosition(pointHit + (i * 3));
        if (cchunkNumber != chunk_number) YarnVoxel::get_singleton()->set_dirty_chunk(cchunkNumber);
    }
}
void YVoxelChunk::add_triangle(const YarnVoxelData::YVTriangleData &yv_triangle_data,SurfaceTool *surface_tool) {
    surface_tool->set_uv(Vector2(0, 0));
    surface_tool->set_smooth_group(1);
    surface_tool->add_vertex(yv_triangle_data.v1);
    surface_tool->add_vertex(yv_triangle_data.v2);
    surface_tool->add_vertex(yv_triangle_data.v3);

    const auto count_of_vertex = static_cast<int>(surface_tool->get_vertex_array().size());
    surface_tool->add_index(count_of_vertex-3);
    surface_tool->add_index(count_of_vertex-2);
    surface_tool->add_index(count_of_vertex-1);
}
void YVoxelChunk::add_triangle(Vector3 vert1,Vector3 vert2,Vector3 vert3,SurfaceTool *surface_tool) {
    surface_tool->set_uv(Vector2(0, 0));
    surface_tool->set_smooth_group(1);
    surface_tool->add_vertex(vert1);
    surface_tool->add_vertex(vert2);
    surface_tool->add_vertex(vert3);

    const auto count_of_vertex = static_cast<int>(surface_tool->get_vertex_array().size());
    surface_tool->add_index(count_of_vertex-3);
    surface_tool->add_index(count_of_vertex-2);
    surface_tool->add_index(count_of_vertex-1);
}

bool YVoxelChunk::compare_float_values_sameish(int16_t f1, int16_t f2) {
    return (ABS(f1 - f2) < MORE_THAN_ZERO_SHORT) || ((f1 > ALMOST_FULL_SHORT && f2 > ALMOST_FULL_SHORT) || (f1 < ALMOST_EMPTY_SHORT && f2 < ALMOST_EMPTY_SHORT));
}

void YVoxelChunk::serialize_to_data() {
    data.clear();
    uint8_t hint_is_count = 254;
    uint16_t count = 0;
    uint8_t value = points[0][YARNVOXEL_CHUNK_HEIGHT-1][0].byteValue;
    int16_t floatValue = points[0][YARNVOXEL_CHUNK_HEIGHT-1][0].floatValue;
    for (int y = YARNVOXEL_CHUNK_HEIGHT-1; y >= 0; --y) {
        for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; ++z) {
            for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; ++x) {
                if (points[x][y][z].byteValue == value && compare_float_values_sameish(floatValue, points[x][y][z].floatValue)) {
                    ++count;
                } else {
                    handle_serialization_count(hint_is_count, count, value, floatValue);
                    value = points[x][y][z].byteValue;
                    floatValue = points[x][y][z].floatValue;
                    count = 1;
                }
            }
        }
    }

    //Gotta call for the last run since the for loop is over by now.
    handle_serialization_count(hint_is_count, count, value, floatValue);
}

void YVoxelChunk::handle_serialization_count(uint8_t hint_is_count, uint16_t count, uint8_t value, int16_t floatValue) {
    if (count > 1) {
        //There's multiple of the same
        //Write max value so it knows to expect a count.
        data.append(hint_is_count);
        //Write the actual value.
        data.append(value);
        //Write float value
        data.append(reinterpret_cast<uint8_t *>(&floatValue)[0]);
        data.append(reinterpret_cast<uint8_t *>(&floatValue)[1]);

        //Write count of same
        data.append(reinterpret_cast<uint8_t *>(&count)[0]);
        data.append(reinterpret_cast<uint8_t *>(&count)[1]);
    } else {
        //It's not a count, it's an actual value, so append it.
        data.append(value);
        //Convert float to int16 and write it.
        data.append(reinterpret_cast<uint8_t *>(&floatValue)[0]);
        data.append(reinterpret_cast<uint8_t *>(&floatValue)[1]);
    }
}

void YVoxelChunk::deserialize_from_data() {
    if (data.size() == 0) {
        WARN_PRINT("[YarnVoxel] Attempt to deserilaize data from an empty buffer when deserializing chunk");
        return;
    }
    int bufferIndex = 0;
    int z = 0, x = 0, y = YARNVOXEL_CHUNK_HEIGHT-1;
    constexpr uint8_t hint_is_count = static_cast<uint8_t>(254);
    int16_t actual_float_value = ZERO_SHORT;
    while (bufferIndex < data.size()) {
        uint8_t value = data[bufferIndex++];
        uint16_t count = 1;
        if (value == hint_is_count) {
            value = data[bufferIndex++];
            const auto byte1 = data[bufferIndex++];
            const auto byte2 = data[bufferIndex++];
            actual_float_value = static_cast<int16_t>(byte1 | (byte2 << 8));
            const auto byte3 = data[bufferIndex++];
            const auto byte4 = data[bufferIndex++];
            count = static_cast<uint16_t>(byte3 | (byte4 << 8));
        } else {
            const auto byte1 = data[bufferIndex++];
            const auto byte2 = data[bufferIndex++];
            actual_float_value = static_cast<int16_t>(byte1 | (byte2 << 8));
        }

        for (uint16_t i = 0; i < count; ++i) {
            points[x][y][z].byteValue = value;
            points[x][y][z].floatValue = actual_float_value;
            if (++x >= YARNVOXEL_CHUNK_WIDTH) { x = 0;
                if (++z >= YARNVOXEL_CHUNK_WIDTH) { z = 0;
                    if (--y < 0) return;
                }
            }
        }
    }

}

void YVoxelChunk::generate() {
    //TIMING!
    auto start = std::chrono::high_resolution_clock::now();
    water_level = YarnVoxel::get_singleton()->water_level;
    const YarnVoxelData::YVPointValue defaultValue = YarnVoxelData::YVPointValue();
    data.clear();

    for (int nbx = 0; nbx < 2; nbx++) {
        for (int nby = 0; nby < 2; nby++) {
            for (int nbz = 0; nbz < 2; nbz++) {
                //print_line(chunk_number,"Neighbour checking ",nbx,",",nby,",",nbz);
                if (nbx == 0 && nby == 0 && nbz == 0) {
                    continue;
                }
                bool has_neighbor = has_neighbour_chunks[nbx][nby][nbz];
                YVoxelChunk* neighbour_chunk = has_neighbor ? neighbour_chunks[nbx][nby][nbz] : nullptr;
                auto currentDirection = Vector3i(nbx, nby, nbz);
                if (nbx == 1 && nby == 1 && nbz == 1) { currentDirection = Vector3i(1,1,1);}
                //print_line(vformat("%s is neighbour fecthing %s,%s,%s currrent direction %s has neighbour %s null %s neighbour chunk number %s",chunk_number,nbx,nby,nbz,currentDirection,has_neighbor,neighbour_chunk == nullptr,(chunk_number + currentDirection)));
               // print_line("");

                if (!has_neighbor) {
                    has_neighbor = YarnVoxel::try_get_chunk(chunk_number + currentDirection, neighbour_chunk);
                    if (has_neighbor) {
                        //print_line(" is the neighbour a null pointer? ",neighbour_chunk == nullptr);
                        if (neighbour_chunk == nullptr) {
                            has_neighbor=false;
                        } else {
                            neighbour_chunks[nbx][nby][nbz] = neighbour_chunk;
                            has_neighbour_chunks[nbx][nby][nbz] = true;
                           // print_line(neighbour_chunks[nbx][nby][nbz]," is it a null pointer? ",neighbour_chunks[nbx][nby][nbz] == nullptr);
                        }
                    }
                }

                if (nbx == 1 && nby == 1 && nbz == 1) {
                    points[YARNVOXEL_CHUNK_WIDTH][YARNVOXEL_CHUNK_HEIGHT][YARNVOXEL_CHUNK_WIDTH] =
                        has_neighbor ? neighbour_chunk->points[0][0][0]: defaultValue;
                    continue;
                }
                constexpr int maxX = YARNVOXEL_CHUNK_WIDTH + 1;
                constexpr int maxY = YARNVOXEL_CHUNK_HEIGHT + 1;
                constexpr int maxZ = YARNVOXEL_CHUNK_WIDTH + 1;
                if (currentDirection == right) {
                    constexpr int x = maxX - 1;
                    for (int y = 0; y < maxY; y++)
                        for (int z = 0; z < maxZ; z++) {
                            points[x][y][z] = has_neighbor ? neighbour_chunk->points[x - nbx * (maxX - 1)][y][z] : defaultValue;
                        }
                } else if (currentDirection == forward) {
                    constexpr int z = maxZ - 1;
                    for (int x = 0; x < maxX; x++)
                        for (int y = 0; y < maxY; y++) {
                            //if (x == 7 && y == 4 && z == 32) {
                            //    print_line(" has neighbour forward? ",has_neighbor," forward chunk ",neighbour_chunk->chunk_number," other chunk position looking ",x,",",y,",",z - nbz * (maxZ - 1));
                            //}
                            points[x][y][z] = has_neighbor ? neighbour_chunk->points[x][y][z - nbz * (maxZ - 1)] : defaultValue;
                        }
                } else if (currentDirection == up) {
                    constexpr int y = maxY - 1;
                    for (int x = 0; x < maxX; x++)
                        for (int z = 0; z < maxZ; z++)
                            points[x][y][z] = has_neighbor ? neighbour_chunk->points[x][y - nby * (maxY - 1)][z] : defaultValue;
                } else if (currentDirection == rightforward) {
                    constexpr int x = maxX - 1, z = maxZ - 1;
                    for (int y = 0; y < maxY; y++)
                        points[x][y][z] = has_neighbor ? neighbour_chunk->points[x - nbx * (maxX - 1)][y][z - nbz * (maxZ - 1)] : defaultValue;
                } else if (currentDirection == rightup) {
                    constexpr int x = maxX - 1, y = maxY - 1;
                    for (int z = 0; z < maxZ; z++)
                        points[x][y][z] = has_neighbor ? neighbour_chunk->points[x - nbx * (maxX -1 )][y - nby * (maxY - 1)][z] : defaultValue;
                } else if (currentDirection == forwardup) {
                    constexpr int z = maxZ - 1, y = maxY - 1;
                    for (int x = 0; x < maxX; x++)
                        points[x][y][z] = has_neighbor ? neighbour_chunk->points[x][y - nby * (maxY - 1)][z - nbz * (maxZ - 1)] : defaultValue;
                }
            }
        }
    }


    constexpr uint8_t hint_is_count = 254;
    uint16_t count = 0;
    uint8_t value = points[0][YARNVOXEL_CHUNK_HEIGHT-1][0].byteValue;
    int16_t floatValue = points[0][YARNVOXEL_CHUNK_HEIGHT-1][0].floatValue;

    uint8_t debugging_config = YarnVoxel::get_singleton()->get_debugging_config();
    for (int y = YARNVOXEL_CHUNK_HEIGHT-1; y >= 0; y--) {
        for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; x++) {
            for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; z++){
                // START SERIALIZATION SECTION
                if (points[x][y][z].byteValue == value && compare_float_values_sameish(floatValue, points[x][y][z].floatValue)) {
                    ++count;
                } else {
                    handle_serialization_count(hint_is_count, count, value, floatValue);
                    value = points[x][y][z].byteValue;
                    floatValue = points[x][y][z].floatValue;
                    count = 1;
                }
                if (z == YARNVOXEL_CHUNK_WIDTH-1 && x == YARNVOXEL_CHUNK_WIDTH -1 && y == 0) {
                    handle_serialization_count(hint_is_count, count, value, floatValue);
                }
                // END SERIALIZATION SECTION

                //if (YarnVoxel::get_singleton()->is_debugging_chunk && (z != 0)) continue;
                int corner_index = 0;
                uint8_t relevant_byte = 0;
                for (const Vector3i corner : YarnVoxelData::CornerTable) {
                    cube[corner_index] = &points[x + corner.x][y + corner.y][z + corner.z];
                    // if (YarnVoxel::get_singleton()->is_debugging_chunk) {
                    //     print_line(vformat("Point %s doing corner %s index %s new point %s",Vector3i(x,y,z),corner,corner_index,Vector3i(x+corner.x,y+corner.y,z+corner.z)));
                    // }
                    if (relevant_byte == 0 && cube[corner_index]->byteValue != 0) {
                        relevant_byte = cube[corner_index]->byteValue;
                    }
                    corner_index = corner_index+1;
                }
                MarchCube(Vector3i(x, y, z),GetCubeConfiguration(),relevant_byte, cube[0]->health,debugging_config);
            }
        }
    }

    Ref<SurfaceTool> surface_tool = Ref<SurfaceTool>(memnew(SurfaceTool));
    surface_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

    set_mesh(memnew(ArrayMesh));

    if (YarnVoxel::get_singleton()->material.is_valid()) {
        if (get_surface_override_material_count() == 0) {
            surface_override_materials.append(YarnVoxel::get_singleton()->material);
        } else {
            set_surface_override_material(0,YarnVoxel::get_singleton()->material);
        }
    }
    auto vertex_array = &surface_tool->get_vertex_array();
    for (auto triangle : mesh_triangles) {
        //auto blockType = static_cast<YarnVoxel::BlockType>(triangle.desiredByte);
        Color desiredColor = YarnVoxelData::BlockTypeToColor[VariantUtilityFunctions::clampi(triangle.desiredByte,0,10)];
        desiredColor.a = YarnVoxelData::ByteToFloat01[triangle.health];
        //print_line("adding triangle ",i," desired color ", desiredColor," blocktype ",triangle.desiredByte);
        surface_tool->set_uv(Vector2(0, 0));
        surface_tool->set_smooth_group(1);

        auto* index1_pointer = output_pos_to_index.getptr(triangle.v1);
        int index1;

        surface_tool->set_color(desiredColor);

        if (index1_pointer == nullptr) {
            index1 = static_cast<int>(vertex_array->size());
            surface_tool->add_vertex(triangle.v1);
            output_pos_to_index[triangle.v1] = index1;
        } else {
            index1 = *index1_pointer;
            if (static_cast<int>(vertex_array->size()) > index1) {
                auto vertexFound = (&(vertex_array->operator[](index1)));
                vertexFound->color = desiredColor;
            }
            //if (triangle.desiredByte != 0) color_index_1 = true;
        }

        auto* index2_pointer = output_pos_to_index.getptr(triangle.v2);
        int index2;
        if (index2_pointer == nullptr) {
            index2 = static_cast<int>(surface_tool->get_vertex_array().size());
            surface_tool->add_vertex(triangle.v2);
            output_pos_to_index[triangle.v2] = index2;
        } else {
            index2 = *index2_pointer;
            if (static_cast<int>(vertex_array->size()) > index2) {
                auto vertexFound = (&(vertex_array->operator[](index2)));
                vertexFound->color = desiredColor;
            }
            //if (triangle.desiredByte != 0) color_index_2 = true;
        }

        auto* index3_pointer = output_pos_to_index.getptr(triangle.v3);
        int index3;
        if (index3_pointer == nullptr) {
            index3 = static_cast<int>(surface_tool->get_vertex_array().size());
            surface_tool->add_vertex(triangle.v3);
            output_pos_to_index[triangle.v3] = index3;
        } else {
            index3 = *index3_pointer;
            if (static_cast<int>(vertex_array->size()) > index3) {
                auto vertexFound = (&(vertex_array->operator[](index3)));
                vertexFound->color = desiredColor;
            }
            //if (triangle.desiredByte != 0) color_index_3 = true;
        }

        surface_tool->add_index(index1);
        surface_tool->add_index(index3);
        surface_tool->add_index(index2);
    }

    if (output_pos_to_index.size() >= 3) {
        //surface_tool->optimize_indices_for_cache();
        //int triangle_count = static_cast<int>(surface_tool->get_vertex_array().size());
        surface_tool->generate_normals();
        surface_tool->generate_tangents();

        auto mesharray = surface_tool->commit();
        auto array_index_length = mesharray->surface_get_array_index_len(0);
        mesharray->set_meta(SNAME("_skip_save_"), true);
        set_mesh(mesharray);
        Ref<ConcavePolygonShape3D> shape;
        shape = get_mesh()->create_trimesh_shape();
        shape->set_meta(SNAME("_skip_save_"), true);
        collision_shape->set_shape(shape);
        set_name(vformat("Chunk: %s tris: %s",chunk_number,array_index_length));
        static_body->set_name(vformat("StaticBody3d %s",get_name()));
        if (static_body->get_owner() == nullptr) {
            static_body->set_owner(get_owner());
        }
        if (collision_shape->get_owner() == nullptr) {
            collision_shape->set_owner(get_owner());
        }
        //print_line("Generated ",get_name());
    } else {
        print_line("Chunk number ",chunk_number," didn't have any triangles to generate " , get_instance_id());
    }

    emit_signal(completed_generation,chunk_number);

    // Stop the clock
    auto stop = std::chrono::high_resolution_clock::now();
    // Calculate the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    // Print the duration
    //print_line("Time taken by generation: ", duration.count(),"ms");
    has_first_generated = true;
    if(generate_grass && grass_multimesh != nullptr) {
        grass_multimesh->get_multimesh()->set_visible_instance_count(grass_count);
        if (grass_count == 0) {
            grass_multimesh->set_visible(false);
        } else {
            grass_multimesh->set_visible(true);
            //print_line("chunk ",chunk_number," grass count ",grass_count," visible count ",grass_multimesh->get_multimesh()->get_visible_instance_count());
        }
    }

    if (Engine::get_singleton()->is_editor_hint()) {
        notify_property_list_changed();
    }
}

void YVoxelChunk::populate_terrain(float height = 8) {
    const auto yvm = YarnVoxel::get_singleton();
    for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; x++)
        for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; z++)
            for (int y = YARNVOXEL_CHUNK_HEIGHT-1; y >= 0; y--) {
                // Get a terrain height using regular old Perlin noise.
                const auto thisHeight = yvm->perlin_noise(static_cast<float>(x) / 16 * 1.5 + 0.001, static_cast<float>(z) / 16 * 1.5f + 0.001f);

                // Set the value of this point in the terrainMap.
                points[x][y][z] = YarnVoxelData::YVPointValue(1,floatToInt16(static_cast<float>(y) - thisHeight));
            }
    yvm->set_dirty_chunk(chunk_number);
}

void YVoxelChunk::test_serialization() {
    if(!has_registered_chunk_number) {
        set_chunk_number(chunk_number);
    }
    serialize_to_data();
    deserialize_from_data();
    YarnVoxel::get_singleton()->set_dirty_chunk(chunk_number);
}

void YVoxelChunk::populate_chunk_3d() {
    if(!has_registered_chunk_number) {
        set_chunk_number(chunk_number);
    }
    //TIMING!
    const auto start = std::chrono::high_resolution_clock::now();
    const auto yvm = YarnVoxel::get_singleton();
    for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; x++)
        for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; z++)
            for (int y = YARNVOXEL_CHUNK_HEIGHT-1; y >= 0; y--) {
                // Get a terrain height using regular old Perlin noise.
                const auto thisHeight = yvm->perlin_noise_3d(static_cast<float>(x) / 16 * 1.5 + 0.001, static_cast<float>(y) / 16 * 1.5f + 0.001f, static_cast<float>(z) / 16 * 1.5f + 0.001f);

                // Set the value of this point in the terrainMap.
                points[x][y][z] = YarnVoxelData::YVPointValue(1,floatToInt16(thisHeight));
            }

    constexpr float underSlopeForStone = -60, aboveSlopeForStone = 60;
    constexpr float underSlopeForGrass = 30, aboveSlopeForGrass = -30;
    for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; x++)
        for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; z++)
            for (int y = YARNVOXEL_CHUNK_HEIGHT-1; y >= 0; y--) {
                const auto desiredCubeConfig = FindCubeConfiguration(x,y,z);
                if (desiredCubeConfig == 0 || desiredCubeConfig == 255) continue;
                const Vector3i pointsPosition = {x,y,z};
                const auto slopeAverage = static_cast<float>(YarnVoxelData::AverageSlopeTable[desiredCubeConfig]);
                if (slopeAverage <= -990.0) continue;
                if ((slopeAverage < underSlopeForStone || slopeAverage > aboveSlopeForStone)) {
                    SetPointSurrounding(pointsPosition, 3);
                } else if (slopeAverage > aboveSlopeForGrass && slopeAverage < underSlopeForGrass) {
                    SetPointSurrounding(pointsPosition, 1);
                }
            }

    // Stop the clock
    const auto stop = std::chrono::high_resolution_clock::now();
    // Calculate the duration in microseconds
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    // Print the duration
    print_line("Time taken by populating 3d terrain: ", duration.count(),"ms");
    yvm->set_dirty_chunk(chunk_number);
}

bool YVoxelChunk::SetPointFromSurrounding(const Vector3i pointNumber, const uint8_t desiredByte, const Vector3i originatorPointNumber) {
    if(is_point_position_in_range_without_neighbours(pointNumber.x,pointNumber.y,pointNumber.z)) {
        const auto vpv = &points[pointNumber.x][pointNumber.y][pointNumber.z];
        vpv->byteValue = desiredByte;
        return true;
    } else {
        const auto desired_pos_other = get_world_pos_from_point_number(pointNumber);
        auto other_chunk_number = YarnVoxel::GetChunkNumberFromPosition(desired_pos_other);
        YVoxelChunk* neighbour_chunk = nullptr;
        if(YarnVoxel::try_get_chunk(other_chunk_number,neighbour_chunk)) {
            const auto other_point_pos = YarnVoxel::GetPointNumberFromPosition(desired_pos_other);
            neighbour_chunk->SetPointFromSurrounding(other_point_pos,desiredByte, originatorPointNumber);
            return true;
        }
    }
    return false;
}
bool YVoxelChunk::SetPoint(const Vector3i pointNumber, const uint8_t desiredByte) {
    if(is_point_position_in_range_without_neighbours(pointNumber.x,pointNumber.y,pointNumber.z)) {
        const auto vpv = &points[pointNumber.x][pointNumber.y][pointNumber.z];
        vpv->byteValue = desiredByte;
        return true;
    } else {
        const auto desired_pos_other = get_world_pos_from_point_number(pointNumber);
        auto other_chunk_number = YarnVoxel::GetChunkNumberFromPosition(desired_pos_other);
        YVoxelChunk* neighbour_chunk = nullptr;
        if(YarnVoxel::try_get_chunk(other_chunk_number,neighbour_chunk)) {
            const auto other_point_pos = YarnVoxel::GetPointNumberFromPosition(desired_pos_other);
            neighbour_chunk->SetPoint(other_point_pos,desiredByte);
            return true;
        }
    }
    return false;
}

Vector3i YVoxelChunk::AddX(Vector3i pointnumber, int add) {
    return {pointnumber.x + add,pointnumber.y,pointnumber.z};
}

Vector3i YVoxelChunk::AddZ(Vector3i pointnumber, int add) {
    return {pointnumber.x,pointnumber.y,pointnumber.z + add};
}

void YVoxelChunk::SetPointSurrounding(Vector3i pointNumber, uint8_t desiredByte) {
    YVoxelChunk::SetPoint(pointNumber, desiredByte);
    YVoxelChunk::SetPoint(AddX(pointNumber,1), desiredByte);
    YVoxelChunk::SetPoint(AddX(pointNumber,-1), desiredByte);
    YVoxelChunk::SetPoint(AddZ(pointNumber,-1), desiredByte);
    YVoxelChunk::SetPoint(AddZ(pointNumber,1), desiredByte);
    YVoxelChunk::SetPoint(AddX(AddZ(pointNumber,1),1), desiredByte);
    YVoxelChunk::SetPoint(AddX(AddZ(pointNumber,1),-1), desiredByte);
    YVoxelChunk::SetPoint(AddX(AddZ(pointNumber,-1),-1), desiredByte);
    YVoxelChunk::SetPoint(AddX(AddZ(pointNumber,-1),1), desiredByte);
}

void YVoxelChunk::clear_triangles() {
    mesh_triangles.clear();
    output_pos_to_index.clear();
}

void YVoxelChunk::MarchCube (Vector3i position, int configIndex, uint8_t desiredByte, uint8_t health, uint8_t debugging_config) {
    // If the configuration of this cube is 0 or 255 (completely inside the terrain or completely outside of it) we don't need to do anything.
    if (configIndex == 0 || configIndex == 255 || (debugging_config != 0 && debugging_config != configIndex)) return;
    Vector3 triangleVert1 = YARNVOXEL_VECTOR3_ZERO, triangleVert2 = YARNVOXEL_VECTOR3_ZERO;
    const Vector3 posBlockBottomCorner = position;
    int currentTriangleCount = 0;
    int trianglesCreated = 0;
    bool already_has_prop = false;
    // Loop through the triangles. There are never more than 5 triangles to a cube and only three vertices to a triangle.
    int edgeIndex = 0;
    for(int i = 0; i < 5; i++) {
        for(int p = 0; p < 3; p++) {
            // Get the current indice. We increment triangleIndex through each loop.
            const int indice = YarnVoxelData::TriangleTable[configIndex][edgeIndex];
            // If the current edgeIndex is -1, there are no more indices and we can exit the function.
            if (indice == -1) return;
            // Get the vertices for the start and end of this edge.
            Vector3 vert1 = posBlockBottomCorner + YarnVoxelData::CornerTable[YarnVoxelData::EdgeIndexes[indice][0]];
            Vector3 vert2 = posBlockBottomCorner + YarnVoxelData::CornerTable[YarnVoxelData::EdgeIndexes[indice][1]];

            // Get the terrain values at either end of our current edge from the cube array created above.
            const float vert1Sample = int16ToFloat(cube[YarnVoxelData::EdgeIndexes[indice][0]]->floatValue);
            const float vert2Sample = int16ToFloat(cube[YarnVoxelData::EdgeIndexes[indice][1]]->floatValue);

            // Calculate the difference between the terrain values.
            float difference = vert2Sample - vert1Sample;

            // If the difference is 0, then the terrain passes through the middle.
            difference = difference == 0 ? YARNVOXEL_TERRAIN_SURFACE : (YARNVOXEL_TERRAIN_SURFACE - vert1Sample) / difference;

            // Calculate the point along the edge that passes through.
            const Vector3 vertPosition = vert1 + ((vert2 - vert1) * difference);
            //auto vertpositionstring = vformat("%s (%s + ((%s - %s) * %s);",vertPosition,vert1,vert2,vert1,difference);
           // print_line("position block marching ",position," trianglevertindex ",currentTriangleCount," vert position ",vertpositionstring);
           // print_line("vert1 ",vert1," vert 2 ",vert2," edge indexes ", YarnVoxelData::EdgeIndexes[indice], " corner table vert1 ",YarnVoxelData::CornerTable[YarnVoxelData::EdgeIndexes[indice][0]],
//                " corner table vert 2 ",YarnVoxelData::CornerTable[YarnVoxelData::EdgeIndexes[indice][1]]);
            // Add to our vertices and triangles list and incremement the edgeIndex.
            if (currentTriangleCount == 0) {
                triangleVert1 = vertPosition;
                currentTriangleCount = 1;
            }
            else {
                if (currentTriangleCount == 1) {
                    triangleVert2 = vertPosition;
                    currentTriangleCount = 2;
                } else {
                    //print_line("position ",position," config index ",configIndex," dbyte ",desiredByte," vert1 ",triangleVert1," vert2 ",triangleVert2," vert3 ",vertPosition);
                    YarnVoxelData::YVTriangleData newTriangleData = YarnVoxelData::YVTriangleData(triangleVert1, triangleVert2, vertPosition, desiredByte,health);
                    mesh_triangles.append(newTriangleData);
                    if (!has_first_generated && !already_has_prop) {
                        auto triangleSlope = YarnVoxelData::SlopeTriangleTable[configIndex][trianglesCreated];
                        if (triangleSlope < 48 && triangleSlope >= 0 && bottom_corner_world_pos.y + triangleVert1.y > water_level + 1.0f) {
                            auto propTriangle = YarnVoxelData::YVPropTriangleData(chunk_number, triangleVert1,triangleVert2,vertPosition, Vector2i(configIndex,trianglesCreated), desiredByte);
                            possible_prop_places.append(propTriangle);
                            if (generate_grass && triangleSlope < 42 && propTriangle.desiredByte == 1 && grass_multimesh != nullptr) {
                                auto worldcenter = propTriangle.center_pos();
                                auto offset_world_center = worldcenter;
                                offset_world_center.y -= 0.018f;
                                auto normal_vector = propTriangle.normal();
                                auto rright = normal_vector.cross({1,0,0});
                                auto fforward = rright.cross(normal_vector).rotated(normal_vector,Math::fmod(Math::abs(worldcenter.x+worldcenter.y+worldcenter.z),Math::deg_to_rad(360.0f)));
                                auto grasstransform = YarnVoxelData::CreateTransformFromPosScaleLook(offset_world_center,fforward,normal_vector,Vector3(0.8f,0.6f,0.8f));
                                grass_multimesh->get_multimesh()->set_instance_transform(grass_count,grasstransform);
                                //print_line("Setting grass transform ",grass_count," pos ",grasstransform.origin," from: ",chunk_number," pos ",offset_world_center);
                                grass_count = grass_count+1;
                                if (grass_count > multimesh_instance_count) {
                                    multimesh_instance_count = multimesh_instance_count + 500;
                                    grass_multimesh->get_multimesh()->set_instance_count(multimesh_instance_count);
                                }
                            }
                            already_has_prop = true;
                        }
                    }
                    currentTriangleCount = 0;
                    trianglesCreated++;
                }
            }
            edgeIndex++;
        }
    }
}

int YVoxelChunk::FindCubeConfiguration(int x, int y, int z) {
    int corner_index = 0;
    int configurationIndex = 0;
    for (const Vector3i corner : YarnVoxelData::CornerTable) {
        const int desired_x = x+corner.x;
        const int desired_y = y+corner.y;
        const int desired_z = z+corner.z;
        if(!is_point_position_in_range(desired_x,desired_y,desired_z))
            continue;
        const auto point_found = &points[desired_x][desired_y][desired_z];
        if (point_found->floatValue > YARNVOXEL_TERRAIN_SURFACE) {
            configurationIndex |= 1 << corner_index;
        }
        corner_index = corner_index+1;
    }
    return configurationIndex;
}

int YVoxelChunk::GetCubeConfiguration() {
    // Starting with a configuration of zero, loop through each point in the cube and check if it is below the terrain surface.
    int configurationIndex = 0;
    for (int i = 0; i < 8; i++) {
        if (cube[i]->floatValue > YARNVOXEL_TERRAIN_SURFACE) {
            configurationIndex |= 1 << i;
        }
    }
    return configurationIndex;
}

void YVoxelChunk::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
        } break;
        // case NOTIFICATION_ENTER_WORLD: {
        // } break;
        case NOTIFICATION_EXIT_TREE: {
            YarnVoxel::yvchunks.erase(chunk_number);
            break;
        }
		case NOTIFICATION_PARENTED: {
            if (Engine::get_singleton()->is_editor_hint()) {
                //print_line("Parented");
                if (get_owner() != nullptr) {
                    if (static_body != nullptr) {
                        static_body->set_owner(get_owner());
                    }
                    if (grass_multimesh != nullptr) {
                        grass_multimesh->set_owner(get_owner());
                    }
                    if (collision_shape != nullptr) {
                        collision_shape->set_owner(get_owner());
                    }
                }
            }
        }
        case NOTIFICATION_READY: {
            do_ready();
        } break;
        case NOTIFICATION_PROCESS: {
            do_process();
        } break;
        default:
            break;
    }
}


void YVoxelChunk::do_ready() {
    if (has_done_ready) {
        return;
    }
    if (Engine::get_singleton()->is_editor_hint()) {
        if (YarnVoxel::get_singleton()->get_main_node() == nullptr) {
            YarnVoxel::get_singleton()->set_main_node(Object::cast_to<Node3D>(get_parent()));
        }
    }
    if (get_child_count() > 0) {
        for (int i = 0; i < get_child_count(); ++i) {
            if (get_child(i)->is_class("StaticBody3D")) {
                static_body = Object::cast_to<StaticBody3D>(get_child(i));
            } else if (get_child(i)->is_class("CollisionShape3D")){
                collision_shape = Object::cast_to<CollisionShape3D>(get_child(i));
            } else if (get_child(i)->is_class("MultiMeshInstance3D")){
                grass_multimesh = Object::cast_to<MultiMeshInstance3D>(get_child(i));
            }
        }
    }
    if (static_body == nullptr) {
        static_body = memnew(StaticBody3D);
        static_body->set_meta(SNAME("_skip_save_"), true);
        add_child(static_body);
        if (Engine::get_singleton()->is_editor_hint()) {
            if (get_owner() != nullptr) {
                static_body->set_owner(get_owner());
            } else {
                static_body->set_owner(SceneTree::get_singleton()->get_current_scene());
            }
        }
        static_body->set_position(YARNVOXEL_VECTOR3_ZERO);
    } else if (static_body->get_child_count() > 0) {
        for (int i = 0; i < static_body->get_child_count(); ++i) {
            if (static_body->get_child(i)->is_class("CollisionShape3D")){
                collision_shape = Object::cast_to<CollisionShape3D>(static_body->get_child(i));
                collision_shape->set_meta(SNAME("_skip_save_"), true);
            }
        }
    }
    if (static_body != nullptr && collision_shape == nullptr) {
        collision_shape = memnew(CollisionShape3D);
        static_body->add_child(collision_shape);
        if (Engine::get_singleton()->is_editor_hint()) {
            collision_shape->set_owner(static_body->get_owner());
        }
        collision_shape->set_position(YARNVOXEL_VECTOR3_ZERO);
    }
    if (generate_grass && grass_multimesh == nullptr) {
        grass_multimesh = memnew(MultiMeshInstance3D);
        add_child(grass_multimesh);
        if (Engine::get_singleton()->is_editor_hint()) {
            grass_multimesh->set_owner(SceneTree::get_singleton()->get_current_scene());
        }
        grass_multimesh->set_name("GrassMultiMesh");
        grass_multimesh->set_meta(SNAME("_skip_save_"), true);
        Ref<MultiMesh> multimesh = memnew(MultiMesh);
        multimesh->set_meta(SNAME("_skip_save_"), true);
        multimesh->set_transform_format(MultiMesh::TRANSFORM_3D);
        multimesh->set_use_colors(false);
        multimesh->set_instance_count(multimesh_instance_count);
        multimesh->set_mesh(YarnVoxel::get_singleton()->grass_mesh);
        grass_multimesh->set_multimesh(multimesh);
        grass_multimesh->set_position(YARNVOXEL_VECTOR3_ZERO);
    }
   // print_line("Static body is nullptr?",static_body!=nullptr);
    if (static_body!= nullptr) {
   //     print_line("My owner ",get_owner()," static body owner ",static_body->get_owner());
    }
    has_done_ready=true;
}
void YVoxelChunk::do_process() {
    const auto yv = YarnVoxel::get_singleton();
    if(!has_done_ready) {
        do_ready();
    }
    if(yv->handle_dirty_chunks()) {
        set_process(false);
    }
}



bool YVoxelChunk::is_point_position_in_range_without_neighbours(int x, int y, int z) {
    return (x >= 0 && x < YARNVOXEL_CHUNK_WIDTH && y >= 0 && y < YARNVOXEL_CHUNK_HEIGHT && z >= 0 && z  < YARNVOXEL_CHUNK_WIDTH);
}
bool YVoxelChunk::is_point_position_in_range(int x, int y, int z) {
    return (x >= 0 && x < YARNVOXEL_CHUNK_WIDTH + 1 && y >= 0 && y < YARNVOXEL_CHUNK_HEIGHT + 1 && z >= 0 && z  < YARNVOXEL_CHUNK_WIDTH + 1);
}

void YVoxelChunk::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate"), &YVoxelChunk::generate);
    // ClassDB::bind_method(D_METHOD("add", "value"), &YVoxelChunk::add);
    // ClassDB::bind_method(D_METHOD("multiply", "value"), &YVoxelChunk::multiply);
    // ClassDB::bind_method(D_METHOD("reset"), &YVoxelChunk::reset);

	ADD_SIGNAL(MethodInfo("completed_generation", PropertyInfo(Variant::VECTOR3I, "chunk_number")));

    ClassDB::bind_method(D_METHOD("set_has_first_generated", "status"), &YVoxelChunk::set_has_first_generated);
    ClassDB::bind_method(D_METHOD("get_has_first_generated"), &YVoxelChunk::get_has_first_generated);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "status"), "set_has_first_generated", "get_has_first_generated");

    ClassDB::bind_method(D_METHOD("get_grass_count"), &YVoxelChunk::get_grass_count);
    ClassDB::bind_method(D_METHOD("get_chunk_number"), &YVoxelChunk::get_chunk_number);
    ClassDB::bind_method(D_METHOD("set_chunk_number","chunk_number"), &YVoxelChunk::set_chunk_number);
    ClassDB::bind_method(D_METHOD("get_bottom_corner_world_pos"), &YVoxelChunk::get_bottom_corner_world_pos);
    ClassDB::bind_method(D_METHOD("set_bottom_corner_world_pos","bottom_corner"), &YVoxelChunk::set_bottom_corner_world_pos);
    ClassDB::bind_method(D_METHOD("populate_terrain","height"), &YVoxelChunk::populate_terrain,DEFVAL(8));
    ClassDB::bind_method(D_METHOD("populate_chunk_3d"), &YVoxelChunk::populate_chunk_3d);

    ClassDB::bind_method(D_METHOD("get_data"), &YVoxelChunk::get_data);
    ClassDB::bind_method(D_METHOD("set_data","data"), &YVoxelChunk::set_data);
    ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "__data__", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_INTERNAL), "set_data", "get_data");

    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "chunk_number"), "set_chunk_number", "get_chunk_number");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "bottom_corner_world_pos"), "set_bottom_corner_world_pos", "get_bottom_corner_world_pos");
}

void YVoxelChunk::set_chunk_number(Vector3i v) {
        if (chunk_number != v || !has_registered_chunk_number) {
            if (YarnVoxel::get_singleton() != nullptr) {
                YVoxelChunk* find_chunk = nullptr;
                if(YarnVoxel::get_singleton()->try_get_chunk(chunk_number,find_chunk) && find_chunk == this) {
                    YarnVoxel::get_singleton()->yvchunks.erase(chunk_number);
                }
                YarnVoxel::get_singleton()->yvchunks[v] = this;
                has_registered_chunk_number=true;
                set_bottom_corner_world_pos(YarnVoxel::get_singleton()->GetBottomCornerForChunkInNumber(chunk_number));
                set_global_position(bottom_corner_world_pos);
            }
            chunk_number = v;
        }
}

Vector3 YVoxelChunk::get_world_pos_from_point_number(Vector3i pointNumber) const {
    return {bottom_corner_world_pos.x + pointNumber.x, bottom_corner_world_pos.y + pointNumber.y, bottom_corner_world_pos.z + pointNumber.z};
}

void YVoxelChunk::initialize(Vector3i initialize_position) {
    chunk_number = initialize_position;
    bottom_corner_world_pos = YarnVoxel::get_singleton()->GetBottomCornerForChunkInNumber(initialize_position);
    set_global_position(bottom_corner_world_pos);
}
YVoxelChunk::YVoxelChunk() {
    completed_generation = StaticCString::create("completed_generation");
}

YVoxelChunk::~YVoxelChunk() {
    if (collision_shape != nullptr) {
        memfree(collision_shape);
        collision_shape=  nullptr;
    }
    if (static_body != nullptr) {
        memfree(static_body);
        static_body=  nullptr;
    }
    if (grass_multimesh != nullptr) {
        memfree(grass_multimesh);
        grass_multimesh = nullptr;
    }
}
