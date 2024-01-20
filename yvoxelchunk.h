//
// Created by daniel on 12/25/23.
//

#ifndef YVOXELCHUNK_H
#define YVOXELCHUNK_H

#include "scene/resources/concave_polygon_shape_3d.h"
#include "scene/3d/physics_body_3d.h"
#include "scene/3d/collision_shape_3d.h"
#include "constants.h"
#include "scene/resources/surface_tool.h"
#include "core/object/ref_counted.h"
#include "scene/resources/material.h"
#include "scene/3d/node_3d.h"
#include "core/math/color.h"
#include "scene/main/node.h"
// #include "scene/3d/visual_instance_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/visual_instance_3d.h"
#include "servers/rendering_server.h"
#include "core/variant/variant.h"
#include "core/math/vector2i.h"
#include "core/math/vector3i.h"
#include "scene/3d/multimesh_instance_3d.h"
// #include "scene/3d/physics_body_3d.h"
// #include "scene/main/node.h"

class MeshInstance3D;

class YVoxelChunk : public MeshInstance3D {
    GDCLASS(YVoxelChunk, MeshInstance3D);

    CollisionShape3D *collision_shape = memnew(CollisionShape3D);
    StaticBody3D *static_body = memnew(StaticBody3D);
    MultiMeshInstance3D *grass_multimesh = nullptr;

protected:
    void _notification(int p_what);

    static void _bind_methods();

public:
	StringName completed_generation;
    Vector<YarnVoxelData::YVTriangleData> mesh_triangles = {};
    Vector<YarnVoxelData::YVPropTriangleData> possible_prop_places = {};
    HashMap<Vector3,int> output_pos_to_index = {};

    Vector3i chunk_number;
    Vector3i get_chunk_number() {return chunk_number;}
    void set_chunk_number(Vector3i v) {chunk_number = v;}

    Vector3 bottom_corner_world_pos;
    Vector3 get_bottom_corner_world_pos() {return bottom_corner_world_pos;}
    void set_bottom_corner_world_pos(Vector3 v) {bottom_corner_world_pos = v;}

    YarnVoxelData::YVPointValue points[YARNVOXEL_CHUNK_WIDTH + 1][YARNVOXEL_CHUNK_HEIGHT+1][YARNVOXEL_CHUNK_WIDTH+1] = {{{}}};

    bool has_neighbour_chunks[2][2][2] = {false};
    YVoxelChunk* neighbour_chunks[2][2][2] = {nullptr};

    Vector3i right = Vector3i(1, 0, 0);
    Vector3i up = Vector3i(0, 1, 0);
    Vector3i forward = Vector3i(0, 0, 1);
    Vector3i rightforward = Vector3i(1, 0, 1);
    Vector3i rightup = Vector3i(1, 1, 0);
    Vector3i forwardup = Vector3i(0, 1, 1);
    float water_level=0.0f;
    int multimesh_instance_count = 1000;
    bool generate_grass = false;
    int grass_count = 0;
    int get_grass_count() {return grass_count;}
    bool has_first_generated = false;
    void set_has_first_generated(bool b) {has_first_generated = b;}
    bool get_has_first_generated() const { return has_first_generated;}

    inline static YarnVoxelData::YVPointValue* cube[8] = {};
    static int GetCubeConfiguration();

    void ProcessMeshTriangles();

    void ClearMeshOutputData();

    void populate_terrain(float height);

    void populate_chunk_3d();

    bool SetPointFromSurrounding(const Vector3i pointNumber, const uint8_t desiredByte, const Vector3i originatorPointNumber);

    bool SetPoint(Vector3i pointNumber, uint8_t desiredByte);

    Vector3i AddX(Vector3i pointnumber, int add);

    Vector3i AddZ(Vector3i pointnumber, int add);

    void SetPointSurrounding(Vector3i pointNumber, uint8_t desiredByte);

    void clear_triangles();

    Vector3 get_world_pos_from_point_number(Vector3i pointNumber) const;


    void AttemptSetDirtyNeighbour(Vector3i pointHit) const;

    static void add_triangle(const YarnVoxelData::YVTriangleData &yv_triangle_data, SurfaceTool *surface_tool);

    static void add_triangle(Vector3 vert1, Vector3 vert2, Vector3 vert3, SurfaceTool *surface_tool);

    void initialize(Vector3i initialize_position);

    void generate();


    bool is_point_position_in_range(int x, int y, int z);
    bool is_point_position_in_range_without_neighbours(int x, int y, int z);

    void MarchCube(Vector3i position, int configIndex, uint8_t desiredByte, uint8_t health,uint8_t debugging_config);

    int FindCubeConfiguration(int x, int y, int z);

    YVoxelChunk();
    ~YVoxelChunk();
};

#endif //YVOXELCHUNK_H
