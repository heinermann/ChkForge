#ifndef OPENBW_GLOBAL_UI_H
#define OPENBW_GLOBAL_UI_H

#include "bwglobal.h"
#include "openbw/ui/native_sound.h"

#include <chrono>

namespace bwgame {


  struct vr4_entry {
	using bitmap_t = std::conditional<is_native_fast_int<uint64_t>::value, uint64_t, uint32_t>::type;
	std::array<bitmap_t, 64 / sizeof(bitmap_t)> bitmap;
	std::array<bitmap_t, 64 / sizeof(bitmap_t)> inverted_bitmap;
  };
  struct vx4_entry {
	std::array<uint16_t, 16> images;
  };

  struct pcx_image {
	size_t width;
	size_t height;
	a_vector<uint8_t> data;
  };

  struct tileset_image_data {
	a_vector<uint8_t> wpe;
	a_vector<vr4_entry> vr4;
	a_vector<vx4_entry> vx4;
	pcx_image dark_pcx;
	std::array<pcx_image, 7> light_pcx;
	grp_t creep_grp;
	int resource_minimap_color;
	std::array<uint8_t, 256> cloak_fade_selector;
  };

  struct image_data {
	std::array<std::array<uint8_t, 8>, 16> player_unit_colors;
	std::array<uint8_t, 16> player_minimap_colors;
	std::array<uint8_t, 24> selection_colors;
	std::array<uint8_t, 24> hp_bar_colors;
	std::array<int, 0x100> creep_edge_frame_index{};
  };

  template<bool bounds_check>
  void draw_tile(tileset_image_data& img, size_t megatile_index, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
	auto* images = &img.vx4.at(megatile_index).images[0];
	size_t x = 0;
	size_t y = 0;
	for (size_t image_iy = 0; image_iy != 4; ++image_iy) {
	  for (size_t image_ix = 0; image_ix != 4; ++image_ix) {
		auto image_index = *images;
		bool inverted = (image_index & 1) == 1;
		auto* bitmap = inverted ? &img.vr4.at(image_index / 2).inverted_bitmap[0] : &img.vr4.at(image_index / 2).bitmap[0];

		for (size_t iy = 0; iy != 8; ++iy) {
		  for (size_t iv = 0; iv != 8 / sizeof(vr4_entry::bitmap_t); ++iv) {
			for (size_t b = 0; b != sizeof(vr4_entry::bitmap_t); ++b) {
			  if (!bounds_check || (x >= offset_x && y >= offset_y && x < width && y < height)) {
				*dst = (uint8_t)(*bitmap >> (8 * b));
			  }
			  ++dst;
			  ++x;
			}
			++bitmap;
		  }
		  x -= 8;
		  ++y;
		  dst -= 8;
		  dst += pitch;
		}
		x += 8;
		y -= 8;
		dst -= pitch * 8;
		dst += 8;
		++images;
	  }
	  x -= 32;
	  y += 8;
	  dst += pitch * 8;
	  dst -= 32;
	}
  }

  static inline void draw_tile(tileset_image_data& img, size_t megatile_index, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
	if (offset_x == 0 && offset_y == 0 && width == 32 && height == 32) {
	  draw_tile<false>(img, megatile_index, dst, pitch, offset_x, offset_y, width, height);
	}
	else {
	  draw_tile<true>(img, megatile_index, dst, pitch, offset_x, offset_y, width, height);
	}
  }

  template<bool bounds_check, bool flipped, bool textured, typename remap_F>
  void draw_frame(const grp_t::frame_t& frame, const uint8_t* texture, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height, remap_F&& remap_f) {
	for (size_t y = 0; y != offset_y; ++y) {
	  dst += pitch;
	  if (textured) texture += frame.size.x;
	}

	for (size_t y = offset_y; y != height; ++y) {

	  if (flipped) dst += frame.size.x - 1;
	  if (textured && flipped) texture += frame.size.x - 1;

	  const uint8_t* d = frame.data_container.data() + frame.line_data_offset.at(y);
	  for (size_t x = flipped ? frame.size.x - 1 : 0; x != (flipped ? (size_t)0 - 1 : frame.size.x);) {
		int v = *d++;
		if (v & 0x80) {
		  v &= 0x7f;
		  x += flipped ? -v : v;
		  dst += flipped ? -v : v;
		  if (textured) texture += flipped ? -v : v;
		}
		else if (v & 0x40) {
		  v &= 0x3f;
		  int c = *d++;
		  for (; v; --v) {
			if (!bounds_check || (x >= offset_x && x < width)) {
			  *dst = remap_f(textured ? *texture : c, *dst);
			}
			dst += flipped ? -1 : 1;
			x += flipped ? -1 : 1;
			if (textured) texture += flipped ? -1 : 1;
		  }
		}
		else {
		  for (; v; --v) {
			int c = *d++;
			if (!bounds_check || (x >= offset_x && x < width)) {
			  *dst = remap_f(textured ? *texture : c, *dst);
			}
			dst += flipped ? -1 : 1;
			x += flipped ? -1 : 1;
			if (textured) texture += flipped ? -1 : 1;
		  }
		}
	  }

	  if (!flipped) dst -= frame.size.x;
	  else ++dst;
	  dst += pitch;
	  if (textured) {
		if (!flipped) texture -= frame.size.x;
		else ++texture;
		texture += frame.size.x;
	  }

	}
  }

  struct no_remap {
	uint8_t operator()(uint8_t new_value, uint8_t old_value) const {
	  return new_value;
	}
  };

  template<typename remap_F = no_remap>
  void draw_frame(const grp_t::frame_t& frame, bool flipped, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height, remap_F&& remap_f = remap_F()) {
	if (offset_x == 0 && offset_y == 0 && width == frame.size.x && height == frame.size.y) {
	  if (flipped) draw_frame<false, true, false>(frame, nullptr, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	  else draw_frame<false, false, false>(frame, nullptr, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	}
	else {
	  if (flipped) draw_frame<true, true, false>(frame, nullptr, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	  else draw_frame<true, false, false>(frame, nullptr, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	}
  }

  template<typename remap_F = no_remap>
  void draw_frame_textured(const grp_t::frame_t& frame, const uint8_t* texture, bool flipped, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height, remap_F&& remap_f = remap_F()) {
	if (offset_x == 0 && offset_y == 0 && width == frame.size.x && height == frame.size.y) {
	  if (flipped) draw_frame<false, true, true>(frame, texture, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	  else draw_frame<false, false, true>(frame, texture, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	}
	else {
	  if (flipped) draw_frame<true, true, true>(frame, texture, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	  else draw_frame<true, false, true>(frame, texture, dst, pitch, offset_x, offset_y, width, height, std::forward<remap_F>(remap_f));
	}
  }

  template<typename data_T>
  pcx_image load_pcx_data(const data_T& data) {
	data_loading::data_reader_le r(data.data(), data.data() + data.size());
	auto base_r = r;
	auto id = r.get<uint8_t>();
	if (id != 0x0a) error("pcx: invalid identifier %#x", id);
	r.get<uint8_t>(); // version
	auto encoding = r.get<uint8_t>(); // encoding
	auto bpp = r.get<uint8_t>(); // bpp
	auto offset_x = r.get<uint16_t>();
	auto offset_y = r.get<uint16_t>();
	auto last_x = r.get<uint16_t>();
	auto last_y = r.get<uint16_t>();

	if (encoding != 1) error("pcx: invalid encoding %#x", encoding);
	if (bpp != 8) error("pcx: bpp is %d, expected 8", bpp);

	if (offset_x != 0 || offset_y != 0) error("pcx: offset %d %d, expected 0 0", offset_x, offset_y);

	r.skip(2 + 2 + 48 + 1);

	auto bit_planes = r.get<uint8_t>();
	auto bytes_per_line = r.get<uint16_t>();

	size_t width = last_x + 1;
	size_t height = last_y + 1;

	pcx_image pcx;
	pcx.width = width;
	pcx.height = height;

	pcx.data.resize(width * height);

	r = base_r;
	r.skip(128);

	auto padding = bytes_per_line * bit_planes - width;
	if (padding != 0) error("pcx: padding not supported");

	uint8_t* dst = pcx.data.data();
	uint8_t* dst_end = pcx.data.data() + pcx.data.size();

	while (dst != dst_end) {
	  auto v = r.get<uint8_t>();
	  if ((v & 0xc0) == 0xc0) {
		v &= 0x3f;
		auto c = r.get<uint8_t>();
		for (; v; --v) {
		  if (dst == dst_end) error("pcx: failed to decode");
		  *dst++ = c;
		}
	  }
	  else {
		*dst = v;
		++dst;
	  }
	}

	return pcx;
  }

  template<typename load_data_file_F>
  void load_image_data(image_data& img, load_data_file_F&& load_data_file) {

	std::array<int, 0x100> creep_edge_neighbors_index{};
	std::array<int, 128> creep_edge_neighbors_index_n{};

	for (size_t i = 0; i != 0x100; ++i) {
	  int v = 0;
	  if (i & 2) v |= 0x10;
	  if (i & 8) v |= 0x24;
	  if (i & 0x10) v |= 9;
	  if (i & 0x40) v |= 2;
	  if ((i & 0xc0) == 0xc0) v |= 1;
	  if ((i & 0x60) == 0x60) v |= 4;
	  if ((i & 3) == 3) v |= 0x20;
	  if ((i & 6) == 6) v |= 8;
	  if ((v & 0x21) == 0x21) v |= 0x40;
	  if ((v & 0xc) == 0xc) v |= 0x40;
	  creep_edge_neighbors_index[i] = v;
	}

	int n = 0;
	for (int i = 0; i != 128; ++i) {
	  auto it = std::find(creep_edge_neighbors_index.begin(), creep_edge_neighbors_index.end(), i);
	  if (it == creep_edge_neighbors_index.end()) continue;
	  creep_edge_neighbors_index_n[i] = n;
	  ++n;
	}

	for (size_t i = 0; i != 0x100; ++i) {
	  img.creep_edge_frame_index[i] = creep_edge_neighbors_index_n[creep_edge_neighbors_index[i]];
	}

	a_vector<uint8_t> tmp_data;
	auto load_pcx_file = [&](a_string filename) {
	  load_data_file(tmp_data, std::move(filename));
	  return load_pcx_data(tmp_data);
	};

	auto tunit_pcx = load_pcx_file("game\\tunit.pcx");
	if (tunit_pcx.width != 128 || tunit_pcx.height != 1) error("tunit.pcx dimensions are %dx%d (128x1 required)", tunit_pcx.width, tunit_pcx.height);
	for (size_t i = 0; i != 16; ++i) {
	  for (size_t i2 = 0; i2 != 8; ++i2) {
		img.player_unit_colors[i][i2] = tunit_pcx.data[i * 8 + i2];
	  }
	}
	auto tminimap_pcx = load_pcx_file("game\\tminimap.pcx");
	if (tminimap_pcx.width != 16 || tminimap_pcx.height != 1) error("tminimap.pcx dimensions are %dx%d (16x1 required)", tminimap_pcx.width, tminimap_pcx.height);
	for (size_t i = 0; i != 16; ++i) {
	  img.player_minimap_colors[i] = tminimap_pcx.data[i];
	}
	auto tselect_pcx = load_pcx_file("game\\tselect.pcx");
	if (tselect_pcx.width != 24 || tselect_pcx.height != 1) error("tselect.pcx dimensions are %dx%d (24x1 required)", tselect_pcx.width, tselect_pcx.height);
	for (size_t i = 0; i != 24; ++i) {
	  img.selection_colors[i] = tselect_pcx.data[i];
	}
	auto thpbar_pcx = load_pcx_file("game\\thpbar.pcx");
	if (thpbar_pcx.width != 19 || thpbar_pcx.height != 1) error("thpbar.pcx dimensions are %dx%d (19x1 required)", thpbar_pcx.width, thpbar_pcx.height);
	for (size_t i = 0; i != 19; ++i) {
	  img.hp_bar_colors[i] = thpbar_pcx.data[i];
	}

  }

  template<typename load_data_file_F>
  void load_tileset_image_data(tileset_image_data& img, size_t tileset_index, load_data_file_F&& load_data_file) {
	using namespace data_loading;

	std::array<const char*, 8> tileset_names = {
		"badlands", "platform", "install", "AshWorld", "Jungle", "Desert", "Ice", "Twilight"
	};

	a_vector<uint8_t> vr4_data;
	a_vector<uint8_t> vx4_data;

	const char* tileset_name = tileset_names.at(tileset_index);

	load_data_file(vr4_data, format("Tileset\\%s.vr4", tileset_name));
	load_data_file(vx4_data, format("Tileset\\%s.vx4", tileset_name));
	load_data_file(img.wpe, format("Tileset\\%s.wpe", tileset_name));

	a_vector<uint8_t> grp_data;
	load_data_file(grp_data, format("Tileset\\%s.grp", tileset_name));
	img.creep_grp = read_grp(data_loading::data_reader_le(grp_data.data(), grp_data.data() + grp_data.size()));

	data_reader<true, false> vr4_r(vr4_data.data(), nullptr);
	img.vr4.resize(vr4_data.size() / 64);
	for (size_t i = 0; i != img.vr4.size(); ++i) {
	  for (size_t i2 = 0; i2 != 8; ++i2) {
		auto r2 = vr4_r;
		auto v = vr4_r.get<uint64_t, true>();
		auto iv = r2.get<uint64_t, false>();
		size_t n = 8 / sizeof(vr4_entry::bitmap_t);
		for (size_t i3 = 0; i3 != n; ++i3) {
		  img.vr4[i].bitmap[i2 * n + i3] = (vr4_entry::bitmap_t)v;
		  img.vr4[i].inverted_bitmap[i2 * n + i3] = (vr4_entry::bitmap_t)iv;
		  v >>= n == 1 ? 0 : 8 * sizeof(vr4_entry::bitmap_t);
		  iv >>= n == 1 ? 0 : 8 * sizeof(vr4_entry::bitmap_t);
		}
	  }
	}
	data_reader<true, false> vx4_r(vx4_data.data(), vx4_data.data() + vx4_data.size());
	img.vx4.resize(vx4_data.size() / 32);
	for (size_t i = 0; i != img.vx4.size(); ++i) {
	  for (size_t i2 = 0; i2 != 16; ++i2) {
		img.vx4[i].images[i2] = vx4_r.get<uint16_t>();
	  }
	}

	a_vector<uint8_t> tmp_data;
	auto load_pcx_file = [&](a_string filename) {
	  load_data_file(tmp_data, std::move(filename));
	  return load_pcx_data(tmp_data);
	};

	img.dark_pcx = load_pcx_file(format("Tileset\\%s\\dark.pcx", tileset_name));
	if (img.dark_pcx.width != 256 || img.dark_pcx.height != 32) error("invalid dark.pcx");
	for (size_t x = 0; x != 256; ++x) {
	  img.dark_pcx.data[256 * 31 + x] = (uint8_t)x;
	}

	std::array<const char*, 7> light_names = { "ofire", "gfire", "bfire", "bexpl", "trans50", "red", "green" };
	for (size_t i = 0; i != 7; ++i) {
	  img.light_pcx[i] = load_pcx_file(format("Tileset\\%s\\%s.pcx", tileset_name, light_names[i]));
	}

	if (img.wpe.size() != 256 * 4) error("wpe size invalid (%d)", img.wpe.size());

	auto get_nearest_color = [&](int r, int g, int b) {
	  size_t best_index = -1;
	  int best_score{};
	  for (size_t i = 0; i != 256; ++i) {
		int dr = r - img.wpe[4 * i + 0];
		int dg = g - img.wpe[4 * i + 1];
		int db = b - img.wpe[4 * i + 2];
		int score = dr * dr + dg * dg + db * db;
		if (best_index == (size_t)-1 || score < best_score) {
		  best_index = i;
		  best_score = score;
		}
	  }
	  return best_index;
	};
	img.resource_minimap_color = get_nearest_color(0, 255, 255);

	for (size_t i = 0; i != 256; ++i) {
	  int r = img.wpe[4 * i + 0];
	  int g = img.wpe[4 * i + 1];
	  int b = img.wpe[4 * i + 2];
	  int strength = (r * 28 + g * 77 + b * 151 + 4096) / 8192;
	  img.cloak_fade_selector[i] = strength;
	}

  }

  struct global_ui_state {
	grp_t cmdicons;
	image_data img;
	std::array<tileset_image_data, 8> all_tileset_img;

	a_vector<uint8_t> creep_random_tile_indices = a_vector<uint8_t>(256 * 256);

	string_table_data images_tbl;

	sound_types_t sound_types;
	a_vector<a_string> sound_filenames;
	a_vector<bool> has_loaded_sound;
	a_vector<std::unique_ptr<native_sound::sound>> loaded_sounds;
	a_vector<std::chrono::high_resolution_clock::time_point> last_played_sound;

	int global_volume = 50;


	std::function<void(a_vector<uint8_t>&, a_string)> load_data_file;

	void draw_icon(int icon_id, uint8_t* dst, size_t pitch, size_t offset_x, size_t offset_y, size_t width, size_t height) {
	  if (icon_id >= cmdicons.frames.size()) return;

	  draw_frame(cmdicons.frames[icon_id], false, dst, pitch, offset_x, offset_y, width, height);
	}

	template<typename load_data_file_F>
	void init(load_data_file_F&& load_data_file) {
	  uint32_t rand_state = (uint32_t)std::chrono::high_resolution_clock::now().time_since_epoch().count();
	  auto rand = [&]() {
		rand_state = rand_state * 22695477 + 1;
		return (rand_state >> 16) & 0x7fff;
	  };
	  for (auto& v : creep_random_tile_indices) {
		if (rand() % 100 < 4) v = 6 + rand() % 7;
		else v = rand() % 6;
	  }

	  a_vector<uint8_t> data;
	  load_data_file(data, "arr\\sfxdata.dat");
	  sound_types = data_loading::load_sfxdata_dat(data);

	  sound_filenames.resize(sound_types.vec.size());
	  has_loaded_sound.resize(sound_filenames.size());
	  loaded_sounds.resize(has_loaded_sound.size());
	  last_played_sound.resize(loaded_sounds.size());

	  string_table_data tbl;
	  load_data_file(tbl.data, "arr\\sfxdata.tbl");
	  for (size_t i = 0; i != sound_types.vec.size(); ++i) {
		size_t index = sound_types.vec[i].filename_index;
		sound_filenames[i] = tbl[index];
	  }
	  native_sound::frequency = 44100;
	  native_sound::channels = 8;
	  native_sound::init();

	  sound_channels.resize(8);

	  load_data_file(images_tbl.data, "arr\\images.tbl");

	  a_vector<uint8_t> grp_data;
	  load_data_file(grp_data, "unit\\cmdbtns\\cmdicons.grp");
	  cmdicons = read_grp(data_loading::data_reader_le(grp_data.data(), grp_data.data() + grp_data.size()));

	  load_image_data(img, load_data_file);
	  for (size_t i = 0; i != 8; ++i) {
		load_tileset_image_data(all_tileset_img[i], i, load_data_file);
	  }
	}


	const sound_type_t* get_sound_type(Sounds id) const {
	  if ((size_t)id >= (size_t)Sounds::None) error("invalid sound id %d", (size_t)id);
	  return &sound_types.vec[(size_t)id];
	}

	struct sound_channel {
	  bool playing = false;
	  const sound_type_t* sound_type = nullptr;
	  int priority = 0;
	  int flags = 0;
	  const unit_type_t* unit_type = nullptr;
	  int volume = 0;
	};
	a_vector<sound_channel> sound_channels;

	void set_volume(int volume) {
	  if (volume < 0) volume = 0;
	  else if (volume > 100) volume = 100;
	  global_volume = volume;
	  for (auto& c : sound_channels) {
		if (c.playing) {
		  native_sound::set_volume(&c - sound_channels.data(), (128 - 4) * (c.volume * global_volume / 100) / 100);
		}
	  }
	}

	sound_channel* get_sound_channel(int priority) {
	  sound_channel* r = nullptr;
	  for (auto& c : sound_channels) {
		if (c.playing) {
		  if (!native_sound::is_playing(&c - sound_channels.data())) {
			c.playing = false;
			r = &c;
		  }
		}
		else r = &c;
	  }
	  if (r) return r;
	  int best_prio = priority;
	  for (auto& c : sound_channels) {
		if (c.flags & 0x20) continue;
		if (c.priority < best_prio) {
		  best_prio = c.priority;
		  r = &c;
		}
	  }
	  return r;
	}

  };

}

#endif
