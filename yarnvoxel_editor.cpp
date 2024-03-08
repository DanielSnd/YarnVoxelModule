// #ifndef TOOLS_ENABLED
// #define TOOLS_ENABLED
// #endif
#include "editor/gui/editor_bottom_panel.h"
#ifdef TOOLS_ENABLED

#include "yvoxelchunk.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/button.h"

#include "yarnvoxel_editor.h"
#include "constants.h"

using namespace godot;

void YarnVoxelEditorPlugin::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
      //   	add_setting("yarnvoxel/general/enabled",
				  // false, Variant::Type::BOOL, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT,true);
            ProjectSettings::get_singleton()->save();
        	if(is_yvoxel_enabled) {
        		add_tool_menu_item("Enable voxel painting",callable_mp(this,&YarnVoxelEditorPlugin::enable_voxel_painting));
        		//
        		if (voxel_edit_button_holder == nullptr) {
        			voxel_edit_button_holder = memnew(Control);
        			voxel_edit_button_holder->set_custom_minimum_size(Size2(120,0));
        			voxel_edit_button_holder->set_name("voxel_editor_activate");
        			voxel_edit_button = memnew(Button);
        			voxel_edit_button_holder->add_child(voxel_edit_button);
        			voxel_edit_button->set_name("voxel_editor_activate_buton");
        			voxel_edit_button->set_toggle_mode(true);
        			voxel_edit_button->set_flat(true);
        			voxel_edit_button->set_text("Voxel Editor");
        			voxel_edit_button->connect("pressed",callable_mp(this,&YarnVoxelEditorPlugin::toggle_bottom_panel));
        			add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU,voxel_edit_button_holder);
        			voxel_edit_button_holder->hide();
        		}

        	}
        } break;
    	case NOTIFICATION_READY: {
    		if (is_yvoxel_enabled && voxel_edit_button_holder != nullptr) {
    			connect("main_screen_changed",callable_mp(this,&YarnVoxelEditorPlugin::editor_changed_scene));
    			set_input_event_forwarding_always_enabled();
    		}
    	}
        case NOTIFICATION_EXIT_TREE: {
        } break;
    	default:
    		break;
    }
}

void YarnVoxelEditorPlugin::toggle_bottom_panel() {
	// if (yvoxel_bottom_panel == nullptr) {
	// 	yvoxel_bottom_panel = memnew(YVoxelEditorBottomPanel);
	// }
	if (!yvoxel_bottom_panel_visible) {
		//EditorNode::get_bottom_panel()->add_item(TTR("YVoxel Editor"), yvoxel_bottom_panel);
		yvoxel_bottom_panel_visible = true;
	} else {
		//EditorNode::get_bottom_panel()->remove_item(yvoxel_bottom_panel);
		yvoxel_bottom_panel_visible = false;
	}
}

void YarnVoxelEditorPlugin::editor_changed_scene(const String &p_editor) {
	if (p_editor != "3D") {
		if (voxel_edit_button_holder != nullptr) {
			voxel_edit_button_holder->hide();
		}
		// if (yvoxel_bottom_panel != nullptr && yvoxel_bottom_panel_visible) {
		// 	EditorNode::get_bottom_panel()->remove_item(yvoxel_bottom_panel);
		// 	yvoxel_bottom_panel_visible = false;
		// }
	} else {
		if (voxel_edit_button_holder != nullptr) {
			voxel_edit_button_holder->show();
		}
	}
}


YarnVoxelEditorPlugin::YarnVoxelEditorPlugin() {
	voxel_edit_button_holder = nullptr;
	voxel_edit_button  = nullptr;
	// yvoxel_bottom_panel = nullptr;
	yvoxel_bottom_panel_visible = false;
	is_yvoxel_enabled = GLOBAL_DEF_RST(PropertyInfo(Variant::BOOL, "YarnVoxel/general/enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), "");
}

YarnVoxelEditorPlugin::~YarnVoxelEditorPlugin() {
	if (voxel_edit_button_holder != nullptr) {
		memdelete(voxel_edit_button_holder);
		voxel_edit_button_holder = nullptr;
	}
	if (voxel_edit_button != nullptr) {
		memdelete(voxel_edit_button);
		voxel_edit_button = nullptr;
	}
	// if (yvoxel_bottom_panel != nullptr) {
	// 	memdelete(yvoxel_bottom_panel);
	// 	yvoxel_bottom_panel = nullptr;
	// }
}

void YarnVoxelEditorPlugin::add_setting(const String& p_name, const Variant& p_default_value, Variant::Type p_type,
                                        PropertyHint p_hint, const String& p_hint_string, int p_usage,bool restart_if_changed) {
	if (!ProjectSettings::get_singleton()->has_setting(p_name)) {
		ProjectSettings::get_singleton()->set_setting(p_name, p_default_value);
	}
	ProjectSettings::get_singleton()->set_custom_property_info(PropertyInfo(p_type, p_name, p_hint, p_hint_string,p_usage));
	ProjectSettings::get_singleton()->set_initial_value(p_name, p_default_value);
	ProjectSettings::get_singleton()->set_restart_if_changed(p_name, restart_if_changed);
}


auto YarnVoxelEditorPlugin::_bind_methods() -> void {}

void YarnVoxelEditorPlugin::enable_voxel_painting() {
	remove_tool_menu_item("Enable voxel painting");
	add_tool_menu_item("Disable voxel painting",callable_mp(this,&YarnVoxelEditorPlugin::disable_voxel_painting));
}

void YarnVoxelEditorPlugin::disable_voxel_painting() {
	remove_tool_menu_item("Disable voxel painting");
	add_tool_menu_item("Enable voxel painting",callable_mp(this,&YarnVoxelEditorPlugin::enable_voxel_painting));
}

/////////////////////////////////////////////////////////////////////////////////

class YVoxelChunkEditorButtons : public Control {
	GDCLASS(YVoxelChunkEditorButtons, Control)

	YVoxelChunk* _yvoxelchunk;
	static constexpr int PREVIEW_HEIGHT = 78;
	static constexpr int PADDING_PREVIEW_INFO = 2;
	Button *_populate_terrain_button = nullptr;
	Button *_test_serialization_terrain_button = nullptr;
	Button *_test_float_stuff_button = nullptr;
	// Button *_3d_space_switch = nullptr;

public:
	YVoxelChunkEditorButtons() {
		_yvoxelchunk = nullptr;
		set_custom_minimum_size(Size2(0, EDSCALE * (PREVIEW_HEIGHT)));
		_populate_terrain_button = memnew(Button);
		_populate_terrain_button->set_text(TTR("Populate Chunk"));
		_populate_terrain_button->set_tooltip_text(TTR("Populates the chunk with 3d noise."));
		_populate_terrain_button->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_populate_terrain_button->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 5);
		_populate_terrain_button->connect("pressed", callable_mp(this, &YVoxelChunkEditorButtons::_on_populate_chunk_button_pressed));
		add_child(_populate_terrain_button);

		_test_serialization_terrain_button = memnew(Button);
		_test_serialization_terrain_button->set_text(TTR("Test Serialization Chunk"));
		_test_serialization_terrain_button->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_test_serialization_terrain_button->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 30);
		_test_serialization_terrain_button->connect("pressed", callable_mp(this, &YVoxelChunkEditorButtons::_on_test_serialize_chunk_button_pressed));
		add_child(_test_serialization_terrain_button);

		_test_float_stuff_button = memnew(Button);
		_test_float_stuff_button->set_text(TTR("Test Float Stuff"));
		_test_float_stuff_button->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO+ 145);
		_test_float_stuff_button->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 30);
		_test_float_stuff_button->connect("pressed", callable_mp(this, &YVoxelChunkEditorButtons::_on_test_float_stuff));
		add_child(_test_float_stuff_button);
	}

	void _on_test_float_stuff() {
		// print_line("0.5 ",_yvoxelchunk->floatToInt16(0.5f)," ", _yvoxelchunk->int16ToFloat(_yvoxelchunk->floatToInt16(0.5f)));
		// print_line("1.0 ",_yvoxelchunk->floatToInt16(1.0f)," ", _yvoxelchunk->int16ToFloat(_yvoxelchunk->floatToInt16(1.0f)));
		// print_line("-1.0 ",_yvoxelchunk->floatToInt16(-1.0f)," ", _yvoxelchunk->int16ToFloat(_yvoxelchunk->floatToInt16(-1.0f)));

		int16_t floatconverted = floatToInt16(-0.00824850890785f);
		auto byte_1 = static_cast<uint8_t>(floatconverted & 0xFF);
		auto byte_2 = static_cast<uint8_t>((floatconverted >> 8) & 0xFF);
		auto actual_float_value = int16ToFloat(static_cast<int16_t>(byte_1 | (byte_2 << 8)));
		print_line("-0.00824850890785 to byte and back ",actual_float_value," int16 ",floatconverted," byte 1 ",byte_1," byte 2 ",byte_2);
		byte_1 = 120;
		byte_2 = 255;
		actual_float_value = int16ToFloat(static_cast<int16_t>(byte_1 | (byte_2 << 8)));
		print_line("in theory this is -0.00824850890785 to byte and back ",actual_float_value," int16 ",floatconverted," byte 1 ",byte_1," byte 2 ",byte_2);
	}

	void _on_populate_chunk_button_pressed() {
		print_line("Calling test serialize on node ",_yvoxelchunk->get_instance_id());
		_yvoxelchunk->populate_chunk_3d();
		//print_line(static_cast<int>(YarnVoxel::get_singleton()->yvchunks.size()) ," has main node? ",YarnVoxel::get_singleton()->get_main_node()->get_name());
	}

	void _on_test_serialize_chunk_button_pressed() {
		print_line("Calling populate on node ",_yvoxelchunk->get_instance_id());
		_yvoxelchunk->test_serialization();
		print_line("serialized buffer size ",_yvoxelchunk->data.size());
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
			default: ;
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
	singleton = this;
	if (EditorNode::get_singleton() == nullptr || !EditorNode::get_singleton()->is_editor_ready()) return;
	inspector_plugin.instantiate();
	add_inspector_plugin(inspector_plugin);
}

YVoxelChunkEditorPlugin::~YVoxelChunkEditorPlugin() {
	if(singleton != nullptr && singleton == this) {
		singleton = nullptr;
	}
	if (inspector_plugin != nullptr && inspector_plugin.is_valid()) {
		inspector_plugin.unref();
	}
}


#endif
