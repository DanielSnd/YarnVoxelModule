#ifdef TOOLS_ENABLED

#ifndef YARNVOXEL_EDITOR_H
#define YARNVOXEL_EDITOR_H

#include "core/config/project_settings.h"
#include "editor/editor_plugin.h"
#include "scene/gui/control.h"
#include "editor/editor_undo_redo_manager.h"
#include "scene/gui/container.h"
#include "scene/gui/button.h"
class YVoxelChunkInspectorPlugin;

namespace godot {


////////////////////////////////////////////////////////////////////////////////////////////////////


    //
    // class YVoxelEditorBottomPanel : public VBoxContainer {
    //     GDCLASS(YVoxelEditorBottomPanel, VBoxContainer);
    //
    // private:
    //     EditorUndoRedoManager *undo_redo;
    //
    // public:
    //
    // };



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

        void enable_voxel_painting();

        void disable_voxel_painting();

        Control* voxel_edit_button_holder;
        Button* voxel_edit_button;
        //YVoxelEditorBottomPanel* yvoxel_bottom_panel;
        bool yvoxel_bottom_panel_visible = false;

    public:
        void _notification(int p_what);

    void toggle_bottom_panel();

    void editor_changed_scene(const String &p_editor);

    YarnVoxelEditorPlugin();
    ~YarnVoxelEditorPlugin();
    bool is_yvoxel_enabled;
};



////////////////////////////////////////////////////////////////////////////////////////////////////




class YVoxelChunkEditorPlugin : public EditorPlugin {
    GDCLASS(YVoxelChunkEditorPlugin, EditorPlugin)

        inline static YVoxelChunkEditorPlugin* singleton = nullptr;

        static YVoxelChunkEditorPlugin* get_singleton();
    protected:
        Ref<YVoxelChunkInspectorPlugin> inspector_plugin;

    public:
        String get_name() const override;
        YVoxelChunkEditorPlugin();
        ~YVoxelChunkEditorPlugin();
    };
}


#endif //YARNVOXEL_EDITOR_H

#endif