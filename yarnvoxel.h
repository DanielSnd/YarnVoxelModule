/* yarnvoxel.h */

#ifndef YARNVOXEL_H
#define YARNVOXEL_H

#include "yvoxelchunk.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/variant/variant.h"
#include "core/templates/vector.h"
#include "scene/3d/node_3d.h"
#include "scene/main/scene_tree.h"
#include "core/os/os.h"
#include "scene/resources/material.h"
#include "core/object/callable_method_pointer.h"
#define DB_PERLIN_IMPL
#include "db_perlin.hpp"

class YVoxelChunk;

class YarnVoxel : public Node3D {
	GDCLASS(YarnVoxel, Node3D);

	int count;

	Vector3i debug_pos;

	bool calculate_custom_normals;
	bool serialize_when_generating;

	float smooth_normal_angle;

	float line_noise_strength = 0.1f;
	float line_noise_frequency = 0.1f;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	uint8_t degugging_level = 0;

	uint64_t ticks_last_started_generating;
	uint64_t ticks_last_completed;
	Vector3i last_chunk_started_generating;
	Vector3i last_chunk_completed;

	HashMap<Vector3i,ObjectID> yvchunks;
	bool is_generating;
	bool using_default_shader = false;
	bool get_is_generating() const {return is_generating;}
	
	bool is_triple_polycount;
	void set_is_triple_polycount(bool b) {is_triple_polycount = b;}
	bool get_is_triple_polycount() const {return is_triple_polycount;}

	bool smoothing = false;

	void set_smoothing(bool b) {
		smoothing = b; }
	bool get_smoothing() {return smoothing;}
	
	bool generate_grass{};
	void set_generate_grass(bool status) {generate_grass = status;}
	bool get_generate_grass() const {return generate_grass;}
	Ref<Mesh> grass_mesh;
	void set_grass_mesh(const Ref<Mesh> &m) {grass_mesh = m;}
	Ref<Mesh> get_grass_mesh() const {return grass_mesh;}
	Ref<Material> grass_material;
	void set_grass_material(const Ref<Material> &m) {grass_material = m;}
	Ref<Material> get_grass_material() const {return grass_material;}
	void set_calculate_custom_normals(bool enabled) {calculate_custom_normals = enabled;}
	bool get_calculate_custom_normals() const {return calculate_custom_normals;}
	void set_smooth_normal_angle(float angle) {smooth_normal_angle = angle;}
	float get_smooth_normal_angle() const {return smooth_normal_angle;}
	void set_serialize_when_generating(bool enabled) {serialize_when_generating = enabled;}
	bool get_serialize_when_generating() const {return serialize_when_generating;}

	void set_line_noise_strength(float strength) { line_noise_strength = strength; }
	float get_line_noise_strength() const { return line_noise_strength; }
	void set_line_noise_frequency(float frequency) { line_noise_frequency = frequency; }
	float get_line_noise_frequency() const { return line_noise_frequency; }

	_FORCE_INLINE_ static Vector<ObjectID> yarnvoxel_instances;

	Vector3i right = Vector3i(1, 0, 0);
	Vector3i up = Vector3i(0, 1, 0);
	Vector3i forward = Vector3i(0, 0, 1);
	Vector3i rightforward = Vector3i(1, 0, 1);
	Vector3i rightup = Vector3i(1, 1, 0);
	Vector3i forwardup = Vector3i(0, 1, 1);
	float water_level;
	Ref<Material> material;
	String default_material_path;
	enum BlockType {
		NONE = 0,
		GRASS = 1,
		DIRT = 2,
		STONE = 3,
		SAND = 4,
		IRON = 5,
		COPPER = 6,
		BRONZE = 7,
		SILVER = 8,
		GOLD = 9,
		DIAMOND = 10
	};
	//UTILITIES:
	static int FastFloor(float f);

	Vector<Vector3i> DirtyChunksQueue;

	TypedArray<Vector3i> get_dirty_chunks_queued() const;

    	String dirty_chunk_queue_info();

	void set_material(const Ref<Material> &p_material);

	Ref<Material> get_material();

	void change_health_at_position(Vector3i position, uint8_t newHealth);

	void change_float_at_position(Vector3i position, float newFloat, uint8_t newBlockType, uint8_t health);

    void modify_line(Vector3i start, Vector3i end, float amount, uint8_t typeOfBlock = 1, int thicknessStart = 1, int thicknessEnd = 1, bool adding = false);

    void add_line(Vector3i start, Vector3i end, float amount, uint8_t typeOfBlock = 1, int thicknessStart = 1, int thicknessEnd = 1);

    void set_line(Vector3i start, Vector3i end, float value, uint8_t typeOfBlock = 1, int thicknessStart = 1, int thicknessEnd = 1);

	void modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type = -1);

	void smoothly_modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type = -1);

	Array find_closest_solid_point_to(Vector3 pos, int search_radius = 2);

	bool damage_voxel_area(Vector3i pos, uint8_t amount, int brushSize);

	void smooth_voxel_area(Vector3i pos, float amount, int brushSize);

	void smooth_voxel_chunk(YVoxelChunk* chunk);

	static bool IsPointNumberInBoundary(Vector3i bnumber);

	TypedArray<YVoxelChunk> get_chunks_from_gd_script() const;

	YVoxelChunk *get_chunk_from_gdscript(Vector3i chunk_number);

	void changeFloat(float newFloat, uint8_t newBlockType, uint8_t health);

	void chunk_generated(Vector3i chunk_completed);

	bool handle_dirty_chunks();

	void regenerate_all_chunks();
	void set_dirty_chunk(Vector3i chunkNumber);
	void set_dirty_chunk_with_pointer(YVoxelChunk* chunk, Vector3i chunkNumber);

	static float static_perlin_noise (float x,float y);
	static float static_perlin_noise_3d (float x,float y, float z);
	//OTHER STUFF:
	Vector2i CalculateGridPosition(Vector3 worldPosition);

	Vector3 CalculateChunkCenterPosition(Vector3i chunkPosition);

	Vector3 CalculateCellCenterPosition(Vector2i gridPosition);
	Vector3 GetBottomCornerForChunkInNumber(Vector3i ChunkNumber);
	Vector3i FindChunkNumberFromPosition(Vector3 pos);

	Vector3i FindPointNumberFromPosition(Vector3 pos);

	uint8_t FindHealthValueForPos(Vector3 pos);
	float FindFloatValueForPosFloat(Vector3 pos);
	int16_t FindFloatValueForPos(Vector3 pos);

	int FindBlockTypeForPos(Vector3 pos);

	void set_point_data_for_wpos(Vector3 pos, YarnVoxelData::YVPointValue pv);

	YarnVoxelData::YVPointValue get_point_data_for_wpos(Vector3 pos);

	bool try_get_chunk(Vector3i chunkPosition, YVoxelChunk*& chunk_pointer);

	YVoxelChunk* last_used_chunk;
	Vector3i last_used_chunk_number;
	bool has_last_used_chunk;

	YVoxelChunk *get_chunk(Vector3i chunkPosition);

	static constexpr int BIG_ENOUGH_INT = 16 * 1024;
	static constexpr double BIG_ENOUGH_FLOOR = BIG_ENOUGH_INT + 0.0000;

	void _on_tree_exiting();
	
	YarnVoxel();

	void empty_all_chunks();

	bool IsPositionValid(Vector3i pos);

	Vector3 find_air_position_with_clearance(Vector3 center_pos, int radius, float required_clearance);
};

VARIANT_ENUM_CAST(YarnVoxel::BlockType);
#endif // YARNVOXEL_H