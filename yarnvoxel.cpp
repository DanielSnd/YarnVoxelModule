/* yarnvoxel.cpp */

#include "yarnvoxel.h"
#include "constants.h"
#include "core/config/project_settings.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_interface.h"
#endif

YarnVoxel* YarnVoxel::singleton;
HashMap<Vector3i, YVoxelChunk*> YarnVoxel::yvchunks;
Ref<Material> material;

YarnVoxel* YarnVoxel::get_singleton() {
	return singleton;
}

void YarnVoxel::clear_all_chunks() {
	for (const KeyValue<Vector3i, YVoxelChunk*> &E : yvchunks) {
		if (E.value != nullptr) {
			E.value->queue_free();
		}
	}
	yvchunks.clear();
}

bool YarnVoxel::IsPositionValid(Vector3i pos) {
	return pos.x >= 0 && pos.x < YARNVOXEL_CHUNK_WIDTH && pos.y >= 0 && pos.y < YARNVOXEL_CHUNK_HEIGHT && pos.z >= 0 && pos.z < YARNVOXEL_CHUNK_WIDTH;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::GetBottomCornerForChunkInNumber(Vector3i ChunkNumber) {
	return {static_cast<real_t>(ChunkNumber.x) * YARNVOXEL_CHUNK_WIDTH, static_cast<real_t>(ChunkNumber.y) * YARNVOXEL_CHUNK_HEIGHT,
	        static_cast<real_t>(ChunkNumber.z) * YARNVOXEL_CHUNK_WIDTH};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3i YarnVoxel::FindChunkNumberFromPosition(const Vector3 pos) {
	return {FastFloor(pos.x / YARNVOXEL_CHUNK_WIDTH), FastFloor(pos.y / YARNVOXEL_CHUNK_HEIGHT), FastFloor(pos.z / YARNVOXEL_CHUNK_WIDTH)};
}

Vector3i YarnVoxel::GetChunkNumberFromPosition(const Vector3 pos) {
	return {FastFloor(pos.x / YARNVOXEL_CHUNK_WIDTH), FastFloor(pos.y / YARNVOXEL_CHUNK_HEIGHT), FastFloor(pos.z / YARNVOXEL_CHUNK_WIDTH)};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3i YarnVoxel::GetPointNumberFromPosition(Vector3 pos) {
	Vector3i PointNumber = Vector3i(static_cast<int>(Math::round(Math::fmod(pos.x,YARNVOXEL_CHUNK_WIDTH))),
	static_cast<int>(Math::round(Math::fmod(pos.y, YARNVOXEL_CHUNK_HEIGHT))),
	static_cast<int>(Math::round(Math::fmod(pos.z, YARNVOXEL_CHUNK_WIDTH))));

	if (PointNumber.x < 0) PointNumber.x = YARNVOXEL_CHUNK_WIDTH - (-PointNumber.x);
	if (PointNumber.y < 0) PointNumber.y = YARNVOXEL_CHUNK_HEIGHT - (-PointNumber.y);
	if (PointNumber.z < 0) PointNumber.z = YARNVOXEL_CHUNK_WIDTH - (-PointNumber.z);

	return PointNumber;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3i YarnVoxel::FindPointNumberFromPosition(Vector3 pos) {
	Vector3i PointNumber = Vector3i(static_cast<int>(Math::round(Math::fmod(pos.x,YARNVOXEL_CHUNK_WIDTH))),
	static_cast<int>(Math::round(Math::fmod(pos.y, YARNVOXEL_CHUNK_HEIGHT))),
	static_cast<int>(Math::round(Math::fmod(pos.z, YARNVOXEL_CHUNK_WIDTH))));

	if (PointNumber.x < 0) PointNumber.x = YARNVOXEL_CHUNK_WIDTH - (-PointNumber.x);
	if (PointNumber.y < 0) PointNumber.y = YARNVOXEL_CHUNK_HEIGHT - (-PointNumber.y);
	if (PointNumber.z < 0) PointNumber.z = YARNVOXEL_CHUNK_WIDTH - (-PointNumber.z);

	return PointNumber;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
int16_t YarnVoxel::FindFloatValueForPos(Vector3 pos) {
	const Vector3i chunkNumber = FindChunkNumberFromPosition(pos);
	const auto ic = get_chunk(chunkNumber);
	if(ic != nullptr){
		const Vector3i blockNumber =	FindPointNumberFromPosition(Vector3(static_cast<float>(pos.x),static_cast<float>(pos.y),static_cast<float>(pos.z)));
		if (IsPositionValid(blockNumber)) {
			return ic->points[blockNumber.x][blockNumber.y][blockNumber.z].floatValue;
		}
	}
	return INT16_MAX;
}

int YarnVoxel::FindBlockTypeForPos(Vector3 pos) {
	const Vector3i chunkNumber = FindChunkNumberFromPosition(pos);
	const auto ic = get_chunk(chunkNumber);
	if(ic != nullptr) {
		const Vector3i blockNumber =	FindPointNumberFromPosition(Vector3(static_cast<float>(pos.x),static_cast<float>(pos.y),static_cast<float>(pos.z)));
		if (IsPositionValid(blockNumber)) {
			return ic->points[blockNumber.x][blockNumber.y][blockNumber.z].byteValue;
		}
	}
	return 0;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
int YarnVoxel::FastFloor(float f) {
	return static_cast<int>(f + BIG_ENOUGH_FLOOR) - BIG_ENOUGH_INT;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
int YarnVoxel::HashKey(Vector3 v) {
	return ((FastFloor(v.x / static_cast<real_t>(singleton->cellSize)) * 73856093) ^ (FastFloor(v.z / static_cast<real_t>(singleton->cellSize)) * 83492791));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector2i YarnVoxel::CalculateGridPosition(Vector3 worldPosition) {
	int x = static_cast<int>(Math::floor(worldPosition.x / static_cast<real_t>(singleton->cellSize)));
	int z = static_cast<int>(Math::floor(worldPosition.z / static_cast<real_t>(singleton->cellSize)));
	return {x, z};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::CalculateChunkCenterPosition(Vector3i chunkPosition) {
	const auto bottom_corner = GetBottomCornerForChunkInNumber(chunkPosition);
	return bottom_corner + (right * (0.5 * YARNVOXEL_CHUNK_WIDTH)) + (forward * (0.5 * YARNVOXEL_CHUNK_WIDTH)) + (up * (0.5 * YARNVOXEL_CHUNK_HEIGHT));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::CalculateCellCenterPosition(Vector2i gridPosition) {
	const float halfCellSize = static_cast<real_t>(singleton->cellSize) * 0.5f;
	float x = (static_cast<real_t>(gridPosition.x) * static_cast<real_t>(singleton->cellSize)) + (halfCellSize);
	float z = (static_cast<real_t>(gridPosition.y) * static_cast<real_t>(singleton->cellSize)) + (halfCellSize);
	return {x, 0, z};
}

bool YarnVoxel::try_get_chunk(Vector3i chunkPosition, YVoxelChunk*& chunk_pointer) {
	if (!yvchunks.has(chunkPosition) || yvchunks[chunkPosition] == nullptr) {
		return false;
	}

	chunk_pointer = yvchunks[chunkPosition];
	//print_line("Try get chunk has found neighbour chunk ",chunkPosition," chunk pointer ",chunk_pointer);
	return true;
}
YVoxelChunk* YarnVoxel::get_chunk(Vector3i chunkPosition) {
	if (!yvchunks.has(chunkPosition) || yvchunks[chunkPosition] == nullptr) {
		YVoxelChunk* chunk = memnew(YVoxelChunk);
		chunk->generate_grass = generate_grass;
		chunk->set_name(vformat("VoxelChunk %s",chunkPosition));
		auto parent_node = get_main_node();
		if (parent_node == nullptr) {
			ERR_PRINT("[YarnVoxel] Main node not found");
		} else {
			parent_node->add_child(chunk);

#ifdef TOOLS_ENABLED
			if(Engine::get_singleton()->is_editor_hint()) {
				// print_line("Add owner to chunk. Current owner? ",chunk->get_owner()," new owner if scenetree ",SceneTree::get_singleton()->get_current_scene());
				// chunk->set_owner(SceneTree::get_singleton()->get_current_scene());
				//print_line("EditorInterface::get_singleton()->get_edited_scene_root() ",EditorInterface::get_singleton()->get_edited_scene_root());
				//if(Engine::get_singleton()->is_editor_hint()) {
					chunk->set_owner(EditorInterface::get_singleton()->get_edited_scene_root());
				//}
			}
#endif
		}
		chunk->set_chunk_number(chunkPosition);
		chunk->initialize(chunkPosition);
		chunk->has_registered_chunk_number = true;
		yvchunks[chunkPosition] = chunk;
		return chunk;
	}
	return yvchunks[chunkPosition];
}

void YarnVoxel::add(int p_value) {
	get_main_node();
	//print_line("boop",YarnVoxelData::SurroundingAndUpDownTable[2]);
	count += p_value;
}

void YarnVoxel::multiply(int p_value) {
	count *= p_value;
	//print_line(Engine::get_singleton()->is_editor_hint());
}

void YarnVoxel::reset() {
	count = 0;
}

int YarnVoxel::get_total() const {
	return count;
}

void YarnVoxel::set_material(const Ref<Material> &p_material) {
	material = p_material;
}

Ref<Material> YarnVoxel::get_material() {
	if(!material.is_valid() || material.is_null() && !default_material_path.is_empty() && default_material_path.is_resource_file()) {
			material = ResourceLoader::load(default_material_path, "Material");
	}
	return material;
}

// ClassDB::bind_method(D_METHOD("set_fractal_octaves", "octave_count"), &FastNoiseLite::set_fractal_octaves);
// ClassDB::bind_method(D_METHOD("get_fractal_octaves"), &FastNoiseLite::get_fractal_octaves);
void YarnVoxel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_chunk","chunkPosition"), &YarnVoxel::get_chunk);
	ClassDB::bind_method(D_METHOD("clear_all_chunks"), &YarnVoxel::clear_all_chunks);
	ClassDB::bind_method(D_METHOD("set_main_node","main_node"), &YarnVoxel::set_main_node);
	ClassDB::bind_method(D_METHOD("get_main_node"), &YarnVoxel::get_main_node);

	ClassDB::bind_method(D_METHOD("get_grid_position","worldPosition"), &YarnVoxel::CalculateGridPosition);
	ClassDB::bind_method(D_METHOD("get_cell_center","gridPosition"), &YarnVoxel::CalculateCellCenterPosition);
	ClassDB::bind_method(D_METHOD("get_chunk_center","chunkNumber"), &YarnVoxel::CalculateChunkCenterPosition);
	ClassDB::bind_method(D_METHOD("get_bottom_corner_for_chunk","chunkNumber"), &YarnVoxel::GetBottomCornerForChunkInNumber);
	ClassDB::bind_method(D_METHOD("get_chunk_number_from_pos","worldPostion"), &YarnVoxel::FindChunkNumberFromPosition);
	ClassDB::bind_method(D_METHOD("get_point_number_from_pos","worldPostion"), &YarnVoxel::FindPointNumberFromPosition);
	ClassDB::bind_method(D_METHOD("get_hash_key","worldPostion"), &YarnVoxel::HashKey);

	ClassDB::bind_method(D_METHOD("get_float_value","worldPostion"), &YarnVoxel::FindFloatValueForPos);
	ClassDB::bind_method(D_METHOD("get_block_type","worldPostion"), &YarnVoxel::FindBlockTypeForPos);

	// static Vector2i CalculateGridPosition(Vector3 worldPosition);
	// static Vector3 CalculateCellCenterPosition(Vector2i gridPosition);
	// static Vector3 GetBottomCornerForChunkInNumber(Vector3i ChunkNumber);
	// static Vector3i FindChunkNumberFromPosition(Vector3 pos);
	// static Vector3i FindPointNumberFromPosition(Vector3 pos);
	// static int HashKey(Vector3 v);

	ClassDB::bind_method(D_METHOD("set_material", "material"), &YarnVoxel::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &YarnVoxel::get_material);
	//ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,StandardMaterial3D,ShaderMaterial"), "set_material", "get_material");

	ClassDB::bind_method(D_METHOD("set_grass_material", "material"), &YarnVoxel::set_grass_material);
	ClassDB::bind_method(D_METHOD("get_grass_material"), &YarnVoxel::get_grass_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grass_material", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,StandardMaterial3D,ShaderMaterial"), "set_grass_material", "get_grass_material");

	ClassDB::bind_method(D_METHOD("set_grass_mesh", "mesh"), &YarnVoxel::set_grass_mesh);
	ClassDB::bind_method(D_METHOD("get_grass_mesh"), &YarnVoxel::get_grass_mesh);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grass_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_grass_mesh", "get_grass_mesh");

	ClassDB::bind_method(D_METHOD("get_cell_size"), &YarnVoxel::get_cell_size);
	ClassDB::bind_method(D_METHOD("set_cell_size","cell_size"), &YarnVoxel::set_cell_size);

	ADD_PROPERTY(PropertyInfo(Variant::INT,"cell_size", PROPERTY_HINT_RANGE,"32,1024,32"),"set_cell_size","get_cell_size");

	ClassDB::bind_method(D_METHOD("set_value", "value","block_type","health"), &YarnVoxel::changeFloat, DEFVAL(GRASS),DEFVAL(255));
	ClassDB::bind_method(D_METHOD("set_value_at","position", "value","block_type","health"), &YarnVoxel::changeFloatAtPosition, DEFVAL(GRASS),DEFVAL(255));

	ClassDB::bind_method(D_METHOD("get_is_debugging_chunk"), &YarnVoxel::get_is_debugging_chunk);
	ClassDB::bind_method(D_METHOD("set_is_debugging_chunk","debug_chunk"), &YarnVoxel::set_is_debugging_chunk);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL,"is_debugging_chunk",PROPERTY_HINT_NONE),"set_is_debugging_chunk","get_is_debugging_chunk");

	ClassDB::bind_method(D_METHOD("get_debugging_chunk"), &YarnVoxel::get_debugging_chunk);
	ClassDB::bind_method(D_METHOD("set_debugging_chunk","debug_chunk"), &YarnVoxel::set_debugging_chunk);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I,"debugging_chunk",PROPERTY_HINT_NONE),"set_debugging_chunk","get_debugging_chunk");

	ClassDB::bind_method(D_METHOD("get_debug_pos"), &YarnVoxel::get_debug_pos);
	ClassDB::bind_method(D_METHOD("set_debug_pos","debug_pos"), &YarnVoxel::set_debug_pos);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I,"debug_pos",PROPERTY_HINT_NONE),"set_debug_pos","get_debug_pos");

	ClassDB::bind_method(D_METHOD("get_debugging_config"), &YarnVoxel::get_debugging_config);
	ClassDB::bind_method(D_METHOD("set_debugging_config","debugging_config"), &YarnVoxel::set_debugging_config);
	ADD_PROPERTY(PropertyInfo(Variant::INT,"debugging_config"),"set_debugging_config","get_debugging_config");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT,"main_node", PROPERTY_HINT_NODE_TYPE,"Node3D"),"set_main_node","get_main_node");

	ClassDB::bind_method(D_METHOD("get_generate_grass"), &YarnVoxel::get_generate_grass);
	ClassDB::bind_method(D_METHOD("set_generate_grass","status"), &YarnVoxel::set_generate_grass);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL,"generate_grass",PROPERTY_HINT_NONE),"set_generate_grass","get_generate_grass");

	ClassDB::bind_method(D_METHOD("regenerate_all_chunks"), &YarnVoxel::regenerate_all_chunks);
	ClassDB::bind_method(D_METHOD("get_is_generating"), &YarnVoxel::get_is_generating);
	ClassDB::bind_method(D_METHOD("get_chunks"), &YarnVoxel::get_chunks_from_gd_script);
	ClassDB::bind_method(D_METHOD("try_get_chunk","chunk_number"), &YarnVoxel::get_chunk_from_gdscript);

	ClassDB::bind_method(D_METHOD("get_perlin","pos_x","pos_y"), &YarnVoxel::perlin_noise);
	ClassDB::bind_method(D_METHOD("get_perlin_3d","pos_x","pos_y","pos_z"), &YarnVoxel::perlin_noise_3d);

	ADD_SIGNAL(MethodInfo("finished"));

	BIND_ENUM_CONSTANT(NONE);
	BIND_ENUM_CONSTANT(GRASS);
	BIND_ENUM_CONSTANT(DIRT);
	BIND_ENUM_CONSTANT(STONE);
	BIND_ENUM_CONSTANT(SAND);
	BIND_ENUM_CONSTANT(IRON);
	BIND_ENUM_CONSTANT(COPPER);
	BIND_ENUM_CONSTANT(BRONZE);
	BIND_ENUM_CONSTANT(SILVER);
	BIND_ENUM_CONSTANT(GOLD);
	BIND_ENUM_CONSTANT(DIAMOND);
}

void YarnVoxel::changeFloatAtPosition(Vector3i position, float newFloat, uint8_t newBlockType, uint8_t health = 255) {
	const Vector3i chunkNumber = FindChunkNumberFromPosition(position);
	const auto ic = get_chunk(chunkNumber);
	if(ic != nullptr){
		const Vector3i blockNumber =	FindPointNumberFromPosition(Vector3(static_cast<float>(position.x),static_cast<float>(position.y),static_cast<float>(position.z)));
		if (IsPositionValid(blockNumber)) {
			if (newFloat >= 0.999999f) {
				ic->points[blockNumber.x][blockNumber.y][blockNumber.z] = YarnVoxelData::YVPointValue(); // Assuming this is default
			} else {
				ic->points[blockNumber.x][blockNumber.y][blockNumber.z] = YarnVoxelData::YVPointValue(newBlockType, floatToInt16(newFloat),health);
			}
			set_dirty_chunk(chunkNumber);
			if(IsPointNumberInBoundary(blockNumber)) {
				ic->AttemptSetDirtyNeighbour(blockNumber);
			}
		}
	}
}

bool YarnVoxel::IsPointNumberInBoundary(Vector3i bnumber) {
	return bnumber.x == 0 || bnumber.x >= YARNVOXEL_CHUNK_WIDTH-1 ||bnumber.y == 0 || bnumber.y >= YARNVOXEL_CHUNK_HEIGHT-1 ||bnumber.z == 0 || bnumber.z >= YARNVOXEL_CHUNK_WIDTH-1;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
TypedArray<YVoxelChunk> YarnVoxel::get_chunks_from_gd_script() const {
	TypedArray<YVoxelChunk> arr;
	arr.resize(static_cast<int>(yvchunks.size()));
	for (const KeyValue<Vector3i, YVoxelChunk*> &E : yvchunks) {
		arr.append(E.value);
	}
	return arr;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
YVoxelChunk *YarnVoxel::get_chunk_from_gdscript(Vector3i chunk_number) {
	YVoxelChunk* tryChunk = nullptr;
	try_get_chunk(chunk_number,tryChunk);
	return tryChunk;
}

void YarnVoxel::changeFloat(float newFloat, uint8_t newBlockType, uint8_t health = 255) {
	changeFloatAtPosition(debug_pos,newFloat,newBlockType,health);
}

void YarnVoxel::chunk_generated (Vector3i chunk_completed) {
	//print_line("YarnVoxel received callback chunk generated from chunk number ",chunk_completed);
	if (is_generating) {
		is_generating=false;
		if (DirtyChunksQueue.size() <= 0) {
			emit_signal(SNAME("finished"));
		}
	}
}

bool YarnVoxel::handle_dirty_chunks() {
	//print_line("Handle dirty chunks called, is generating? ",is_generating," dirty queue size? ",DirtyChunksQueue.size());
	if(is_generating) {
		return false;
	}
	if(DirtyChunksQueue.size() <=0) {
		return true;
	}
	is_generating = true;

	const auto current_chunk = DirtyChunksQueue[0];
	DirtyChunksQueue.remove_at(0);
	const auto found_chunk = get_chunk(current_chunk);
	found_chunk->clear_triangles();
	found_chunk->connect(found_chunk->completed_generation,callable_chunk_completed_callback, CONNECT_ONE_SHOT);
	//print_line("connecting ",found_chunk->completed_generation," to ", callable_chunk_completed_callback);
	found_chunk->generate();
	return false;
}

void YarnVoxel::regenerate_all_chunks() {
	DirtyChunksQueue.clear();
	for (const KeyValue<Vector3i, YVoxelChunk*> &E : yvchunks) {
		set_dirty_chunk(E.value->chunk_number);
	}
}

void YarnVoxel::set_dirty_chunk(Vector3i chunkNumber) {
	if(!DirtyChunksQueue.has(chunkNumber)) {
		DirtyChunksQueue.append(chunkNumber);
		const auto ic = get_chunk(chunkNumber);
		//print_line("Setting dirty chunk ",chunkNumber ," ",ic);
		if (!Engine::get_singleton()->is_editor_hint()) {
			ic->set_process(true);
		} else {
			ic->do_process();
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeStatic
float YarnVoxel::perlin_noise(float x, float y) {
	return static_perlin_noise(x,y);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
float YarnVoxel::perlin_noise_3d(float x, float y, float z) {
	return static_perlin_noise_3d(x,y,z);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
float YarnVoxel::static_perlin_noise(float x, float y) {
	return db::perlin(x,y);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
float YarnVoxel::static_perlin_noise_3d(float x, float y, float z) {
	return db::perlin(x,y,z);
}

Vector3i YarnVoxel::get_debug_pos() const {
	return debug_pos;
}
void YarnVoxel::set_debug_pos(Vector3i val) {
	debug_pos = val;
}

int YarnVoxel::get_cell_size() const {
	return cellSize;
}
void YarnVoxel::set_cell_size(int val) {
	cellSize = val;
}
int YarnVoxel::get_debugging_config() const {
	return static_cast<int>(debugging_config);
}
void YarnVoxel::set_debugging_config(int val) {
	debugging_config = static_cast<uint8_t>(val);
}

Node3D* YarnVoxel::get_main_node() {
	if (main_node_pointer != nullptr) {
		return main_node_pointer;
	}
	if (Engine::get_singleton()->is_editor_hint()) {
		if(yvchunks.size()>0) {
			main_node_pointer = Object::cast_to<Node3D>(yvchunks.last()->value->get_parent());
		} else {
			if (SceneTree::get_singleton() != nullptr) {
				main_node_pointer = Object::cast_to<Node3D>(SceneTree::get_singleton()->get_current_scene());
			}
		}
		return main_node_pointer;
	}
	MainLoop *ml = OS::get_singleton()->get_main_loop();
	if (ml == nullptr) {
		ERR_PRINT("MAIN LOOP NOT FOUND");
		return nullptr;
	}
	auto const *sml = Object::cast_to<SceneTree>(ml);
	Node *current_scene = sml->get_current_scene();
	main_node_pointer = memnew(Node3D);
	if (main_node_pointer == nullptr) {
		ERR_FAIL_NULL_V(main_node_pointer, nullptr);
	}
	current_scene->add_child(main_node_pointer);
	main_node_pointer->set_name("YarnVoxels");
	return main_node_pointer;
}

void YarnVoxel::set_main_node(Node3D* obj) {
	main_node_pointer = obj;
}

YarnVoxel::YarnVoxel() {
	is_generating=false;
    chunk_completed_callback = StaticCString::create("chunk_generated");
	callable_chunk_completed_callback = callable_mp(this,&YarnVoxel::chunk_generated);
	count = 0;
	debugging_config = 0;
	is_debugging_chunk = false;
	debuggin_chunk = Vector3i{0,0,0};
	singleton = this;
	cellSize = 256;
	terrainSurface = 0;
	ChunkWidth = 32;
	ChunkHeight = 32;
	DivideForChunkWidth = 0.015625;
	DivideForChunkHeight = 0.015625;
	BaseTerrainHeight = 60; // Minimum height of terrain.
	TerrainHeightRange = 10; // The max height (above BaseTerrainHeight) our terrain can be.
	// material = nullptr;
	main_node_pointer = nullptr;
	// grass_mesh = nullptr;
	// grass_material = nullptr;
	generate_grass=false;
	water_level = 2.0;
	default_material_path = GLOBAL_DEF_RST(PropertyInfo(Variant::STRING, "yarnvoxel/general/default_material", PROPERTY_HINT_FILE, "*.tres,*.res", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), "");
	//print_line("default_material_path ",default_material_path);
	material = Ref<Material>();
}

YarnVoxel::~YarnVoxel() {
	material.unref();
	main_node_pointer = nullptr;
	grass_mesh.unref();
	grass_material.unref();
}
