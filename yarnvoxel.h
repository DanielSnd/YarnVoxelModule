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

class YarnVoxel : public Object {
	GDCLASS(YarnVoxel, Object);

	Node3D* main_node_pointer;
	int count;
	uint8_t debugging_config = 0;
	int cellSize;
	float terrainSurface;
	int ChunkWidth;
	int ChunkHeight;
	Vector3i debug_pos;
	float DivideForChunkWidth;
	float DivideForChunkHeight;
	float BaseTerrainHeight; // Minimum height of terrain.
	float TerrainHeightRange; // The max height (above BaseTerrainHeight) our terrain can be.
	bool calculate_custom_normals;
	bool serialize_when_generating;

protected:
	static void _bind_methods();
	static YarnVoxel* singleton;
        void _notification(int p_what);

public:
	uint64_t ticks_last_started_generating;
	uint64_t ticks_last_completed;
	Vector3i last_chunk_started_generating;
	Vector3i last_chunk_completed;
	static HashMap<Vector3i,YVoxelChunk *> yvchunks;
	bool is_generating;
	bool using_default_shader = false;
	bool get_is_generating() const {return is_generating;}
	bool is_debugging_chunk;
	bool is_triple_polycount;
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
	void set_serialize_when_generating(bool enabled) {serialize_when_generating = enabled;}
	bool get_serialize_when_generating() const {return serialize_when_generating;}


	Vector3i debuggin_chunk;
	Vector3i right = Vector3i(1, 0, 0);
	Vector3i up = Vector3i(0, 1, 0);
	Vector3i forward = Vector3i(0, 0, 1);
	Vector3i rightforward = Vector3i(1, 0, 1);
	Vector3i rightup = Vector3i(1, 1, 0);
	Vector3i forwardup = Vector3i(0, 1, 1);
	float water_level;
	Ref<Material> material;
	String default_material_path;
	Callable callable_chunk_completed_callback;
	StringName chunk_completed_callback;
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

	bool get_is_debugging_chunk() {return is_debugging_chunk;}
	void set_is_debugging_chunk(bool val) {is_debugging_chunk = val;}

	Vector3i get_debugging_chunk() {return debuggin_chunk;}
	void set_debugging_chunk(Vector3i val) {debuggin_chunk = val;}
	//PROPERTIES
	int get_cell_size() const;

	void set_cell_size(int val);

	int get_debugging_config() const;

	void set_debugging_config(int val);

	Node3D* get_main_node();
	void set_fallback_main_node(Node3D* fallback_main);
	void create_main_node_under(Node3D *obj);

	void set_main_node(Node3D* obj);

	Vector3i get_debug_pos() const;
	void set_debug_pos(Vector3i val);

	//OLD STUFF FROM TUTORIAL:
	void add(int p_value);
	void multiply(int p_value);
	void reset();
	int get_total() const;

	void set_material(const Ref<Material> &p_material);

	Ref<Material> get_material();

	void changeFloatAtPosition(Vector3i position, float newFloat, uint8_t newBlockType, uint8_t health);

	void modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type = -1);

	Array find_closest_solid_point_to(Vector3 pos, int search_radius = 2);

	bool damage_voxel_area(Vector3i pos, uint8_t amount, int brushSize);

	void smooth_voxel_area(Vector3i pos, float amount, int brushSize);

	static bool IsPointNumberInBoundary(Vector3i bnumber);

	TypedArray<YVoxelChunk> get_chunks_from_gd_script() const;

	YVoxelChunk *get_chunk_from_gdscript(Vector3i chunk_number);

	void changeFloat(float newFloat, uint8_t newBlockType, uint8_t health);

	void chunk_generated(Vector3i chunk_completed);

	bool handle_dirty_chunks();

	void regenerate_all_chunks();
	void set_dirty_chunk(Vector3i chunkNumber);
	float perlin_noise (float x,float y);
	float perlin_noise_3d (float x,float y, float z);

	static float static_perlin_noise (float x,float y);
	static float static_perlin_noise_3d (float x,float y, float z);
	//OTHER STUFF:
	Vector2i CalculateGridPosition(Vector3 worldPosition);

	Vector3 CalculateChunkCenterPosition(Vector3i chunkPosition);

	Vector3 CalculateCellCenterPosition(Vector2i gridPosition);
	Vector3 GetBottomCornerForChunkInNumber(Vector3i ChunkNumber);
	Vector3i FindChunkNumberFromPosition(Vector3 pos);
	static Vector3i GetChunkNumberFromPosition(Vector3 pos);

	static Vector3i GetPointNumberFromPosition(Vector3 pos);
	Vector3i FindPointNumberFromPosition(Vector3 pos);

	uint8_t FindHealthValueForPos(Vector3 pos);
	float FindFloatValueForPosFloat(Vector3 pos);
	int16_t FindFloatValueForPos(Vector3 pos);

	int FindBlockTypeForPos(Vector3 pos);

	void set_point_data_for_wpos(Vector3 pos, YarnVoxelData::YVPointValue pv);

	YarnVoxelData::YVPointValue get_point_data_for_wpos(Vector3 pos);

	int HashKey(Vector3 v);

	static bool try_get_chunk(Vector3i chunkPosition, YVoxelChunk*& chunk_pointer);

	YVoxelChunk *get_chunk(Vector3i chunkPosition);

	static constexpr int BIG_ENOUGH_INT = 16 * 1024;
	static constexpr double BIG_ENOUGH_FLOOR = BIG_ENOUGH_INT + 0.0000;

	YarnVoxel();
	~YarnVoxel() override;
	static YarnVoxel* get_singleton();

	void empty_all_chunks();

	void clear_all_chunks();

	static bool IsPositionValid(Vector3i pos);

	Vector3 find_air_position_with_clearance(Vector3 center_pos, int radius, float required_clearance);
};

VARIANT_ENUM_CAST(YarnVoxel::BlockType);
#endif // YARNVOXEL_H