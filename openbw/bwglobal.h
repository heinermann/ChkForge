#pragma once
#ifndef OPENBW_GLOBALS_H
#define OPENBW_GLOBALS_H

#include <functional>

#include "openbw/data_types.h"
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

		using namespace iscript_opcodes;
		std::array<const char*, 69> ins_data;

		ins_data[opc_playfram] = "2";
		ins_data[opc_playframtile] = "2";
		ins_data[opc_sethorpos] = "s1";
		ins_data[opc_setvertpos] = "s1";
		ins_data[opc_setpos] = "s1s1";
		ins_data[opc_wait] = "1";
		ins_data[opc_waitrand] = "11";
		ins_data[opc_goto] = "j";
		ins_data[opc_imgol] = "211";
		ins_data[opc_imgul] = "211";
		ins_data[opc_imgolorig] = "2";
		ins_data[opc_switchul] = "2";
		ins_data[opc___0c] = "";
		ins_data[opc_imgoluselo] = "211";
		ins_data[opc_imguluselo] = "211";
		ins_data[opc_sprol] = "2s1s1";
		ins_data[opc_highsprol] = "211";
		ins_data[opc_lowsprul] = "211";
		ins_data[opc_uflunstable] = "2";
		ins_data[opc_spruluselo] = "211";
		ins_data[opc_sprul] = "211";
		ins_data[opc_sproluselo] = "21";
		ins_data[opc_end] = "e";
		ins_data[opc_setflipstate] = "1";
		ins_data[opc_playsnd] = "2";
		ins_data[opc_playsndrand] = "v";
		ins_data[opc_playsndbtwn] = "22";
		ins_data[opc_domissiledmg] = "";
		ins_data[opc_attackmelee] = "v";
		ins_data[opc_followmaingraphic] = "";
		ins_data[opc_randcondjmp] = "1b";
		ins_data[opc_turnccwise] = "1";
		ins_data[opc_turncwise] = "1";
		ins_data[opc_turn1cwise] = "";
		ins_data[opc_turnrand] = "1";
		ins_data[opc_setspawnframe] = "1";
		ins_data[opc_sigorder] = "1";
		ins_data[opc_attackwith] = "1";
		ins_data[opc_attack] = "";
		ins_data[opc_castspell] = "";
		ins_data[opc_useweapon] = "1";
		ins_data[opc_move] = "1";
		ins_data[opc_gotorepeatattk] = "";
		ins_data[opc_engframe] = "1";
		ins_data[opc_engset] = "1";
		ins_data[opc___2d] = "";
		ins_data[opc_nobrkcodestart] = "";
		ins_data[opc_nobrkcodeend] = "";
		ins_data[opc_ignorerest] = "";
		ins_data[opc_attkshiftproj] = "1";
		ins_data[opc_tmprmgraphicstart] = "";
		ins_data[opc_tmprmgraphicend] = "";
		ins_data[opc_setfldirect] = "1";
		ins_data[opc_call] = "b";
		ins_data[opc_return] = "";
		ins_data[opc_setflspeed] = "2";
		ins_data[opc_creategasoverlays] = "1";
		ins_data[opc_pwrupcondjmp] = "b";
		ins_data[opc_trgtrangecondjmp] = "2b";
		ins_data[opc_trgtarccondjmp] = "22b";
		ins_data[opc_curdirectcondjmp] = "22b";
		ins_data[opc_imgulnextid] = "11";
		ins_data[opc___3e] = "";
		ins_data[opc_liftoffcondjmp] = "b";
		ins_data[opc_warpoverlay] = "2";
		ins_data[opc_orderdone] = "1";
		ins_data[opc_grdsprol] = "211";
		ins_data[opc___43] = "";
		ins_data[opc_dogrddamage] = "";

		a_unordered_map<int, a_vector<size_t>> animation_pc;
		a_vector<int> program_data;

		program_data.push_back(0); // invalid/null pc

		using data_loading::data_reader_le;

		a_vector<uint8_t> data;
		load_data_file(data, "scripts\\iscript.bin");
		data_reader_le base_r(data.data(), data.data() + data.size());
		auto r = base_r;
		size_t id_list_offset = r.get<uint32_t>();
		r.seek(id_list_offset);
		while (r.left()) {
		  int id = r.get<int16_t>();
		  if (id == -1) break;
		  size_t script_address = r.get<uint16_t>();
		  auto script_r = base_r;
		  script_r.skip(script_address);
		  auto signature = script_r.get<std::array<char, 4>>();
		  (void)signature;

		  a_unordered_map<size_t, size_t> decode_map;

		  auto decode_at = [&](size_t initial_address) {
			a_circular_vector<std::tuple<size_t, size_t>> branches;
			std::function<size_t(size_t)> decode = [&](size_t initial_address) {
			  if (!initial_address) error("iscript load: attempt to decode instruction at null address");
			  auto in = decode_map.emplace(initial_address, 0);
			  if (!in.second) {
				return in.first->second;
			  }
			  size_t initial_pc = program_data.size();
			  in.first->second = initial_pc;
			  auto r = base_r;
			  r.skip(initial_address);
			  bool done = false;
			  while (!done) {
				size_t pc = program_data.size();
				size_t cur_address = r.ptr - base_r.ptr;
				if ((size_t)(int)pc != pc || (size_t)(int)cur_address != cur_address) error("iscript too big");
				if (cur_address != initial_address) {
				  auto in = decode_map.emplace(cur_address, pc);
				  if (!in.second) {
					program_data.push_back(opc_goto + 0x808091);
					program_data.push_back((int)in.first->second);
					break;
				  }
				}
				int opcode = r.get<uint8_t>();
				if ((size_t)opcode >= ins_data.size()) error("iscript load: at 0x%04x: invalid instruction %d", cur_address, opcode);
				program_data.push_back(opcode + 0x808091);
				const char* c = ins_data[opcode];
				while (*c) {
				  if (*c == 's') {
					++c;
					if (*c == '1') program_data.push_back(r.get<int8_t>());
					else if (*c == '2') program_data.push_back(r.get<int16_t>());
				  }
				  else if (*c == '1') program_data.push_back(r.get<uint8_t>());
				  else if (*c == '2') program_data.push_back(r.get<uint16_t>());
				  else if (*c == 'v') {
					int n = r.get<uint8_t>();
					program_data.push_back(n);
					for (; n; --n) program_data.push_back(r.get<uint16_t>());
				  }
				  else if (*c == 'j') {
					size_t jump_address = r.get<uint16_t>();
					auto jump_pc_it = decode_map.find(jump_address);
					if (jump_pc_it == decode_map.end()) {
					  program_data.pop_back();
					  r = base_r;
					  r.skip(jump_address);
					}
					else {
					  program_data.push_back((int)jump_pc_it->second);
					  done = true;
					}
				  }
				  else if (*c == 'b') {
					size_t branch_address = r.get<uint16_t>();
					branches.emplace_back(branch_address, program_data.size());
					program_data.push_back(0);
				  }
				  else if (*c == 'e') {
					done = true;
				  }
				  ++c;
				}
			  }
			  return initial_pc;
			};
			size_t initial_pc = decode(initial_address);
			while (!branches.empty()) {
			  auto v = branches.front();
			  branches.pop_front();
			  size_t pc = decode(std::get<0>(v));
			  if ((size_t)(int)pc != pc) error("iscript load: 0x%x does not fit in an int", pc);
			  program_data[std::get<1>(v)] = (int)pc;
			}
			return initial_pc;
		  };

		  auto& anim_funcs = animation_pc[id];

		  size_t highest_animation = script_r.get<uint32_t>();
		  size_t animations = (highest_animation + 1 + 1) & -2;
		  for (size_t i = 0; i != animations; ++i) {
			size_t anim_address = script_r.get<uint16_t>();
			if (!anim_address) {
			  anim_funcs.push_back(0);
			  continue;
			}
			auto anim_r = base_r;
			anim_r.skip(anim_address);
			anim_funcs.push_back(decode_at(anim_address));
		  }
		}

		iscript.program_data = std::move(program_data);
		iscript.scripts.clear();
		for (auto& v : animation_pc) {
		  auto& s = iscript.scripts[v.first];
		  s.id = v.first;
		  s.animation_pc = std::move(v.second);
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
