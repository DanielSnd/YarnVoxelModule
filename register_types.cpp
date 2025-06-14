/* register_types.cpp */

#include "register_types.h"
#include "constants.h"
#include "ige_curve.h"
#include "ige_falloff.h"
#include "ige_fastnoise.h"
#include "ige_mathoperation.h"
#include "ige_sample_image.h"
#include "ige_terrace.h"
#include "island_generator.h"
#include "island_gen_effect.h"
#include "core/object/class_db.h"
#include "yarnvoxel.h"
#include "yvoxelchunk.h"
#include "island_gen_node.h"

#ifdef TOOLS_ENABLED
#include "islandgen_editor_plugin.h"
#include "yarnvoxel_editor.h"
#include "editor/plugins/editor_plugin.h"
#endif

using namespace godot;

void initialize_yarnvoxel_module(ModuleInitializationLevel p_level) {
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ClassDB::register_class<YarnVoxelEditorPlugin>();
		EditorPlugins::add_by_type<YarnVoxelEditorPlugin>();
		EditorPlugins::add_by_type<IslandGeneratorEditorPlugin>();
		EditorPlugins::add_by_type<IslandGenEffectEditorPlugin>();
		ClassDB::register_class<YVoxelChunkEditorPlugin>();
		ClassDB::register_class<YVoxelEditorBottomPanel>();
		EditorPlugins::add_by_type<YVoxelChunkEditorPlugin>();
	}
#endif

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
	ClassDB::register_class<YarnVoxel>();
	ClassDB::register_class<YVoxelChunk>();
	ClassDB::register_class<IslandGeneratorHelper>();
	ClassDB::register_class<IslandGenerator>();
	ClassDB::register_class<IslandGenEffect>();
	ClassDB::register_class<IGE_FastNoise>();
	ClassDB::register_class<IGE_SampleImage>();
	ClassDB::register_class<IGE_Falloff>();
	ClassDB::register_class<IGE_MathOperation>();
	ClassDB::register_class<IGE_Curve>();
	ClassDB::register_class<IGE_Terrace>();
}

void uninitialize_yarnvoxel_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
}