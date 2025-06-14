/* yarnvoxel.cpp */

#include "yarnvoxel.h"
#include "constants.h"
#include "core/config/project_settings.h"
#include "modules/noise/fastnoise_lite.h"
#include "modules/noise/noise_texture_2d.h"
#include "scene/resources/gradient_texture.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_interface.h"
#endif

void YarnVoxel::empty_all_chunks() {
	for (const KeyValue<Vector3i, ObjectID>& E : yvchunks) {
		if (auto chunk = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(E.value))) {
			if (chunk != nullptr) {
				chunk->queue_free();
			}
		}
	}
	yvchunks.clear();
}

void YarnVoxel::clear_all_chunks() {
	empty_all_chunks();
}

bool YarnVoxel::IsPositionValid(Vector3i pos) {
	return pos.x >= 0 && pos.x < YARNVOXEL_CHUNK_WIDTH && pos.y >= 0 && pos.y < YARNVOXEL_CHUNK_HEIGHT && pos.z >= 0 && pos.z < YARNVOXEL_CHUNK_WIDTH;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::CalculateChunkCenterPosition(Vector3i chunkPosition) {
	const auto bottom_corner = GetBottomCornerForChunkInNumber(chunkPosition);
	return bottom_corner + (right * (0.5 * YARNVOXEL_CHUNK_WIDTH)) + (forward * (0.5 * YARNVOXEL_CHUNK_WIDTH)) + (up * (0.5 * YARNVOXEL_CHUNK_HEIGHT));
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

// ReSharper disable once CppMemberFunctionMayBeStatic
uint8_t YarnVoxel::FindHealthValueForPos(Vector3 pos) {
	const Vector3i chunkNumber = FindChunkNumberFromPosition(pos);
	const auto ic = get_chunk(chunkNumber);
	if(ic != nullptr){
		const Vector3i blockNumber =	FindPointNumberFromPosition(Vector3(static_cast<float>(pos.x),static_cast<float>(pos.y),static_cast<float>(pos.z)));
		if (IsPositionValid(blockNumber)) {
			return ic->points[blockNumber.x][blockNumber.y][blockNumber.z].health;
		}
	}
	return 0;
}

float YarnVoxel::FindFloatValueForPosFloat(Vector3 pos) {
	return int16ToFloat(FindFloatValueForPos(pos));
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

void YarnVoxel::set_point_data_for_wpos(Vector3 pos,YarnVoxelData::YVPointValue pv) {
	const Vector3i chunkNumber = FindChunkNumberFromPosition(pos);
	const auto ic = get_chunk(chunkNumber);
	if(ic != nullptr) {
		const Vector3i blockNumber = FindPointNumberFromPosition(Vector3(static_cast<float>(pos.x),static_cast<float>(pos.y),static_cast<float>(pos.z)));
		if (IsPositionValid(blockNumber)) {
			ic->points[blockNumber.x][blockNumber.y][blockNumber.z] = pv;
			ic->deferred_set_dirty();
		}
	}
}

YarnVoxelData::YVPointValue YarnVoxel::get_point_data_for_wpos(Vector3 pos) {
	const Vector3i chunkNumber = FindChunkNumberFromPosition(pos);
	const auto ic = get_chunk(chunkNumber);
	if(ic != nullptr) {
		const Vector3i blockNumber =	FindPointNumberFromPosition(Vector3(static_cast<float>(pos.x),static_cast<float>(pos.y),static_cast<float>(pos.z)));
		if (IsPositionValid(blockNumber)) {
			return ic->points[blockNumber.x][blockNumber.y][blockNumber.z];
		}
	}
	return YarnVoxelData::YVPointValue{};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
int YarnVoxel::FastFloor(float f) {
	return static_cast<int>(f + BIG_ENOUGH_FLOOR) - BIG_ENOUGH_INT;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
int YarnVoxel::HashKey(Vector3 v) {
	return ((FastFloor(v.x / static_cast<real_t>(cellSize)) * 73856093) ^ (FastFloor(v.z / static_cast<real_t>(cellSize)) * 83492791));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector2i YarnVoxel::CalculateGridPosition(Vector3 worldPosition) {
	int x = static_cast<int>(Math::floor(worldPosition.x / static_cast<real_t>(cellSize)));
	int z = static_cast<int>(Math::floor(worldPosition.z / static_cast<real_t>(cellSize)));
	return {x, z};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::CalculateCellCenterPosition(Vector2i gridPosition) {
	const float halfCellSize = static_cast<real_t>(cellSize) * 0.5f;
	float x = (static_cast<real_t>(gridPosition.x) * static_cast<real_t>(cellSize)) + (halfCellSize);
	float z = (static_cast<real_t>(gridPosition.y) * static_cast<real_t>(cellSize)) + (halfCellSize);
	return {x, 0, z};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::GetBottomCornerForChunkInNumber(Vector3i ChunkNumber) {
	return {static_cast<real_t>(ChunkNumber.x) * YARNVOXEL_CHUNK_WIDTH, static_cast<real_t>(ChunkNumber.y) * YARNVOXEL_CHUNK_HEIGHT,
	        static_cast<real_t>(ChunkNumber.z) * YARNVOXEL_CHUNK_WIDTH};
}

bool YarnVoxel::try_get_chunk(Vector3i chunkPosition, YVoxelChunk*& chunk_pointer) {
	if (!yvchunks.has(chunkPosition)) {
		return false;
	}

	chunk_pointer = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(yvchunks[chunkPosition]));
	//print_line("Try get chunk has found neighbour chunk ",chunkPosition," chunk pointer ",chunk_pointer);
	return true;
}

YVoxelChunk* YarnVoxel::get_chunk(Vector3i chunkPosition) {
	if (!yvchunks.has(chunkPosition) || ObjectDB::get_instance(yvchunks[chunkPosition]) == nullptr) {
		YVoxelChunk* chunk = memnew(YVoxelChunk);
		chunk->set_collision_layer_value(1,true);
		chunk->generate_grass = generate_grass;
		chunk->set_name(vformat("VoxelChunk %s",chunkPosition));
		add_child(chunk);
#ifdef TOOLS_ENABLED
		if(Engine::get_singleton()->is_editor_hint()) {
				chunk->set_owner(EditorInterface::get_singleton()->get_edited_scene_root());
		}
#endif
		chunk->set_chunk_number(chunkPosition);
		chunk->initialize(chunkPosition);
		chunk->has_registered_chunk_number = true;
		yvchunks[chunkPosition] = chunk->get_instance_id();
		// Debug print for chunk creation
		print_line(vformat("[YarnVoxel::get_chunk] Creating chunk at %s | YARNVOXEL_CHUNK_WIDTH: %d, YARNVOXEL_CHUNK_HEIGHT: %d", chunkPosition, YARNVOXEL_CHUNK_WIDTH, YARNVOXEL_CHUNK_HEIGHT));
		Vector3 bottom_corner = GetBottomCornerForChunkInNumber(chunkPosition);
		chunk->set_bottom_corner_world_pos(bottom_corner);
		chunk->set_position(bottom_corner);
		print_line(vformat("[YarnVoxel::get_chunk] Bottom corner world pos for chunk %s: %s", chunkPosition, bottom_corner));
		return chunk;
	}
	return Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(yvchunks[chunkPosition]));
}


void YarnVoxel::add(int p_value) {
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
	return material;
}

void YarnVoxel::changeFloat(float newFloat, uint8_t newBlockType, uint8_t health) {
	changeFloatAtPosition(debug_pos, newFloat, newBlockType, health);
}

bool YarnVoxel::handle_dirty_chunks() {
	if (is_generating) {
		return false;
	}
	if (DirtyChunksQueue.size() <= 0) {
		return true;
	}
	is_generating = true;
	ticks_last_started_generating = OS::get_singleton()->get_ticks_usec() / 1000;
	const auto current_chunk = DirtyChunksQueue[0];
	last_chunk_started_generating = current_chunk;
	DirtyChunksQueue.remove_at(0);
	YVoxelChunk* found_chunk = nullptr;
	if (try_get_chunk(current_chunk, found_chunk)) {
		if (found_chunk->is_inside_tree()) {
			found_chunk->clear_triangles();
			found_chunk->connect(found_chunk->completed_generation, callable_mp(this, &YarnVoxel::chunk_generated), CONNECT_ONE_SHOT);
			found_chunk->generate();
		} else {
			is_generating = false;
		}
	} else {
		WARN_PRINT(vformat("Failed to find chunk number %s. Current chunks in dictionary %d", current_chunk, yvchunks.size()));
		is_generating = false;
	}
	return false;
}

void YarnVoxel::regenerate_all_chunks() {
	DirtyChunksQueue.clear();
	for (const KeyValue<Vector3i, ObjectID> &E : yvchunks) {
		if (auto chunk = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(E.value))) {
			set_dirty_chunk(chunk->chunk_number);
		}
	}
}

TypedArray<YVoxelChunk> YarnVoxel::get_chunks_from_gd_script() const {
	TypedArray<YVoxelChunk> arr;
	for (const KeyValue<Vector3i, ObjectID> &E : yvchunks) {
		if (auto chunk = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(E.value))) {
			arr.append(chunk);
		}
	}
	return arr;
}

TypedArray<Vector3i> YarnVoxel::get_dirty_chunks_queued() const {
	TypedArray<Vector3i> dirty_chunks;
	for (const Vector3i &chunk : DirtyChunksQueue) {
		dirty_chunks.push_back(chunk);
	}
	return dirty_chunks;
}

String YarnVoxel::dirty_chunk_queue_info() {
	String info = "";
	info += vformat("Dirty chunks queue size: %d is generating? %s", DirtyChunksQueue.size(), is_generating);
	info += vformat("Last Started: %s %s\n", ticks_last_started_generating, last_chunk_started_generating);
	info += vformat("Last Completed: %s %s\n", ticks_last_completed, last_chunk_completed);
	for (const Vector3i &chunk : DirtyChunksQueue) {
		info += vformat("- Dirty chunk: %s\n", chunk);
	}
	return info;
}

// ClassDB::bind_method(D_METHOD("set_fractal_octaves", "octave_count"), &FastNoiseLite::set_fractal_octaves);
// ClassDB::bind_method(D_METHOD("get_fractal_octaves"), &FastNoiseLite::get_fractal_octaves);
void YarnVoxel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add", "value"), &YarnVoxel::add);
	ClassDB::bind_method(D_METHOD("multiply", "value"), &YarnVoxel::multiply);
	ClassDB::bind_method(D_METHOD("reset"), &YarnVoxel::reset);
	ClassDB::bind_method(D_METHOD("get_total"), &YarnVoxel::get_total);
	ClassDB::bind_method(D_METHOD("set_material", "material"), &YarnVoxel::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &YarnVoxel::get_material);
	ClassDB::bind_method(D_METHOD("changeFloatAtPosition", "position", "newFloat", "newBlockType", "health"), &YarnVoxel::changeFloatAtPosition, DEFVAL(255));
	ClassDB::bind_method(D_METHOD("modify_voxel_area", "pos", "amount", "brushSize", "block_type"), &YarnVoxel::modify_voxel_area, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("damage_voxel_area", "pos", "amount", "brushSize"), &YarnVoxel::damage_voxel_area);
	ClassDB::bind_method(D_METHOD("smooth_voxel_area", "pos", "amount", "brushSize"), &YarnVoxel::smooth_voxel_area);
	ClassDB::bind_method(D_METHOD("find_closest_solid_point_to", "pos", "search_radius"), &YarnVoxel::find_closest_solid_point_to);
	ClassDB::bind_method(D_METHOD("find_air_position_with_clearance", "center_pos", "radius", "required_clearance"), &YarnVoxel::find_air_position_with_clearance);
	ClassDB::bind_method(D_METHOD("try_get_chunk", "chunk_number"), &YarnVoxel::get_chunk_from_gdscript);
	ClassDB::bind_method(D_METHOD("set_dirty_chunk", "chunkNumber"), &YarnVoxel::set_dirty_chunk);
	ClassDB::bind_static_method("YarnVoxel",D_METHOD("perlin_noise", "x", "y"), &YarnVoxel::static_perlin_noise);
	ClassDB::bind_static_method("YarnVoxel",D_METHOD("perlin_noise_3d", "x", "y", "z"), &YarnVoxel::static_perlin_noise_3d);
	ClassDB::bind_method(D_METHOD("get_debug_pos"), &YarnVoxel::get_debug_pos);
	ClassDB::bind_method(D_METHOD("set_debug_pos", "val"), &YarnVoxel::set_debug_pos);
	ClassDB::bind_method(D_METHOD("get_cell_size"), &YarnVoxel::get_cell_size);
	ClassDB::bind_method(D_METHOD("set_cell_size", "val"), &YarnVoxel::set_cell_size);
	ClassDB::bind_method(D_METHOD("get_debugging_config"), &YarnVoxel::get_debugging_config);
	ClassDB::bind_method(D_METHOD("set_debugging_config", "val"), &YarnVoxel::set_debugging_config);
	ClassDB::bind_method(D_METHOD("get_simplification_distance"), &YarnVoxel::get_simplification_distance);
	ClassDB::bind_method(D_METHOD("set_simplification_distance", "distance"), &YarnVoxel::set_simplification_distance);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "debug_pos"), "set_debug_pos", "get_debug_pos");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_size"), "set_cell_size", "get_cell_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "debugging_config"), "set_debugging_config", "get_debugging_config");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "simplification_distance"), "set_simplification_distance", "get_simplification_distance");

	ADD_SIGNAL(MethodInfo("destroyed_voxels", PropertyInfo(Variant::ARRAY, "positions")));
	ADD_SIGNAL(MethodInfo("finished"));
}

void YarnVoxel::_on_tree_exiting() {
	empty_all_chunks();
	if (yarnvoxel_instances.has(get_instance_id())) {
		yarnvoxel_instances.erase(get_instance_id());
	}
}

void YarnVoxel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			yarnvoxel_instances.append(get_instance_id());
			// Initialize when added to scene
			Callable clear_when_exiting = callable_mp(this, &YarnVoxel::_on_tree_exiting);
			if (!is_connected(SceneStringNames::get_singleton()->tree_exiting, clear_when_exiting)) {
				connect(SceneStringNames::get_singleton()->tree_exiting, clear_when_exiting);
			}
			break;
		}
		case NOTIFICATION_EXIT_TREE: {
			break;
		}
	}
}

bool YarnVoxel::IsPointNumberInBoundary(Vector3i bnumber) {
	return bnumber.x == 0 || bnumber.x >= YARNVOXEL_CHUNK_WIDTH - 1 ||bnumber.y == 0 || bnumber.y >= YARNVOXEL_CHUNK_HEIGHT - 1 ||bnumber.z == 0 || bnumber.z >= YARNVOXEL_CHUNK_WIDTH - 1;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
YVoxelChunk *YarnVoxel::get_chunk_from_gdscript(Vector3i chunk_number) {
	YVoxelChunk* tryChunk = nullptr;
	try_get_chunk(chunk_number,tryChunk);
	return tryChunk;
}

void YarnVoxel::changeFloatAtPosition(Vector3i position, float newFloat, uint8_t newBlockType, uint8_t health) {
	YVoxelChunk* chunk = get_chunk(GetChunkNumberFromPosition(Vector3(position)));
	if (chunk) {
		Vector3i pointNumber = GetPointNumberFromPosition(Vector3(position));
		chunk->SetPointDensity(pointNumber, newFloat, newBlockType);
	}
}

void YarnVoxel::modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type) {
	Vector3i chunk_number = GetChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return;

	bool modify_float = Math::abs(amount) > 0.001f;
	float brushSizeSquared = static_cast<float>(brushSize * brushSize);

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushSizeSquared) continue;

				Vector3i check_chunk_number = GetChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					modify_chunk->deferred_set_dirty();
					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
					if (!modify_chunk) continue;
				}

				Vector3i point_position = GetPointNumberFromPosition(Vector3(x,y,z));
				if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
					continue;
				}

				auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
				if (modify_float) {
					int16_t modifiedAmount = floatToInt16(amount);
					if (modifiedAmount > 0 && point_value.floatValue > INT16_MAX - modifiedAmount) {
						point_value.floatValue = INT16_MAX;
					} else if (modifiedAmount < 0 && point_value.floatValue < INT16_MIN - modifiedAmount) {
						point_value.floatValue = INT16_MIN;
					} else {
						point_value.floatValue += modifiedAmount;
					}

					if (point_value.byteValue == 0) {
						if (point_value.floatValue < YARNVOXEL_TERRAIN_SURFACE) {
							point_value.byteValue = block_type > 0 ? static_cast<uint8_t>(block_type) : 2;
						}
					} else if (point_value.floatValue > YARNVOXEL_TERRAIN_SURFACE) {
						point_value.byteValue = 0;
					}
				} else if (block_type > 0) {
					point_value.byteValue = static_cast<uint8_t>(block_type);
				}

				if (IsPointNumberInBoundary(point_position)) {
					modify_chunk->AttemptSetDirtyNeighbour(point_position);
				}
			}
		}
	}
	modify_chunk->deferred_set_dirty();
}

bool YarnVoxel::damage_voxel_area(Vector3i pos, uint8_t amount, int brushSize) {
	Vector3i chunk_number = GetChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return false;

	Array emit_destroyed;
	Vector<Vector3i> should_be_dirty_chunks;
	bool damaged_any = false;
	bool damaged_current_chunk = false;
	float brushSizeSquared = static_cast<float>(brushSize * brushSize);

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushSizeSquared) continue;

				Vector3i check_chunk_number = GetChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					if (damaged_current_chunk && !should_be_dirty_chunks.has(chunk_number)) {
						should_be_dirty_chunks.append(chunk_number);
					}
					damaged_current_chunk = false;
					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
					if (!modify_chunk) continue;
				}

				Vector3i point_position = GetPointNumberFromPosition(Vector3(x,y,z));
				if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
					continue;
				}

				auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
				if (point_value.byteValue == 0 || point_value.get_float_value_as_float() > 0.01f) {
					continue;
				}

				uint8_t damage_amount = amount < point_value.health ? amount : point_value.health;
				point_value.health -= damage_amount;
				if (damage_amount > 0) {
					damaged_any = true;
					damaged_current_chunk = true;
					float current_float = point_value.get_float_value_as_float();
					float health_percent = static_cast<float>(point_value.health) / 255.0f;
					float target_float = Math::lerp(0.0f, current_float, health_percent);
					point_value.set_float_value_as_float(target_float);
				}

				if (point_value.health <= 1) {
					point_value.health = 255;
					emit_destroyed.append(Vector4i(x,y,z,point_value.byteValue));
					point_value.byteValue = 0;
					point_value.floatValue = INT16_MAX;
				}

				if (IsPointNumberInBoundary(point_position)) {
					modify_chunk->AttemptSetDirtyNeighbour(point_position);
				}
			}
		}
	}

	if (emit_destroyed.size() > 0) {
		emit_signal("destroyed_voxels", emit_destroyed);
	}

	if (damaged_any) {
		if (should_be_dirty_chunks.size() > 0) {
			for (int i = 0; i < should_be_dirty_chunks.size(); ++i) {
				set_dirty_chunk(should_be_dirty_chunks[i]);
			}
		} else {
			set_dirty_chunk(chunk_number);
		}
	}

	return damaged_any;
}

void YarnVoxel::smooth_voxel_area(Vector3i pos, float amount, int brushSize) {
	Vector<Vector3> temp_wpos;
	Vector<float> temp_grid;
	float brushSizeSquared = static_cast<float>(brushSize * brushSize);

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushSizeSquared) continue;

				auto point_data = get_point_data_for_wpos(Vector3(x,y,z));
				float current_float_value = point_data.get_float_value_as_float();
				if (current_float_value >= 0.0) continue;

				float total = 0.0f;
				float weight_sum = 0.0f;
				int sample_radius = 2;

				for (int nx = x - sample_radius; nx <= x + sample_radius; ++nx) {
					for (int ny = y - sample_radius; ny <= y + sample_radius; ++ny) {
						for (int nz = z - sample_radius; nz <= z + sample_radius; ++nz) {
							float neighbor_dist = static_cast<float>(
								(nx - x) * (nx - x) + 
								(ny - y) * (ny - y) + 
								(nz - z) * (nz - z)
							);
							if (neighbor_dist > sample_radius * sample_radius) continue;

							auto neighbor_data = get_point_data_for_wpos(Vector3(nx,ny,nz));
							float weight = 1.0f / (1.0f + Math::sqrt(neighbor_dist));
							total += (neighbor_data.get_float_value_as_float() + 1.0f) * weight;
							weight_sum += weight;
						}
					}
				}

				if (weight_sum <= 0.0f) continue;

				temp_wpos.append(Vector3(x,y,z));
				temp_grid.append((total / weight_sum) - 1.0f);
			}
		}
	}

	YVoxelChunk* modify_chunk = nullptr;
	Vector3i current_chunk_number = GetChunkNumberFromPosition(pos);
	if (!try_get_chunk(current_chunk_number, modify_chunk)) return;

	modify_chunk->deferred_set_dirty();

	for (int i = 0; i < temp_wpos.size(); i++) {
		if (i >= temp_grid.size()) break;

		Vector3 current_pos = temp_wpos[i];
		Vector3i chunk_number = GetChunkNumberFromPosition(current_pos);

		if (chunk_number != current_chunk_number) {
			if (!try_get_chunk(chunk_number, modify_chunk)) continue;
			current_chunk_number = chunk_number;
			modify_chunk->deferred_set_dirty();
		}

		Vector3i block_number = GetPointNumberFromPosition(current_pos);
		if (!IsPositionValid(block_number)) continue;

		auto& point_data = modify_chunk->points[block_number.x][block_number.y][block_number.z];
		float current_float_value = point_data.get_float_value_as_float();
		float dist_to_center = current_pos.distance_to(Vector3(pos));
		float smooth_factor = amount * (1.0f - (dist_to_center / static_cast<float>(brushSize)));
		smooth_factor = CLAMP(smooth_factor, 0.0f, 1.0f);

		point_data.set_float_value_as_float(Math::lerp(current_float_value, temp_grid[i], smooth_factor));

		if (point_data.floatValue > YARNVOXEL_TERRAIN_SURFACE && point_data.byteValue != 0) {
			point_data.byteValue = 0;
		}

		if (IsPointNumberInBoundary(block_number)) {
			modify_chunk->AttemptSetDirtyNeighbour(block_number);
		}
	}
}

Array YarnVoxel::find_closest_solid_point_to(Vector3 pos, int search_radius) {
	Array return_array;
	Vector3i chunk_number = GetChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return return_array;

	float best_distance = 999999.0f;
	bool found_any = false;
	Vector3i best_solid_pos = Vector3i(0,0,0);

	for (int x = pos.x - search_radius; x <= pos.x + search_radius; ++x) {
		for (int y = pos.y - search_radius; y <= pos.y + search_radius; ++y) {
			for (int z = pos.z - search_radius; z <= pos.z + search_radius; ++z) {
				Vector3 current_checking = Vector3(x,y,z);
				Vector3i found_chunk_number = GetChunkNumberFromPosition(current_checking);
				
				YVoxelChunk* check_chunk = modify_chunk;
				if (found_chunk_number != chunk_number) {
					check_chunk = get_chunk(found_chunk_number);
					if (!check_chunk) continue;
				}
				
				Vector3i find_point_number = GetPointNumberFromPosition(current_checking);
				if (!IsPositionValid(find_point_number)) continue;
				
				auto find_value = check_chunk->points[find_point_number.x][find_point_number.y][find_point_number.z];
				float float_value = find_value.get_float_value_as_float();
				if (find_value.byteValue == 0 || float_value > 0.0f) continue;
				
				float distance = pos.distance_squared_to(Vector3(static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)));
				if (distance < best_distance) {
					best_distance = distance;
					best_solid_pos = Vector3i(x,y,z);
					found_any = true;
				}
			}
		}
	}
	
	if (found_any) {
		return_array.append(best_solid_pos);
	}
	return return_array;
}

Vector3 YarnVoxel::find_air_position_with_clearance(Vector3 center_pos, int radius, float required_clearance) {
	float clearance_squared = required_clearance * required_clearance;
	
	for (int r = 0; r <= radius; r++) {
		for (int x = center_pos.x - r; x <= center_pos.x + r; ++x) {
			for (int y = center_pos.y - r; y <= center_pos.y + r; ++y) {
				for (int z = center_pos.z - r; z <= center_pos.z + r; ++z) {
					Vector3 test_pos(x, y, z);
					
					if (test_pos.distance_squared_to(center_pos) > radius * radius) {
						continue;
					}

					auto point_data = get_point_data_for_wpos(test_pos);
					if (point_data.byteValue != 0 || point_data.floatValue <= YARNVOXEL_TERRAIN_SURFACE) {
						continue;
					}

					bool has_clearance = true;
					int check_radius = Math::ceil(required_clearance);

					for (int cx = x - check_radius; cx <= x + check_radius && has_clearance; ++cx) {
						for (int cy = y - check_radius; cy <= y + check_radius && has_clearance; ++cy) {
							for (int cz = z - check_radius; cz <= z + check_radius && has_clearance; ++cz) {
								Vector3 check_pos(cx, cy, cz);
								
								if (check_pos.distance_squared_to(test_pos) > clearance_squared) {
									continue;
								}

								auto check_data = get_point_data_for_wpos(check_pos);
								if (check_data.byteValue != 0 || check_data.floatValue <= YARNVOXEL_TERRAIN_SURFACE) {
									has_clearance = false;
									break;
								}
							}
						}
					}

					if (has_clearance) {
						return test_pos;
					}
				}
			}
		}
	}

	return center_pos;
}

void YarnVoxel::chunk_generated(Vector3i chunk_completed) {
	ticks_last_completed = OS::get_singleton()->get_ticks_usec() / 1000;
	last_chunk_completed = chunk_completed;
	if (is_generating) {
		is_generating = false;
		if (DirtyChunksQueue.size() <= 0) {
			emit_signal(SNAME("finished"));
		}
	}
}

void YarnVoxel::set_dirty_chunk(Vector3i chunkNumber) {
	if (!DirtyChunksQueue.has(chunkNumber)) {
		YVoxelChunk* chunk = get_chunk(chunkNumber);
		if (chunk) {
			DirtyChunksQueue.append(chunkNumber);
			chunk->set_process(true);
		}
	}
}

float YarnVoxel::static_perlin_noise(float x, float y) {
	return db::perlin(x,y);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
float YarnVoxel::static_perlin_noise_3d(float x, float y, float z) {
	return db::perlin(x,y,z);
}

void YarnVoxel::set_debug_pos(Vector3i val) {
	debug_pos = val;
}

int YarnVoxel::get_debug_pos() const {
	return debug_pos.x;
}

void YarnVoxel::set_cell_size(int val) {
	cellSize = val;
}

int YarnVoxel::get_cell_size() const {
	return cellSize;
}

void YarnVoxel::set_debugging_config(int val) {
	debugging_config = val;
}

int YarnVoxel::get_debugging_config() const {
	return debugging_config;
}

YarnVoxel::YarnVoxel() {
	// Initialize member variables
	count = 0;
	cellSize = 1;
	terrainSurface = 0.0f;
	ChunkWidth = 16;
	ChunkHeight = 16;
	debug_pos = Vector3i(0, 0, 0);
	DivideForChunkWidth = 1.0f / ChunkWidth;
	DivideForChunkHeight = 1.0f / ChunkHeight;
	BaseTerrainHeight = 0.0f;
	TerrainHeightRange = 32.0f;
	calculate_custom_normals = false;
	serialize_when_generating = false;
	simplification_distance = 0.0f;
	is_generating = false;
	is_debugging_chunk = false;
	is_triple_polycount = false;
	generate_grass = false;
	water_level = -1.8f;
}
