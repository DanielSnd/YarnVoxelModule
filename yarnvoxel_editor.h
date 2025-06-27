#ifndef YARNVOXEL_EDITOR_H
#define YARNVOXEL_EDITOR_H

#ifdef TOOLS_ENABLED

#include "scene/gui/slider.h"
#include "yvoxelchunk.h"
#include "editor/editor_inspector.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/button.h"
#include "constants.h"
#include "editor/gui/editor_bottom_panel.h"
#include "core/config/project_settings.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/gui/control.h"
#include "editor/editor_undo_redo_manager.h"
#include "scene/gui/container.h"
#include "scene/gui/button.h"
#include "core/input/input_event.h"
#include "scene/3d/camera_3d.h"
#include "servers/physics_server_3d.h"
#include "scene/gui/label.h"
#include "scene/gui/separator.h"
#include "scene/main/viewport.h"
#include "editor/editor_interface.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/gui/option_button.h"
#include "constants.h"

class YVoxelChunkInspectorPlugin;

////////////////////////////////////////////////////////////////////////////////////////////////////



class YVoxelEditorBottomPanel : public VBoxContainer {
    GDCLASS(YVoxelEditorBottomPanel, VBoxContainer);

private:
    EditorUndoRedoManager *undo_redo;
protected:
    HBoxContainer* top_container;
    Label* debug_label;
    HSlider* brush_slider;
    HSlider* brush_strength_slider;
    Label* brush_slider_label;
    Label* brush_strength_slider_label;
    OptionButton* current_tool;
    OptionButton* current_point_type;
public:
    void set_debug_label_text(const String &string) {
        if (debug_label != nullptr) debug_label->set_text(string);
    }
    float brush_size_value;
    float brush_strength_value;
    int current_tool_selected;
    uint8_t current_block_type_selected;
    YVoxelEditorBottomPanel();

    void changed_tool_selection(int new_selection);

    void changed_point_type_selection(int new_selection);

    void strength_slider_value_changed(float p_new_value);

    void size_slider_value_changed(float p_new_value);

    ~YVoxelEditorBottomPanel();
};



////////////////////////////////////////////////////////////////////////////////////////////////////


class YarnVoxelEditorPlugin : public EditorPlugin  {
    GDCLASS(YarnVoxelEditorPlugin, EditorPlugin)

    private:
        static void add_setting(const String& p_name,
          const Variant& p_default_value,
          Variant::Type p_type,
          PropertyHint p_hint = PROPERTY_HINT_NONE,
          const String& p_hint_string = "",
          int p_usage = PROPERTY_USAGE_DEFAULT,
          bool requires_restart = false
        );

    protected:
        static void _bind_methods();
        //
        // void enable_voxel_painting();
        //
        // void disable_voxel_painting();
        Control* voxel_edit_button_holder;
        Button* voxel_edit_button;
        Vector3i chunk_number;
        YVoxelEditorBottomPanel* yvoxel_bottom_panel;
        MeshInstance3D* preview_brush;
        MeshInstance3D* preview_brush_above;
        bool toggle_can_edit_yvoxel = false;
        bool yvoxel_bottom_panel_visible = false;
        bool is_holding_left_click=false;
        bool is_holding_ctrl = false;
        bool is_holding_shift = false;
        bool is_raycast_hit = false;
        bool is_possible_new_position = false;
        bool is_holding_sculpt = false;
        uint64_t last_actually_sculpted_time = 0ULL;
        Vector3 raycast_hit_pos = Vector3(0.0f,0.0f,0.0f);
        Vector3 raycast_hit_normal = Vector3(0.0f,1.0f,0.0f);
    public:
        void _notification(int p_what);

    void update_bottom_visibility();

    void toggle_bottom_panel();

    void create_display_brush();

    void display_brush();

    virtual EditorPlugin::AfterGUIInput forward_3d_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) override;

    void handle_holding_down_sculpt();

    EditorPlugin::AfterGUIInput handle_input_event(const Ref<InputEvent> &p_event);

    void raycast(Camera3D *p_camera, const Ref<InputEvent> &p_event);

    void editor_changed_scene_deferred();

    bool last_editor_was_3d = false;

    void editor_changed_scene(const String &p_editor);

    YarnVoxelEditorPlugin();
    ~YarnVoxelEditorPlugin();
    bool is_yvoxel_enabled;
};



////////////////////////////////////////////////////////////////////////////////////////////////////




class YVoxelChunkEditorPlugin : public EditorPlugin {
    GDCLASS(YVoxelChunkEditorPlugin, EditorPlugin)

        inline static YVoxelChunkEditorPlugin* singleton = nullptr;

        _FORCE_INLINE_ static YVoxelChunkEditorPlugin* get_singleton() {
            return singleton;
        }
    protected:
        Ref<YVoxelChunkInspectorPlugin> inspector_plugin;

    public:
    	virtual String get_plugin_name() const override { return  YVoxelChunkEditorPlugin::get_class_static(); }
        YVoxelChunkEditorPlugin();
        ~YVoxelChunkEditorPlugin();
    };
#endif

#endif //YARNVOXEL_EDITOR_H
