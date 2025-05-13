#ifdef TOOLS_ENABLED


#include "yarnvoxel_editor.h"

using namespace godot;

void YarnVoxelEditorPlugin::_notification(int p_what) {
    switch (p_what) {
        case NOTIFICATION_ENTER_TREE: {
        	if(is_yvoxel_enabled) {
        		// add_tool_menu_item("Enable voxel painting",callable_mp(this,&YarnVoxelEditorPlugin::enable_voxel_painting));

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
    	} break;
        case NOTIFICATION_EXIT_TREE: {
        } break;
    	case NOTIFICATION_PROCESS : {
			//print_line(vformat("Doing process %d",OS::get_singleton()->get_ticks_msec()));
    	} break;
    	default:
    		break;
    }
}

void YarnVoxelEditorPlugin::update_bottom_visibility() {
	yvoxel_bottom_panel_visible = yvoxel_bottom_panel->is_visible();
	if (preview_brush != nullptr) {
		preview_brush->set_visible(false);
		preview_brush_above ->set_visible(false);
	}
}

void YarnVoxelEditorPlugin::toggle_bottom_panel() {
	if (yvoxel_bottom_panel == nullptr) {
		yvoxel_bottom_panel = memnew(YVoxelEditorBottomPanel);
		yvoxel_bottom_panel->connect("visibility_changed", callable_mp(this, &YarnVoxelEditorPlugin::update_bottom_visibility));
	}
	auto main_node = YarnVoxel::get_singleton()->get_main_node();
	if (main_node == nullptr) {
		if (EditorInterface::get_singleton()) {
			Node *current_scene = EditorInterface::get_singleton()->get_edited_scene_root();
			if (current_scene != nullptr) {
				TypedArray<Node> nchildren = current_scene->find_children("*","Camera3D",true);
				if (nchildren.size() > 0) {
					auto found_camera = Object::cast_to<Camera3D>(nchildren.pop_front());
					if (found_camera != nullptr) {
						auto camera_parent = Object::cast_to<Node3D>(found_camera->get_parent());
						if (camera_parent != nullptr) {
							YarnVoxel::get_singleton()->create_main_node_under(camera_parent);
						}
					}
				} else {
					ERR_PRINT("[YVoxel] Camera 3d not found");
				}
			} else {
				ERR_PRINT("CURRENT SCENE IS NULL");
			}
			if (YarnVoxel::get_singleton()->get_main_node() == nullptr) {
				ERR_PRINT("[YVoxel] YVoxels require a camera 3d in the scene");
			}
		}
	}
	if (!toggle_can_edit_yvoxel) {
		if (preview_brush == nullptr) {
			create_display_brush();
		}
		EditorNode::get_bottom_panel()->add_item(TTR("YVoxel Editor"), yvoxel_bottom_panel);
		set_process(true);
		yvoxel_bottom_panel_visible = true;
		EditorNode::get_bottom_panel()->make_item_visible(yvoxel_bottom_panel);
		preview_brush->set_visible(true);
		preview_brush_above->set_visible(true);
		toggle_can_edit_yvoxel=true;
	} else {
		EditorNode::get_bottom_panel()->remove_item(yvoxel_bottom_panel);
		yvoxel_bottom_panel_visible = false;
		toggle_can_edit_yvoxel=false;
		set_process(false);
		if (preview_brush != nullptr) {
			preview_brush->set_visible(false);
			preview_brush_above->set_visible(false);
		}
	}
}

void YarnVoxelEditorPlugin::create_display_brush() {
	preview_brush = memnew(MeshInstance3D);
	add_child(preview_brush);
	preview_brush_above = memnew(MeshInstance3D);
	add_child(preview_brush_above);
	Ref<SphereMesh> sphere_mesh;
	sphere_mesh.instantiate();
	preview_brush->set_mesh(sphere_mesh);
	Ref<ShaderMaterial> preview_material;
	preview_material.instantiate();
	Ref<Shader> preview_material_shader;
	preview_material_shader.instantiate();
	preview_material_shader->set_code(R"(
shader_type spatial;
render_mode blend_add,depth_draw_opaque,cull_back,unshaded,shadows_disabled;
varying float v;
void vertex(){ v = VERTEX.y; }
void fragment() {
	ALBEDO = vec3(0.2,0.79,0.8);
	ALPHA = pow(clamp(v,0.0,1.0),1.5);
}
)");
	preview_material->set_shader(preview_material_shader);
	sphere_mesh->surface_set_material(0,preview_material);
	preview_brush_above->set_mesh(sphere_mesh);
	preview_brush_above->set_scale(Vector3(0.3,0.3,0.3));
}

void YarnVoxelEditorPlugin::display_brush() {
	//print_line("is raycast hit? ",is_raycast_hit);
	if (is_raycast_hit) {
		preview_brush->set_visible(true);
		preview_brush_above->set_visible(true);
		preview_brush->set_global_position(raycast_hit_pos);
		preview_brush->set_scale(YARNVOXEL_VECTOR3_ONE * yvoxel_bottom_panel->brush_size_value);
		preview_brush_above->set_scale(preview_brush->get_scale() * 0.25);
		preview_brush_above->set_global_position(raycast_hit_pos + raycast_hit_normal * (preview_brush->get_scale().length() * 0.65f));
	} else {
		//preview_brush->set_visible(false);
		if (is_possible_new_position) {
			preview_brush->set_visible(true);
			preview_brush_above->set_visible(true);
			preview_brush->set_global_position(raycast_hit_pos);
			preview_brush->set_scale(YARNVOXEL_VECTOR3_ONE * yvoxel_bottom_panel->brush_size_value);
			preview_brush_above->set_scale(preview_brush->get_scale() * 0.25);
			preview_brush_above->set_global_position(raycast_hit_pos + raycast_hit_normal * (preview_brush->get_scale().length() * 0.65f));
		}
	}
}

EditorPlugin::AfterGUIInput YarnVoxelEditorPlugin::forward_3d_gui_input(Camera3D *p_camera,	const Ref<InputEvent> &p_event) {
	if (!yvoxel_bottom_panel_visible) return EditorPlugin::forward_3d_gui_input(p_camera, p_event);
	Ref<InputEventMouseMotion> motion = p_event;
	if (motion.is_valid()) {
		// Check if the mouse position is within the 3D viewport
		Vector2 mouse_pos = motion->get_position();
		Rect2 viewport_rect = p_camera->get_viewport()->get_visible_rect();

		// Get the size of the viewport
		Size2 viewport_size = viewport_rect.get_size();

		// Define a margin value to consider proximity to the edges
		float edge_margin = 10.0; // Adjust as needed

		// If the mouse is close to any of the edges
		if (mouse_pos.x < edge_margin || mouse_pos.x > viewport_size.width - edge_margin ||
			mouse_pos.y < edge_margin || mouse_pos.y > viewport_size.height - edge_margin) {
			is_raycast_hit = false;
			is_possible_new_position = false;
			if (preview_brush != nullptr && preview_brush->is_visible()) {
				preview_brush->set_visible(false);
				preview_brush_above->set_visible(false);
			}
			return AFTER_GUI_INPUT_PASS;
		}
	}

	Ref<InputEventKey> key_event = p_event;
	if (key_event.is_valid()) {
		if(key_event->get_physical_keycode() == Key::CMD_OR_CTRL) {
			if(key_event->is_pressed()) {
				is_holding_ctrl = true;
			} else if(key_event->is_released()) {
				is_holding_ctrl = false;
			}
		}
		if(key_event->get_physical_keycode() == Key::SHIFT) {
			if(key_event->is_pressed()) {
				is_holding_shift = true;
			} else if(key_event->is_released()) {
				is_holding_shift = false;
			}
		}
	}

	display_brush();
	raycast(p_camera,p_event);

	if (is_raycast_hit || is_possible_new_position) {
		return handle_input_event(p_event);;
	} else {
		if (is_holding_left_click) is_holding_left_click = false;
	}
	return EditorPlugin::forward_3d_gui_input(p_camera, p_event);
}

void YarnVoxelEditorPlugin::handle_holding_down_sculpt() {
	if (last_actually_sculpted_time + 120UL < OS::get_singleton()->get_ticks_msec()) {
		last_actually_sculpted_time = OS::get_singleton()->get_ticks_msec();
		YVoxelChunk* tryChunk = nullptr;
		//auto pointsPosition = YarnVoxel::GetPointNumberFromPosition(raycast_hit_pos);
		if(!YarnVoxel::try_get_chunk(chunk_number, tryChunk)) tryChunk = YarnVoxel::get_singleton()->get_chunk(chunk_number);
		if (tryChunk != nullptr) {
			if (is_holding_shift && yvoxel_bottom_panel->current_tool_selected == 0) {
				YarnVoxel::get_singleton()->smooth_voxel_area(raycast_hit_pos,abs(yvoxel_bottom_panel->brush_strength_value),static_cast<int>(yvoxel_bottom_panel->brush_size_value));
			} else {
				//tryChunk->SetPointDensity(pointsPosition,-0.5,YarnVoxel::BlockType::GRASS);
				YarnVoxel::get_singleton()->modify_voxel_area(raycast_hit_pos,yvoxel_bottom_panel->brush_strength_value * (yvoxel_bottom_panel->current_tool_selected == 1 ? -1 : 1) * (is_holding_ctrl ? -1 : 1) * (yvoxel_bottom_panel->current_tool_selected == 2 ? 0 : 1), static_cast<int>(yvoxel_bottom_panel->brush_size_value),yvoxel_bottom_panel->current_block_type_selected + 1);
				//YarnVoxel::get_singleton()->set_dirty_chunk(chunk_number);
			}
		}
	}
}


EditorPlugin::AfterGUIInput  YarnVoxelEditorPlugin::handle_input_event(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
		if (mb->is_pressed()) {
			is_holding_sculpt = true;
			is_holding_left_click = true;
			handle_holding_down_sculpt();
			return AFTER_GUI_INPUT_STOP;
		}
		if (mb->is_released()) {
			is_holding_sculpt = false;
			is_holding_left_click = false;
		}
	}
	return AFTER_GUI_INPUT_PASS;
}

void YarnVoxelEditorPlugin::raycast(Camera3D *p_camera, const Ref<InputEvent> &p_event) {
	if (p_camera == nullptr)
		return;
	if (!yvoxel_bottom_panel_visible) {
		is_raycast_hit= false;
		is_possible_new_position = false;
		if (preview_brush != nullptr) {
			preview_brush->set_visible(false);
			preview_brush_above ->set_visible(false);
		}
	}
	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_null() || !mm.is_valid()) return;
	Vector3 ray_origin = p_camera->project_ray_origin(mm->get_position());
	Vector3 ray_dir = p_camera->project_ray_normal(mm->get_position());
	float ray_dist = p_camera->get_far();
	ERR_FAIL_COND(get_viewport() == nullptr || get_viewport()->get_world_3d().is_null() || !get_viewport()->get_world_3d().is_valid());
	auto space_state = get_viewport()->get_world_3d()->get_direct_space_state();
	ERR_FAIL_COND(space_state == nullptr);
	PhysicsDirectSpaceState3D::RayResult result;
	PhysicsDirectSpaceState3D::RayParameters ray_params;
	ray_params.from = ray_origin;
	ray_params.to = ray_origin + ray_dir * ray_dist;
	if (space_state->intersect_ray(ray_params, result)) {
		is_raycast_hit = true;
		is_possible_new_position = false;
		raycast_hit_normal = result.normal;
		raycast_hit_pos = result.position + (result.normal * (is_holding_ctrl ? -0.1f : 0.1f));
	} else {
		is_raycast_hit = false;
		is_possible_new_position = true;
		float t = (-ray_origin.y) / ray_dir.y;
		raycast_hit_pos = ray_origin + t * ray_dir;
		raycast_hit_normal = Vector3(0.0f,1.0f,0.0f);
	}

	Vector3i test_chunkNumber = YarnVoxel::GetChunkNumberFromPosition(raycast_hit_pos);
	if (test_chunkNumber != chunk_number) {
		chunk_number = test_chunkNumber;
	}

	if(is_holding_left_click && (is_raycast_hit || is_possible_new_position)) {
		handle_holding_down_sculpt();
	}
	YVoxelChunk* find_chunk = nullptr;
	bool updated_point_data = false;
	if (YarnVoxel::try_get_chunk(chunk_number, find_chunk)) {
		const Vector3i blockNumber = YarnVoxel::GetPointNumberFromPosition(Vector3(static_cast<float>(raycast_hit_pos.x),static_cast<float>(raycast_hit_pos.y),static_cast<float>(raycast_hit_pos.z)));
		if (YarnVoxel::IsPositionValid(blockNumber)) {
			auto found_point_data = find_chunk->points[blockNumber.x][blockNumber.y][blockNumber.z];
			yvoxel_bottom_panel->set_debug_label_text(vformat("Hit Pos: %s, New? %s, Chunk: %s, Point Pos %s, Point Type: %s, Float Value: %s",raycast_hit_pos,is_possible_new_position,chunk_number,blockNumber,found_point_data.byteValue,found_point_data.get_float_value_as_float()));
		}
	}
	if (!updated_point_data)
		yvoxel_bottom_panel->set_debug_label_text(vformat("Hit Pos: %s, New? %s, Chunk: %s",raycast_hit_pos,is_possible_new_position,chunk_number));
}

void YarnVoxelEditorPlugin::editor_changed_scene_deferred() {
	if (!last_editor_was_3d) {
		if (voxel_edit_button_holder != nullptr) {
			voxel_edit_button_holder->hide();
		}
		if (yvoxel_bottom_panel != nullptr && yvoxel_bottom_panel_visible) {
			EditorNode::get_bottom_panel()->remove_item(yvoxel_bottom_panel);
			yvoxel_bottom_panel_visible = false;
		}
	} else {
		if (voxel_edit_button_holder != nullptr) {
			voxel_edit_button_holder->show();
		}
	}
}

void YarnVoxelEditorPlugin::editor_changed_scene(const String &p_editor) {
	last_editor_was_3d = p_editor == "3D";
	callable_mp(this,&YarnVoxelEditorPlugin::editor_changed_scene_deferred).call_deferred();
}


YarnVoxelEditorPlugin::YarnVoxelEditorPlugin() {
	voxel_edit_button_holder = nullptr;
	voxel_edit_button  = nullptr;
	yvoxel_bottom_panel = nullptr;
	preview_brush = nullptr;
	preview_brush_above = nullptr;
	yvoxel_bottom_panel_visible = false;
	is_yvoxel_enabled = GLOBAL_DEF_RST(PropertyInfo(Variant::BOOL, "YarnVoxel/general/enabled", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_RESTART_IF_CHANGED), "");
}

YarnVoxelEditorPlugin::~YarnVoxelEditorPlugin() {
	// if (voxel_edit_button_holder != nullptr) {
	// 	memdelete(voxel_edit_button_holder);
	// 	voxel_edit_button_holder = nullptr;
	// }
	// if (voxel_edit_button != nullptr) {
	// 	memdelete(voxel_edit_button);
	// 	voxel_edit_button = nullptr;
	// }
	// if (yvoxel_bottom_panel != nullptr) {
	// 	memdelete(yvoxel_bottom_panel);
	// 	yvoxel_bottom_panel = nullptr;
	// }
	// if (preview_brush != nullptr) {
	// 	memdelete(preview_brush);
	// 	preview_brush = nullptr;
	// }
	// if (preview_brush_above != nullptr) {
	// 	memdelete(preview_brush_above);
	// 	preview_brush_above = nullptr;
	// }
}

YVoxelEditorBottomPanel::YVoxelEditorBottomPanel() {
	undo_redo = EditorUndoRedoManager::get_singleton();
	top_container = memnew(HBoxContainer);
	add_child(top_container);
	debug_label = memnew(Label);
	add_child(debug_label);

	top_container->add_child(memnew(VSeparator));

	current_tool = memnew(OptionButton);
	top_container->add_child(current_tool);
	current_tool->add_item("Sculpting");
	current_tool->add_item("Erasing");
	current_tool->add_item("Painting");
	current_tool->connect("item_selected",callable_mp(this,&YVoxelEditorBottomPanel::changed_tool_selection));

	current_point_type = memnew(OptionButton);
	top_container->add_child(current_point_type);
	bool first_passed=false;
	for (auto yvname: YarnVoxelData::BlockTypeToName) {
		if(!first_passed) { first_passed=true; continue; }
		current_point_type->add_item(yvname);
	}
	current_point_type->connect("item_selected",callable_mp(this,&YVoxelEditorBottomPanel::changed_point_type_selection));

	//path_hb->add_child(memnew(Label(TTR("Path:"))));
	top_container->add_child(memnew(VSeparator));
	//add_child(memnew(HSeparator));
	brush_slider_label = memnew(Label);
	top_container->add_child(brush_slider_label);
	brush_slider_label->set_text("Brush Size: 2.0");

	brush_slider = memnew(HSlider);
	brush_slider->set_custom_minimum_size(Size2(126.0,0.0));
	brush_slider->set_min(1.0);
	brush_slider->set_max(10.0);
	brush_slider->set_custom_step(0.01);
	brush_slider->set_value(2.0);
	brush_slider->set_v_size_flags(SIZE_SHRINK_CENTER);
	brush_size_value = 2.0;
	brush_slider->connect("value_changed",callable_mp(this,&YVoxelEditorBottomPanel::size_slider_value_changed));
	top_container->add_child(brush_slider);

	brush_strength_slider_label = memnew(Label);
	top_container->add_child(brush_strength_slider_label);
	brush_strength_slider_label->set_text(vformat("Brush Strength: %.2f",0.33));

	brush_strength_slider = memnew(HSlider);
	brush_strength_slider->set_custom_minimum_size(Size2(200.0,0.0));
	brush_strength_slider->set_min(-100.0);
	brush_strength_slider->set_max(100.0);
	brush_strength_slider->set_value(33.0);
	brush_strength_slider->set_step(0.0001);
	brush_strength_slider->set_v_size_flags(SIZE_SHRINK_CENTER);
	brush_strength_slider->set_clip_contents(false);
	brush_strength_value = -0.33;
	brush_strength_slider->connect("value_changed",callable_mp(this,&YVoxelEditorBottomPanel::strength_slider_value_changed));
	top_container->add_child(brush_strength_slider);

}

void YVoxelEditorBottomPanel::changed_tool_selection(int new_selection) {
	current_tool_selected = new_selection;
}

void YVoxelEditorBottomPanel::changed_point_type_selection(int new_selection) {
	current_block_type_selected = new_selection;
}
void YVoxelEditorBottomPanel::strength_slider_value_changed(float p_new_value) {
	brush_strength_value = static_cast<float>(p_new_value) * -0.01f;
	brush_strength_slider_label->set_text(vformat("Brush Strength: %.2f",brush_strength_value * -1.0f));
}

void YVoxelEditorBottomPanel::size_slider_value_changed(float p_new_value) {
	brush_size_value = p_new_value;
	brush_slider_label->set_text(vformat("Brush Size: %.2f",p_new_value));
}

YVoxelEditorBottomPanel::~YVoxelEditorBottomPanel() {

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
//
// void YarnVoxelEditorPlugin::enable_voxel_painting() {
// 	remove_tool_menu_item("Enable voxel painting");
// 	add_tool_menu_item("Disable voxel painting",callable_mp(this,&YarnVoxelEditorPlugin::disable_voxel_painting));
// }
//
// void YarnVoxelEditorPlugin::disable_voxel_painting() {
// 	remove_tool_menu_item("Disable voxel painting");
// 	add_tool_menu_item("Enable voxel painting",callable_mp(this,&YarnVoxelEditorPlugin::enable_voxel_painting));
// }

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
		//print_line("-0.00824850890785 to byte and back ",actual_float_value," int16 ",floatconverted," byte 1 ",byte_1," byte 2 ",byte_2);
		byte_1 = 120;
		byte_2 = 255;
		actual_float_value = int16ToFloat(static_cast<int16_t>(byte_1 | (byte_2 << 8)));
		//print_line("in theory this is -0.00824850890785 to byte and back ",actual_float_value," int16 ",floatconverted," byte 1 ",byte_1," byte 2 ",byte_2);
	}

	void _on_populate_chunk_button_pressed() {
		//print_line("Calling test serialize on node ",_yvoxelchunk->get_instance_id());
		_yvoxelchunk->populate_chunk_3d();
		//print_line(static_cast<int>(YarnVoxel::get_singleton()->yvchunks.size()) ," has main node? ",YarnVoxel::get_singleton()->get_main_node()->get_name());
	}

	void _on_test_serialize_chunk_button_pressed() {
		//print_line("Calling populate on node ",_yvoxelchunk->get_instance_id());
		_yvoxelchunk->test_serialization();
		//print_line("serialized buffer size ",_yvoxelchunk->data.size());
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
