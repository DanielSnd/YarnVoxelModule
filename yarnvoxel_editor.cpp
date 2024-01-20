// #ifndef TOOLS_ENABLED
// #define TOOLS_ENABLED
// #endif
#include "yvoxelchunk.h"
#include "editor/editor_inspector.h"
#include "editor/editor_scale.h"
#include "scene/gui/button.h"
#ifdef TOOLS_ENABLED

#include "yarnvoxel_editor.h"
#include "constants.h"

using namespace godot;

void YarnVoxelEditorPlugin::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
            ProjectSettings &ps = *ProjectSettings::get_singleton();
            add_setting(ps,
                  vformat("%s/%s/%s", YARNVOXEL_SETTINGS_BASE_PATH, INITIALIZE_BASE_PATH, CELL_SIZE_OPTION),
                  256,
                  Variant::Type::INT
                );

            ps.get_singleton()->save();
        } break;
        case NOTIFICATION_EXIT_TREE: {
        } break;
    }
}

void YarnVoxelEditorPlugin::add_setting(ProjectSettings& ps, const String& p_name, const Variant& p_default_value, Variant::Type p_type,
PropertyHint p_hint, const String& p_hint_string, int p_usage, bool requires_restart) {
    if (!ps.get_singleton()->has_setting(p_name)) {
        ps.get_singleton()->set_setting(p_name, p_default_value);
    }
    ps.get_singleton()->set_custom_property_info(PropertyInfo(p_type, p_name, p_hint, p_hint_string,p_usage));
    ps.set_initial_value(p_name, p_default_value);
    ps.set_restart_if_changed(p_name, requires_restart);
}

auto YarnVoxelEditorPlugin::_bind_methods() -> void {}


/////////////////////////////////////////////////////////////////////////////////

class YVoxelChunkEditorButtons : public Control {
	GDCLASS(YVoxelChunkEditorButtons, Control)

	YVoxelChunk* _yvoxelchunk;
	static const int PREVIEW_HEIGHT = 32;
	static const int PADDING_PREVIEW_INFO = 2;
	Button *_populate_terrain_button = nullptr;
	// Button *_3d_space_switch = nullptr;

public:
	YVoxelChunkEditorButtons() {
		set_custom_minimum_size(Size2(0, EDSCALE * (PREVIEW_HEIGHT)));
		_populate_terrain_button = memnew(Button);
		_populate_terrain_button->set_text(TTR("Populate Chunk"));
		_populate_terrain_button->set_tooltip_text(TTR("Populates the chunk with 3d noise."));
		_populate_terrain_button->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_populate_terrain_button->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 5);
		_populate_terrain_button->connect("pressed", callable_mp(this, &YVoxelChunkEditorButtons::_on_populate_chunk_button_pressed));
		add_child(_populate_terrain_button);
	}

	void _on_populate_chunk_button_pressed() {
		_yvoxelchunk->populate_chunk_3d();
	}

	void set_yvoxelchunk(YVoxelChunk* p_yvoxelchunk) {
		if (_yvoxelchunk == p_yvoxelchunk) {
			return;
		}
		_yvoxelchunk = p_yvoxelchunk;
		if (_yvoxelchunk!= nullptr) {

		}
	}

private:
		void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_RESIZED: {
			} break;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////

class YVoxelChunkInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(YVoxelChunkInspectorPlugin, EditorInspectorPlugin)
public:
	bool can_handle(Object *p_object) override {
		return Object::cast_to<YVoxelChunk>(p_object) != nullptr;
	}

	void parse_begin(Object *p_object) override {
		if (auto *yvxlptr = Object::cast_to<YVoxelChunk>(p_object)) {
				YVoxelChunkEditorButtons *viewer = memnew(YVoxelChunkEditorButtons);
				viewer->set_yvoxelchunk(yvxlptr);
				add_custom_control(viewer);
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

String YVoxelChunkEditorPlugin::get_name() const {
	return YVoxelChunkEditorPlugin::get_class_static();
}

YVoxelChunkEditorPlugin::YVoxelChunkEditorPlugin() {
	Ref<YVoxelChunkInspectorPlugin> plugin;
	plugin.instantiate();
	add_inspector_plugin(plugin);
}


#endif