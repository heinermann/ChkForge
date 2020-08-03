#include "common.h"
#include "../bwgame.h"
#include "../replay.h"
#include "../../bwglobal_ui.h"

#include "native_window_drawing.h"
#include "native_sound.h"

namespace bwgame {

struct ui_util_functions: replay_functions {

	explicit ui_util_functions(state& st, action_state& action_st, replay_state& replay_st) : replay_functions(st, action_st, replay_st) {}

	rect sprite_clickable_bounds(const sprite_t* sprite) const {
		rect r{{(int)game_st.map_width - 1, (int)game_st.map_height - 1}, {0, 0}};
		for (const image_t* image : ptr(sprite->images)) {
			if (!i_flag(image, image_t::flag_clickable)) continue;
			xy pos = get_image_map_position(image);
			auto size = image->grp->frames.at(image->frame_index).size;
			xy to = pos + xy((int)size.x, (int)size.y);
			if (pos.x < r.from.x) r.from.x = pos.x;
			if (pos.y < r.from.y) r.from.y = pos.y;
			if (to.x > r.to.x) r.to.x = to.x;
			if (to.y > r.to.y) r.to.y = to.y;
		}
		return r;
	}

	bool unit_can_be_selected(const unit_t* u) const {
		if (unit_is(u, UnitTypes::Terran_Nuclear_Missile)) return false;
		if (unit_is(u, UnitTypes::Protoss_Scarab)) return false;
		if (unit_is(u, UnitTypes::Spell_Disruption_Web)) return false;
		if (unit_is(u, UnitTypes::Spell_Dark_Swarm)) return false;
		if (unit_is(u, UnitTypes::Special_Upper_Level_Door)) return false;
		if (unit_is(u, UnitTypes::Special_Right_Upper_Level_Door)) return false;
		if (unit_is(u, UnitTypes::Special_Pit_Door)) return false;
		if (unit_is(u, UnitTypes::Special_Right_Pit_Door)) return false;
		return true;
	}

	bool image_has_data_at(const image_t* image, xy pos) const {
		auto& frame = image->grp->frames.at(image->frame_index);
		xy map_pos = get_image_map_position(image);
		int x = pos.x - map_pos.x;
		if (i_flag(image, image_t::flag_horizontally_flipped)) x = image->grp->width - 2 * frame.offset.x - x;
		int y = pos.y - map_pos.y;
		if ((size_t)x >= frame.size.x) return false;
		if ((size_t)y >= frame.size.y) return false;

		const uint8_t* d = frame.data_container.data() + frame.line_data_offset.at(y);
		while (x > 0) {
			int v = *d++;
			if (v & 0x80) {
				v &= 0x7f;
				x -= v;
				if (x <= 0) return false;
			} else if (v & 0x40) {
				v &= 0x3f;
				d++;
				x -= v;
			} else {
				x -= v;
			}
		}
		return true;
	}

	bool unit_has_clickable_image_data_at(const unit_t* u, xy pos) const {
		if (!is_in_bounds(pos, sprite_clickable_bounds(u->sprite))) return false;
		if (ut_flag(u, unit_type_t::flag_100)) {
			for (const image_t* image : ptr(u->sprite->images)) {
				if (!i_flag(image, image_t::flag_clickable)) continue;
				if (image_has_data_at(image, pos)) return true;
			}
			return false;
		} else {
			return image_has_data_at(u->sprite->main_image, pos);
		}
	}

	unit_t* select_get_unit_at(xy pos) const {
		rect area = square_at(pos, 32);
		area.to += xy(game_st.max_unit_width, game_st.max_unit_height);
		unit_t* best_unit = nullptr;
		int best_unit_size{};
		for (unit_t* u : find_units_noexpand(area)) {
			if (!is_in_bounds(pos, sprite_clickable_bounds(u->sprite))) continue;
			u = unit_main_unit(u);
			if (!unit_can_be_selected(u)) continue;
			if (!best_unit) {
				best_unit = u;
				best_unit_size = u->unit_type->placement_size.x * u->unit_type->placement_size.y;
				continue;
			}
			if (sprite_depth_order(u->sprite) >= sprite_depth_order(best_unit->sprite)) {
				if (unit_has_clickable_image_data_at(u, pos) || (u->subunit && unit_has_clickable_image_data_at(u->subunit, pos))) {
					best_unit = u;
					best_unit_size = u->sprite->width * u->sprite->height;
					continue;
				}
			} else {
				if (unit_has_clickable_image_data_at(best_unit, pos)) continue;
				if (best_unit->subunit && unit_has_clickable_image_data_at(best_unit->subunit, pos)) continue;
			}
			if (u->unit_type->placement_size.x * u->unit_type->placement_size.y < best_unit_size) {
				best_unit = u;
				best_unit_size = u->unit_type->placement_size.x * u->unit_type->placement_size.y;
			}
		}
		return best_unit;
	}

	uint32_t sprite_depth_order(const sprite_t* sprite) const {
		uint32_t score = 0;
		score |= sprite->elevation_level;
		score <<= 13;
		score |= sprite->elevation_level <= 4 ? sprite->position.y : 0;
		score <<= 1;
		score |= s_flag(sprite, sprite_t::flag_turret) ? 1 : 0;
		return score;
	}

};

struct ui_functions: ui_util_functions {
	global_ui_state glob_ui;

	xy screen_pos;

	size_t screen_width;
	size_t screen_height;

	game_player player;
	replay_state current_replay_state;
	action_state current_action_state;

	ui_functions(game_player player) : ui_util_functions(player.st(), current_action_state, current_replay_state), player(std::move(player)) {
	}

	virtual void play_sound(int id, xy position, const unit_t* source_unit, bool add_race_index) override {
	  if (glob_ui.global_volume == 0) return;
	  if (add_race_index) id += 1;
	  if ((size_t)id >= glob_ui.has_loaded_sound.size()) return;
	  if (!glob_ui.has_loaded_sound[id]) {
		glob_ui.has_loaded_sound[id] = true;
		a_vector<uint8_t> data;
		glob_ui.load_data_file(data, "sound\\" + glob_ui.sound_filenames[id]);
		glob_ui.loaded_sounds[id] = native_sound::load_wav(data.data(), data.size());
	  }
	  auto& s = glob_ui.loaded_sounds[id];
	  if (!s) return;

	  auto now = clock.now();
	  if (now - glob_ui.last_played_sound[id] <= std::chrono::milliseconds(80)) return;
	  glob_ui.last_played_sound[id] = now;

	  const sound_type_t* sound_type = glob_ui.get_sound_type((Sounds)id);

	  int volume = sound_type->min_volume;

	  if (position != xy()) {
		int distance = 0;
		if (position.x < screen_pos.x) distance += screen_pos.x - position.x;
		else if (position.x > screen_pos.x + (int)screen_width) distance += position.x - (screen_pos.x + (int)screen_width);
		if (position.y < screen_pos.y) distance += screen_pos.y - position.y;
		else if (position.y > screen_pos.y + (int)screen_height) distance += position.y - (screen_pos.y + (int)screen_height);

		int distance_volume = 99 - 99 * distance / 512;

		if (distance_volume > volume) volume = distance_volume;
	  }

	  if (volume > 10) {
		int pan = 0;
		//			if (position != xy()) {
		//				int pan_x = (position.x - (screen_pos.x + (int)screen_width / 2)) / 32;
		//				bool left = pan_x < 0;
		//				if (left) pan_x = -pan_x;
		//				if (pan_x <= 2) pan = 0;
		//				else if (pan_x <= 5) pan = 52;
		//				else if (pan_x <= 10) pan = 127;
		//				else if (pan_x <= 20) pan = 191;
		//				else if (pan_x <= 40) pan = 230;
		//				else pan = 255;
		//				if (left) pan = -pan;
		//			}

		const unit_type_t* unit_type = source_unit ? source_unit->unit_type : nullptr;

		if (sound_type->flags & 0x10) {
		  for (auto& c : glob_ui.sound_channels) {
			if (c.playing && c.sound_type == sound_type) {
			  if (native_sound::is_playing(&c - glob_ui.sound_channels.data())) return;
			  c.playing = false;
			}
		  }
		}
		else if (sound_type->flags & 2 && unit_type) {
		  for (auto& c : glob_ui.sound_channels) {
			if (c.playing && c.unit_type == unit_type && c.flags & 2) {
			  if (native_sound::is_playing(&c - glob_ui.sound_channels.data())) return;
			  c.playing = false;
			}
		  }
		}

		auto* c = glob_ui.get_sound_channel(sound_type->priority);
		if (c) {
		  native_sound::play(c - glob_ui.sound_channels.data(), &*s, (128 - 4) * (volume * glob_ui.global_volume / 100) / 100, pan);
		  c->playing = true;
		  c->sound_type = sound_type;
		  c->flags = sound_type->flags;
		  c->priority = sound_type->priority;
		  c->unit_type = unit_type;
		  c->volume = volume;
		}
	  }
	}

	virtual void on_action(int owner, int action) override {
	}

	size_t view_width;
	size_t view_height;
	fp16 view_scale;

	rect_t<xy_t<size_t>> screen_tile_bounds() {
		size_t from_tile_y = screen_pos.y / 32u;
		if (from_tile_y >= game_st.map_tile_height) from_tile_y = 0;
		size_t to_tile_y = (screen_pos.y + view_height + 31) / 32u;
		if (to_tile_y > game_st.map_tile_height) to_tile_y = game_st.map_tile_height;
		size_t from_tile_x = screen_pos.x / 32u;
		if (from_tile_x >= game_st.map_tile_width) from_tile_x = 0;
		size_t to_tile_x = (screen_pos.x + view_width + 31) / 32u;
		if (to_tile_x > game_st.map_tile_width) to_tile_x = game_st.map_tile_width;

		return {{from_tile_x, from_tile_y}, {to_tile_x, to_tile_y}};
	}

	tileset_image_data tileset_img;

	void draw_tiles(uint8_t* data, size_t data_pitch) {

		auto screen_tile = screen_tile_bounds();

		size_t tile_index = screen_tile.from.y * game_st.map_tile_width + screen_tile.from.x;
		auto* megatile_index = &st.tiles_mega_tile_index[tile_index];
		auto* tile = &st.tiles[tile_index];
		size_t width = screen_tile.to.x - screen_tile.from.x;

		xy dirs[9] = {{1, 1}, {0, 1}, {-1, 1}, {1, 0}, {-1, 0}, {1, -1}, {0, -1}, {-1, -1}, {0, 0}};

		for (size_t tile_y = screen_tile.from.y; tile_y != screen_tile.to.y; ++tile_y) {
			for (size_t tile_x = screen_tile.from.x; tile_x != screen_tile.to.x; ++tile_x) {

				int screen_x = tile_x * 32 - screen_pos.x;
				int screen_y = tile_y * 32 - screen_pos.y;

				size_t offset_x = 0;
				size_t offset_y = 0;
				if (screen_x < 0) {
					offset_x = -screen_x;
				}
				if (screen_y < 0) {
					offset_y = -screen_y;
				}

				uint8_t* dst = data + screen_y * data_pitch + screen_x;

				size_t width = 32;
				size_t height = 32;

				width = std::min(width, screen_width - screen_x);
				height = std::min(height, screen_height - screen_y);

				size_t index = *megatile_index;
				if (tile->flags & tile_t::flag_has_creep) {
					index = game_st.cv5.at(1).mega_tile_index[glob_ui.creep_random_tile_indices[tile_x + tile_y * game_st.map_tile_width]];
				}
				draw_tile(tileset_img, index, dst, data_pitch, offset_x, offset_y, width, height);

				if (~tile->flags & tile_t::flag_has_creep) {
					size_t creep_index = 0;
					for (size_t i = 0; i != 9; ++i) {
						int add_x = dirs[i].x;
						int add_y = dirs[i].y;
						if (tile_x + add_x >= game_st.map_tile_width) continue;
						if (tile_y + add_y >= game_st.map_tile_height) continue;
						if (st.tiles[tile_x + add_x + (tile_y + add_y) * game_st.map_tile_width].flags & tile_t::flag_has_creep) creep_index |= 1 << i;
					}
					size_t creep_frame = glob_ui.img.creep_edge_frame_index[creep_index];

					if (creep_frame) {

						auto& frame = tileset_img.creep_grp.frames.at(creep_frame - 1);

						screen_x += frame.offset.x;
						screen_y += frame.offset.y;

						size_t width = frame.size.x;
						size_t height = frame.size.y;

						if (screen_x < (int)screen_width && screen_y < (int)screen_height) {
							if (screen_x + (int)width > 0 && screen_y + (int)height > 0) {

								size_t offset_x = 0;
								size_t offset_y = 0;
								if (screen_x < 0) {
									offset_x = -screen_x;
								}
								if (screen_y < 0) {
									offset_y = -screen_y;
								}

								uint8_t* dst = data + screen_y * data_pitch + screen_x;

								width = std::min(width, screen_width - screen_x);
								height = std::min(height, screen_height - screen_y);

								draw_frame(frame, false, dst, data_pitch, offset_x, offset_y, width, height);
							}
						}
					}
				}

				++megatile_index;
				++tile;
			}
			megatile_index -= width;
			megatile_index += game_st.map_tile_width;
			tile -= width;
			tile += game_st.map_tile_width;
		}
	}

	a_vector<uint8_t> temporary_warp_texture_buffer;

	void draw_image(const image_t* image, uint8_t* data, size_t data_pitch, size_t color_index) {

		xy map_pos = get_image_map_position(image);

		int screen_x = map_pos.x - screen_pos.x;
		int screen_y = map_pos.y - screen_pos.y;

		if (screen_x >= (int)screen_width || screen_y >= (int)screen_height) return;

		auto& frame = image->grp->frames.at(image->frame_index);

		size_t width = frame.size.x;
		size_t height = frame.size.y;

		if (screen_x + (int)width <= 0 || screen_y + (int)height <= 0) return;

		size_t offset_x = 0;
		size_t offset_y = 0;
		if (screen_x < 0) {
			offset_x = -screen_x;
		}
		if (screen_y < 0) {
			offset_y = -screen_y;
		}

		uint8_t* dst = data + screen_y * data_pitch + screen_x;

		width = std::min(width, screen_width - screen_x);
		height = std::min(height, screen_height - screen_y);

		auto draw_alpha = [&](size_t index, auto remap_f) {
			auto& data = tileset_img.light_pcx.at(index).data;
			uint8_t* ptr = data.data();
			size_t size = data.size() / 256;
			auto glow = [ptr, size, remap_f](uint8_t new_value, uint8_t old_value) {
				new_value = remap_f(new_value, old_value);
				--new_value;
				if (new_value >= size) return (uint8_t)0;
				return ptr[256u * new_value + old_value];
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, glow);
		};

		if (image->modifier == 0 || image->modifier == 1) {
			uint8_t* ptr = glob_ui.img.player_unit_colors.at(color_index).data();
			auto player_color = [ptr](uint8_t new_value, uint8_t) {
				if (new_value >= 8 && new_value < 16) return ptr[new_value - 8];
				return new_value;
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, player_color);
		} else if (image->modifier == 2 || image->modifier == 4) {
			uint8_t* color_ptr = glob_ui.img.player_unit_colors.at(color_index).data();
			draw_alpha(4, [color_ptr](uint8_t new_value, uint8_t) {
				if (new_value >= 8 && new_value < 16) return color_ptr[new_value - 8];
				return new_value;
			});
			uint8_t* selector = tileset_img.cloak_fade_selector.data();
			int value = image->modifier_data1;
			auto cloaking = [color_ptr, selector, value](uint8_t new_value, uint8_t old_value) {
				if (selector[new_value] <= value) return old_value;
				if (new_value >= 8 && new_value < 16) return color_ptr[new_value - 8];
				return new_value;
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, cloaking);
		} else if (image->modifier == 3) {
			uint8_t* color_ptr = glob_ui.img.player_unit_colors.at(color_index).data();
			draw_alpha(4, [color_ptr](uint8_t new_value, uint8_t) {
				if (new_value >= 8 && new_value < 16) return color_ptr[new_value - 8];
				return new_value;
			});
		} else if (image->modifier == 8) {
			size_t data_size = data_pitch * screen_height;
			auto distortion = [data_size, dst](uint8_t new_value, uint8_t& old_value) {
				size_t offset = &old_value - dst;
				if (offset >= new_value && data_size - offset > new_value) return *(&old_value + new_value);
				return old_value;
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, distortion);
		} else if (image->modifier == 10) {
			uint8_t* ptr = &tileset_img.dark_pcx.data[256 * 18];
			auto shadow = [ptr](uint8_t, uint8_t old_value) {
				return ptr[old_value];
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, shadow);
		} else if (image->modifier == 9) {
			draw_alpha(image->image_type->color_shift - 1, no_remap());
		} else if (image->modifier == 12) {
			if (temporary_warp_texture_buffer.size() < frame.size.x * frame.size.y) temporary_warp_texture_buffer.resize(frame.size.x * frame.size.y);
			auto& texture_frame = global_st.image_grp[(size_t)ImageTypes::IMAGEID_Warp_Texture]->frames.at(image->modifier_data1);
			draw_frame(texture_frame, false, temporary_warp_texture_buffer.data(), frame.size.x, 0, 0, frame.size.x, frame.size.y);
			draw_frame_textured(frame, temporary_warp_texture_buffer.data(), i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height);
		} else if (image->modifier == 17) {
			auto& data = tileset_img.light_pcx.at(0).data;
			uint8_t* ptr = &data.at(256u * (image->modifier_data1 - 1));
			size_t size = data.data() + data.size() - ptr;
			auto glow = [ptr, size](uint8_t, uint8_t old_value) {
				if (old_value >= size) return (uint8_t)0;
				return ptr[old_value];
			};
			draw_frame(frame, i_flag(image, image_t::flag_horizontally_flipped), dst, data_pitch, offset_x, offset_y, width, height, glow);
		} else error("don't know how to draw image modifier %d", image->modifier);

	}

	a_vector<const unit_t*> current_selection_sprites_set = a_vector<const unit_t*>(2500);
	a_vector<const sprite_t*> current_selection_sprites;

	void draw_selection_circle(const sprite_t* sprite, const unit_t* u, uint8_t* data, size_t data_pitch) {
		auto* image_type = get_image_type((ImageTypes)((int)ImageTypes::IMAGEID_Selection_Circle_22pixels + sprite->sprite_type->selection_circle));

		xy map_pos = sprite->position + xy(0, sprite->sprite_type->selection_circle_vpos);

		auto* grp = global_st.image_grp[(size_t)image_type->id];
		auto& frame = grp->frames.at(0);

		map_pos.x += int(frame.offset.x - grp->width / 2);
		map_pos.y += int(frame.offset.y - grp->height / 2);

		int screen_x = map_pos.x - screen_pos.x;
		int screen_y = map_pos.y - screen_pos.y;

		if (screen_x >= (int)screen_width || screen_y >= (int)screen_height) return;

		size_t width = frame.size.x;
		size_t height = frame.size.y;

		if (screen_x + (int)width <= 0 || screen_y + (int)height <= 0) return;

		size_t offset_x = 0;
		size_t offset_y = 0;
		if (screen_x < 0) {
			offset_x = -screen_x;
		}
		if (screen_y < 0) {
			offset_y = -screen_y;
		}

		uint8_t* dst = data + screen_y * data_pitch + screen_x;

		width = std::min(width, screen_width - screen_x);
		height = std::min(height, screen_height - screen_y);

		size_t color_index = st.players[sprite->owner].color;
		uint8_t color = glob_ui.img.player_unit_colors.at(color_index)[0];
		if (unit_is_mineral_field(u) || unit_is(u, UnitTypes::Resource_Vespene_Geyser)) {
			color = tileset_img.resource_minimap_color;
		}
		auto player_color = [color](uint8_t new_value, uint8_t) {
			if (new_value >= 0 && new_value < 8) return color;
			return new_value;
		};
		draw_frame(frame, false, dst, data_pitch, offset_x, offset_y, width, height, player_color);

	}

	void draw_health_bars(const sprite_t* sprite, const unit_t* u, uint8_t* data, size_t data_pitch) {

		auto* selection_circle_image_type = get_image_type((ImageTypes)((int)ImageTypes::IMAGEID_Selection_Circle_22pixels + sprite->sprite_type->selection_circle));

		auto* selection_circle_grp = global_st.image_grp[(size_t)selection_circle_image_type->id];
		auto& selection_circle_frame = selection_circle_grp->frames.at(0);

		int offsety = sprite->sprite_type->selection_circle_vpos + selection_circle_frame.size.y / 2 + 8;

		bool has_shield = u->unit_type->has_shield;
		bool has_energy = ut_has_energy(u) || u_hallucination(u) || unit_is(u, UnitTypes::Zerg_Broodling);

		int width = sprite->sprite_type->health_bar_size;
		width -= (width - 1) % 3;
		if (width < 19) width = 19;
		int orig_width = width;
		int height = 5;
		if (has_shield) height += 2;
		if (has_energy) height += 6;

		xy map_pos = sprite->position + xy(0, offsety);

		map_pos.x += int(0 - width / 2);
		map_pos.y += int(0 - height / 2);

		int screen_x = map_pos.x - screen_pos.x;
		int screen_y = map_pos.y - screen_pos.y;

		if (screen_x >= (int)screen_width || screen_y >= (int)screen_height) return;
		if (screen_x + width <= 0 || screen_y + height <= 0) return;

		auto filled_width = [&](int percent) {
			int r = percent * width / 100;
			if (r < 3) r = 3;
			else if (r % 3) {
				if (r % 3 > 1) r += 3 - (r % 3);
				else r -= r % 3;
			}
			return r;
		};

		int hp_percent = unit_hp_percent(u);
		int dw = filled_width(hp_percent);

		int shield_dw = 0;
		if (has_shield) {
			int shield_percent = (int)u->shield_points.integer_part() * 100 / std::max(u->unit_type->shield_points, 1);
			shield_dw = filled_width(shield_percent);
		}

		int energy_dw = 0;
		if (has_energy) {
			int energy_percent;
			if (ut_has_energy(u)) energy_percent = (int)u->energy.integer_part() * 100 / std::max((int)unit_max_energy(u).integer_part(), 1);
			else energy_percent = (int)u->remove_timer / std::max((int)default_remove_timer(u), 1);
			energy_dw = filled_width(energy_percent);
		}

		const int no_shield_colors_66[] = {18, 0, 1, 2, 18};
		const int no_shield_colors_33[] = {18, 3, 4, 5, 18};
		const int no_shield_colors_0[] = {18, 6, 7, 8, 18};
		const int no_shield_colors_bg[] = {18, 15, 16, 17, 18};

		const int with_shield_colors_66[] = {18, 0, 0, 1, 1, 2, 18};
		const int with_shield_colors_33[] = {18, 3, 3, 4, 4, 5, 18};
		const int with_shield_colors_0[] = {18, 6, 6, 7, 7, 8, 18};
		const int with_shield_colors_bg[] = {18, 15, 15, 16, 16, 17, 18};

		const int* colors_66 = has_shield ? with_shield_colors_66 : no_shield_colors_66;
		const int* colors_33 = has_shield ? with_shield_colors_33 : no_shield_colors_33;
		const int* colors_0 = has_shield ? with_shield_colors_0 : no_shield_colors_0;
		const int* colors_bg = has_shield ? with_shield_colors_bg : no_shield_colors_bg;

		int offset_x = 0;
		int offset_y = 0;
		if (screen_x < 0) {
			offset_x = -screen_x;
			dw = std::max(dw + screen_x, 0);
			shield_dw = std::max(shield_dw + screen_x, 0);
			energy_dw = std::max(energy_dw + screen_x, 0);
			width = std::max(width + screen_x, 0);
			screen_x = 0;
		}
		if (screen_y < 0) {
			offset_y = -screen_y;
			height += screen_y;
			screen_y = 0;
		}

		uint8_t* dst = data + screen_y * data_pitch + screen_x;

		width = std::min(width, (int)screen_width - screen_x);
		height = std::min(height, (int)screen_height - screen_y);

		if (dw > width) dw = width;
		if (shield_dw > width) shield_dw = width;
		if (energy_dw > width) energy_dw = width;

		int hp_height = std::min(std::max((has_shield ? 7 : 5) - offset_y, 0), height);

		for (int i = offset_y; i < offset_y + hp_height; ++i) {
			int ci = hp_percent >= 66 ? colors_66[i] : hp_percent >= 33 ? colors_33[i] : colors_0[i];
			int c = glob_ui.img.hp_bar_colors.at(ci);

			if (dw > 0) memset(dst, c, dw);
			if (width - dw > 0) {
				c = glob_ui.img.hp_bar_colors.at(colors_bg[i]);
				memset(dst + dw, c, width - dw);
			}
			dst += data_pitch;
		}

		if (has_shield) {
			const int shield_colors[] = {18, 10, 11, 18};
			const int shield_colors_bg[] = {18, 16, 17, 18};

			dst = data + screen_y * data_pitch + screen_x;

			for (int i = offset_y; i < std::min(4, height); ++i) {
				int c = glob_ui.img.hp_bar_colors.at(shield_colors[i]);

				if (shield_dw > 0) memset(dst, c, shield_dw);
				if (width - shield_dw > 0) {
					c = glob_ui.img.hp_bar_colors.at(shield_colors_bg[i]);
					memset(dst + shield_dw, c, width - shield_dw);
				}
				dst += data_pitch;
			}
		}

		int energy_offset = std::max((has_shield ? 8 : 6) - offset_y, 0);
		int energy_begin = std::max(offset_y - (has_shield ? 8 : 6), 0);
		int energy_end = std::min(5, offset_y + height - (has_shield ? 8 : 6));

		if (has_energy ) {
			dst = data + (screen_y + energy_offset) * data_pitch + screen_x;
			const int energy_colors[] = {18, 12, 13, 14, 18};
			for (int i = energy_begin; i < energy_end; ++i) {
				int c = glob_ui.img.hp_bar_colors.at(energy_colors[i]);

				if (energy_dw > 0) memset(dst, c, energy_dw);
				if (width - energy_dw > 0) {
					c = glob_ui.img.hp_bar_colors.at(no_shield_colors_bg[i]);
					memset(dst + energy_dw, c, width - energy_dw);
				}
				dst += data_pitch;
			}
		}

		dst = data + screen_y * data_pitch + screen_x;
		if (offset_x % 3) dst += 3 - offset_x % 3;

		int c = glob_ui.img.hp_bar_colors.at(18);
		for (int x = 0; x < orig_width; x += 3) {
			if (x < offset_x || x >= offset_x + width) continue;
			for (int y = 0; y != hp_height; ++y) {
				*dst = c;
				dst += data_pitch;
			}
			if (has_energy && energy_end > energy_begin) {
				if (energy_offset) dst += data_pitch;
				for (int i = energy_begin; i != energy_end; ++i) {
					*dst = c;
					dst += data_pitch;
				}
				if (energy_offset) dst -= data_pitch;
				dst -= (energy_end - energy_begin) * data_pitch;
			}
			dst -= hp_height * data_pitch;
			dst += 3;
		}

	}

	void draw_sprite(const sprite_t* sprite, uint8_t* data, size_t data_pitch) {
		const unit_t* draw_selection_u = current_selection_sprites_set.at(sprite->index);
		const unit_t* draw_health_bars_u = draw_selection_u;
		for (auto* image : ptr(reverse(sprite->images))) {
			if (i_flag(image, image_t::flag_hidden)) continue;
			if (draw_selection_u && image->modifier != 10) {
				draw_selection_circle(sprite, draw_selection_u, data, data_pitch);
				draw_selection_u = nullptr;
			}
			draw_image(image, data, data_pitch, st.players[sprite->owner].color);
		}
		if (draw_health_bars_u && !u_invincible(draw_health_bars_u)) {
			draw_health_bars(sprite, draw_health_bars_u, data, data_pitch);
		}
	}

	a_vector<std::pair<uint32_t, const sprite_t*>> sorted_sprites;

	void draw_sprites(uint8_t* data, size_t data_pitch) {

		sorted_sprites.clear();

		auto screen_tile = screen_tile_bounds();

		size_t from_y = screen_tile.from.y;
		if (from_y < 4) from_y = 0;
		else from_y -= 4;
		size_t to_y = screen_tile.to.y;
		if (to_y >= game_st.map_tile_height - 4) to_y = game_st.map_tile_height - 1;
		else to_y += 4;
		for (size_t y = from_y; y != to_y; ++y) {
			for (auto* sprite : ptr(st.sprites_on_tile_line.at(y))) {
				if (s_hidden(sprite)) continue;
				sorted_sprites.emplace_back(sprite_depth_order(sprite), sprite);
			}
		}

		std::sort(sorted_sprites.begin(), sorted_sprites.end());

		for (auto uid : current_selection) {
			auto* u = get_unit(uid);
			if (!u) continue;
			current_selection_sprites_set.at(u->sprite->index) = u;
			current_selection_sprites.push_back(u->sprite);
		}

		for (auto& v : sorted_sprites) {
			draw_sprite(v.second, data, data_pitch);
		}

		for (auto* s : current_selection_sprites) {
			current_selection_sprites_set.at(s->index) = nullptr;
		}
		current_selection_sprites.clear();
	}

	void fill_rectangle(uint8_t* data, size_t data_pitch, rect area, uint8_t index) {
		if (area.from.x < 0) area.from.x = 0;
		if (area.from.y < 0) area.from.y = 0;
		if (area.to.x > (int)screen_width) area.to.x = screen_width;
		if (area.to.y > (int)screen_height) area.to.y = screen_height;
		if (area.from.x >= area.to.x || area.from.y >= area.to.y) return;
		size_t width = area.to.x - area.from.x;
		size_t pitch = data_pitch;
		size_t from_y = area.from.y;
		size_t to_y = area.to.y;
		uint8_t* ptr = data + data_pitch * from_y + area.from.x;
		for (size_t i = from_y; i != to_y; ++i) {
			memset(ptr, index, width);
			ptr += pitch;
		}
	}

	bool unit_visble_on_minimap(unit_t* u) {
		if (u->owner < 8 && u->sprite->visibility_flags == 0) return false;
		if (ut_turret(u)) return false;
		if (unit_is_trap(u)) return false;
		if (unit_is(u, UnitTypes::Spell_Dark_Swarm)) return false;
		if (unit_is(u, UnitTypes::Spell_Disruption_Web)) return false;
		return true;
	}

	xy get_minimap_size() {
		size_t minimap_width = std::max(game_st.map_tile_width, game_st.map_tile_height);
		size_t minimap_height = std::max(game_st.map_tile_width, game_st.map_tile_height);

		if (game_st.map_width < game_st.map_height) {
			minimap_width = minimap_width * minimap_width * game_st.map_tile_width / (minimap_height * game_st.map_tile_height);
		} else if (game_st.map_height < game_st.map_width) {
			minimap_height = minimap_height * minimap_height * game_st.map_tile_height / (minimap_width* game_st.map_tile_width);
		}
		if (screen_width < minimap_width || screen_height < minimap_height) return {};
		
		return xy{ (int)minimap_width, (int)minimap_height };
	}

	void draw_minimap(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height) {
	  xy minimap_size = get_minimap_size();

	  if (minimap_size.x > surface_width) return;
	  if (minimap_size.y > surface_height) return;

	  if (minimap_size.x != game_st.map_tile_width) return;
	  if (minimap_size.y != game_st.map_tile_height) return;
	  fill_rectangle(data, data_pitch, rect{ xy{ 0, 0 }, minimap_size }, 0);

	  uint8_t* p = data;

	  size_t pitch = data_pitch - game_st.map_tile_width;
	  for (size_t y = 0; y != game_st.map_tile_height; ++y) {
		  for (size_t x = 0; x != game_st.map_tile_width; ++x) {
			  size_t index;
			  if (~st.tiles[y * game_st.map_tile_width + x].flags & tile_t::flag_has_creep) index = st.tiles_mega_tile_index[y * game_st.map_tile_width + x];
			  else index = game_st.cv5.at(1).mega_tile_index[glob_ui.creep_random_tile_indices[y * game_st.map_tile_width + x]];
			  auto* images = &tileset_img.vx4.at(index).images[0];
			  auto* bitmap = &tileset_img.vr4.at(*images / 2).bitmap[0];
			  auto val = bitmap[55 / sizeof(vr4_entry::bitmap_t)];
			  size_t shift = 8 * (55 % sizeof(vr4_entry::bitmap_t));
			  val >>= shift;
			  *p++ = (uint8_t)val;
		  }
		  p += pitch;
	  }

	  for (size_t i = 12; i != 0;) {
		  --i;
		  for (unit_t* u : ptr(st.player_units[i])) {
			  if (!unit_visble_on_minimap(u)) continue;
			  int color = glob_ui.img.player_minimap_colors.at(st.players[u->owner].color);
			  size_t w = u->unit_type->placement_size.x / 32u;
			  size_t h = u->unit_type->placement_size.y / 32u;
			  if (unit_is_mineral_field(u) || unit_is(u, UnitTypes::Resource_Vespene_Geyser)) {
				  color = tileset_img.resource_minimap_color;
			  }
			  if (ut_building(u)) {
				  if (w > 4) w = 4;
				  if (h > 4) h = 4;
			  }
			  if (w < 2) w = 2;
			  if (h < 2) h = 2;
			  rect unit_area;
			  unit_area.from = (u->sprite->position - u->unit_type->placement_size / 2) / 32u;
			  unit_area.to = unit_area.from + xy(w, h);
			  fill_rectangle(data, data_pitch, unit_area, color);
		  }
	  }
	}

	native_window_drawing::color* get_palette() {
	  if (want_new_palette) set_image_data();
	  return palette_colors;
	}

	int replay_frame = 0;

	virtual void draw_callback(uint8_t* data, size_t data_pitch) {
	}

	fp8 game_speed = fp8::integer(1);

	bool want_new_palette = true;
	native_window_drawing::color palette_colors[256];
	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point last_draw;
	std::chrono::high_resolution_clock::time_point last_input_poll;
	std::chrono::high_resolution_clock::time_point last_fps;
	int fps_counter = 0;
	size_t scroll_speed_n = 0;

	void resize(int width, int height) {
		screen_width = width;
		screen_height = height;
		//view_scale = fp16::integer(1) - (fp16::integer(1) / 4);
		view_scale = fp16::integer(1);
		view_width = (fp16::integer(screen_width) / view_scale).integer_part();
		view_height = (fp16::integer(screen_height) / view_scale).integer_part();
		view_scale = (ufp16::integer(screen_width) / view_width).as_signed();
	}

	a_vector<unit_id> current_selection;

	bool current_selection_is_selected(unit_t* u) {
		auto uid = get_unit_id(u);
		return std::find(current_selection.begin(), current_selection.end(), uid) != current_selection.end();
	}

	void current_selection_add(unit_t* u) {
		auto uid = get_unit_id(u);
		if (std::find(current_selection.begin(), current_selection.end(), uid) != current_selection.end()) return;
		current_selection.push_back(uid);
	}

	void current_selection_clear() {
		current_selection.clear();
	}

	void current_selection_remove(const unit_t* u) {
		auto uid = get_unit_id(u);
		auto i = std::find(current_selection.begin(), current_selection.end(), uid);
		if (i != current_selection.end()) current_selection.erase(i);
	}

	void select_units(bool double_clicked, bool shift, bool ctrl, int from_x, int from_y, int to_x, int to_y) {
	  if (from_x > to_x) std::swap(from_x, to_x);
	  if (from_y > to_y) std::swap(from_y, to_y);
	  if (to_x - from_x <= 4 || to_y - from_y <= 4) {
		unit_t* u = select_get_unit_at(screen_pos + xy(from_x, from_y));
		if (u) {
		  if (double_clicked || ctrl) {
			if (!shift) current_selection_clear();
			auto is_tank = [&](unit_t* a) {
			  return unit_is(a, UnitTypes::Terran_Siege_Tank_Siege_Mode) || unit_is(a, UnitTypes::Terran_Siege_Tank_Tank_Mode);
			};
			auto is_same_type = [&](unit_t* a, unit_t* b) {
			  if (unit_is_mineral_field(a) && unit_is_mineral_field(b)) return true;
			  if (is_tank(a) && is_tank(b)) return true;
			  return a->unit_type == b->unit_type;
			};
			for (unit_t* u2 : find_units({ screen_pos, screen_pos + xy(view_width, view_height) })) {
			  if (u2->owner != u->owner) continue;
			  if (!is_same_type(u, u2)) continue;
			  current_selection_add(u2);
			}
		  }
		  else {
			if (shift) {
			  if (current_selection_is_selected(u)) current_selection_remove(u);
			  else current_selection_add(u);
			}
			else {
			  current_selection_clear();
			  current_selection_add(u);
			}
		  }
		}
	  }
	  else {
		if (!shift) current_selection_clear();
		auto r = rect{ {from_x, from_y}, {to_x, to_y} };
		if (r.from.x > r.to.x) std::swap(r.from.x, r.to.x);
		if (r.from.y > r.to.y) std::swap(r.from.y, r.to.y);
		a_vector<unit_t*> new_units;
		bool any_non_neutrals = false;
		for (unit_t* u : find_units(translate_rect(r, screen_pos))) {
		  if (!unit_can_be_selected(u)) continue;
		  new_units.push_back(u);
		  if (u->owner != 11) any_non_neutrals = true;
		}
		for (unit_t* u : new_units) {
		  if (u->owner == 11 && any_non_neutrals) continue;
		  current_selection_add(u);
		}
	  }
	}

	void set_screen_pos(int x, int y) {
	  screen_pos.x = x;
	  screen_pos.y = y;
	  if (screen_pos.y + view_height > game_st.map_height) screen_pos.y = game_st.map_height - view_height;
	  if (screen_pos.y < 0) screen_pos.y = 0;
	  if (screen_pos.x + view_width > game_st.map_width) screen_pos.x = game_st.map_width - view_width;
	  if (screen_pos.x < 0) screen_pos.x = 0;
	}
	
	void draw_game(uint8_t* data, size_t data_pitch, size_t surface_width, size_t surface_height) {
		auto now = clock.now();

		if (now - last_fps >= std::chrono::seconds(1)) {
			//ui::log("draw fps: %g\n", fps_counter / std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1>>>(now - last_fps).count());
			last_fps = now;
			fps_counter = 0;
		}
		++fps_counter;

		if (want_new_palette) set_image_data();

		/*
		if (wnd) {
			auto input_poll_speed = std::chrono::milliseconds(12);
		
			auto input_poll_t = now - last_input_poll;
			while (input_poll_t >= input_poll_speed) {
				if (input_poll_t >= input_poll_speed * 20) last_input_poll = now - input_poll_speed;
				else last_input_poll += input_poll_speed;
				std::array<int, 6> scroll_speeds = {2, 2, 4, 6, 6, 8};
		
				if (!is_drag_selecting) {
					int scroll_speed = scroll_speeds[scroll_speed_n];
					auto prev_screen_pos = screen_pos;
					if (wnd.get_key_state(81)) screen_pos.y += scroll_speed;
					else if (wnd.get_key_state(82)) screen_pos.y -= scroll_speed;
					if (wnd.get_key_state(79)) screen_pos.x += scroll_speed;
					else if (wnd.get_key_state(80)) screen_pos.x -= scroll_speed;
					if (screen_pos != prev_screen_pos) {
						if (scroll_speed_n != scroll_speeds.size() - 1) ++scroll_speed_n;
					} else scroll_speed_n = 0;
				}
		
				input_poll_t = now - last_input_poll;
			}
		}*/

		draw_tiles(data, data_pitch);
		draw_sprites(data, data_pitch);

		draw_callback(data, data_pitch);
	}

	void move_minimap(int mouse_x, int mouse_y) {
	  auto minimap_size = get_minimap_size();
	  if (minimap_size.x == 0 || minimap_size.y == 0) return;
	  
	  if (mouse_x < 0) mouse_x = 0;
	  else if (mouse_x >= minimap_size.x) mouse_x = minimap_size.x - 1;
	  
	  if (mouse_y < 0) mouse_y = 0;
	  else if (mouse_y >= minimap_size.y) mouse_y = minimap_size.y - 1;
	  
	  int x = mouse_x * game_st.map_tile_width / minimap_size.x;
	  int y = mouse_y * game_st.map_tile_height / minimap_size.y;
	  set_screen_pos(32 * x - view_width / 2, 32 * y - view_height / 2);
	}

	void set_image_data() {
		tileset_img = glob_ui.all_tileset_img.at(game_st.tileset_index);

		want_new_palette = false;

		if (tileset_img.wpe.size() != 256 * 4) error("wpe size invalid (%d)", tileset_img.wpe.size());
		for (size_t i = 0; i != 256; ++i) {
			palette_colors[i].r = tileset_img.wpe[4 * i + 0];
			palette_colors[i].g = tileset_img.wpe[4 * i + 1];
			palette_colors[i].b = tileset_img.wpe[4 * i + 2];
			palette_colors[i].a = tileset_img.wpe[4 * i + 3];
		}
	}

	void reset() {
		replay_frame = 0;
		auto& game = *st.game;
		st = state();
		game = game_state();
		replay_st = replay_state();
		action_st = action_state();

		st.global = &global_st;
		st.game = &game;
	}
};

}

