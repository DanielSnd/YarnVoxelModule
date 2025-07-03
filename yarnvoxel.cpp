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
	has_last_used_chunk = false;
	last_used_chunk = nullptr;
	last_used_chunk_number = Vector3i();
	for (const KeyValue<Vector3i, ObjectID>& E : yvchunks) {
		if (auto chunk = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(E.value))) {
			if (chunk != nullptr) {
				chunk->queue_free();
			}
		}
	}
	DirtyChunksQueue.clear();
	yvchunks.clear();
}

bool YarnVoxel::IsPositionValid(Vector3i pos) {
	return pos.x >= 0 && pos.x < YARNVOXEL_CHUNK_WIDTH && pos.y >= 0 && pos.y < YARNVOXEL_CHUNK_HEIGHT && pos.z >= 0 && pos.z < YARNVOXEL_CHUNK_WIDTH;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::CalculateChunkCenterPosition(Vector3i chunkPosition) {
	const auto bottom_corner = GetBottomCornerForChunkInNumber(chunkPosition);
	return bottom_corner + (right * (0.5 * YARNVOXEL_CHUNK_WIDTH)) + 
	       (forward * (0.5 * YARNVOXEL_CHUNK_WIDTH)) + 
	       (up * (0.5 * YARNVOXEL_CHUNK_HEIGHT));
}


// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3i YarnVoxel::FindChunkNumberFromPosition(const Vector3 pos) {
	return {FastFloor(pos.x / (YARNVOXEL_CHUNK_WIDTH)), 
	        FastFloor(pos.y / (YARNVOXEL_CHUNK_HEIGHT)), 
	        FastFloor(pos.z / (YARNVOXEL_CHUNK_WIDTH))};
}

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3i YarnVoxel::FindPointNumberFromPosition(Vector3 pos) {
	Vector3i PointNumber = Vector3i(static_cast<int>(Math::round(Math::fmod(pos.x, YARNVOXEL_CHUNK_WIDTH) )),
	static_cast<int>(Math::round(Math::fmod(pos.y, YARNVOXEL_CHUNK_HEIGHT) )),
	static_cast<int>(Math::round(Math::fmod(pos.z, YARNVOXEL_CHUNK_WIDTH) )));

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


// // ReSharper disable once CppMemberFunctionMayBeStatic
// Vector2i YarnVoxel::CalculateGridPosition(Vector3 worldPosition) {
// 	int x = static_cast<int>(Math::floor(worldPosition.x / static_cast<real_t>(cellSize)));
// 	int z = static_cast<int>(Math::floor(worldPosition.z / static_cast<real_t>(cellSize)));
// 	return {x, z};
// }

// // ReSharper disable once CppMemberFunctionMayBeStatic
// Vector3 YarnVoxel::CalculateCellCenterPosition(Vector2i gridPosition) {
// 	const float halfCellSize = static_cast<real_t>(cellSize) * 0.5f;
// 	float x = (static_cast<real_t>(gridPosition.x) * static_cast<real_t>(cellSize)) + (halfCellSize);
// 	float z = (static_cast<real_t>(gridPosition.y) * static_cast<real_t>(cellSize)) + (halfCellSize);
// 	return {x, 0, z};
// }

// ReSharper disable once CppMemberFunctionMayBeStatic
Vector3 YarnVoxel::GetBottomCornerForChunkInNumber(Vector3i ChunkNumber) {
	return {static_cast<real_t>(ChunkNumber.x) * YARNVOXEL_CHUNK_WIDTH, 
	        static_cast<real_t>(ChunkNumber.y) * YARNVOXEL_CHUNK_HEIGHT,
	        static_cast<real_t>(ChunkNumber.z) * YARNVOXEL_CHUNK_WIDTH};
}

bool YarnVoxel::try_get_chunk(Vector3i chunkPosition, YVoxelChunk*& chunk_pointer) {
	if (has_last_used_chunk && last_used_chunk_number == chunkPosition && last_used_chunk != nullptr) {
		chunk_pointer = last_used_chunk;
		return true;
	}

	if (!yvchunks.has(chunkPosition)) {
		return false;
	}

	chunk_pointer = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(yvchunks[chunkPosition]));
	//print_line("Try get chunk has found neighbour chunk ",chunkPosition," chunk pointer ",chunk_pointer);
	return true;
}

YVoxelChunk* YarnVoxel::get_chunk(Vector3i chunkPosition) {
	if (has_last_used_chunk && last_used_chunk_number == chunkPosition && last_used_chunk != nullptr) {
		return last_used_chunk;
	}
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
		// if (debugging_config >= 1) {
		// 	print_line(vformat("[YarnVoxel::get_chunk] Creating chunk at %s | YARNVOXEL_CHUNK_WIDTH: %d, YARNVOXEL_CHUNK_HEIGHT: %d", chunkPosition, YARNVOXEL_CHUNK_WIDTH, YARNVOXEL_CHUNK_HEIGHT));
		// }
		Vector3 bottom_corner = GetBottomCornerForChunkInNumber(chunkPosition);
		chunk->set_bottom_corner_world_pos(bottom_corner);
		chunk->set_global_position(bottom_corner);
		
		// if (debugging_config>= 1) {
		// 	print_line(vformat("[YarnVoxel::get_chunk] Bottom corner world pos for chunk %s: %s", chunkPosition, bottom_corner));
		// }
		return chunk;
	}
	return Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(yvchunks[chunkPosition]));
}

void YarnVoxel::set_material(const Ref<Material> &p_material) {
	material = p_material;
}

Ref<Material> YarnVoxel::get_material() {
	return material;
}

void YarnVoxel::changeFloat(float newFloat, uint8_t newBlockType, uint8_t health) {
	change_float_at_position(debug_pos, newFloat, newBlockType, health);
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
	ClassDB::bind_method(D_METHOD("set_material", "material"), &YarnVoxel::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &YarnVoxel::get_material);
	
	ClassDB::bind_method(D_METHOD("change_float_at_position", "position", "newFloat", "newBlockType", "health"), &YarnVoxel::change_float_at_position, DEFVAL(255));
	ClassDB::bind_method(D_METHOD("change_health_at_position", "position", "newHealth"), &YarnVoxel::change_health_at_position);

	ClassDB::bind_method(D_METHOD("add_line", "start", "end", "amount", "typeOfBlock", "thicknessStart", "thicknessEnd"), &YarnVoxel::add_line, DEFVAL(1), DEFVAL(1), DEFVAL(1));
	ClassDB::bind_method(D_METHOD("set_line", "start", "end", "value", "typeOfBlock", "thicknessStart", "thicknessEnd"), &YarnVoxel::set_line, DEFVAL(1), DEFVAL(1), DEFVAL(1));

	ClassDB::bind_method(D_METHOD("modify_voxel_area", "pos", "amount", "brushSize", "block_type"), &YarnVoxel::modify_voxel_area, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("smoothly_modify_voxel_area", "pos", "amount", "brushSize", "block_type"), &YarnVoxel::smoothly_modify_voxel_area, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("smooth_voxel_chunk", "chunk"), &YarnVoxel::smooth_voxel_chunk);

	ClassDB::bind_method(D_METHOD("damage_voxel_area", "pos", "amount", "brushSize"), &YarnVoxel::damage_voxel_area);
	ClassDB::bind_method(D_METHOD("smooth_voxel_area", "pos", "amount", "brushSize"), &YarnVoxel::smooth_voxel_area);
	ClassDB::bind_method(D_METHOD("find_closest_solid_point_to", "pos", "search_radius"), &YarnVoxel::find_closest_solid_point_to);
	ClassDB::bind_method(D_METHOD("find_air_position_with_clearance", "center_pos", "radius", "required_clearance"), &YarnVoxel::find_air_position_with_clearance);
	ClassDB::bind_method(D_METHOD("try_get_chunk", "chunk_number"), &YarnVoxel::get_chunk_from_gdscript);
	ClassDB::bind_method(D_METHOD("get_or_create_chunk", "chunk_number"), &YarnVoxel::get_chunk);
	ClassDB::bind_method(D_METHOD("set_dirty_chunk", "chunkNumber"), &YarnVoxel::set_dirty_chunk);
	ClassDB::bind_static_method("YarnVoxel",D_METHOD("perlin_noise", "x", "y"), &YarnVoxel::static_perlin_noise);
	ClassDB::bind_static_method("YarnVoxel",D_METHOD("perlin_noise_3d", "x", "y", "z"), &YarnVoxel::static_perlin_noise_3d);
	
	ClassDB::bind_method(D_METHOD("get_generate_grass"), &YarnVoxel::get_generate_grass);
	ClassDB::bind_method(D_METHOD("set_generate_grass", "val"), &YarnVoxel::set_generate_grass);
	ClassDB::bind_method(D_METHOD("get_grass_mesh"), &YarnVoxel::get_grass_mesh);
	ClassDB::bind_method(D_METHOD("set_grass_mesh", "val"), &YarnVoxel::set_grass_mesh);
	ClassDB::bind_method(D_METHOD("get_grass_material"), &YarnVoxel::get_grass_material);
	ClassDB::bind_method(D_METHOD("set_grass_material", "val"), &YarnVoxel::set_grass_material);
	ClassDB::bind_method(D_METHOD("set_calculate_custom_normals", "val"), &YarnVoxel::set_calculate_custom_normals);
	ClassDB::bind_method(D_METHOD("get_calculate_custom_normals"), &YarnVoxel::get_calculate_custom_normals);
	ClassDB::bind_method(D_METHOD("set_smooth_normal_angle", "angle"), &YarnVoxel::set_smooth_normal_angle);
	ClassDB::bind_method(D_METHOD("get_smooth_normal_angle"), &YarnVoxel::get_smooth_normal_angle);
	ClassDB::bind_method(D_METHOD("get_serialize_when_generating"), &YarnVoxel::get_serialize_when_generating);
	ClassDB::bind_method(D_METHOD("set_serialize_when_generating", "val"), &YarnVoxel::set_serialize_when_generating);
	ClassDB::bind_method(D_METHOD("set_smoothing", "val"), &YarnVoxel::set_smoothing);
	ClassDB::bind_method(D_METHOD("get_smoothing"), &YarnVoxel::get_smoothing);
	
	ClassDB::bind_method(D_METHOD("set_line_noise_strength", "strength"), &YarnVoxel::set_line_noise_strength);
	ClassDB::bind_method(D_METHOD("get_line_noise_strength"), &YarnVoxel::get_line_noise_strength);
	ClassDB::bind_method(D_METHOD("set_line_noise_frequency", "frequency"), &YarnVoxel::set_line_noise_frequency);
	ClassDB::bind_method(D_METHOD("get_line_noise_frequency"), &YarnVoxel::get_line_noise_frequency);

	ClassDB::bind_method(D_METHOD("get_is_triple_polycount"), &YarnVoxel::get_is_triple_polycount);
	ClassDB::bind_method(D_METHOD("set_is_triple_polycount", "val"), &YarnVoxel::set_is_triple_polycount, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("empty_all_chunks"), &YarnVoxel::empty_all_chunks);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial"), "set_material", "get_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "generate_grass"), "set_generate_grass", "get_generate_grass");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grass_mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh"), "set_grass_mesh", "get_grass_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "grass_material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial"), "set_grass_material", "get_grass_material");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "smoothing"), "set_smoothing", "get_smoothing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "calculate_custom_normals"), "set_calculate_custom_normals", "get_calculate_custom_normals");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "smooth_normal_angle", PROPERTY_HINT_RANGE, "0,180,0.1"), "set_smooth_normal_angle", "get_smooth_normal_angle");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "serialize_when_generating"), "set_serialize_when_generating", "get_serialize_when_generating");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_triple_polycount"), "set_is_triple_polycount", "get_is_triple_polycount");

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_noise_strength", PROPERTY_HINT_RANGE, "0.0,2.0,0.01"), "set_line_noise_strength", "get_line_noise_strength");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "line_noise_frequency", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_line_noise_frequency", "get_line_noise_frequency");
	

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

void YarnVoxel::change_health_at_position(Vector3i position, uint8_t newHealth) {
	YVoxelChunk* chunk = get_chunk(FindChunkNumberFromPosition(Vector3(position)));
	if (chunk) {
		Vector3i pointNumber = FindPointNumberFromPosition(Vector3(position));
		chunk->SetPointHealth(pointNumber, newHealth);
	}
}

void YarnVoxel::change_float_at_position(Vector3i position, float newFloat, uint8_t newBlockType, uint8_t health) {
	Vector3i desired_chunk_number = FindChunkNumberFromPosition(Vector3(position));
	if (has_last_used_chunk && last_used_chunk_number == desired_chunk_number && last_used_chunk != nullptr) {
		last_used_chunk->SetPointDensityTypeAndHealth(FindPointNumberFromPosition(Vector3(position)), newFloat, newBlockType, health);
		return;
	}
	YVoxelChunk* chunk = get_chunk(desired_chunk_number);
	if (chunk) {
		last_used_chunk = chunk;
		last_used_chunk_number = desired_chunk_number;
		has_last_used_chunk = true;
		Vector3i pointNumber = FindPointNumberFromPosition(Vector3(position));
		chunk->SetPointDensityTypeAndHealth(pointNumber, newFloat, newBlockType, health);
	}
}

void YarnVoxel::add_line(Vector3i start, Vector3i end, float amount, uint8_t typeOfBlock, int thicknessStart, int thicknessEnd) {
	modify_line(start, end, amount, typeOfBlock, thicknessStart, thicknessEnd, true);
}

void YarnVoxel::set_line(Vector3i start, Vector3i end, float value, uint8_t typeOfBlock, int thicknessStart, int thicknessEnd) {
	modify_line(start, end, value, typeOfBlock, thicknessStart, thicknessEnd, false);
}

void YarnVoxel::modify_line(Vector3i start, Vector3i end, float amount, uint8_t typeOfBlock, int thicknessStart, int thicknessEnd, bool adding) {
	
	Vector3i chunk_number = FindChunkNumberFromPosition(start);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return;

	int xDiff = Math::abs(end.x - start.x);
	int yDiff = Math::abs(end.y - start.y);
	int zDiff = Math::abs(end.z - start.z);

	int xStep = start.x == end.x ? 0 : (start.x < end.x ? 1 : -1);
	int yStep = start.y == end.y ? 0 : (start.y < end.y ? 1 : -1);
	int zStep = start.z == end.z ? 0 : (start.z < end.z ? 1 : -1);
	
	float totalStepCount = MAX(xDiff, MAX(yDiff, zDiff));

	for (int i = 0; i <= totalStepCount; i++)
	{
		float lerpInverse = Math::inverse_lerp(0, totalStepCount, i);
		int thickness =
			static_cast<int>(Math::lerp(thicknessStart, thicknessEnd,lerpInverse));
		int thicknessSquared = thickness * thickness;
		int xIteration = static_cast<int>(Math::lerp(0, xDiff, lerpInverse));
		int yIteration = static_cast<int>(Math::lerp(0, yDiff, lerpInverse));
		int zIteration = static_cast<int>(Math::lerp(0, zDiff, lerpInverse));
		
		int x = start.x + (int) (xStep * xIteration);
		int y = start.y + (int) (yStep * yIteration);
		int z = start.z + (int) (zStep * zIteration);

		if ((xStep < 0 && x <= end.x) || (xStep >= 0 && x >= end.x)) x = end.x;
		if ((yStep < 0 && y <= end.y) || (yStep >= 0 && y >= end.y)) y = end.y;
		if ((zStep < 0 && z <= end.z) || (zStep >= 0 && z >= end.z)) z = end.z;
		//Debug.Log($"Iteration {i} Spot {x},{y},{z}");

		for (int xThick = x - thickness; xThick <= x + thickness; xThick++)
		{
			int xThickSquared = (xThick - x) * (xThick - x);
			for (int yThick = y - thickness; yThick <= y + thickness; yThick++)
			{
				int yThickSquared = (yThick - y) * (yThick - y);
				for (int zThick = z - thickness; zThick <= z + thickness; zThick++)
				{
					int zThickSquared = (zThick - z) * (zThick - z);
					int totalcurrentThicknessSquared = Math::abs(xThickSquared + yThickSquared + zThickSquared);
					// Check if the thickened position is within bounds
					Vector3i desiredSpot = Vector3i(xThick, yThick, zThick);
					float lerpDistance = Math::inverse_lerp(0.0f, thicknessSquared, totalcurrentThicknessSquared);
					if (totalcurrentThicknessSquared <= thicknessSquared)
					{
						Vector3i check_chunk_number = FindChunkNumberFromPosition(Vector3(xThick, yThick, zThick));
						if (check_chunk_number != chunk_number) {
							modify_chunk->deferred_set_dirty();
							chunk_number = check_chunk_number;
							modify_chunk = get_chunk(chunk_number);
							if (!modify_chunk) continue;
						}

						Vector3i point_position = FindPointNumberFromPosition(Vector3(xThick, yThick, zThick));
						if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
							continue;
						}

						auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
						if (amount != 0.0f) {
							float noise = static_perlin_noise_3d(xThick * line_noise_frequency, yThick * line_noise_frequency, zThick * line_noise_frequency) * line_noise_strength;
							int16_t modifiedAmount = floatToInt16((1.0f - lerpDistance) * (amount + noise));
							if (adding) {
								if (modifiedAmount > 0 && point_value.floatValue > INT16_MAX - modifiedAmount) {
									point_value.floatValue = INT16_MAX;
								} else if (modifiedAmount < 0 && point_value.floatValue < INT16_MIN - modifiedAmount) {
									point_value.floatValue = INT16_MIN;
								} else {
									point_value.floatValue += modifiedAmount;
								}
							} else {
								point_value.floatValue = modifiedAmount;
							}
							
							if (point_value.byteValue == 0) {
								if (point_value.floatValue < YARNVOXEL_TERRAIN_SURFACE) {
									point_value.byteValue = typeOfBlock > 0 ? static_cast<uint8_t>(typeOfBlock) : 2;
								}
							} else if (point_value.floatValue > YARNVOXEL_TERRAIN_SURFACE) {
								point_value.byteValue = 0;
							}
						} else if (typeOfBlock > 0) {
							point_value.byteValue = static_cast<uint8_t>(typeOfBlock);
						}

						if (IsPointNumberInBoundary(point_position)) {
							modify_chunk->AttemptSetDirtyNeighbour(point_position);
						}
					}
				}
			}
		}
	}
	modify_chunk->deferred_set_dirty();
}

void YarnVoxel::modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type) {
	Vector3i chunk_number = FindChunkNumberFromPosition(pos);
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

				Vector3i check_chunk_number = FindChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					modify_chunk->deferred_set_dirty();
					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
					if (!modify_chunk) continue;
				}

				Vector3i point_position = FindPointNumberFromPosition(Vector3(x,y,z));
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


void YarnVoxel::smoothly_modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type) {
	Vector3i chunk_number = FindChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return;

	bool modify_float = Math::abs(amount) > 0.001f;
	float brushRadius = static_cast<float>(brushSize);
	float brushRadiusSquared = brushRadius * brushRadius;

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushRadiusSquared) continue;

				// Calculate smooth falloff using SDF approach
				float distance = Math::sqrt(distanceSquared);
				float normalizedDistance = distance / brushRadius;
				
				// Use a more aggressive falloff curve that maintains higher values
				// This ensures we get visible results on the first call
				float smoothFactor = 1.0f - Math::smoothstep(0.0f, 0.8f, normalizedDistance);
				
				// Alternative: Use a power curve for more aggressive falloff
				// float smoothFactor = Math::pow(1.0f - normalizedDistance, 2.0f);
				
				// Alternative: Use linear falloff with higher minimum
				// float smoothFactor = Math::max(0.3f, 1.0f - normalizedDistance);

				Vector3i check_chunk_number = FindChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					modify_chunk->deferred_set_dirty();
					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
					if (!modify_chunk) continue;
				}

				Vector3i point_position = FindPointNumberFromPosition(Vector3(x,y,z));
				if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
					continue;
				}

				auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
				if (modify_float) {
					// Apply smooth falloff to the modification amount
					float smoothAmount = amount * smoothFactor;
					int16_t modifiedAmount = floatToInt16(smoothAmount);
					
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
					// For block type changes, we can also apply smooth falloff
					// Only set block type if we're close enough to the center
					if (smoothFactor > 0.3f) {
						point_value.byteValue = static_cast<uint8_t>(block_type);
					}
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
	Vector3i chunk_number = FindChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = nullptr;
	if (!try_get_chunk(chunk_number, modify_chunk)) return false;

	Array emit_destroyed;
	Vector<Vector3i> should_be_dirty_chunks;
	bool damaged_any = false;
	bool damaged_current_chunk = false;
	float brushRadius = static_cast<float>(brushSize);
	float brushRadiusSquared = brushRadius * brushRadius;

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushRadiusSquared) continue;

				// Calculate smooth falloff for the damage brush
				float distance = Math::sqrt(distanceSquared);
				float normalizedDistance = distance / brushRadius;
				float damageFactor = 1.0f - Math::smoothstep(0.0f, 0.8f, normalizedDistance);

				Vector3i check_chunk_number = FindChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					if (damaged_current_chunk && !should_be_dirty_chunks.has(chunk_number)) {
						should_be_dirty_chunks.append(chunk_number);
					}
					damaged_current_chunk = false;
					chunk_number = check_chunk_number;
					if (!try_get_chunk(chunk_number, modify_chunk)) continue;
				}

				Vector3i point_position = FindPointNumberFromPosition(Vector3(x,y,z));
				if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
					continue;
				}

				auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
				if (point_value.byteValue == 0 || point_value.get_float_value_as_float() > 0.01f) {
					continue;
				}

				// Apply smooth falloff to damage amount
				uint8_t scaled_damage = static_cast<uint8_t>(amount * damageFactor);
				uint8_t damage_amount = scaled_damage < point_value.health ? scaled_damage : point_value.health;
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

void YarnVoxel::smooth_voxel_chunk(YVoxelChunk* chunk) {
	if (!chunk || !chunk->is_inside_tree()) return;

	int sampleRadius = 2; // Radius for neighbor sampling
	Vector3 bottom_corner = chunk->get_bottom_corner_world_pos();

	// Create a temporary array to store smoothed values to avoid affecting neighbor calculations
	// Using a flat array for better performance and to avoid const issues
	float smoothed_values[YARNVOXEL_CHUNK_WIDTH][YARNVOXEL_CHUNK_HEIGHT][YARNVOXEL_CHUNK_WIDTH];

	// First pass: calculate smoothed values for all voxels in the chunk
	for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; ++x) {
		for (int y = 0; y < YARNVOXEL_CHUNK_HEIGHT; ++y) {
			for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; ++z) {
				float current_value = chunk->points[x][y][z].get_float_value_as_float();

				// Sample neighbors for smoothing
				float neighbor_sum = 0.0f;
				float total_weight = 0.0f;
				int neighbor_count = 0;

				for (int nx = x - sampleRadius; nx <= x + sampleRadius; ++nx) {
					for (int ny = y - sampleRadius; ny <= y + sampleRadius; ++ny) {
						for (int nz = z - sampleRadius; nz <= z + sampleRadius; ++nz) {
							// Skip the center point itself
							if (nx == x && ny == y && nz == z) continue;

							float neighbor_value = 0.0f;
							bool valid_neighbor = false;

							// Check if neighbor is within this chunk
							if (nx >= 0 && nx < YARNVOXEL_CHUNK_WIDTH &&
								ny >= 0 && ny < YARNVOXEL_CHUNK_HEIGHT &&
								nz >= 0 && nz < YARNVOXEL_CHUNK_WIDTH) {
								neighbor_value = chunk->points[nx][ny][nz].get_float_value_as_float();
								valid_neighbor = true;
							} else {
								// Neighbor is in a different chunk, only sample if the chunk already exists
								Vector3 neighbor_world_pos = bottom_corner + Vector3(nx, ny, nz);
								Vector3i neighbor_chunk_number = FindChunkNumberFromPosition(neighbor_world_pos);
								YVoxelChunk* neighbor_chunk = nullptr;
								
								if (try_get_chunk(neighbor_chunk_number, neighbor_chunk)) {
									// Only sample if the neighbor chunk exists
									Vector3i neighbor_point_pos = FindPointNumberFromPosition(neighbor_world_pos);
									if (IsPositionValid(neighbor_point_pos)) {
										neighbor_value = neighbor_chunk->points[neighbor_point_pos.x][neighbor_point_pos.y][neighbor_point_pos.z].get_float_value_as_float();
										valid_neighbor = true;
									}
								}
								// If neighbor chunk doesn't exist, we skip this neighbor (don't create new chunks)
							}

							if (valid_neighbor) {
								float neighbor_dist = static_cast<float>(
									(nx - x) * (nx - x) + 
									(ny - y) * (ny - y) + 
									(nz - z) * (nz - z)
								);
								if (neighbor_dist <= sampleRadius * sampleRadius) {
									// Use inverse distance weighting for smoother results
									float weight = 1.0f / (1.0f + Math::sqrt(neighbor_dist));
									neighbor_sum += neighbor_value * weight;
									total_weight += weight;
									neighbor_count++;
								}
							}
						}
					}
				}

				// Calculate smoothed value
				float smoothed_value = current_value;
				if (total_weight > 0.0f && neighbor_count > 0) {
					smoothed_value = neighbor_sum / total_weight;
				}

				// Store the smoothed value for later application
				smoothed_values[x][y][z] = smoothed_value;
			}
		}
	}

	// Second pass: apply the smoothed values
	for (int x = 0; x < YARNVOXEL_CHUNK_WIDTH; ++x) {
		for (int y = 0; y < YARNVOXEL_CHUNK_HEIGHT; ++y) {
			for (int z = 0; z < YARNVOXEL_CHUNK_WIDTH; ++z) {
				YarnVoxelData::YVPointValue& point_value = chunk->points[x][y][z];
				float current_value = point_value.get_float_value_as_float();
				float smoothed_value = smoothed_values[x][y][z];

				// Apply smoothing (you can adjust the smoothing strength here)
				float smooth_strength = 0.5f; // 50% smoothing strength
				float new_value = Math::lerp(current_value, smoothed_value, smooth_strength);
				point_value.set_float_value_as_float(new_value);

				// Update block type based on surface threshold
				if (point_value.floatValue > YARNVOXEL_TERRAIN_SURFACE && point_value.byteValue != 0) {
					point_value.byteValue = 0;
				} else if (point_value.floatValue < YARNVOXEL_TERRAIN_SURFACE && point_value.byteValue == 0) {
					point_value.byteValue = 2; // Default block type
				}
			}
		}
	}

	// Mark the chunk as dirty for regeneration
	chunk->deferred_set_dirty();
}

void YarnVoxel::smooth_voxel_area(Vector3i pos, float amount, int brushSize) {
	Vector3i chunk_number = FindChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return;

	float brushRadius = static_cast<float>(brushSize);
	float brushRadiusSquared = brushRadius * brushRadius;
	int sampleRadius = 2; // Radius for neighbor sampling

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushRadiusSquared) continue;

				// Calculate smooth falloff for the brush
				float distance = Math::sqrt(distanceSquared);
				float normalizedDistance = distance / brushRadius;
				float smoothFactor = 1.0f - Math::smoothstep(0.0f, 0.8f, normalizedDistance);

				Vector3i check_chunk_number = FindChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					modify_chunk->deferred_set_dirty();
					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
					if (!modify_chunk) continue;
				}

				Vector3i point_position = FindPointNumberFromPosition(Vector3(x,y,z));
				if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x, point_position.y, point_position.z)) {
					continue;
				}

				auto& point_value = modify_chunk->points[point_position.x][point_position.y][point_position.z];
				float current_value = point_value.get_float_value_as_float();

				// Sample neighbors for smoothing
				float neighbor_sum = 0.0f;
				float neighbor_count = 0.0f;
				float total_weight = 0.0f;

				for (int nx = x - sampleRadius; nx <= x + sampleRadius; ++nx) {
					for (int ny = y - sampleRadius; ny <= y + sampleRadius; ++ny) {
						for (int nz = z - sampleRadius; nz <= z + sampleRadius; ++nz) {
							// Skip the center point itself
							if (nx == x && ny == y && nz == z) continue;

							float neighbor_dist = static_cast<float>(
								(nx - x) * (nx - x) + 
								(ny - y) * (ny - y) + 
								(nz - z) * (nz - z)
							);
							if (neighbor_dist > sampleRadius * sampleRadius) continue;

							float neighbor_value = 0.0f;
							bool valid_neighbor = false;

							// Check if neighbor is in the current chunk
							Vector3i neighbor_chunk_number = FindChunkNumberFromPosition(Vector3(nx,ny,nz));
							if (neighbor_chunk_number == chunk_number) {
								// Neighbor is in the same chunk
								Vector3i neighbor_point_pos = FindPointNumberFromPosition(Vector3(nx,ny,nz));
								if (IsPositionValid(neighbor_point_pos)) {
									neighbor_value = modify_chunk->points[neighbor_point_pos.x][neighbor_point_pos.y][neighbor_point_pos.z].get_float_value_as_float();
									valid_neighbor = true;
								}
							} else {
								// Neighbor is in a different chunk, only sample if the chunk already exists
								YVoxelChunk* neighbor_chunk = nullptr;
								if (try_get_chunk(neighbor_chunk_number, neighbor_chunk)) {
									// Only sample if the neighbor chunk exists
									Vector3i neighbor_point_pos = FindPointNumberFromPosition(Vector3(nx,ny,nz));
									if (IsPositionValid(neighbor_point_pos)) {
										neighbor_value = neighbor_chunk->points[neighbor_point_pos.x][neighbor_point_pos.y][neighbor_point_pos.z].get_float_value_as_float();
										valid_neighbor = true;
									}
								}
								// If neighbor chunk doesn't exist, we skip this neighbor (don't create new chunks)
							}

							if (valid_neighbor) {
								// Use inverse distance weighting for smoother results
								float weight = 1.0f / (1.0f + Math::sqrt(neighbor_dist));
								neighbor_sum += neighbor_value * weight;
								total_weight += weight;
								neighbor_count += 1.0f;
							}
						}
					}
				}

				// Calculate smoothed target value
				float smoothed_value = current_value;
				if (total_weight > 0.0f && neighbor_count > 0.0f) {
					smoothed_value = neighbor_sum / total_weight;
				}

				// Apply smoothing with falloff
				float final_smooth_factor = amount * smoothFactor;
				final_smooth_factor = CLAMP(final_smooth_factor, 0.0f, 1.0f);
				
				float new_value = Math::lerp(current_value, smoothed_value, final_smooth_factor);
				point_value.set_float_value_as_float(new_value);

				// Update block type based on surface threshold
				if (point_value.floatValue > YARNVOXEL_TERRAIN_SURFACE && point_value.byteValue != 0) {
					point_value.byteValue = 0;
				} else if (point_value.floatValue < YARNVOXEL_TERRAIN_SURFACE && point_value.byteValue == 0) {
					point_value.byteValue = 2; // Default block type
				}

				if (IsPointNumberInBoundary(point_position)) {
					modify_chunk->AttemptSetDirtyNeighbour(point_position);
				}
			}
		}
	}
	modify_chunk->deferred_set_dirty();
}

Array YarnVoxel::find_closest_solid_point_to(Vector3 pos, int search_radius) {
	Array return_array;
	Vector3i chunk_number = FindChunkNumberFromPosition(pos);
	YVoxelChunk* modify_chunk = get_chunk(chunk_number);
	if (!modify_chunk) return return_array;

	float best_distance = 999999.0f;
	bool found_any = false;
	Vector3i best_solid_pos = Vector3i(0,0,0);

	for (int x = pos.x - search_radius; x <= pos.x + search_radius; ++x) {
		for (int y = pos.y - search_radius; y <= pos.y + search_radius; ++y) {
			for (int z = pos.z - search_radius; z <= pos.z + search_radius; ++z) {
				Vector3 current_checking = Vector3(x,y,z);
				Vector3i found_chunk_number = FindChunkNumberFromPosition(current_checking);
				
				YVoxelChunk* check_chunk = modify_chunk;
				if (found_chunk_number != chunk_number) {
					check_chunk = get_chunk(found_chunk_number);
					if (!check_chunk) continue;
				}
				
				Vector3i find_point_number = FindPointNumberFromPosition(current_checking);
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
			// All chunks are done generating, synchronize edge normals across all chunks
			// if (debugging_config > 1) {
			// 	print_line(vformat("[YarnVoxel] All chunks generated, synchronizing edge normals across %d chunks", yvchunks.size()));
			// }
			
			// Synchronize edge normals for all chunks
			for (const KeyValue<Vector3i, ObjectID>& E : yvchunks) {
				if (auto chunk = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(E.value))) {
					if (chunk && chunk->is_inside_tree()) {
						chunk->synchronize_edge_normals_with_neighbors();
					}
				}
			}

			// After synchronization, update all chunk meshes with the new normals
			for (const KeyValue<Vector3i, ObjectID>& E : yvchunks) {
				if (auto chunk = Object::cast_to<YVoxelChunk>(ObjectDB::get_instance(E.value))) {
					if (chunk && chunk->is_inside_tree()) {
						chunk->update_mesh_from_data();
					}
				}
			}
			
			emit_signal(SNAME("finished"));
		}
	}
}

void YarnVoxel::set_dirty_chunk(Vector3i chunkNumber) {
	if (!DirtyChunksQueue.has(chunkNumber)) {
		YVoxelChunk* chunk = nullptr;
		if (try_get_chunk(chunkNumber, chunk)) {
			DirtyChunksQueue.append(chunkNumber);
			chunk->set_process(true);
		}
	}
}

void YarnVoxel::set_dirty_chunk_with_pointer(YVoxelChunk* chunk, Vector3i chunkNumber) {
	if (!DirtyChunksQueue.has(chunkNumber)) {
		if (chunk != nullptr) {
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

YarnVoxel::YarnVoxel() {
	// Initialize member variables
	count = 0;
	last_used_chunk = nullptr;
	last_used_chunk_number = Vector3i();
	has_last_used_chunk = false;

	debug_pos = Vector3i(0, 0, 0);
	calculate_custom_normals = false;
	serialize_when_generating = true;
	
	smooth_normal_angle = 30.0f;
	is_generating = false;
	
	is_triple_polycount = false;
	generate_grass = false;
	water_level = -1.8f;
}
