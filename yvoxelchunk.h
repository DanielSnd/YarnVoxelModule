//
// Created by daniel on 12/25/23.
//

#ifndef YVOXELCHUNK_H
#define YVOXELCHUNK_H


#include "constants.h"
#include "scene/resources/surface_tool.h"
#include "core/object/ref_counted.h"
#include "scene/resources/material.h"
#include "scene/3d/node_3d.h"
#include "core/math/color.h"
#include "scene/main/node.h"
// #include "scene/3d/visual_instance_3d.h"
#include "yarnvoxel.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/visual_instance_3d.h"
#include "servers/rendering_server.h"
#include "core/variant/variant.h"
#include "core/math/vector2i.h"
#include "core/math/vector3i.h"
#include "core/variant/variant_utility.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/resources/3d/concave_polygon_shape_3d.h"
#include "thirdparty/meshoptimizer/meshoptimizer.h"
#include "core/io/marshalls.h"
#include "core/math/geometry_2d.h"
#include "core/math/triangulate.h"
#include "scene/3d/importer_mesh_instance_3d.h"
#include "scene/resources/3d/importer_mesh.h"
// #include "scene/3d/physics_body_3d.h"
// #include "scene/main/node.h"

class MeshInstance3D;
class YarnVoxel;

class YVoxelChunk : public MeshInstance3D {
    GDCLASS(YVoxelChunk, MeshInstance3D);

    // CollisionShape3D *collision_shape = nullptr;
    // StaticBody3D *static_body = nullptr;
    // MultiMeshInstance3D *grass_multimesh = nullptr;

protected:
    void _notification(int p_what);
    static void _bind_methods();
    uint32_t collision_layer = 1;
    uint32_t collision_mask = 1;
    real_t collision_priority = 1.0;
    RID root_collision_instance;
    RID root_collision_shape;

public:
    bool has_done_ready = false;
    bool has_registered_chunk_number = false;
    float simplification_distance = 0.1f;
	StringName completed_generation;
    Vector<YarnVoxelData::YVTriangleData> mesh_triangles = {};
    Vector<YarnVoxelData::YVPropTriangleData> possible_prop_places = {};
    HashMap<Vector3,int> output_pos_to_index = {};

    Vector<Vector3> vertices;
    Vector<Vector3> normals;
    Vector<Vector2> uvs;
    Vector<int> indices;
    Vector<Color> colors;
    Vector<Vector3> faces;

    Vector3i chunk_number;
    Vector3i get_chunk_number() {return chunk_number;}
    void deferred_set_dirty();
    void set_chunk_number(Vector3i v);

    void do_ready();
    void do_process();
    void do_physics_process();

    int get_prop_places_count() { return possible_prop_places.size(); }
    int get_prop_place_byte(int index) {
        if (index < 0 || index >= possible_prop_places.size()) {    
            return 0;
        }
        return possible_prop_places[index].desiredByte;
    }
    Vector3 get_prop_place_position(int index) {   
        if (index < 0 || index >= possible_prop_places.size()) {
            return Vector3();
        }
        return possible_prop_places[index].world_center_pos();
    }
    Vector3 get_prop_place_normal(int index) {   
        if (index < 0 || index >= possible_prop_places.size()) {
            return Vector3();
        }
        return possible_prop_places[index].normal();
    }
    float get_prop_place_slope(int index) {   
        if (index < 0 || index >= possible_prop_places.size()) {
            return 0.0f;
        }
        return possible_prop_places[index].average_slope();
    }
    float get_prop_place_dot_product(int index) {   
        if (index < 0 || index >= possible_prop_places.size()) {
            return 0.0f;
        }
        return possible_prop_places[index].dot_product();
    }
    


    void set_collision_layer(uint32_t p_layer);

    uint32_t get_collision_layer() const;

    void set_collision_mask(uint32_t p_mask);

    uint32_t get_collision_mask() const;

    void set_collision_layer_value(int p_layer_number, bool p_value);

    bool compare_float_values_sameish(int16_t f1, int16_t f2);

    void do_work(uint8_t hint_is_count, uint16_t count, uint8_t value, float floatValue);

    void serialize_to_data();

    void handle_serialization_count(uint8_t hint_is_count, uint16_t count, uint8_t value, int16_t floatValue);

    void deserialize_from_data();

    void clear_all_points();

    Vector3 bottom_corner_world_pos;
    Vector3 get_bottom_corner_world_pos() {return bottom_corner_world_pos;}
    void set_bottom_corner_world_pos(Vector3 v) {
        bottom_corner_world_pos = v;
    }

    YarnVoxelData::YVPointValue points[YARNVOXEL_CHUNK_WIDTH + 1][YARNVOXEL_CHUNK_HEIGHT+1][YARNVOXEL_CHUNK_WIDTH+1];

    bool has_neighbour_chunks[2][2][2] = {{{false}}};
    YVoxelChunk* neighbour_chunks[2][2][2] = {{{nullptr}}};

    Vector<uint8_t> data;
    void set_data (const Vector<uint8_t> &val) {
        data = val;
        deserialize_from_data();
    }
    Vector<uint8_t> get_data () {
        //print_line("Get data is being called");
        //serialize_to_data();
        return data;
    }
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

    void test_serialization();

    void populate_chunk_3d();

    bool SetPointFromSurrounding(const Vector3i pointNumber, const uint8_t desiredByte, const Vector3i originatorPointNumber);

    bool SetPointDensity(Vector3i pointNumber, float desired_density, uint8_t desiredByte);

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

    bool get_collision_layer_value(int p_layer_number) const;

    void set_collision_mask_value(int p_layer_number, bool p_value);

    bool get_collision_mask_value(int p_layer_number) const;

    void set_collision_priority(real_t p_priority);

    real_t get_collision_priority() const;

    void set_simplification_distance(float p_distance) {simplification_distance = p_distance;}
    float get_simplification_distance() const {return simplification_distance;}

    bool is_point_position_in_range_without_neighbours(int x, int y, int z);

    void optimize_faces(float p_simplification_dist = 0.1f);

    void MarchCube(Vector3i position, int configIndex, uint8_t desiredByte, uint8_t health,uint8_t debugging_config);

    int FindCubeConfiguration(int x, int y, int z);

    float get_density_at_point(Vector3i point_pos);

    YVoxelChunk();
    ~YVoxelChunk();
};

#endif //YVOXELCHUNK_H
