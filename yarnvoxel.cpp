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

YarnVoxel* YarnVoxel::singleton;
HashMap<Vector3i, YVoxelChunk*> YarnVoxel::yvchunks;
Ref<Material> material;

YarnVoxel* YarnVoxel::get_singleton() {
	return singleton;
}

void YarnVoxel::empty_all_chunks() {
	for (const KeyValue<Vector3i, YVoxelChunk*> &E : yvchunks) {
		if (E.value != nullptr) {
			for (int i = 0; i <= YARNVOXEL_CHUNK_WIDTH; ++i) {
				for (int j = 0; j <= YARNVOXEL_CHUNK_HEIGHT; ++j) {
					for (int k = 0; k <= YARNVOXEL_CHUNK_WIDTH; ++k) {
						E.value->points[i][j][k] = YarnVoxelData::YVPointValue(); // Assuming YVPointValue has a default constructor
					}
				}
			}
			E.value->deferred_set_dirty();
		}
	}
	//WARN_PRINT("Cleared all chunks");
}
void YarnVoxel::clear_all_chunks() {
	for (const KeyValue<Vector3i, YVoxelChunk*> &E : yvchunks) {
		if (E.value != nullptr) {
			E.value->queue_free();
		}
	}
	yvchunks.clear();
	//WARN_PRINT("Cleared all chunks");
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
		YVoxelChunk* chunk = memnew(YVoxelChunk);\
		chunk->set_collision_layer_value(1,true);
		chunk->generate_grass = generate_grass;
		chunk->set_name(vformat("VoxelChunk %s",chunkPosition));
		auto parent_node = get_main_node();
		if (parent_node == nullptr) {
			ERR_PRINT("[YarnVoxel] Main node not found");
			// *((char *)0) = '\0';
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
	if(material.is_null() || !material.is_valid()) {
		if (!default_material_path.is_empty() && default_material_path.is_resource_file()) {
			material = ResourceLoader::load(default_material_path, "Material");
		} else {
			Ref<ShaderMaterial> preview_material;
			preview_material.instantiate();
			Ref<Shader> preview_material_shader;
			preview_material_shader.instantiate();
			preview_material_shader->set_code(R"(
shader_type spatial;
render_mode depth_draw_opaque,cull_back;
uniform sampler2D edge_noise:source_color;
uniform sampler2D gradient:source_color;
uniform float water_level = -1.8;
uniform vec3 hit_position = vec3(0.0);
uniform float hit_intensity = 0.0;
uniform float hit_radius = 5.0;
varying float hit_effect;
varying vec3 uv1_edge_noise_pos; varying vec3 uv1_power_normal;
varying vec3 world_pos; varying vec3 sand_tex; varying vec3 rock_tex;
varying vec3 dirt_tex; varying vec3 grass_tex; varying vec3 water_tex;

void vertex() {
	uv1_power_normal = pow(abs(mat3(MODEL_MATRIX) * NORMAL),vec3(0.5));
	vec3 vertex_world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
	world_pos = vertex_world_pos;
	
	// Calculate hit effect
	float dist_to_hit = distance(vertex_world_pos, hit_position);
	float hit_factor = 1.0 - smoothstep(0.0, hit_radius, dist_to_hit);
	hit_effect = hit_factor * hit_intensity;
	
	// Apply wobble displacement
	if (hit_effect > 0.0) {
		float wobble = sin(dist_to_hit * 3.0 - TIME * 10.0) * hit_effect * 0.5;
		vec3 dir_to_hit = normalize(vertex_world_pos - hit_position);
		VERTEX += (NORMAL * wobble + dir_to_hit * wobble * 0.5);
	}
	
	uv1_edge_noise_pos = world_pos * 0.15;
	uv1_power_normal/=dot(uv1_power_normal,vec3(1.0));
	sand_tex = texture(gradient,vec2(0.256,0.5)).rgb;
	dirt_tex = texture(gradient,vec2(0.33,0.5)).rgb;
	rock_tex = texture(gradient,vec2(0.98,0.5)).rgb;
	grass_tex = texture(gradient,vec2(0.5,0.5)).rgb;
	water_tex = texture(gradient,vec2(0.01,0.5)).rgb;
}
vec4 triplanar_texture(sampler2D p_sampler,vec3 p_weights,vec3 p_triplanar_pos) {
	vec4 samp=vec4(0.0);
	samp+= texture(p_sampler,p_triplanar_pos.xy) * p_weights.z;
	samp+= texture(p_sampler,p_triplanar_pos.xz) * p_weights.y;
	samp+= texture(p_sampler,p_triplanar_pos.zy * vec2(-1.0,1.0)) * p_weights.x;
	return samp;
}

float mask_to_mix(float vcolor,float minn, float maxx) {
	return smoothstep(minn,maxx+0.5,clamp(vcolor,0.0,1.0));
}

void fragment() {
	float original_noisy_blobs = triplanar_texture(edge_noise,uv1_power_normal,uv1_edge_noise_pos).r;
	float noisy_blobs = mask_to_mix(original_noisy_blobs,0.12,0.2);
	vec3 mixed_grassdirt = mix(grass_tex,dirt_tex,mask_to_mix(COLOR.g + (noisy_blobs * 0.55),0.55,0.581));
	vec3 mixed_previouswithrock = mix(mixed_grassdirt,rock_tex,mask_to_mix(COLOR.b,0.301,0.325));
	vec3 mixed_previouswithsand = mix(mixed_previouswithrock,sand_tex,mask_to_mix(COLOR.r + (original_noisy_blobs * 0.4) ,0.31,0.325));
	vec3 mixed_previouswithwater = mix(mixed_previouswithsand,water_tex,mask_to_mix((1.0-smoothstep(water_level,water_level+0.2,world_pos.y)) + (original_noisy_blobs * 0.8) ,0.58,0.325));
	
	// Add hit effect glow
	vec3 final_color = mixed_previouswithwater;
	if (hit_effect > 0.0) {
		float glow = hit_effect * 0.5;
		final_color = mix(final_color, vec3(1.0, 0.2, 0.1), glow);
	}
	
	ALBEDO = final_color;
}
)");
			preview_material->set_shader(preview_material_shader);
			Ref<NoiseTexture2D> noise_texture_2d;
			noise_texture_2d.instantiate();
			Ref<FastNoiseLite> temp_fast_noise_lite;
			temp_fast_noise_lite.instantiate();
			Ref<Gradient> temp_color_ramp;
			temp_color_ramp.instantiate();
			noise_texture_2d->set_color_ramp(temp_color_ramp);
			noise_texture_2d->set_noise(temp_fast_noise_lite);
			preview_material->set_shader_parameter("edge_noise",noise_texture_2d);
			Ref<GradientTexture1D> temp_grad_texture;
			temp_grad_texture.instantiate();
			Ref<Gradient> temp_terrain_colors;
			temp_terrain_colors.instantiate();
			temp_terrain_colors->set_interpolation_mode(Gradient::GRADIENT_INTERPOLATE_CUBIC);
			temp_terrain_colors->set_offsets({0, 0.2125, 0.25625, 0.320833, 0.39375, 0.795833, 0.827083});
			temp_terrain_colors->set_colors({Color(0.369637, 0.613445, 0.951805, 1), Color(0.501961, 0.698039, 0.972549, 1), Color(0.972549, 0.776471, 0.501961, 1), Color(0.990579, 0.69581, 0.404316, 1), Color(0.467362, 0.714254, 0.467767, 1), Color(0.653113, 0.884651, 0.647313, 1), Color(0.674644, 0.911331, 0.926547, 1)});
			temp_grad_texture->set_gradient(temp_terrain_colors);
			preview_material->set_shader_parameter("gradient",temp_grad_texture);
			using_default_shader = true;
			material = preview_material;
		}
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

	ClassDB::bind_method(D_METHOD("damage_voxel_area","world_pos","amount","size"), &YarnVoxel::damage_voxel_area,DEFVAL(30),DEFVAL(1));
	ClassDB::bind_method(D_METHOD("smooth_voxel_area","world_pos","amount","size"), &YarnVoxel::smooth_voxel_area,DEFVAL(1),DEFVAL(1));
	//void YarnVoxel::damage_voxel_area(Vector3i pos, uint8_t amount, int brushSize) {

	ClassDB::bind_method(D_METHOD("get_float_value","worldPostion"), &YarnVoxel::FindFloatValueForPosFloat);
	ClassDB::bind_method(D_METHOD("get_health_value","worldPostion"), &YarnVoxel::FindHealthValueForPos);
	ClassDB::bind_method(D_METHOD("get_block_type","worldPostion"), &YarnVoxel::FindBlockTypeForPos);

	ClassDB::bind_method(D_METHOD("get_dirty_chunks_queued"), &YarnVoxel::get_dirty_chunks_queued);
	ClassDB::bind_method(D_METHOD("dirty_chunk_queue_info"), &YarnVoxel::dirty_chunk_queue_info);
	
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

	ClassDB::bind_method(D_METHOD("set_calculate_custom_normals", "enabled"), &YarnVoxel::set_calculate_custom_normals, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("get_calculate_custom_normals"), &YarnVoxel::get_calculate_custom_normals);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "calculate_custom_normals", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_calculate_custom_normals", "get_calculate_custom_normals");

	ClassDB::bind_method(D_METHOD("set_serialize_when_generating", "enabled"), &YarnVoxel::set_serialize_when_generating);
	ClassDB::bind_method(D_METHOD("get_serialize_when_generating"), &YarnVoxel::get_serialize_when_generating);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "serialize_when_generating", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_serialize_when_generating", "get_serialize_when_generating");

	ClassDB::bind_method(D_METHOD("get_cell_size"), &YarnVoxel::get_cell_size);
	ClassDB::bind_method(D_METHOD("set_cell_size","cell_size"), &YarnVoxel::set_cell_size);

	ADD_PROPERTY(PropertyInfo(Variant::INT,"cell_size", PROPERTY_HINT_RANGE,"32,1024,32"),"set_cell_size","get_cell_size");

	ClassDB::bind_method(D_METHOD("set_value", "value","block_type","health"), &YarnVoxel::changeFloat, DEFVAL(GRASS),DEFVAL(255));
	ClassDB::bind_method(D_METHOD("set_value_at","position", "value","block_type","health"), &YarnVoxel::changeFloatAtPosition, DEFVAL(GRASS),DEFVAL(255));

	ClassDB::bind_method(D_METHOD("get_is_debugging_chunk"), &YarnVoxel::get_is_debugging_chunk);
	ClassDB::bind_method(D_METHOD("set_is_debugging_chunk","debug_chunk"), &YarnVoxel::set_is_debugging_chunk);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL,"is_debugging_chunk",PROPERTY_HINT_NONE),"set_is_debugging_chunk","get_is_debugging_chunk");


	ClassDB::bind_method(D_METHOD("find_closest_solid_point_to","world_position", "search_radius"), &YarnVoxel::find_closest_solid_point_to, DEFVAL(2));

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

	ClassDB::bind_method(D_METHOD("find_air_position_with_clearance", "center_pos", "radius", "required_clearance"), 
		&YarnVoxel::find_air_position_with_clearance);

	ADD_SIGNAL(MethodInfo("finished"));
	ADD_SIGNAL(MethodInfo("destroyed_voxels", PropertyInfo(Variant::ARRAY, "blocks")));

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

void YarnVoxel::_notification(int p_what) {
	switch (p_what) {
		case Node::NOTIFICATION_PROCESS : {
			//print_line(vformat("Doing process %d",OS::get_singleton()->get_ticks_msec()));
		} break;
		default:
			break;
	}
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

// Function to modify terrain by digging with a configurable brush size
void YarnVoxel::modify_voxel_area(Vector3i pos, float amount, int brushSize, int block_type) {
	// Iterate over a cubic area around the center point
	Vector3i chunk_number = GetChunkNumberFromPosition(pos);
	YVoxelChunk *modify_chunk = get_chunk(chunk_number);
	bool modify_float = Math::abs(amount) > 0.001f;

	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				// Calculate the distance from the current voxel to the center point
				float distance = Math::sqrt(static_cast<float>((x - pos.x) * (x - pos.x) + (y - pos.y) * (y - pos.y) + (z - pos.z) * (z - pos.z)));
				if (distance < static_cast<float>(brushSize)) {
					int16_t modifiedAmount = floatToInt16(amount);

				Vector3i check_chunk_number = GetChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
						modify_chunk->deferred_set_dirty();

					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
				}


				auto point_position = YarnVoxel::GetPointNumberFromPosition(Vector3(x,y,z));
					if (modify_chunk->is_point_position_in_range_without_neighbours(point_position.x,point_position.y,point_position.z)) {



				auto chunkpointvalue = modify_chunk->points[point_position.x][point_position.y][point_position.z];
						if (modify_float) {
							if (modifiedAmount > 0 && chunkpointvalue.floatValue > INT16_MAX - modifiedAmount) {
								chunkpointvalue.floatValue = INT16_MAX;
							} else if (modifiedAmount < 0 && chunkpointvalue.floatValue < INT16_MIN - modifiedAmount) {
								chunkpointvalue.floatValue = INT16_MIN;
							} else {
								chunkpointvalue.floatValue = chunkpointvalue.floatValue + modifiedAmount;
							}
							if (chunkpointvalue.byteValue == 0) {
								if (chunkpointvalue.floatValue < YARNVOXEL_TERRAIN_SURFACE)
									chunkpointvalue.byteValue = block_type > 0 ? static_cast<uint8_t>(block_type) : 2;
							} else {
								if (chunkpointvalue.floatValue > YARNVOXEL_TERRAIN_SURFACE)

					chunkpointvalue.byteValue = 0;

				}
						} else {
							if (block_type > 0) {
								chunkpointvalue.byteValue = static_cast<uint8_t>(block_type);
							}
						}
						//WARN_PRINT(vformat("Setting pos %s chunk %s point %s byte value %d float value %d",Vector3(x,y,z),chunk_number,point_position,chunkpointvalue.byteValue,chunkpointvalue.get_float_value_as_float()));
				modify_chunk->points[point_position.x][point_position.y][point_position.z] = chunkpointvalue;


				if(IsPointNumberInBoundary(point_position)) {
					modify_chunk->AttemptSetDirtyNeighbour(point_position);
				}
			}	
		}
	}

	}

	}
	modify_chunk->deferred_set_dirty();
}

// Function to modify terrain by damaging voxels in a spherical area
bool YarnVoxel::damage_voxel_area(Vector3i pos, uint8_t amount, int brushSize) {
	// Get the initial chunk at the center position
	Vector3i chunk_number = GetChunkNumberFromPosition(pos);
	YVoxelChunk *modify_chunk = nullptr;
	if (!try_get_chunk(chunk_number,modify_chunk))
		return false;

	// Track destroyed blocks for event emission and chunks that need updating
	Array emit_destroyed;
	Vector<Vector3i> should_be_dirty_chunks;
	bool damaged_any = false;
	bool damaged_current_chunk = false;

	// Calculate squared brush size for more efficient distance checks
	float brushSizeSquared = static_cast<float>(brushSize * brushSize);

	// Iterate over a cubic area around the center point
	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				// Use squared distance for efficiency (avoids square root)
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);

				// Skip if outside the spherical brush area
				if (distanceSquared >= brushSizeSquared) continue;

				// Check if we've moved to a new chunk
				Vector3i check_chunk_number = GetChunkNumberFromPosition(Vector3(x,y,z));
				if (check_chunk_number != chunk_number) {
					// Mark previous chunk as dirty if not already marked
					if (damaged_current_chunk && !should_be_dirty_chunks.has(chunk_number))
						should_be_dirty_chunks.append(chunk_number);
					damaged_current_chunk = false;
					// Switch to the new chunk
					chunk_number = check_chunk_number;
					modify_chunk = get_chunk(chunk_number);
				}

				// Get the point position within the chunk
				auto point_position = YarnVoxel::GetPointNumberFromPosition(Vector3(x,y,z));
				if (!modify_chunk->is_point_position_in_range_without_neighbours(point_position.x,point_position.y,point_position.z))
					continue;

				// Get the current voxel data
				auto chunkpointvalue = modify_chunk->points[point_position.x][point_position.y][point_position.z];
				
				// Skip empty or air voxels
				if (chunkpointvalue.byteValue == 0 || chunkpointvalue.get_float_value_as_float() > 0.01f)
					continue;

				// Apply damage to the voxel
				uint8_t damage_amount = amount < chunkpointvalue.health ? amount : chunkpointvalue.health;
				chunkpointvalue.health -= damage_amount;
				if (damage_amount > 0) {
					damaged_any = true;
					damaged_current_chunk = true;
					// Lerp the float value towards 0 based on remaining health percentage
					float current_float = chunkpointvalue.get_float_value_as_float();
					float health_percent = static_cast<float>(chunkpointvalue.health) / 255.0f;
					float target_float = Math::lerp(0.0f, current_float, health_percent);
					chunkpointvalue.set_float_value_as_float(target_float);
				}

				// If voxel is destroyed, reset it and track for event emission
				if (chunkpointvalue.health <= 1) {
					chunkpointvalue.health = 255;
					emit_destroyed.append(Vector4i(x,y,z,chunkpointvalue.byteValue));
					chunkpointvalue.byteValue = 0;
					chunkpointvalue.floatValue = INT16_MAX;
				}

				// Update the voxel data
				modify_chunk->points[point_position.x][point_position.y][point_position.z] = chunkpointvalue;

				// If point is on chunk boundary, update neighboring chunks
				if(IsPointNumberInBoundary(point_position)) {
					modify_chunk->AttemptSetDirtyNeighbour(point_position);
				}
			}
		}
	}

	// Emit event for destroyed blocks if any
	if (emit_destroyed.size() > 0) {
		emit_signal("destroyed_voxels",emit_destroyed);
	}

	if (damaged_any) {
		if (should_be_dirty_chunks.size() > 0) {
			// Mark all affected chunks as dirty
			for (int i = 0; i < should_be_dirty_chunks.size(); ++i) {
				set_dirty_chunk(should_be_dirty_chunks[i]);
			}
		} else {
			set_dirty_chunk(chunk_number);
		}
	}

	return damaged_any;
}

Array YarnVoxel::find_closest_solid_point_to(Vector3 pos, int search_radius) {
	Array return_array;
	Vector3i chunk_number = GetChunkNumberFromPosition(pos);
	YVoxelChunk *modify_chunk = nullptr;
	if (!try_get_chunk(chunk_number,modify_chunk))
		return return_array;
	float best_distance = 999999.0f;
	bool found_any = false;
	Vector3i best_solid_pos = Vector3i(0,0,0);
	
	// Search in a cube with sides of length 2*search_radius + 1
	for (int x = pos.x - search_radius; x <= pos.x + search_radius; ++x) {
		for (int y = pos.y - search_radius; y <= pos.y + search_radius; ++y) {
			for (int z = pos.z - search_radius; z <= pos.z + search_radius; ++z) {
				Vector3 current_checking = Vector3(x,y,z);
				auto found_chunk_number = GetChunkNumberFromPosition(current_checking);
				
				// If we're in a different chunk, try to get that chunk
				YVoxelChunk* check_chunk = modify_chunk;
				if (found_chunk_number != chunk_number) {
					if (!try_get_chunk(found_chunk_number, check_chunk))
						continue;
				}
				
				auto find_point_number = GetPointNumberFromPosition(current_checking);
				if (!YarnVoxel::IsPositionValid(find_point_number)) continue;
				
				YarnVoxelData::YVPointValue find_value = check_chunk->points[find_point_number.x][find_point_number.y][find_point_number.z];
				float float_value = int16ToFloat(find_value.floatValue);
				if (find_value.byteValue == 0 || float_value > 0.0f)
					continue;
				
				// Calculate the distance from the current voxel to the center point
				float distance = pos.distance_squared_to(Vector3(static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)));
				if (distance < best_distance) {
					best_distance = distance;
					best_solid_pos = Vector3i(x,y,z);
					found_any = true;
				}
			}
		}
	}
	
	if (found_any)
		return_array.append(best_solid_pos);
	return return_array;
}

// Function to perform terrain smoothing using weighted averaging in a spherical area
void YarnVoxel::smooth_voxel_area(Vector3i pos, float amount, int brushSize) {
	// Create temporary storage for smoothed values before applying them
	Vector<Vector3> temp_wpos;
	Vector<float> temp_grid;

	// Calculate squared brush size for more efficient distance checks
	float brushSizeSquared = static_cast<float>(brushSize * brushSize);

	// First pass: Calculate smoothed values for all affected voxels
	for (int x = pos.x - brushSize; x <= pos.x + brushSize; ++x) {
		for (int y = pos.y - brushSize; y <= pos.y + brushSize; ++y) {
			for (int z = pos.z - brushSize; z <= pos.z + brushSize; ++z) {
				// Check if current position is within the spherical brush area
				float distanceSquared = static_cast<float>(
					(x - pos.x) * (x - pos.x) + 
					(y - pos.y) * (y - pos.y) + 
					(z - pos.z) * (z - pos.z)
				);
				if (distanceSquared >= brushSizeSquared) continue;

				// Get the current voxel data
				auto initial_point_data = YarnVoxel::get_point_data_for_wpos(Vector3(x,y,z));
				float current_float_value = initial_point_data.get_float_value_as_float();
				
				// Skip if current voxel is empty/air
				if (current_float_value >= 0.0) continue;

				// Calculate weighted average of neighboring voxels
				float total = 0.0f;
				float weight_sum = 0.0f;

				// Sample neighboring voxels in a smaller radius
				int sample_radius = 2;
				for (int nx = x - sample_radius; nx <= x + sample_radius; ++nx) {
					for (int ny = y - sample_radius; ny <= y + sample_radius; ++ny) {
						for (int nz = z - sample_radius; nz <= z + sample_radius; ++nz) {
							// Calculate distance-based weight for this neighbor
							float neighbor_dist = static_cast<float>(
								(nx - x) * (nx - x) + 
								(ny - y) * (ny - y) + 
								(nz - z) * (nz - z)
							);
							if (neighbor_dist > sample_radius * sample_radius) continue;

							// Get neighbor's value and apply distance-based weight
							auto point_data = YarnVoxel::get_point_data_for_wpos(Vector3(nx,ny,nz));
							float weight = 1.0f / (1.0f + Math::sqrt(neighbor_dist));
							total += (point_data.get_float_value_as_float() + 1.0f) * weight;
							weight_sum += weight;
						}
					}
				}

				// Skip if no valid neighbors found
				if (weight_sum <= 0.0f) continue;

				// Store position and calculated average for later application
				temp_wpos.append(Vector3(x,y,z));
				temp_grid.append((total / weight_sum) - 1.0f);
			}
		}
	}

	// Second pass: Apply smoothed values
	YVoxelChunk *modify_chunk = nullptr;
	Vector3i current_chunk_number = FindChunkNumberFromPosition(pos);
	if (try_get_chunk(current_chunk_number, modify_chunk)) {
		modify_chunk->deferred_set_dirty();

		// Apply all calculated smoothed values
		for (int i = 0; i < temp_wpos.size(); i++) {
			if (i >= temp_grid.size()) break;

			Vector3 current_pos = temp_wpos[i];
			Vector3i chunk_number = FindChunkNumberFromPosition(current_pos);

			// Handle chunk transitions
			if (chunk_number != current_chunk_number) {
				if (try_get_chunk(chunk_number, modify_chunk)) {
					current_chunk_number = chunk_number;
					modify_chunk->deferred_set_dirty();
				} else {
					continue;
				}
			}

			// Update voxel with smoothed value
			const Vector3i block_number = FindPointNumberFromPosition(current_pos);
			if (IsPositionValid(block_number)) {
				auto point_data = modify_chunk->points[block_number.x][block_number.y][block_number.z];
				float current_float_value = point_data.get_float_value_as_float();

				// Calculate distance-based smoothing factor
				float dist_to_center = current_pos.distance_to(Vector3(pos));
				float smooth_factor = amount * (1.0f - (dist_to_center / static_cast<float>(brushSize)));
				smooth_factor = CLAMP(smooth_factor, 0.0f, 1.0f);

				// Interpolate between current and smoothed value
				point_data.set_float_value_as_float(Math::lerp(current_float_value, temp_grid[i], smooth_factor));

				// Update block type if necessary
				if (point_data.floatValue > YARNVOXEL_TERRAIN_SURFACE && point_data.byteValue != 0) {
					point_data.byteValue = 0;
				}

				// Apply the changes
				modify_chunk->points[block_number.x][block_number.y][block_number.z] = point_data;

				// Update neighboring chunks if necessary
				if(IsPointNumberInBoundary(block_number)) {
					modify_chunk->AttemptSetDirtyNeighbour(block_number);
				}
			}
		}
	}
}

bool YarnVoxel::IsPointNumberInBoundary(Vector3i bnumber) {
	return bnumber.x == 0 || bnumber.x >= YARNVOXEL_CHUNK_WIDTH - 1 ||bnumber.y == 0 || bnumber.y >= YARNVOXEL_CHUNK_HEIGHT - 1 ||bnumber.z == 0 || bnumber.z >= YARNVOXEL_CHUNK_WIDTH - 1;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
TypedArray<YVoxelChunk> YarnVoxel::get_chunks_from_gd_script() const {
	TypedArray<YVoxelChunk> arr;
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
	ticks_last_completed = OS::get_singleton()->get_ticks_usec() / 1000;
	last_chunk_completed = chunk_completed;
	if (is_generating) {
		is_generating=false;
		if (DirtyChunksQueue.size() <= 0) {
			emit_signal(SNAME("finished"));
		}
	}
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
	info += vformat("Dirty chunks queue size: %d is generating? %s",DirtyChunksQueue.size(), is_generating);
	info += vformat("Last Started: %s %s\n",ticks_last_started_generating, last_chunk_started_generating);
	info += vformat("Last Completed: %s %s\n",ticks_last_completed, last_chunk_completed);
	for (const Vector3i &chunk : DirtyChunksQueue) {
		info+= vformat("- Dirty chunk: %s\n",chunk);
	}
	return info;
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
	ticks_last_started_generating = OS::get_singleton()->get_ticks_usec() / 1000;
	const auto current_chunk = DirtyChunksQueue[0];
	last_chunk_started_generating = current_chunk;
	DirtyChunksQueue.remove_at(0);
	YVoxelChunk* found_chunk = nullptr;
	if(try_get_chunk(current_chunk, found_chunk)) {
		if (found_chunk->is_inside_tree()) {
			found_chunk->clear_triangles();
			found_chunk->connect(found_chunk->completed_generation,callable_chunk_completed_callback, CONNECT_ONE_SHOT);
			//print_line("connecting ",found_chunk->completed_generation," to ", callable_chunk_completed_callback);
			found_chunk->generate();
		} else {
			is_generating = false;
		}
	} else {
		WARN_PRINT(vformat("Failed to find chunk number %s. Current chunks in dictionary %d",current_chunk,yvchunks.size()));
		is_generating=false;
	}
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
		YVoxelChunk* ic = nullptr;
		if(try_get_chunk(chunkNumber,ic) && ic != nullptr) {
			DirtyChunksQueue.append(chunkNumber);
			ic->set_process(true);
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
#ifdef  TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint()) {
		if(yvchunks.size()>0) {
			main_node_pointer = Object::cast_to<Node3D>(yvchunks.last()->value->get_parent());
		} else {
			if (SceneTree::get_singleton() != nullptr) {
				main_node_pointer = Object::cast_to<Node3D>(SceneTree::get_singleton()->get_current_scene());
				if (main_node_pointer == nullptr) {
					if (EditorInterface::get_singleton()) {
						Node *current_scene = EditorInterface::get_singleton()->get_edited_scene_root();
						if (current_scene != nullptr) {
							TypedArray<Node> nchildren = current_scene->find_children("*","Node3D",true);
							if (nchildren.size() > 0) {
								auto found_main = Object::cast_to<Node3D>(nchildren.pop_front());
								if (found_main != nullptr) {
									main_node_pointer = found_main;
								}
							}
						}
					}
				}
			}
		}
		return main_node_pointer;
	}
#endif
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
	main_node_pointer->set_name("YVoxels");
	//print_line("[YVoxel] Created a YVoxel Node");
	return main_node_pointer;
}

void YarnVoxel::set_fallback_main_node(Node3D *fallback_main) {
	if (main_node_pointer == nullptr)
		main_node_pointer = fallback_main;
}

void YarnVoxel::create_main_node_under(Node3D* obj) {
	main_node_pointer = memnew(Node3D);
	if (main_node_pointer == nullptr) {
		ERR_PRINT("[YVoxel] Failed creating main node");
	}
	obj->add_child(main_node_pointer);
	main_node_pointer->set_owner(obj->get_owner() != nullptr ? obj->get_owner() : obj);
	main_node_pointer->set_name("YVoxels");
	//print_line(vformat("[YVoxel] Created main node %s",main_node_pointer->get_path()));
}

void YarnVoxel::set_main_node(Node3D* obj) {
	main_node_pointer = obj;
}

YarnVoxel::YarnVoxel() {
	is_generating=false;
    chunk_completed_callback = StaticCString::create("chunk_generated");
	callable_chunk_completed_callback = callable_mp(this,&YarnVoxel::chunk_generated);
	ticks_last_started_generating = 0;
	ticks_last_completed = 0;
	last_chunk_started_generating = Vector3i(0,0,0);
	last_chunk_completed = Vector3i(0,0,0);	
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
	GLOBAL_DEF_RST_BASIC("YarnVoxel/general/enabled", false);
	serialize_when_generating = GLOBAL_DEF_RST_BASIC("YarnVoxel/general/serialize_when_generating", true);
	is_triple_polycount = GLOBAL_DEF_RST_BASIC("YarnVoxel/general/use_triple_polycount", false);
	calculate_custom_normals = GLOBAL_DEF_RST_BASIC("YarnVoxel/general/calculate_custom_normals", false);
	default_material_path = GLOBAL_DEF(PropertyInfo(Variant::STRING, "YarnVoxel/general/default_material", PROPERTY_HINT_FILE, "*.tres,*.res", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), "");
	//print_line("default_material_path ",default_material_path);
	material = Ref<Material>();
}

YarnVoxel::~YarnVoxel() {
	material.unref();
	main_node_pointer = nullptr;
	grass_mesh.unref();
	grass_material.unref();
}

// Function to find a suitable air position with enough clearance
Vector3 YarnVoxel::find_air_position_with_clearance(Vector3 center_pos, int radius, float required_clearance) {
    // Convert required clearance to squared distance for efficiency
    float clearance_squared = required_clearance * required_clearance;
    
    // Search in expanding spheres from the center position
    for (int r = 0; r <= radius; r++) {
        // Calculate search bounds for this radius
        for (int x = center_pos.x - r; x <= center_pos.x + r; ++x) {
            for (int y = center_pos.y - r; y <= center_pos.y + r; ++y) {
                for (int z = center_pos.z - r; z <= center_pos.z + r; ++z) {
                    Vector3 test_pos(x, y, z);
                    
                    // Skip positions outside our search sphere
                    if (test_pos.distance_squared_to(center_pos) > radius * radius) {
                        continue;
                    }

                    // First quick check - is this position itself air?
                    auto point_data = get_point_data_for_wpos(test_pos);
                    if (point_data.byteValue != 0 || point_data.floatValue <= YARNVOXEL_TERRAIN_SURFACE) {
                        continue;
                    }

                    bool has_clearance = true;
                    int check_radius = Math::ceil(required_clearance);

                    // Efficient clearance check - only check points within the required clearance sphere
                    for (int cx = x - check_radius; cx <= x + check_radius && has_clearance; ++cx) {
                        for (int cy = y - check_radius; cy <= y + check_radius && has_clearance; ++cy) {
                            for (int cz = z - check_radius; cz <= z + check_radius && has_clearance; ++cz) {
                                Vector3 check_pos(cx, cy, cz);
                                
                                // Skip points outside our clearance sphere
                                if (check_pos.distance_squared_to(test_pos) > clearance_squared) {
                                    continue;
                                }

                                // Check if this position is solid
                                auto check_data = get_point_data_for_wpos(check_pos);
                                if (check_data.byteValue != 0 || check_data.floatValue <= YARNVOXEL_TERRAIN_SURFACE) {
                                    has_clearance = false;
                                    break;
                                }
                            }
                        }
                    }

                    // If we found a position with enough clearance, return it
                    if (has_clearance) {
                        return test_pos;
                    }
                }
            }
        }
    }

    // If no suitable position found, return the center position
    return center_pos;
}
