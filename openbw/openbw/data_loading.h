#ifndef BWGAME_DATA_LOADING_H
#define BWGAME_DATA_LOADING_H

#include "util.h"
#include "containers.h"
#include "data_types.h"

#include <type_traits>
#include <array>
#include <cstring>
#include <cstdio>

#include <MappingCoreLib/MpqFile.h>
#include <CascLib.h>

namespace bwgame {
namespace data_loading {

static_assert(std::is_same<uint8_t, unsigned char>::value || std::is_same<uint8_t, char>::value, "uint8_t must be (unsigned) char (we use it for aliasing)");

template<typename T>
struct is_std_array : std::false_type {};
template<typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type{};

template<typename T, bool little_endian, typename std::enable_if<!is_std_array<T>::value>::type* = nullptr>
static inline T value_at(const uint8_t* ptr) {
	static_assert(std::is_integral<T>::value, "can only read integers and arrays of integers");
	union endian_t {uint32_t a; uint8_t b;};
	const bool native_little_endian = (endian_t{1}).b == 1;
	if (little_endian == native_little_endian) {
		if (((uintptr_t)(void*)ptr & (alignof(T) - 1)) == 0) {
			return *(T*)(void*)ptr;
		} else {
			typename std::aligned_storage<sizeof(T), alignof(T)>::type buf;
			memcpy(&buf, ptr, sizeof(T));
			return *(T*)(void*)&buf;
		}
	} else {
		T r = 0;
		for (size_t i = 0; i < sizeof(T); ++i) {
			r |= (T)ptr[i] << ((little_endian ? i : sizeof(T) - 1 - i) * 8);
		}
		return r;
	}
}
template<typename T, bool little_endian, typename std::enable_if<is_std_array<T>::value>::type* = nullptr>
static inline T value_at(const uint8_t* ptr) {
	T r;
	for (auto& v : r) {
		v = value_at<typename std::remove_reference<decltype(v)>::type, little_endian>(ptr);
		ptr += sizeof(v);
	}
	return r;
}

template<bool little_endian, typename T>
static inline void set_value_at(uint8_t* ptr, T value) {
	static_assert(std::is_integral<T>::value, "can only write integers");
	union endian_t {uint32_t a; uint8_t b;};
	const bool native_little_endian = (endian_t{1}).b == 1;
	if (little_endian == native_little_endian) {
		if (((uintptr_t)(void*)ptr & (alignof(T) - 1)) == 0) {
			*(T*)(void*)ptr = value;
		} else {
			memcpy(ptr, &value, sizeof(T));
		}
	} else {
		for (size_t i = 0; i < sizeof(T); ++i) {
			ptr[i] = value >> ((little_endian ? i : sizeof(T) - 1 - i) * 8);
		}
	}
}

template<typename T, bool little_endian, typename self_T>
T get_impl(self_T& self) {
	typename std::aligned_storage<sizeof(T), alignof(T)>::type buf;
	self.get_bytes((uint8_t*)&buf, sizeof(T));
	return value_at<T, little_endian>((uint8_t*)&buf);
}

template<bool default_little_endian = true, bool bounds_checking = true>
struct data_reader {
	const uint8_t* ptr = nullptr;
	const uint8_t* begin = nullptr;
	const uint8_t* end = nullptr;
	data_reader() = default;
	data_reader(const uint8_t* ptr, const uint8_t* end) : ptr(ptr), begin(ptr), end(end) {}
	template<typename T, bool little_endian = default_little_endian>
	T get() {
		if (bounds_checking && left() < sizeof(T)) error("data_reader: attempt to read past end");
		ptr += sizeof(T);
		return value_at<T, little_endian>(ptr - sizeof(T));
	}
	const uint8_t* get_n(size_t n) {
		const uint8_t* r = ptr;
		if (bounds_checking && left() < n) error("data_reader: attempt to read past end");
		ptr += n;
		return r;
	}
	template<typename T, bool little_endian = default_little_endian>
	a_vector<T> get_vec(size_t n) {
		const uint8_t* data = get_n(n*sizeof(T));
		a_vector<T> r(n);
		for (size_t i = 0; i < n; ++i, data += sizeof(T)) {
			r[i] = value_at<T, little_endian>(data);
		}
		return r;
	}
	void skip(size_t n) {
		if (bounds_checking && left() < n) error("data_reader: attempt to seek past end");
		ptr += n;
	}
	void get_bytes(uint8_t* dst, size_t n) {
		memcpy(dst, get_n(n), n);
	}
	void seek(size_t offset) {
		if (offset > size()) error("data_reader: attempt to seek past end");
		ptr = begin + offset;
	}
	size_t left() const {
		return end - ptr;
	}
	size_t size() const {
		return (size_t)(end - begin);
	}
	size_t tell() const {
		return (size_t)(ptr - begin);
	}
};

using data_reader_le = data_reader<true>;
using data_reader_be = data_reader<false>;

template<bool default_little_endian = true>
struct file_reader {
	a_string filename;
	FILE* f = nullptr;
	file_reader() = default;
	explicit file_reader(a_string filename) {
		open(std::move(filename));
	}
	~file_reader() {
		if (f) fclose(f);
	}
	file_reader(const file_reader&) = delete;
	file_reader(file_reader&& n) {
		f = n.f;
		n.f = nullptr;
	}
	file_reader& operator=(const file_reader&) = delete;
	file_reader& operator=(file_reader&& n) {
		std::swap(f, n.f);
		return *this;
	}

	void open(a_string filename) {
		if (f) fclose(f);
		f = fopen(filename.c_str(), "rb");
		if (!f) error("file_reader: failed to open %s for reading", filename.c_str());
		this->filename = std::move(filename);
	}

	void get_bytes(uint8_t* dst, size_t n) {
		if (!fread(dst, n, 1, f)) {
			if (feof(f)) error("file_reader: %s: attempt to read past end", filename);
			error("file_reader: %s: read error", filename);
		}
	}

	template<typename T, bool little_endian = default_little_endian>
	T get() {
		uint8_t buffer[sizeof(T)];
		get_bytes(buffer, sizeof(T));
		return value_at<T, little_endian>(buffer);
	}

	template<typename T, bool little_endian = default_little_endian, typename std::enable_if<sizeof(T) == 1>::type* = nullptr>
	a_vector<T> get_vec(size_t n) {
		a_vector<T> r(n);
		get_bytes((uint8_t*)(void*)r.data(), n * sizeof(T));
		return r;
	}

	template<typename T, bool little_endian = default_little_endian, typename std::enable_if<(sizeof(T) > 1)>::type* = nullptr>
	a_vector<T> get_vec(size_t n) {
		a_vector<T> r(n);
		for (size_t i = 0; i < n; ++i) {
			r[i] = get<T, little_endian>();
		}
		return r;
	}

	void seek(size_t offset) {
		if ((size_t)(long)offset != offset || fseek(f, (long)offset, SEEK_SET)) error("file_reader: %s: failed to seek to offset %d", filename, offset);
	}

	bool eof() {
		return feof(f);
	}
	
	size_t left() const {
		return size() - tell();
	}
	size_t tell() const {
		return (size_t)ftell(f);
	}

	size_t size() {
		auto prev_pos = ftell(f);
		fseek(f, 0, SEEK_END);
		auto r = ftell(f);
		fseek(f, prev_pos, SEEK_SET);
		return r;
	}

};

template<typename base_reader_T, bool default_little_endian = true>
struct bit_reader {
	base_reader_T& r;
	uint64_t data{};
	size_t bits_n = 0;
	explicit bit_reader(base_reader_T& r) : r(r) {}
	void next() {
		auto left = r.left();
		if (left >= 4) {
			data |= (uint64_t)r.template get<uint32_t, true>() << bits_n;
			bits_n += 32;
			return;
		}
		switch (left) {
		case 0:
		case 1:
			data |= (uint64_t)r.template get<uint8_t>() << bits_n;
			bits_n += 8;
			break;
		case 2:
			data |= (uint64_t)r.template get<uint16_t, true>() << bits_n;
			bits_n += 16;
			break;
		case 3:
			data |= (uint64_t)r.template get<uint16_t, true>() << bits_n;
			bits_n += 16;
			data |= (uint64_t)r.template get<uint8_t>() << bits_n;
			bits_n += 8;
			break;
		}
	}
	template<size_t bits, bool little_endian = default_little_endian>
	auto get_bits() {
		static_assert(bits <= 32, "bit_reader: only up to 32 bit reads are supported");
		using r_t = uint_fastn_t<bits>;
		if (bits_n < bits) {
			next();
		}
		r_t r = data & (((r_t)1 << bits) - 1);
		data >>= bits;
		bits_n -= bits;
		return r;
	}
	template<typename T, bool little_endian = default_little_endian>
	T get() {
		return get_bits<int_bits<T>::value, little_endian>();
	}
};

template<bool little_endian = true, typename base_reader_T>
auto make_bit_reader(base_reader_T& reader) {
	return bit_reader<base_reader_T, little_endian>(reader);
}

template<bool little_endian = true>
void decompress(uint8_t* input, size_t input_size, uint8_t* output, size_t output_size) {
	data_reader<little_endian> source_r(input, input + input_size);
	auto r = make_bit_reader(source_r);
	int type = r.template get<uint8_t>();
	int distance_bits = r.template get<uint8_t>();

	if (distance_bits != 4 && distance_bits != 5 && distance_bits != 6) error("decompress: invalid distance bits %d", distance_bits);

	auto get_length = [&]() {
		switch (r.template get_bits<2>()) {
		case 3: return 1;
		case 0:
			switch (r.template get_bits<2>()) {
			case 3: return 6;
			case 0:
				switch (r.template get_bits<6>()) {
				case 3: return 22;
				case 7: return 23;
				case 11: return 24;
				case 15: return 25;
				case 19: return 26;
				case 23: return 27;
				case 27: return 28;
				case 31: return 29;
				case 35: return 30;
				case 39: return 31;
				case 43: return 32;
				case 47: return 33;
				case 51: return 34;
				case 55: return 35;
				case 59: return 36;
				case 63: return 37;
				case 0: return 262 + 8 * r.template get_bits<5>();
				case 1: return r.template get_bits<1>() ? 54 : 38;
				case 2: return 70 + 16 * r.template get_bits<2>();
				case 4: return 134 + 8 * r.template get_bits<4>();
				case 5: return r.template get_bits<1>() ? 55 : 39;
				case 6: return 71 + 16 * r.template get_bits<2>();
				case 8: return 263 + 8 * r.template get_bits<5>();
				case 9: return r.template get_bits<1>() ? 56 : 40;
				case 10: return 72 + 16 * r.template get_bits<2>();
				case 12: return 135 + 8 * r.template get_bits<4>();
				case 13: return r.template get_bits<1>() ? 57 : 41;
				case 14: return 73 + 16 * r.template get_bits<2>();
				case 16: return 264 + 8 * r.template get_bits<5>();
				case 17: return r.template get_bits<1>() ? 58 : 42;
				case 18: return 74 + 16 * r.template get_bits<2>();
				case 20: return 136 + 8 * r.template get_bits<4>();
				case 21: return r.template get_bits<1>() ? 59 : 43;
				case 22: return 75 + 16 * r.template get_bits<2>();
				case 24: return 265 + 8 * r.template get_bits<5>();
				case 25: return r.template get_bits<1>() ? 60 : 44;
				case 26: return 76 + 16 * r.template get_bits<2>();
				case 28: return 137 + 8 * r.template get_bits<4>();
				case 29: return r.template get_bits<1>() ? 61 : 45;
				case 30: return 77 + 16 * r.template get_bits<2>();
				case 32: return 266 + 8 * r.template get_bits<5>();
				case 33: return r.template get_bits<1>() ? 62 : 46;
				case 34: return 78 + 16 * r.template get_bits<2>();
				case 36: return 138 + 8 * r.template get_bits<4>();
				case 37: return r.template get_bits<1>() ? 63 : 47;
				case 38: return 79 + 16 * r.template get_bits<2>();
				case 40: return 267 + 8 * r.template get_bits<5>();
				case 41: return r.template get_bits<1>() ? 64 : 48;
				case 42: return 80 + 16 * r.template get_bits<2>();
				case 44: return 139 + 8 * r.template get_bits<4>();
				case 45: return r.template get_bits<1>() ? 65 : 49;
				case 46: return 81 + 16 * r.template get_bits<2>();
				case 48: return 268 + 8 * r.template get_bits<5>();
				case 49: return r.template get_bits<1>() ? 66 : 50;
				case 50: return 82 + 16 * r.template get_bits<2>();
				case 52: return 140 + 8 * r.template get_bits<4>();
				case 53: return r.template get_bits<1>() ? 67 : 51;
				case 54: return 83 + 16 * r.template get_bits<2>();
				case 56: return 269 + 8 * r.template get_bits<5>();
				case 57: return r.template get_bits<1>() ? 68 : 52;
				case 58: return 84 + 16 * r.template get_bits<2>();
				case 60: return 141 + 8 * r.template get_bits<4>();
				case 61: return r.template get_bits<1>() ? 69 : 53;
				case 62: return 85 + 16 * r.template get_bits<2>();
				}
			case 1:
				switch (r.template get_bits<1>()) {
				case 1: return 7;
				case 0: return r.template get_bits<1>() ? 9 : 8;
				}
			case 2:
				switch (r.template get_bits<3>()) {
				case 1: return 10;
				case 3: return 11;
				case 5: return 12;
				case 7: return 13;
				case 0: return r.template get_bits<1>() ? 18 : 14;
				case 2: return r.template get_bits<1>() ? 19 : 15;
				case 4: return r.template get_bits<1>() ? 20 : 16;
				case 6: return r.template get_bits<1>() ? 21 : 17;
				}
			}
		case 1: return r.template get_bits<1>() ? 0 : 2;
		case 2:
			switch (r.template get_bits<1>()) {
			case 1: return 3;
			case 0: return r.template get_bits<1>() ? 4 : 5;
			}
		}
		return -1;
	};

	auto get_distance = [&]() {
		switch (r.template get_bits<2>()) {
		case 3: return 0;
		case 0:
			switch (r.template get_bits<5>()) {
			case 1: return 39;
			case 2: return 47;
			case 3: return 31;
			case 5: return 35;
			case 6: return 43;
			case 7: return 27;
			case 9: return 37;
			case 10: return 45;
			case 11: return 29;
			case 13: return 33;
			case 14: return 41;
			case 15: return 25;
			case 17: return 38;
			case 18: return 46;
			case 19: return 30;
			case 21: return 34;
			case 22: return 42;
			case 23: return 26;
			case 25: return 36;
			case 26: return 44;
			case 27: return 28;
			case 29: return 32;
			case 30: return 40;
			case 31: return 24;
			case 0: return r.template get_bits<1>() ? 62 : 63;
			case 4: return r.template get_bits<1>() ? 54 : 55;
			case 8: return r.template get_bits<1>() ? 58 : 59;
			case 12: return r.template get_bits<1>() ? 50 : 51;
			case 16: return r.template get_bits<1>() ? 60 : 61;
			case 20: return r.template get_bits<1>() ? 52 : 53;
			case 24: return r.template get_bits<1>() ? 56 : 57;
			case 28: return r.template get_bits<1>() ? 48 : 49;
			}
		case 1:
			switch (r.template get_bits<2>()) {
			case 1: return 2;
			case 3: return 1;
			case 0: return r.template get_bits<1>() ? 5 : 6;
			case 2: return r.template get_bits<1>() ? 3 : 4;
			}
		case 2:
			switch (r.template get_bits<4>()) {
			case 1: return 14;
			case 2: return 18;
			case 3: return 10;
			case 4: return 20;
			case 5: return 12;
			case 6: return 16;
			case 7: return 8;
			case 8: return 21;
			case 9: return 13;
			case 10: return 17;
			case 11: return 9;
			case 12: return 19;
			case 13: return 11;
			case 14: return 15;
			case 15: return 7;
			case 0: return r.template get_bits<1>() ? 22 : 23;
			}
		}
		return -1;
	};


	size_t out_pos = 0;

	if (type == 0) {

		while (out_pos != output_size) {
			if (r.template get_bits<1>()) {

				size_t len = 2 + get_length();
				size_t distance = 0;

				if (len == 519) error("decompress: eof marker found too early");

				if (len == 2) {
					distance = get_distance() << 2;
					distance |= r.template get_bits<2>();
				} else {
					distance = get_distance() << distance_bits;
					if (distance_bits == 4) distance |= r.template get_bits<4>();
					else if (distance_bits == 5) distance |= r.template get_bits<5>();
					else distance |= r.template get_bits<6>();
				}
				size_t src_pos = out_pos - 1 - distance;
				if (src_pos > output_size) {
					len = 0;
				}
				if (src_pos + len > output_size) {
					len = output_size - src_pos;
				}
				if (out_pos + len > output_size) {
					len = output_size - out_pos;
				}
				for (size_t i = 0; i != len; ++i) {
					output[out_pos + i] = output[src_pos + i];
				}
				out_pos += len;

			} else {
				output[out_pos] = r.template get<uint8_t>();
				++out_pos;
			}
		}

	} else error("decompress: type %d not supported", type);
}

struct mpq_file {
	MpqFile mpq{ false, false };
	explicit mpq_file(a_string filename) {
	  mpq.open(filename);
	}
	void operator()(a_vector<uint8_t>& dst, a_string filename) {
	  mpq.getFile(filename, dst);
	}
};

struct casc_archive {
  HANDLE h_casc = nullptr;
  a_string filename;

  casc_archive() = default;

  explicit casc_archive(a_string filename)
  {
	open_archive(filename);
  }
  casc_archive(const casc_archive& other)
  {
	open_archive(other.filename);
  }

  casc_archive(casc_archive&& other) noexcept
	: h_casc(other.h_casc)
	, filename(other.filename)
  {
	other.h_casc = nullptr;
  }

  bool open_archive(a_string filename) {
	this->filename = filename;
	std::wstring w_filename{ filename.cbegin(), filename.cend() };
	if (!CascOpenStorage(w_filename.c_str(), 0, &h_casc) || !h_casc) {
	  error("Failed to open CASC storage: %s", filename);
	  return false;
	}
	return true;
  }

  ~casc_archive() {
	if (h_casc != nullptr) CascCloseStorage(h_casc);
  }
  void operator()(a_vector<uint8_t>& dst, a_string filename) {
	get_file(dst, filename);
  }

  void get_file(a_vector<uint8_t>& dst, a_string filename) {

	auto raise_error = [&]() { error("Filename: %s\nError: 0x%08x", filename, GetLastError()); };

	HANDLE h_file = nullptr;
	if (!CascOpenFile(h_casc, filename.c_str(), 0, 0, &h_file) || !h_file) {
	  raise_error();
	}

	DWORD filesize = CascGetFileSize(h_file, nullptr);
	if (filesize == CASC_INVALID_SIZE) {
	  raise_error();
	}

	DWORD read_filesize = 0;
	dst.resize(filesize);
	if (!CascReadFile(h_file, dst.data(), filesize, &read_filesize)) {
	  raise_error();
	}
	if (filesize != read_filesize) error("Read byte count was not the expected filesize for file: %s", filename);

	CascCloseFile(h_file);
  }
};

template<typename mpq_file_T = mpq_file, typename casc_archive_T = casc_archive>
struct data_files_loader {
	casc_archive_T casc;

	void open_casc_archive(a_string filename) {
	  casc.open_archive(filename);
	}

	void operator()(a_vector<uint8_t>& dst, a_string filename) {
	  casc.get_file(dst, filename);
	}
};

template<typename data_files_loader_T = data_files_loader<>>
data_files_loader_T data_files_directory(a_string path) {
	data_files_loader_T r;
	r.open_casc_archive(path);
	return r;
}

template<typename to_T, typename from_T>
struct data_type_cast_helper {
	to_T operator()(from_T v) {
		static_assert(std::is_integral<from_T>::value, "from_T must be integral");
		to_T r = (to_T)v;
		if ((from_T)(intmax_t)r != v) error("value 0x%x of type %s does not fit in type %s", v, typeid(from_T).name(), typeid(to_T).name());
		return r;
	}
};
template<typename to_T, typename from_T>
to_T data_type_cast(from_T v) {
	return data_type_cast_helper<to_T, from_T>()(v);
}
template<typename from_T>
struct data_type_cast_helper<bool, from_T> {
	bool operator()(from_T v) {
		static_assert(std::is_integral<from_T>::value, "from_T must be integral");
		if (v != 0 && v != 1) error("value 0x%x is not a boolean", v);
		return v != 0;
	}
};
template<>
struct data_type_cast_helper<fp8, int32_t> {
	fp8 operator()(int32_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<direction_t, int8_t> {
	direction_t operator()(int8_t v) {
		return direction_t::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp8, int8_t> {
	fp8 operator()(int8_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp8, uint8_t> {
	fp8 operator()(uint8_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp8, int16_t> {
	fp8 operator()(int16_t v) {
		return fp8::from_raw(v);
	}
};
template<>
struct data_type_cast_helper<fp1, uint8_t> {
	fp1 operator()(uint8_t v) {
		return fp1::from_raw(v);
	}
};
template<typename T, typename from_T>
struct data_type_cast_helper<type_id<T>, from_T> {
	type_id<T> operator()(from_T v) {
		return type_id<T>(data_type_cast<decltype(std::declval<T>().id), from_T>(v));
	}
};

template<typename load_T, typename field_T>
struct read_data {
	static const size_t read_n = 1;
	template<typename reader_T>
	static field_T read(reader_T& r) {
		return data_type_cast<field_T>(r.template get<load_T>());
	}
};
template<typename load_T>
struct read_data<load_T, xy> {
	static const size_t read_n = 2;
	template<typename reader_T>
	static xy read(reader_T& r) {
		int x = data_type_cast<int>(r.template get<load_T>());
		int y = data_type_cast<int>(r.template get<load_T>());
		return xy(x, y);
	}
};
template<typename load_T>
struct read_data<load_T, rect> {
	static const size_t read_n = 4;
	template<typename reader_T>
	static rect read(reader_T& r) {
		int x0 = data_type_cast<int>(r.template get<load_T>());
		int y0 = data_type_cast<int>(r.template get<load_T>());
		int x1 = data_type_cast<int>(r.template get<load_T>());
		int y1 = data_type_cast<int>(r.template get<load_T>());
		return { {x0, y0}, {x1, y1} };
	}
};

template<typename load_T, typename reader_T, typename ptr_F>
void read_array(reader_T& r, size_t num, ptr_F&& ptr_f) {
	using read_data_t = read_data<load_T, typename std::remove_reference<decltype(*ptr_f(0))>::type>;
	size_t total_size = sizeof(load_T) * read_data_t::read_n * num;
	const uint8_t* ptr = r.get_n(total_size);
	for (size_t i = 0; i < num; ++i) {
		data_reader<true, false> r2(ptr, nullptr);
		auto* field = ptr_f(i);
		*field = read_data_t::read(r2);
		ptr += sizeof(load_T) * read_data_t::read_n;
	}
}

#define rawr(load_type, name, num) read_array<load_type>(r, num, [&](size_t index) {return &arr[index].name;})
#define rawro(load_type, name, num, offset) read_array<load_type>(r, num, [&](size_t index) {return &arr[offset + index].name;})

template<typename data_T>
unit_types_t load_units_dat(const data_T& data) {
	static const size_t total_count = 228;
	static const size_t units_count = 106;
	static const size_t buildings_count = 96;

	unit_types_t unit_types;
	unit_types.vec.resize(total_count);
	for (size_t i = 0; i < total_count; ++i) {
		auto& v = unit_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (UnitTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = unit_types.vec;

	rawr(uint8_t, flingy, total_count);
	rawr(uint16_t, turret_unit_type, total_count);
	rawr(uint16_t, subunit2, total_count);
	rawro(uint16_t, infestation_unit, buildings_count, units_count);
	rawr(uint32_t, construction_animation, total_count);
	rawr(uint8_t, unit_direction, total_count);
	rawr(uint8_t, has_shield, total_count);
	rawr(uint16_t, shield_points, total_count);
	rawr(int32_t, hitpoints, total_count);
	rawr(uint8_t, elevation_level, total_count);
	rawr(uint8_t, unknown1, total_count);
	rawr(uint8_t, sublabel, total_count);
	rawr(uint8_t, computer_ai_idle, total_count);
	rawr(uint8_t, human_ai_idle, total_count);
	rawr(uint8_t, return_to_idle, total_count);
	rawr(uint8_t, attack_unit, total_count);
	rawr(uint8_t, attack_move, total_count);
	rawr(uint8_t, ground_weapon, total_count);
	rawr(uint8_t, max_ground_hits, total_count);
	rawr(uint8_t, air_weapon, total_count);
	rawr(uint8_t, max_air_hits, total_count);
	rawr(uint8_t, ai_internal, total_count);
	rawr(uint32_t, flags, total_count);
	rawr(uint8_t, target_acquisition_range, total_count);
	rawr(uint8_t, sight_range, total_count);
	rawr(uint8_t, armor_upgrade, total_count);
	rawr(uint8_t, unit_size, total_count);
	rawr(uint8_t, armor, total_count);
	rawr(uint8_t, right_click_action, total_count);
	rawr(uint16_t, ready_sound, units_count);
	rawr(uint16_t, first_what_sound, total_count);
	rawr(uint16_t, last_what_sound, total_count);
	rawr(uint16_t, first_pissed_sound, units_count);
	rawr(uint16_t, last_pissed_sound, units_count);
	rawr(uint16_t, first_yes_sound, units_count);
	rawr(uint16_t, last_yes_sound, units_count);
	rawr(int16_t, placement_size, total_count);
	rawro(int16_t, addon_position, buildings_count, units_count);
	rawr(int16_t, dimensions, total_count);
	rawr(uint16_t, portrait, total_count);
	rawr(uint16_t, mineral_cost, total_count);
	rawr(uint16_t, gas_cost, total_count);
	rawr(uint16_t, build_time, total_count);
	rawr(uint16_t, unknown2, total_count);
	rawr(uint8_t, group_flags, total_count);
	rawr(uint8_t, supply_provided, total_count);
	rawr(uint8_t, supply_required, total_count);
	rawr(uint8_t, space_required, total_count);
	rawr(uint8_t, space_provided, total_count);
	rawr(uint16_t, build_score, total_count);
	rawr(uint16_t, destroy_score, total_count);
	rawr(uint16_t, unit_map_string_index, total_count);
	rawr(uint8_t, is_broodwar, total_count);
	rawr(uint16_t, staredit_availability_flags, total_count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "units.dat", r.left());

	return unit_types;
}

template<typename data_T>
weapon_types_t load_weapons_dat(const data_T& data) {
	static const size_t count = 130;

	weapon_types_t weapon_types;
	weapon_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = weapon_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (WeaponTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = weapon_types.vec;

	rawr(uint16_t, label, count);
	rawr(uint32_t, flingy, count);
	rawr(uint8_t, unused, count);
	rawr(uint16_t, target_flags, count);
	rawr(uint32_t, min_range, count);
	rawr(uint32_t, max_range, count);
	rawr(uint8_t, damage_upgrade, count);
	rawr(uint8_t, damage_type, count);
	rawr(uint8_t, bullet_type, count);
	rawr(uint8_t, lifetime, count);
	rawr(uint8_t, hit_type, count);
	rawr(uint16_t, inner_splash_radius, count);
	rawr(uint16_t, medium_splash_radius, count);
	rawr(uint16_t, outer_splash_radius, count);
	rawr(uint16_t, damage_amount, count);
	rawr(uint16_t, damage_bonus, count);
	rawr(uint8_t, cooldown, count);
	rawr(uint8_t, bullet_count, count);
	rawr(uint8_t, attack_angle, count);
	rawr(int8_t, bullet_heading_offset, count);
	rawr(int8_t, forward_offset, count);
	rawr(int8_t, upward_offset, count);
	rawr(uint16_t, target_error_message, count);
	rawr(uint16_t, icon, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "weapons.dat", r.left());

	return weapon_types;
}

template<typename data_T>
upgrade_types_t load_upgrades_dat(const data_T& data) {
	static const size_t count = 61;

	upgrade_types_t upgrade_types;
	upgrade_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = upgrade_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (UpgradeTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = upgrade_types.vec;

	rawr(uint16_t, mineral_cost_base, count);
	rawr(uint16_t, mineral_cost_factor, count);
	rawr(uint16_t, gas_cost_base, count);
	rawr(uint16_t, gas_cost_factor, count);
	rawr(uint16_t, time_cost_base, count);
	rawr(uint16_t, time_cost_factor, count);
	rawr(uint16_t, unknown, count);
	rawr(uint16_t, icon, count);
	rawr(uint16_t, label, count);
	rawr(uint8_t, race, count);
	rawr(uint8_t, max_level, count);
	rawr(uint8_t, is_broodwar, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "upgrades.dat", r.left());

	return upgrade_types;
}

template<typename data_T>
tech_types_t load_techdata_dat(const data_T& data) {
	static const size_t count = 44;

	tech_types_t tech_types;
	tech_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = tech_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (TechTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = tech_types.vec;

	rawr(uint16_t, mineral_cost, count);
	rawr(uint16_t, gas_cost, count);
	rawr(uint16_t, research_time, count);
	rawr(uint16_t, energy_cost, count);
	rawr(uint32_t, unknown, count);
	rawr(uint16_t, icon, count);
	rawr(uint16_t, label, count);
	rawr(uint8_t, race, count);
	rawr(uint16_t, flags, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "techdata.dat", r.left());

	return tech_types;
}

template<typename data_T>
flingy_types_t load_flingy_dat(const data_T& data) {
	static const size_t count = 209;

	flingy_types_t flingy_types;
	flingy_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = flingy_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (FlingyTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = flingy_types.vec;

	rawr(uint16_t, sprite, count);
	rawr(int32_t, top_speed, count);
	rawr(int16_t, acceleration, count);
	rawr(int32_t, halt_distance, count);
	rawr(uint8_t, turn_rate, count);
	rawr(uint8_t, unused, count);
	rawr(uint8_t, movement_type, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "flingy.dat", r.left());

	return flingy_types;
}

template<typename data_T>
sprite_types_t load_sprites_dat(const data_T& data) {
	static const size_t count = 517;
	static const size_t selectable_count = 387;
	static const size_t non_selectable_count = 130;

	sprite_types_t sprite_types;
	sprite_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = sprite_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (SpriteTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = sprite_types.vec;

	rawr(uint16_t, image, count);
	rawro(uint8_t, health_bar_size, selectable_count, non_selectable_count);
	rawr(uint8_t, unk0, count);
	rawr(uint8_t, visible, count);
	rawro(uint8_t, selection_circle, selectable_count, non_selectable_count);
	rawro(uint8_t, selection_circle_vpos, selectable_count, non_selectable_count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "sprites.dat", r.left());

	return sprite_types;
}

template<typename data_T>
image_types_t load_images_dat(const data_T& data) {
	static const size_t count = 999;

	image_types_t image_types;
	image_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = image_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (ImageTypes)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = image_types.vec;

	rawr(uint32_t, grp_filename_index, count);
	rawr(uint8_t, has_directional_frames, count);
	rawr(uint8_t, is_clickable, count);
	rawr(uint8_t, has_iscript_animations, count);
	rawr(uint8_t, always_visible, count);
	rawr(uint8_t, modifier, count);
	rawr(uint8_t, color_shift, count);
	rawr(uint32_t, iscript_id, count);
	rawr(uint32_t, shield_filename_index, count);
	rawr(uint32_t, attack_filename_index, count);
	rawr(uint32_t, damage_filename_index, count);
	rawr(uint32_t, special_filename_index, count);
	rawr(uint32_t, landing_dust_filename_index, count);
	rawr(uint32_t, lift_off_filename_index, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "images.dat", r.left());

	return image_types;
}

template<typename data_T>
order_types_t load_orders_dat(const data_T& data) {
	static const size_t count = 189;

	order_types_t order_types;
	order_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = order_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (Orders)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = order_types.vec;

	rawr(uint16_t, label, count);
	rawr(uint8_t, targets_enemies, count);
	rawr(uint8_t, background, count);
	rawr(uint8_t, unused3, count);
	rawr(uint8_t, valid_for_turret, count);
	rawr(uint8_t, unused5, count);
	rawr(uint8_t, can_be_interrupted, count);
	rawr(uint8_t, unk7, count);
	rawr(uint8_t, can_be_queued, count);
	rawr(uint8_t, unk9, count);
	rawr(uint8_t, can_be_obstructed, count);
	rawr(uint8_t, unk11, count);
	rawr(uint8_t, unused12, count);
	rawr(uint8_t, weapon, count);
	rawr(uint8_t, tech_type, count);
	rawr(uint8_t, animation, count);
	rawr(int16_t, highlight, count);
	rawr(uint16_t, dep_index, count);
	rawr(uint8_t, target_order, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "orders.dat", r.left());

	return order_types;
}

template<typename data_T>
sound_types_t load_sfxdata_dat(const data_T& data) {
	static const size_t count = 1144;

	sound_types_t sound_types;
	sound_types.vec.resize(count);
	for (size_t i = 0; i < count; ++i) {
		auto& v = sound_types.vec[i];
		memset(&v, 0, sizeof(v));
		v.id = (Sounds)i;
	}

	data_reader_le r(data.data(), data.data() + data.size());

	auto& arr = sound_types.vec;

	rawr(uint32_t, filename_index, count);
	rawr(uint8_t, priority, count);
	rawr(uint8_t, flags, count);
	rawr(uint16_t, race, count);
	rawr(uint8_t, min_volume, count);

	if (r.left()) error("%s: %d bytes left (incorrect version?)\n", "sfxdata.dat", r.left());

	return sound_types;
}

#undef rawro
#undef rawr

}
}

#endif
