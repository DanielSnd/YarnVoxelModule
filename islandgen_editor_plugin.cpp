//
// Created by Daniel on 2024-01-07.
//


#ifdef TOOLS_ENABLED

#include "islandgen_editor_plugin.h"

class IslandGeneratorPreview : public Control {
	GDCLASS(IslandGeneratorPreview, Control)

	static const int PREVIEW_HEIGHT = 256;
	static const int PADDING_PREVIEW_INFO = 2;
	Label *_preview_info_label = nullptr;
	Button *_randomize_seed_button = nullptr;
	Button *_clear_all_button = nullptr;
	Button *_generate_button = nullptr;
	CheckButton *_auto_generate_button = nullptr;
	Color use_color;
	Ref<IslandGenerator> _island_generator;
	Size2i _preview_texture_size;

	TextureRect *_texture_rect = nullptr;
	TextureRect *_cave_texture_rect = nullptr;
	//Button *_3d_space_switch = nullptr;

public:
	IslandGeneratorPreview() {
		auto hbox = memnew(HBoxContainer);
		add_child(hbox);
		hbox->set_anchors_and_offsets_preset(Control::PRESET_TOP_WIDE);
		set_custom_minimum_size(Size2(0, EDSCALE * (PREVIEW_HEIGHT)));
		_texture_rect = memnew(TextureRect);
		_texture_rect->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
		_texture_rect->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH_PROPORTIONAL);
		_texture_rect->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
		_texture_rect->set_offset(SIDE_TOP,42);
		add_child(_texture_rect);

		_preview_info_label = memnew(Label);
		_preview_info_label->set_text(TTR("3D"));
		_preview_info_label->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_preview_info_label->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 25);
		add_child(_preview_info_label);

		_randomize_seed_button = memnew(Button);
		_randomize_seed_button->set_text(TTR("Random Seed"));
		_randomize_seed_button->set_tooltip_text(TTR("Randomizes the seed."));
		_randomize_seed_button->connect("pressed", callable_mp(this, &IslandGeneratorPreview::_on_random_seed_button_pressed));
		hbox->add_child(_randomize_seed_button);

		_clear_all_button = memnew(Button);
		_clear_all_button->set_text(TTR("Clear Chunks"));
		_clear_all_button->set_tooltip_text(TTR("Clear all chunks."));
		_clear_all_button->connect("pressed", callable_mp(this, &IslandGeneratorPreview::_on_clear_all_button_pressed));
		hbox->add_child(_clear_all_button);

		_generate_button = memnew(Button);
		_generate_button->set_text(TTR("Generate"));
		_generate_button->set_tooltip_text(TTR("Generate island."));
		_generate_button->connect("pressed", callable_mp(this, &IslandGeneratorPreview::_on_generate_button_pressed));
		hbox->add_child(_generate_button);


		// Wide empty separation control. (like BoxContainer::add_spacer())
		Control *c = memnew(Control);
		c->set_mouse_filter(MOUSE_FILTER_PASS);
		c->set_h_size_flags(SIZE_EXPAND_FILL);
		hbox->add_child(c);

		_auto_generate_button = memnew(CheckButton);
		_auto_generate_button->set_text(TTR("Auto-generate"));
		_auto_generate_button->set_tooltip_text(TTR("Auto Generate island as you edit values."));
		_auto_generate_button->set_pressed(false);
		hbox->add_child(_auto_generate_button);

		_cave_texture_rect = memnew(TextureRect);
		_cave_texture_rect->set_expand_mode(TextureRect::EXPAND_FIT_WIDTH);
		_cave_texture_rect->set_stretch_mode(TextureRect::STRETCH_SCALE);
		_cave_texture_rect->set_offset(SIDE_LEFT, PADDING_PREVIEW_INFO);
		_cave_texture_rect->set_offset(SIDE_TOP, PADDING_PREVIEW_INFO * 54);
		_cave_texture_rect->set_custom_minimum_size(Size2(64+32,64+32));
		add_child(_cave_texture_rect);
	}

	void _on_generate_button_pressed() {
		YarnVoxel::get_singleton()->empty_all_chunks();
		_island_generator->generate_island(Vector3((static_cast<float>(_island_generator->map_size.x)*-0.5f),0,static_cast<float>(_island_generator->map_size.y)*0.5f));
	}

	void _on_clear_all_button_pressed() {
		YarnVoxel::get_singleton()->clear_all_chunks();
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
			if (_island_generator->color_ramp.is_null() || !_island_generator->color_ramp.is_valid()) {
				_island_generator->color_ramp.instantiate();
				_island_generator->color_ramp->set_interpolation_mode(Gradient::GRADIENT_INTERPOLATE_CUBIC);
				_island_generator->color_ramp->set_offsets({0, 0.2125, 0.25625, 0.320833, 0.39375, 0.795833, 0.827083});
				_island_generator->color_ramp->set_colors({Color(0.369637, 0.613445, 0.951805, 1), Color(0.501961, 0.698039, 0.972549, 1), Color(0.972549, 0.776471, 0.501961, 1), Color(0.990579, 0.69581, 0.404316, 1), Color(0.467362, 0.714254, 0.467767, 1), Color(0.653113, 0.884651, 0.647313, 1), Color(0.674644, 0.911331, 0.926547, 1)});
			}
			if (!_island_generator->color_ramp.is_null() && _island_generator->color_ramp.is_valid()) {
				_island_generator->color_ramp->connect_changed(callable_mp(this, &IslandGeneratorPreview::update_preview));
			}
			if (_island_generator->gen_effects.is_empty()) {
				Ref<IGE_FastNoise> fast_noise;
				fast_noise.instantiate();
				fast_noise->noise.instantiate();
				_island_generator->gen_effects.append(fast_noise);
				fast_noise->connect_changed(callable_mp(this, &IslandGeneratorPreview::update_preview));
				fast_noise->noise->connect_changed(callable_mp(this, &IslandGeneratorPreview::update_preview));
			}
			update_preview();
		}
	}

private:
	Ref<Image> img;
	Ref<ImageTexture> imgTexture;
	Ref<Image> caveimg;
	Ref<ImageTexture> caveimgTexture;
	int waiting_auto_regen = 1;
	bool started_process=false;
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_RESIZED: {
				_preview_texture_size = get_size();
				update_preview();
			} break;
			case NOTIFICATION_PROCESS: {
				if (waiting_auto_regen > 0) {
					if (_auto_generate_button != nullptr && _auto_generate_button->is_pressed()) {
						waiting_auto_regen+=1;
						if (waiting_auto_regen > 12) {
							waiting_auto_regen = 0;
							set_process(false);
							started_process=false;
							_on_generate_button_pressed();
						}
					} else {
						waiting_auto_regen = 0;
						set_process(false);
						started_process=false;
					}
				}
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
		if (_auto_generate_button != nullptr && _auto_generate_button->is_pressed()) {
			waiting_auto_regen = 1;
			if (!started_process) {
				started_process = true;
				set_process(true);
			}
		} else {
			if (started_process) {
				started_process = false;
				set_process(false);
			}
		}
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

IslandGenEffectEditorPlugin::IslandGenEffectEditorPlugin() {
	Ref<IslandGenEffectInspectorPlugin> plugin;
	plugin.instantiate();
	add_inspector_plugin(plugin);
}



#endif