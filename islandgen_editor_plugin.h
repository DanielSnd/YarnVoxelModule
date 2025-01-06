//
// Created by Daniel on 2024-01-07.
//

#ifndef ISLANDGEN_EDITOR_PLUGIN_H
#define ISLANDGEN_EDITOR_PLUGIN_H


#ifdef TOOLS_ENABLED

#include "editor/plugins/editor_plugin.h"

#include "ige_fastnoise.h"
#include "core/math/random_number_generator.h"
#include "scene/gui/label.h"
#include "scene/resources/image_texture.h"
#include "editor/editor_inspector.h"
#include "island_generator.h"

#include "core/math/vector2i.h"
#include "editor/editor_inspector.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/button.h"
#include "scene/gui/texture_rect.h"

#include "scene/gui/check_button.h"

class IslandGeneratorEditorPlugin : public EditorPlugin {
    GDCLASS(IslandGeneratorEditorPlugin, EditorPlugin)

public:
	virtual String get_plugin_name() const override { return IslandGenerator::get_class_static(); }

    IslandGeneratorEditorPlugin();
};


class IslandGenEffectEditorPlugin : public EditorPlugin {
    GDCLASS(IslandGenEffectEditorPlugin, EditorPlugin)

public:
	virtual String get_plugin_name() const override { return  IslandGenerator::get_class_static(); }

    IslandGenEffectEditorPlugin();
};
#endif

#endif //ISLANDGEN_EDITOR_PLUGIN_H
