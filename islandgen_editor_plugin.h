//
// Created by Daniel on 2024-01-07.
//

#ifndef ISLANDGEN_EDITOR_PLUGIN_H
#define ISLANDGEN_EDITOR_PLUGIN_H


#ifdef TOOLS_ENABLED

#include "editor/editor_plugin.h"

class IslandGeneratorEditorPlugin : public EditorPlugin {
    GDCLASS(IslandGeneratorEditorPlugin, EditorPlugin)

public:
    String get_name() const override;

    IslandGeneratorEditorPlugin();
};


class IslandGenEffectEditorPlugin : public EditorPlugin {
    GDCLASS(IslandGenEffectEditorPlugin, EditorPlugin)

public:
    String get_name() const override;

    IslandGenEffectEditorPlugin();
};
#endif

#endif //ISLANDGEN_EDITOR_PLUGIN_H
