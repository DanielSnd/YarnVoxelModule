//
// Created by daniel on 12/25/23.
//
#include <iostream>
#include <chrono>
#include "yvoxelchunk.h"
#include "yarnvoxel.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_interface.h"
#endif

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
    pointHit = get_world_pos_from_point_number(pointHit);
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
    //print_line("Chunk number ",chunk_number," data size ",data.size());
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
            if (++z >= YARNVOXEL_CHUNK_WIDTH) { z = 0;
                if (++x >= YARNVOXEL_CHUNK_WIDTH) { x = 0;
                    if (--y < 0) return;
                }
            }
        }
    }
}

void YVoxelChunk::clear_all_points() {
    for (int i = 0; i <= YARNVOXEL_CHUNK_WIDTH; ++i) {
        for (int j = 0; j <= YARNVOXEL_CHUNK_HEIGHT; ++j) {
            for (int k = 0; k <= YARNVOXEL_CHUNK_WIDTH; ++k) {
                points[i][j][k] = YarnVoxelData::YVPointValue(); // Default constructor
            }
        }
    }
    data.clear();
}

void YVoxelChunk::generate() {
    //print_line("Calling generate on ",chunk_number, " Editor hint: ",Engine::get_singleton()->is_editor_hint()," Is inside Tree: ", !is_inside_tree());
#ifdef TOOLS_ENABLED
    if(Engine::get_singleton()->is_editor_hint() && !is_inside_tree()) {
        if (!SceneTree::get_singleton()->is_connected("process_frame",callable_mp(this,&YVoxelChunk::generate)))
            SceneTree::get_singleton()->connect(SNAME("process_frame"),callable_mp(this,&YVoxelChunk::generate),CONNECT_ONE_SHOT);
        return;
    }
#endif
    if(!root_collision_instance.is_valid()) {
        if(!root_collision_shape.is_valid() || root_collision_shape.is_null()) {
            root_collision_shape.instantiate();
        }
        root_collision_shape->set_meta(SNAME("_skip_save_"), true);
        root_collision_instance = PhysicsServer3D::get_singleton()->body_create();
        PhysicsServer3D::get_singleton()->body_set_mode(root_collision_instance, PhysicsServer3D::BODY_MODE_STATIC);
        PhysicsServer3D::get_singleton()->body_set_state(root_collision_instance, PhysicsServer3D::BODY_STATE_TRANSFORM, get_global_transform());
        PhysicsServer3D::get_singleton()->body_add_shape(root_collision_instance, root_collision_shape->get_rid());
        PhysicsServer3D::get_singleton()->body_set_space(root_collision_instance, get_world_3d()->get_space());
        PhysicsServer3D::get_singleton()->body_attach_object_instance_id(root_collision_instance, get_instance_id());
        set_collision_layer(collision_layer);
        set_collision_mask(collision_mask);
        set_collision_priority(collision_priority);
    }
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

    Ref<Material> use_material = YarnVoxel::get_singleton()->get_material();
    if (use_material.is_valid()) {
        if (get_surface_override_material_count() == 0) {
            surface_override_materials.append(use_material);
        } else {
            set_surface_override_material(0,use_material);
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
        root_collision_shape->set_faces(get_mesh()->create_trimesh_shape()->get_faces());
        root_collision_shape->set_meta(SNAME("_skip_save_"), true);
        set_name(vformat("Chunk: %s tris: %s",chunk_number,array_index_length));
        //print_line("Generated ",get_name());
    } else {
        //print_line("Chunk number ",chunk_number," didn't have any triangles to generate " , get_instance_id());
    }

    emit_signal(completed_generation,chunk_number);

    // Stop the clock
    auto stop = std::chrono::high_resolution_clock::now();
    // Calculate the duration in microseconds
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    // Print the duration
    //print_line(get_name()," Time taken by generation: ", duration.count(),"ms");
    has_first_generated = true;
    //TODO: GRASS
    // if(generate_grass && grass_multimesh != nullptr) {
    //     grass_multimesh->get_multimesh()->set_visible_instance_count(grass_count);
    //     if (grass_count == 0) {
    //         grass_multimesh->set_visible(false);
    //     } else {
    //         grass_multimesh->set_visible(true);
    //         //print_line("chunk ",chunk_number," grass count ",grass_count," visible count ",grass_multimesh->get_multimesh()->get_visible_instance_count());
    //     }
    // }
#ifdef TOOLS_ENABLED
    if (Engine::get_singleton()->is_editor_hint()) {
        notify_property_list_changed();
    }
#endif

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
    //serialize_to_data();
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
    //print_line("Time taken by populating 3d terrain: ", duration.count(),"ms");
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

bool YVoxelChunk::SetPointDensity(const Vector3i pointNumber,const float desired_density, const uint8_t desiredByte) {
    auto my_desired_density = floatToInt16(desired_density);
    if(is_point_position_in_range_without_neighbours(pointNumber.x,pointNumber.y,pointNumber.z)) {
        const auto vpv = &points[pointNumber.x][pointNumber.y][pointNumber.z];
        vpv->byteValue = desiredByte;
        vpv->floatValue = my_desired_density;
        return true;
    } else {
        const auto desired_pos_other = get_world_pos_from_point_number(pointNumber);
        auto other_chunk_number = YarnVoxel::GetChunkNumberFromPosition(desired_pos_other);
        YVoxelChunk* neighbour_chunk = nullptr;
        if(YarnVoxel::try_get_chunk(other_chunk_number,neighbour_chunk)) {
            const auto other_point_pos = YarnVoxel::GetPointNumberFromPosition(desired_pos_other);
            neighbour_chunk->SetPointDensity(other_point_pos,desired_density,desiredByte);
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
    bool is_triple_polycount = YarnVoxel::get_singleton()->is_triple_polycount;
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
                    if (!has_first_generated && !already_has_prop) {
                        auto triangleSlope = YarnVoxelData::SlopeTriangleTable[configIndex][trianglesCreated];
                        if (triangleSlope < 48 && triangleSlope >= 0 && bottom_corner_world_pos.y + triangleVert1.y > water_level + 1.0f) {
                            auto propTriangle = YarnVoxelData::YVPropTriangleData(chunk_number, triangleVert1,triangleVert2,vertPosition, Vector2i(configIndex,trianglesCreated), desiredByte);
                            possible_prop_places.append(propTriangle);
                            //TODO: GRASS
                            // if (generate_grass && triangleSlope < 42 && propTriangle.desiredByte == 1 && grass_multimesh != nullptr) {
                            //     auto worldcenter = propTriangle.center_pos();
                            //     auto offset_world_center = worldcenter;
                            //     offset_world_center.y -= 0.018f;
                            //     auto normal_vector = propTriangle.normal();
                            //     auto rright = normal_vector.cross({1,0,0});
                            //     auto fforward = rright.cross(normal_vector).rotated(normal_vector,Math::fmod(Math::abs(worldcenter.x+worldcenter.y+worldcenter.z),Math::deg_to_rad(360.0f)));
                            //     auto grasstransform = YarnVoxelData::CreateTransformFromPosScaleLook(offset_world_center,fforward,normal_vector,Vector3(0.8f,0.6f,0.8f));
                            //     grass_multimesh->get_multimesh()->set_instance_transform(grass_count,grasstransform);
                            //     //print_line("Setting grass transform ",grass_count," pos ",grasstransform.origin," from: ",chunk_number," pos ",offset_world_center);
                            //     grass_count = grass_count+1;
                            //     if (grass_count > multimesh_instance_count) {
                            //         multimesh_instance_count = multimesh_instance_count + 500;
                            //         grass_multimesh->get_multimesh()->set_instance_count(multimesh_instance_count);
                            //     }
                            // }
                            already_has_prop = true;
                        }
                    }

                    if (is_triple_polycount) {
                        auto triangle_area = YarnVoxelData::TriangleAreaTable[configIndex][trianglesCreated];
                        if (triangle_area>30) {
                            Vector3 middle_pos = triangleVert1.lerp(triangleVert2,0.5).lerp(vertPosition,0.5);
                            YarnVoxelData::YVTriangleData newTriangleData1 = YarnVoxelData::YVTriangleData(triangleVert1, triangleVert2, middle_pos, desiredByte,health);
                            YarnVoxelData::YVTriangleData newTriangleData2 = YarnVoxelData::YVTriangleData(middle_pos, triangleVert2, vertPosition, desiredByte,health);
                            YarnVoxelData::YVTriangleData newTriangleData3 = YarnVoxelData::YVTriangleData(vertPosition,triangleVert1, middle_pos, desiredByte,health);
                            mesh_triangles.append(newTriangleData1);
                            mesh_triangles.append(newTriangleData2);
                            mesh_triangles.append(newTriangleData3);
                        } else {
                            mesh_triangles.append(newTriangleData);
                        }
                    } else {
                        mesh_triangles.append(newTriangleData);
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
        }
		case NOTIFICATION_PROCESS: {
            if(has_done_ready && is_inside_tree()) {
                do_process();
            }
		}
        break;
        case NOTIFICATION_PARENTED: {
        }
        break;
        case NOTIFICATION_READY: {
            if(is_inside_tree()) {
                do_ready();
            } else {
                SceneTree::get_singleton()->connect(SNAME("process_frame"),callable_mp(this,&YVoxelChunk::do_ready),CONNECT_ONE_SHOT);
            }
        } break;
        case NOTIFICATION_TRANSFORM_CHANGED: {
            if (root_collision_instance.is_valid() && is_inside_tree()) {
                PhysicsServer3D::get_singleton()->body_set_state(root_collision_instance, PhysicsServer3D::BODY_STATE_TRANSFORM, get_global_transform());
            }
            //_on_transform_changed();
        } break;
        default:
            break;
    }
}

void YVoxelChunk::deferred_set_dirty() {
    YarnVoxel::get_singleton()->set_dirty_chunk(chunk_number);
}

void YVoxelChunk::do_ready() {
    //print_line(vformat("do_ready ychunk %s , has done ready? %s has registered chunk number? %s, what's the chunk number? %s",get_name(),has_done_ready,has_registered_chunk_number,chunk_number));
    if (has_done_ready) {
        return;
    }
    if(!data.is_empty() && data.size() > 10) {
        YarnVoxel::get_singleton()->set_fallback_main_node(Object::cast_to<Node3D>(get_parent()));
        //print_line("Doing ready ",chunk_number," yvchunks registered ",YarnVoxel::yvchunks.size());
        if(!has_registered_chunk_number){
            YarnVoxel::yvchunks[chunk_number] = this;
            //print_line("Doing Do_REady Chunk number ",chunk_number," count of yvchunks registered ",YarnVoxel::yvchunks.size());
        }
        YarnVoxel::get_singleton()->set_dirty_chunk(chunk_number);
    }
    // if (generate_grass && grass_multimesh == nullptr) {
    //     grass_multimesh = memnew(MultiMeshInstance3D);
    //     add_child(grass_multimesh);
    //     if (Engine::get_singleton()->is_editor_hint()) {
    //         grass_multimesh->set_owner(SceneTree::get_singleton()->get_current_scene());
    //     }
    //     grass_multimesh->set_name("GrassMultiMesh");
    //     grass_multimesh->set_meta(SNAME("_skip_save_"), true);
    //     Ref<MultiMesh> multimesh = memnew(MultiMesh);
    //     multimesh->set_meta(SNAME("_skip_save_"), true);
    //     multimesh->set_transform_format(MultiMesh::TRANSFORM_3D);
    //     multimesh->set_use_colors(false);
    //     multimesh->set_instance_count(multimesh_instance_count);
    //     multimesh->set_mesh(YarnVoxel::get_singleton()->grass_mesh);
    //     grass_multimesh->set_multimesh(multimesh);
    //     grass_multimesh->set_position(YARNVOXEL_VECTOR3_ZERO);
    // }
    has_done_ready=true;
}

void YVoxelChunk::do_process() {
    const auto yv = YarnVoxel::get_singleton();
    // if(!has_done_ready) {
    //     do_ready();
    // }
    if(has_done_ready && yv->handle_dirty_chunks()) {
        set_process(false);
    }
}


void YVoxelChunk::set_collision_layer(uint32_t p_layer) {
    collision_layer = p_layer;
    if (root_collision_instance.is_valid()) {
        PhysicsServer3D::get_singleton()->body_set_collision_layer(root_collision_instance, p_layer);
    }
}

uint32_t YVoxelChunk::get_collision_layer() const {
    return collision_layer;
}

void YVoxelChunk::set_collision_mask(uint32_t p_mask) {
    collision_mask = p_mask;
    if (root_collision_instance.is_valid()) {
        PhysicsServer3D::get_singleton()->body_set_collision_mask(root_collision_instance, p_mask);
    }
}

uint32_t YVoxelChunk::get_collision_mask() const {
    return collision_mask;
}

void YVoxelChunk::set_collision_layer_value(int p_layer_number, bool p_value) {
    ERR_FAIL_COND_MSG(p_layer_number < 1, "Collision layer number must be between 1 and 32 inclusive.");
    ERR_FAIL_COND_MSG(p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
    uint32_t layer = get_collision_layer();
    if (p_value) {
        layer |= 1 << (p_layer_number - 1);
    } else {
        layer &= ~(1 << (p_layer_number - 1));
    }
    set_collision_layer(layer);
}

bool YVoxelChunk::get_collision_layer_value(int p_layer_number) const {
    ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Collision layer number must be between 1 and 32 inclusive.");
    ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
    return get_collision_layer() & (1 << (p_layer_number - 1));
}

void YVoxelChunk::set_collision_mask_value(int p_layer_number, bool p_value) {
    ERR_FAIL_COND_MSG(p_layer_number < 1, "Collision layer number must be between 1 and 32 inclusive.");
    ERR_FAIL_COND_MSG(p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
    uint32_t mask = get_collision_mask();
    if (p_value) {
        mask |= 1 << (p_layer_number - 1);
    } else {
        mask &= ~(1 << (p_layer_number - 1));
    }
    set_collision_mask(mask);
}

bool YVoxelChunk::get_collision_mask_value(int p_layer_number) const {
    ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Collision layer number must be between 1 and 32 inclusive.");
    ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
    return get_collision_mask() & (1 << (p_layer_number - 1));
}

void YVoxelChunk::set_collision_priority(real_t p_priority) {
    collision_priority = p_priority;
    if (root_collision_instance.is_valid()) {
        PhysicsServer3D::get_singleton()->body_set_collision_priority(root_collision_instance, p_priority);
    }
}

real_t YVoxelChunk::get_collision_priority() const {
    return collision_priority;
}

bool YVoxelChunk::is_point_position_in_range_without_neighbours(int x, int y, int z) {
    return (x >= 0 && x < YARNVOXEL_CHUNK_WIDTH && y >= 0 && y < YARNVOXEL_CHUNK_HEIGHT && z >= 0 && z  < YARNVOXEL_CHUNK_WIDTH);
}
bool YVoxelChunk::is_point_position_in_range(int x, int y, int z) {
    return (x >= 0 && x < YARNVOXEL_CHUNK_WIDTH + 1 && y >= 0 && y < YARNVOXEL_CHUNK_HEIGHT + 1 && z >= 0 && z  < YARNVOXEL_CHUNK_WIDTH + 1);
}

void YVoxelChunk::_bind_methods() {
    ClassDB::bind_method(D_METHOD("generate"), &YVoxelChunk::generate);
    ClassDB::bind_method(D_METHOD("deferred_set_dirty"), &YVoxelChunk::deferred_set_dirty);
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


    //
    // ClassDB::bind_method(D_METHOD("set_use_collision", "operation"), &YVoxelChunk::set_use_collision);
    // ClassDB::bind_method(D_METHOD("is_using_collision"), &YVoxelChunk::is_using_collision);

    ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &YVoxelChunk::set_collision_layer);
    ClassDB::bind_method(D_METHOD("get_collision_layer"), &YVoxelChunk::get_collision_layer);

    ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &YVoxelChunk::set_collision_mask);
    ClassDB::bind_method(D_METHOD("get_collision_mask"), &YVoxelChunk::get_collision_mask);

    ClassDB::bind_method(D_METHOD("set_collision_mask_value", "layer_number", "value"), &YVoxelChunk::set_collision_mask_value);
    ClassDB::bind_method(D_METHOD("get_collision_mask_value", "layer_number"), &YVoxelChunk::get_collision_mask_value);

    ClassDB::bind_method(D_METHOD("set_collision_layer_value", "layer_number", "value"), &YVoxelChunk::set_collision_layer_value);
    ClassDB::bind_method(D_METHOD("get_collision_layer_value", "layer_number"), &YVoxelChunk::get_collision_layer_value);

    ClassDB::bind_method(D_METHOD("set_collision_priority", "priority"), &YVoxelChunk::set_collision_priority);
    ClassDB::bind_method(D_METHOD("get_collision_priority"), &YVoxelChunk::get_collision_priority);

    ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "__data__", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_INTERNAL), "set_data", "get_data");

    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "chunk_number"), "set_chunk_number", "get_chunk_number");
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "bottom_corner_world_pos"), "set_bottom_corner_world_pos", "get_bottom_corner_world_pos");

    ADD_GROUP("Collision", "collision_");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_priority"), "set_collision_priority", "get_collision_priority");

}

void YVoxelChunk::set_chunk_number(Vector3i v) {
    //print_line(get_name(),"Set chunk number (current ",chunk_number," new ",v,") has registered? ",has_registered_chunk_number," has yarn voxel singleton? ",YarnVoxel::get_singleton() != nullptr);
        if (chunk_number != v || !has_registered_chunk_number) {
            YVoxelChunk* find_chunk = nullptr;
            if(YarnVoxel::try_get_chunk(chunk_number,find_chunk) && find_chunk == this) {
                YarnVoxel::yvchunks.erase(chunk_number);
            }
            YarnVoxel::yvchunks[v] = this;
            has_registered_chunk_number=true;
            if(is_inside_tree()) {
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
    for (int i = 0; i <= YARNVOXEL_CHUNK_WIDTH; ++i) {
        for (int j = 0; j <= YARNVOXEL_CHUNK_HEIGHT; ++j) {
            for (int k = 0; k <= YARNVOXEL_CHUNK_WIDTH; ++k) {
                points[i][j][k] = YarnVoxelData::YVPointValue(); // Assuming YVPointValue has a default constructor
            }
        }
    }
    chunk_number = Vector3i{-99999,-99999,-99999};
    completed_generation = StaticCString::create("completed_generation");
}

YVoxelChunk::~YVoxelChunk() {
    YarnVoxel::yvchunks.erase(chunk_number);
    if (root_collision_instance.is_valid()) {
        if (PhysicsServer3D::get_singleton() != nullptr)
            PhysicsServer3D::get_singleton()->free(root_collision_instance);
        root_collision_instance = RID();
    }
    if (!root_collision_shape.is_null() && root_collision_shape.is_valid()) {
        root_collision_shape.unref();
    }
}
