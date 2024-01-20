//
// Created by Daniel on 2024-01-07.
//

#include "islandgen_editor_plugin.h"

#include "core/math/random_number_generator.h"
#include "editor/editor_inspector.h"
#include "scene/gui/label.h"
#include "scene/resources/image_texture.h"

#ifdef TOOLS_ENABLED

#include "island_generator.h"

#include "core/math/vector2i.h"
#include "editor/editor_inspector.h"
#include "editor/editor_scale.h"
#include "scene/gui/button.h"
#include "scene/gui/texture_rect.h"

class IslandGeneratorPreview : public Control {
	GDCLASS(IslandGeneratorPreview, Control)

	static const int PREVIEW_HEIGHT = 256;
	static const int PADDING_PREVIEW_INFO = 2;
	Label *_preview_info_label = nullptr;
	Button *_randomize_seed_button = nullptr;
	Color use_color;
	Ref<IslandGenerator> _island_generator;
	Size2i _preview_texture_size;

	TextureRect *_texture_rect = nullptr;
	TextureRect *_cave_texture_rect = nullptr;
	//Button *_3d_space_switch = nullptr;

public:
	IslandGeneratorPreview() {
		set_custom_minimum_size(Size2(0, EDSCALE * (PREVIEW_HEIGHT)));
		_texture_rect = memnew(TextureRect);
		_texture_rect->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
		_texture_rect->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH_PROPORTIONAL);
		_texture_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
		add_child(_texture_rect);

		_preview_info_label = memnew(Label);
		_preview_info_label->set_text(TTR("3D"));
		_preview_info_label->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_preview_info_label->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO);
		add_child(_preview_info_label);

		_randomize_seed_button = memnew(Button);
		_randomize_seed_button->set_text(TTR("Random Seed"));
		_randomize_seed_button->set_tooltip_text(TTR("Randomizes the seed."));
		_randomize_seed_button->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_randomize_seed_button->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 30);
		_randomize_seed_button->connect("pressed", callable_mp(this, &IslandGeneratorPreview::_on_random_seed_button_pressed));
		add_child(_randomize_seed_button);

		_cave_texture_rect = memnew(TextureRect);
		_cave_texture_rect->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH);
		_cave_texture_rect->set_stretch_mode(TextureRect::STRETCH_SCALE);
		_cave_texture_rect->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_cave_texture_rect->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 54);
		_cave_texture_rect->set_custom_minimum_size(Size2(64+32,64+32));
		add_child(_cave_texture_rect);
	}

	void _on_random_seed_button_pressed() {
		Ref<RandomNumberGenerator> rng = memnew(RandomNumberGenerator);
		_island_generator->set_seed(rand());
	}
	void set_island_generator(Ref<IslandGenerator> island_generator) {
		if (_island_generator == island_generator) {
			return;
		}
		_island_generator = island_generator;
		if (_island_generator.is_valid()) {
			_island_generator->connect_changed(callable_mp(this, &IslandGeneratorPreview::update_preview));
			update_preview();
		}
	}

private:
	Ref<Image> img;
	Ref<ImageTexture> imgTexture;
	Ref<Image> caveimg;
	Ref<ImageTexture> caveimgTexture;
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_RESIZED: {
				_preview_texture_size = get_size();
				update_preview();
			} break;
		}
	}

	void update_preview() {
		update_cave_preview();
		if (MIN(_preview_texture_size.width, _preview_texture_size.height) <= 0 || !_island_generator.is_valid())
			return;
		Vector2 desired_size = Vector2i(
			CLAMP(_island_generator->map_size.x, 32, 128),
			CLAMP(_island_generator->map_size.y, 32, 128));
		if(!img.is_valid() || !imgTexture.is_valid()) {
			img = Image::create_empty(desired_size.x,desired_size.y,false, Image::FORMAT_RGB8);
			set_preview_pixels(desired_size);
			imgTexture = ImageTexture::create_from_image(img);
			return;
		}
		if (img->get_size().x != desired_size.x || img->get_size().y != desired_size.y) {
			img->resize(desired_size.x,desired_size.y);
			set_preview_pixels(desired_size);
			imgTexture->set_image(img);
			return;
		}
		set_preview_pixels(desired_size);
		imgTexture->update(img);
		_texture_rect->set_texture(imgTexture);
	}

	void update_cave_preview() {
		if (!_island_generator.is_valid())
			return;
		if(!caveimg.is_valid() || !caveimgTexture.is_valid()) {
			caveimg = Image::create_empty(32,32,false, Image::FORMAT_RGB8);
			set_preview_pixels_cave();
			caveimgTexture = ImageTexture::create_from_image(caveimg);
			return;
		}
		if (caveimg->get_size().x != 32 || caveimg->get_size().y != 32) {
			caveimg->resize(32,32);
			set_preview_pixels_cave();
			caveimgTexture->set_image(caveimg);
			return;
		}
		set_preview_pixels_cave();
		caveimgTexture->update(caveimg);
		_cave_texture_rect->set_texture(caveimgTexture);
	}

	void set_preview_pixels(Vector2 &desired_size) {
		float min_value_happen = 9999.0;
		float max_value_happen = -9999.0;
		for (int x = 0; x < desired_size.x; x++) {
			for (int y = 0; y < desired_size.y; y++) {
				const float orig_x = static_cast<float>(x) / desired_size.x * static_cast<float>(_island_generator->map_size.x);
				const float orig_y = static_cast<float>(y) / desired_size.y * static_cast<float>(_island_generator->map_size.y);

				if (_island_generator->color_ramp.is_valid()) {
					auto terrain_height = ((_island_generator->GetTerrainHeight(Vector2(orig_x,orig_y),false)+1.0f) / 2.0f );
					if (terrain_height < min_value_happen) {
						min_value_happen = terrain_height;
					}
					if (terrain_height > max_value_happen) {
						max_value_happen = terrain_height;
					}
					use_color = _island_generator->color_ramp->get_color_at_offset(terrain_height);
					img->set_pixel(x,y, use_color);
				} else {
					const auto terrain_height = ((_island_generator->GetTerrainHeight(Vector2(orig_x,orig_y),false)+1.0f) / 2.0f );
					if (terrain_height < min_value_happen) {
						min_value_happen = terrain_height;
					}
					if (terrain_height > max_value_happen) {
						max_value_happen = terrain_height;
					}
					use_color = Color{terrain_height,terrain_height,terrain_height,1.0f};
					img->set_pixel(x,y, use_color);
				}
			}
		}
		if (_preview_info_label != nullptr) {
			_preview_info_label->set_text(vformat("Min %f\nMax %f",min_value_happen,max_value_happen));
		}
	}
	void set_preview_pixels_cave() {
		if (_island_generator == nullptr || _island_generator->fnl == nullptr) {
			return;
		}
		_island_generator->fnl->set_fractal_type(FastNoiseLite::FRACTAL_FBM);
		_island_generator->fnl->set_frequency(_island_generator->cave_smoothness);
		_island_generator->fnl->set_fractal_lacunarity(_island_generator->cave_lacunarity);
		_island_generator->fnl->set_seed(_island_generator->seed);
		for (int x = 0; x < 32; x++) {
			for (int y = 0; y < 32; y++) {
					const float orig_x = static_cast<float>(x) / 32 * static_cast<float>(_island_generator->map_size.x);
					const float orig_y = static_cast<float>(y) / 32 * static_cast<float>(_island_generator->map_size.y);
					const float noiseToCheckIfShouldReplaceWith3DDensity = _island_generator->fnl->get_noise_3d(static_cast<float>(orig_x) + 1024.0f, 20.0f + 1024.0f, static_cast<float>(orig_y) + 1024.0f);
					float density = 1.0f;
					if (noiseToCheckIfShouldReplaceWith3DDensity > _island_generator->cave_if_above) {
						float gotNoise = _island_generator->fnl->get_noise_3d(static_cast<float>(orig_x), 20.0f, static_cast<float>(orig_y));
						if (gotNoise > 0.0f)
							density = gotNoise;
					}
					use_color = Color{density,density,density,1.0f};
					caveimg->set_pixel(x,y, use_color);
				}
			}
		}

};

/////////////////////////////////////////////////////////////////////////////////

class IslandGeneratorInspectorPlugin : public EditorInspectorPlugin {
    GDCLASS(IslandGeneratorInspectorPlugin, EditorInspectorPlugin)
public:
    bool can_handle(Object *p_object) override {
        return Object::cast_to<IslandGenerator>(p_object) != nullptr;
    }

    void parse_begin(Object *p_object) override {
        IslandGenerator *islandgen_ptr = Object::cast_to<IslandGenerator>(p_object);
        if (islandgen_ptr) {
            Ref<IslandGenerator> islandgen(islandgen_ptr);

            IslandGeneratorPreview *viewer = memnew(IslandGeneratorPreview);
            viewer->set_island_generator(islandgen);
            add_custom_control(viewer);
        }
    }
};

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

String IslandGeneratorEditorPlugin::get_name() const {
	return IslandGenerator::get_class_static();
}

IslandGeneratorEditorPlugin::IslandGeneratorEditorPlugin() {
	Ref<IslandGeneratorInspectorPlugin> plugin;
	plugin.instantiate();
	add_inspector_plugin(plugin);
}
/////////////////////////////////////////////////////////////////////////////////

class IslandGenEffectPreview : public Control {
	GDCLASS(IslandGenEffectPreview, Control)

	static const int PREVIEW_HEIGHT = 64;

	Color use_color;
	Ref<IslandGenEffect> _island_geneffect;
	Size2i _preview_texture_size;

	TextureRect *_texture_rect = nullptr;
	// Button *_3d_space_switch = nullptr;

public:
	IslandGenEffectPreview() {
		set_custom_minimum_size(Size2(0, EDSCALE * (PREVIEW_HEIGHT)));
		_texture_rect = memnew(TextureRect);
		_texture_rect->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
		_texture_rect->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH_PROPORTIONAL);
		_texture_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
		add_child(_texture_rect);
	}

	void set_island_geneffect(Ref<IslandGenEffect> island_geneffect) {
		if (_island_geneffect == island_geneffect) {
			return;
		}
		_island_geneffect = island_geneffect;
		if (_island_geneffect.is_valid()) {
			_island_geneffect->connect_changed(callable_mp(this, &IslandGenEffectPreview::update_preview));
			update_preview();
		}
	}

private:
	Ref<Image> img;
	Ref<ImageTexture> imgTexture;
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_RESIZED: {
				_preview_texture_size = get_size();
				update_preview();
			} break;
		}
	}

	void update_preview() {
		if (MIN(_preview_texture_size.width, _preview_texture_size.height) <= 0 || !_island_geneffect.is_valid())
			return;
		Vector2 desired_size = Vector2i(64,64);
		if(!img.is_valid() || !imgTexture.is_valid()) {
			img = Image::create_empty(desired_size.x,desired_size.y,false, Image::FORMAT_RGB8);
			set_preview_pixels(desired_size);
			imgTexture = ImageTexture::create_from_image(img);
			return;
		}
		if (img->get_size().x != desired_size.x || img->get_size().y != desired_size.y) {
			img->resize(desired_size.x,desired_size.y);
			set_preview_pixels(desired_size);
			imgTexture->set_image(img);
			return;
		}
		set_preview_pixels(desired_size);
		imgTexture->update(img);
		_texture_rect->set_texture(imgTexture);
	}

	void set_preview_pixels(Vector2 &desired_size) {
		for (int x = 0; x < desired_size.x; x++) {
			for (int y = 0; y < desired_size.y; y++) {
				float mask = _island_geneffect->GetMask(x,y);
				use_color = Color{mask,mask,mask,1.0f};
				if(_island_geneffect->mask_minimum > mask) {
					use_color = Color(0.4,0.0,0.3);
				}
				img->set_pixel(x,y, use_color);
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////

class IslandGenEffectInspectorPlugin : public EditorInspectorPlugin {
	GDCLASS(IslandGenEffectInspectorPlugin, EditorInspectorPlugin)
public:
	bool can_handle(Object *p_object) override {
		return Object::cast_to<IslandGenEffect>(p_object) != nullptr;
	}

	void parse_begin(Object *p_object) override {
		if (auto *islandgen_ptr = Object::cast_to<IslandGenEffect>(p_object)) {
			Ref<IslandGenEffect> islandgen(islandgen_ptr);
			if(islandgen.is_valid() && islandgen->use_mask) {
				IslandGenEffectPreview *viewer = memnew(IslandGenEffectPreview);
				viewer->set_island_geneffect(islandgen);
				add_custom_control(viewer);
			}
		}
	}
};

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

String IslandGenEffectEditorPlugin::get_name() const {
	return IslandGenerator::get_class_static();
}

IslandGenEffectEditorPlugin::IslandGenEffectEditorPlugin() {
	Ref<IslandGenEffectInspectorPlugin> plugin;
	plugin.instantiate();
	add_inspector_plugin(plugin);
}



#endif