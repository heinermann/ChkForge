#pragma once
#ifndef OPENBW_GLOBALS_H
#define OPENBW_GLOBALS_H

#include <functional>

#include "openbw/data_types.h"
#include "openbw/game_types.h"
#include "openbw/data_loading.h"

namespace bwgame {

  template<typename reader_T>
  grp_t read_grp(reader_T&& r) {
	auto base_r = r;
	grp_t grp;
	size_t frame_count = r.template get<uint16_t>();
	grp.width = r.template get<uint16_t>();
	grp.height = r.template get<uint16_t>();
	grp.frames.resize(frame_count);
	for (size_t i = 0; i != frame_count; ++i) {
	  auto& f = grp.frames[i];
	  f.offset.x = r.template get<uint8_t>();
	  f.offset.y = r.template get<uint8_t>();
	  f.size.x = r.template get<uint8_t>();
	  f.size.y = r.template get<uint8_t>();
	  size_t file_offset = r.template get<uint32_t>();
	  auto line_offset_r = base_r;
	  line_offset_r.skip(file_offset);
	  f.line_data_offset.reserve(f.size.y);
	  for (size_t y = 0; y != f.size.y; ++y) {
		auto line_r = base_r;
		line_r.skip(file_offset + line_offset_r.template get<uint16_t>());
		f.line_data_offset.push_back(f.data_container.size());
		for (size_t x = 0; x != f.size.x;) {
		  auto v = line_r.template get<uint8_t>();
		  if (v & 0x80) {
			v &= 0x7f;
			if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
			f.data_container.push_back(0x80 | v);
			x += v;
		  }
		  else if (v & 0x40) {
			v &= 0x3f;
			if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
			f.data_container.push_back(0x40 | v);
			f.data_container.push_back(line_r.template get<uint8_t>());
			x += v;
		  }
		  else {
			if (v > f.size.x - x) v = (uint8_t)(f.size.x - x);
			f.data_container.push_back(v);
			for (size_t i = 0; i != v; ++i) {
			  f.data_container.push_back(line_r.template get<uint8_t>());
			}
			x += v;
		  }
		}
	  }
	  f.data_container.shrink_to_fit();
	}
	return grp;
  }

  struct global_state {

	global_state() = default;
	global_state(global_state&) = delete;
	global_state(global_state&&) = delete;
	global_state& operator=(global_state&) = delete;
	global_state& operator=(global_state&&) = delete;

	flingy_types_t flingy_types;
	sprite_types_t sprite_types;
	image_types_t image_types;
	order_types_t order_types;
	iscript_t iscript;

	a_vector<grp_t> grps;
	a_vector<grp_t*> image_grp;
	a_vector<a_vector<a_vector<xy>>> lo_offsets;
	a_vector<std::array<a_vector<a_vector<xy>>*, 6>> image_lo_offsets;

	a_vector<uint8_t> units_dat;
	a_vector<uint8_t> weapons_dat;
	a_vector<uint8_t> upgrades_dat;
	a_vector<uint8_t> techdata_dat;

	a_vector<uint8_t> melee_trg;

	std::array<a_vector<uint8_t>, 8> tileset_vf4;
	std::array<a_vector<uint8_t>, 8> tileset_cv5;

	std::array<a_vector<cv5_entry>, 8> cv5;
	std::array<a_vector<vf4_entry>, 8> vf4;
	std::array<a_vector<uint16_t>, 8>  mega_tile_flags;

	a_vector<cv5_entry>& get_cv5(int tileset) {
	  if (cv5.at(tileset).empty()) {
		auto& cv5_data = tileset_cv5.at(tileset);
		data_loading::data_reader_le r(cv5_data.data(), cv5_data.data() + cv5_data.size());
		cv5[tileset].reserve(cv5_data.size() / 52);
		while (r.left()) {
		  cv5[tileset].emplace_back();
		  auto& e = cv5[tileset].back();
		  r.get<uint16_t>();
		  e.flags = r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  r.get<uint16_t>();
		  for (size_t i = 0; i != 16; ++i) {
			e.mega_tile_index[i] = r.get<uint16_t>();
		  }
		}
	  }
	  return cv5[tileset];
	}

	a_vector<vf4_entry>& get_vf4(int tileset) {
	  if (vf4.at(tileset).empty()) {
		auto& vf4_data = tileset_vf4.at(tileset);
		data_loading::data_reader_le r(vf4_data.data(), vf4_data.data() + vf4_data.size());
		vf4[tileset].reserve(vf4_data.size() / 32);
		while (r.left()) {
		  vf4[tileset].emplace_back();
		  auto& e = vf4[tileset].back();
		  for (size_t i = 0; i != 16; ++i) {
			e.flags[i] = r.get<uint16_t>();
		  }
		}
	  }
	  return vf4[tileset];
	}

	a_vector<uint16_t>& get_mega_tile_flags(int tileset) {
	  if (mega_tile_flags.at(tileset).empty()) {
		auto& vf4 = get_vf4(tileset);
		mega_tile_flags[tileset].resize(vf4.size());
		for (size_t i = 0; i < mega_tile_flags[tileset].size(); ++i) {
		  int flags = 0;
		  auto& mt = vf4[i];
		  int walkable_count = 0;
		  int middle_count = 0;
		  int high_count = 0;
		  int very_high_count = 0;
		  for (size_t y = 0; y < 4; ++y) {
			for (size_t x = 0; x < 4; ++x) {
			  if (mt.flags[y * 4 + x] & vf4_entry::flag_walkable) ++walkable_count;
			  if (mt.flags[y * 4 + x] & vf4_entry::flag_middle) ++middle_count;
			  if (mt.flags[y * 4 + x] & vf4_entry::flag_high) ++high_count;
			  if (mt.flags[y * 4 + x] & vf4_entry::flag_very_high) ++very_high_count;
			}
		  }
		  if (walkable_count > 12) flags |= tile_t::flag_walkable;
		  else flags |= tile_t::flag_unwalkable;
		  if (walkable_count && walkable_count != 0x10) flags |= tile_t::flag_partially_walkable;
		  if (high_count < 12 && middle_count + high_count >= 12) flags |= tile_t::flag_middle;
		  if (high_count >= 12) flags |= tile_t::flag_high;
		  if (very_high_count) flags |= tile_t::flag_very_high;
		  mega_tile_flags[tileset][i] = flags;
		}
	  }
	  return mega_tile_flags[tileset];
	}

	template<typename load_data_file_F>
	void init(load_data_file_F&& load_data_file) {

	  auto get_sprite_type = [&](SpriteTypes id) {
		if ((size_t)id >= 517) error("invalid sprite id %d", (size_t)id);
		return &sprite_types.vec[(size_t)id];
	  };
	  auto get_image_type = [&](ImageTypes id) {
		if ((size_t)id >= 999) error("invalid image id %d", (size_t)id);
		return &image_types.vec[(size_t)id];
	  };

	  auto load_iscript_bin = [&]() {
		using data_loading::data_reader_le;

		load_data_file(iscript.data, "scripts\\iscript.bin");

		data_reader_le base_r(iscript.data.data(), iscript.data.data() + iscript.data.size());
		data_reader_le r = base_r;

		uint16_t begin_offset = r.get<uint16_t>();
		r.seek(begin_offset);

		uint16_t id, offset;
		while (id = r.get<uint16_t>(), offset = r.get<uint16_t>(), id != 0xFFFF && offset != 0x0000) {
		  auto anim_r = base_r;
		  anim_r.seek(offset + 8);

		  for (int anim_num = 0; anim_num < iscript_anims::MAX; ++anim_num) {
			iscript.script_anim_offsets[id][anim_num] = anim_r.get<uint16_t>();
		  }
		}
	  };

	  auto load_images = [&]() {

		using data_loading::data_reader_le;

		a_vector<uint8_t> data;
		load_data_file(data, "arr\\images.tbl");
		data_reader_le base_r(data.data(), data.data() + data.size());

		auto r = base_r;
		size_t file_count = r.get<uint16_t>();
		(void)file_count;

		auto load_grp = [&](data_reader_le r) {
		  size_t index = grps.size();
		  grps.push_back(read_grp(r));
		  return index;
		};
		auto load_offsets = [&](data_reader_le r) {
		  auto base_r = r;
		  lo_offsets.emplace_back();
		  auto& offs = lo_offsets.back();

		  size_t frame_count = r.get<uint32_t>();
		  size_t offset_count = r.get<uint32_t>();
		  for (size_t f = 0; f < frame_count; ++f) {
			size_t file_offset = r.get<uint32_t>();
			auto r2 = base_r;
			r2.skip(file_offset);
			offs.emplace_back();
			auto& vec = offs.back();
			vec.resize(offset_count);
			for (size_t i = 0; i != offset_count; ++i) {
			  int x = r2.get<int8_t>();
			  int y = r2.get<int8_t>();
			  vec[i] = { x, y };
			}
		  }

		  return lo_offsets.size() - 1;
		};

		a_unordered_map<size_t, size_t> loaded;
		auto load = [&](int index, std::function<size_t(data_reader_le)> f) {
		  if (!index) return (size_t)0;
		  auto in = loaded.emplace(index, 0);
		  if (!in.second) return in.first->second;
		  auto r = base_r;
		  r.skip(2 + (index - 1) * 2);
		  size_t fn_offset = r.get<uint16_t>();
		  r = base_r;
		  r.skip(fn_offset);
		  a_string fn;
		  while (char c = r.get<char>()) fn += c;

		  a_vector<uint8_t> data;
		  load_data_file(data, format("unit\\%s", fn));
		  data_reader_le data_r(data.data(), data.data() + data.size());
		  size_t loaded_index = f(data_r);
		  in.first->second = loaded_index;
		  return loaded_index;
		};

		a_vector<size_t> image_grp_index;
		std::array<a_vector<size_t>, 6> lo_indices;

		grps.emplace_back(); // null/invalid entry
		lo_offsets.emplace_back();

		for (size_t i = 0; i != 999; ++i) {
		  const image_type_t* image_type = get_image_type((ImageTypes)i);
		  image_grp_index.push_back(load(image_type->grp_filename_index, load_grp));
		  lo_indices[0].push_back(load(image_type->attack_filename_index, load_offsets));
		  lo_indices[1].push_back(load(image_type->damage_filename_index, load_offsets));
		  lo_indices[2].push_back(load(image_type->special_filename_index, load_offsets));
		  lo_indices[3].push_back(load(image_type->landing_dust_filename_index, load_offsets));
		  lo_indices[4].push_back(load(image_type->lift_off_filename_index, load_offsets));
		  lo_indices[5].push_back(load(image_type->shield_filename_index, load_offsets));
		}

		image_grp.resize(image_grp_index.size());
		for (size_t i = 0; i != image_grp_index.size(); ++i) {
		  image_grp[i] = &grps.at(image_grp_index[i]);
		}
		
		image_lo_offsets.resize(999);
		for (size_t i = 0; i != 6; ++i) {
		  for (size_t i2 = 0; i2 != 999; ++i2) {
			image_lo_offsets.at(i2).at(i) = &lo_offsets.at(lo_indices[i].at(i2));
		  }
		}

	  };

	  load_data_file(units_dat, "arr\\units.dat");
	  load_data_file(weapons_dat, "arr\\weapons.dat");
	  load_data_file(upgrades_dat, "arr\\upgrades.dat");
	  load_data_file(techdata_dat, "arr\\techdata.dat");

	  load_data_file(melee_trg, "triggers\\Melee.trg");

	  a_vector<uint8_t> buf;
	  load_data_file(buf, "arr\\flingy.dat");
	  flingy_types = data_loading::load_flingy_dat(buf);
	  load_data_file(buf, "arr\\sprites.dat");
	  sprite_types = data_loading::load_sprites_dat(buf);
	  load_data_file(buf, "arr\\images.dat");
	  image_types = data_loading::load_images_dat(buf);
	  load_data_file(buf, "arr\\orders.dat");
	  order_types = data_loading::load_orders_dat(buf);

	  auto fixup_sprite_type = [&](auto& ptr) {
		SpriteTypes index{ ptr };
		if (index == SpriteTypes::None) ptr = nullptr;
		else ptr = get_sprite_type(index);
	  };
	  auto fixup_image_type = [&](auto& ptr) {
		ImageTypes index{ ptr };
		if (index == ImageTypes::None) ptr = nullptr;
		else ptr = get_image_type(index);
	  };

	  for (auto& v : flingy_types.vec) {
		fixup_sprite_type(v.sprite);
	  }
	  for (auto& v : sprite_types.vec) {
		fixup_image_type(v.image);
	  }

	  load_iscript_bin();
	  load_images();

	  std::array<const char*, 8> tileset_names = {
		  "badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
	  };

	  for (size_t i = 0; i != 8; ++i) {
		load_data_file(tileset_vf4[i], format("Tileset\\%s.vf4", tileset_names.at(i)));
		load_data_file(tileset_cv5[i], format("Tileset\\%s.cv5", tileset_names.at(i)));
	  }

	}
  };

  extern global_state global_st;
}

#endif
