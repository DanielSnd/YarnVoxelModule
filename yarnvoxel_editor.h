#ifdef TOOLS_ENABLED

#ifndef YARNVOXEL_EDITOR_H
#define YARNVOXEL_EDITOR_H

#include "core/config/project_settings.h"
#include "editor/editor_plugin.h"
class YVoxelChunkInspectorPlugin;

namespace godot {
    class YarnVoxelEditorPlugin : public EditorPlugin  {
        GDCLASS(YarnVoxelEditorPlugin, EditorPlugin)

    public:
	    void _notification(int p_what);
        YarnVoxelEditorPlugin() = default;
        ~YarnVoxelEditorPlugin() = default;
        static constexpr const char* INITIALIZE_BASE_PATH = "settings";

    private:
        static void add_setting(
          ProjectSettings& ps,
          const String& p_name,
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
    };

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