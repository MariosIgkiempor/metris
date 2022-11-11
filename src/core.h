
#pragma once

#include <fmt/core.h>

#include "logging.h"
#include "result.h"

#include <cstdint>
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using usize = std::size_t;
using f32 = float;
using f64 = double;

#include <string>
using String = std::string;

#include <string_view>
using StringView = std::string_view;

#include <filesystem>
using Path = std::filesystem::path;

#include <vector>
template <typename T>
using Vec = std::vector<T>;

#include <map>
template <typename K, typename V>
using Table = std::map<K, V>;

#include <source_location>
using SourceLocation = std::source_location;

#include <memory>
template <typename T>
using RefPtr = std::shared_ptr<T>;
template <typename T>
using OwnPtr = std::unique_ptr<T>;

// Source: https://www.foonathan.net/2022/05/recursive-variant-box/
template<typename T> struct Box {
  std::unique_ptr<T> pointer;
  Box(T&& object) : pointer(new T(std::move(object))) {}
  Box(const T& object) : pointer(new T(object)) {}
  Box(const Box& other) : Box(*other.pointer) {}

  Box &operator=(const Box& other) {
    *pointer = *other.pointer;
    return *this;
  }

  ~Box() = default; // std::unique pointer will destroy the object.

  T& operator*() { return *pointer; }
  const T& operator*() const { return *pointer; }
  T* operator->() { return pointer.get(); }
  const T* operator->() const { return pointer.get(); }
};



#include <variant>
template <typename... Ts>
using Variant = std::variant<Ts...>;


bool is_space(char c);
bool is_letter(char c);
bool is_digit(char c);

bool is_power_of_two(usize n);

// Defer statement
// https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})


// Files

struct File {
  Path   path = {};
  String contents = "";
};

struct ReadFileError {
  enum class Kind {
    none,
    file_not_found,
    file_not_readable,
  };

  ReadFileError::Kind error_kind = ReadFileError::Kind::none;
  Path                path       = {};
};

Result<File, ReadFileError> read_file(String input_filepath);





#define KiB(x)   ((usize) (x) << 10)
#define MiB(x)   ((usize) (x) << 20)

constexpr auto default_memory_alignment = 2 * sizeof(void*);




// Memory arena
// @Source: https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
struct MemoryArena {
  u8* memory = nullptr;
  usize size = 0;
  usize previous_offset = 0;
  usize current_offset = 0;
};

MemoryArena make_memory_arena(void* memory, usize memory_size);
void  memory_arena_free(MemoryArena* arena, void* pointer);

void* memory_arena_resize_aligned(MemoryArena* arena, void* pointer, usize old_size, usize new_size, usize alignment);
void* memory_arena_resize(MemoryArena* arena, void* pointer, usize old_size, usize new_size);

void memory_arena_reset(MemoryArena* arena);

void* memory_arena_allocate_aligned(MemoryArena* arena, usize size, usize alignment);
void* memory_arena_allocate(MemoryArena* arena, usize size);




// Linear algebra
template<typename T>
struct Vector2 {
  T x = {};
  T y = {};
};

template<typename T>
Vector2<T> make_vector2(T x, T y) {
  Vector2<T> result;
  result.x = x;
  result.y = y;
  return result;
}

template<typename T>
bool vector2_equal(Vector2<T> a, Vector2<T> b) {
  return a.x == b.x && a.y == b.y;
}

template<typename T>
Vector2<T> vector2_add(Vector2<T> a, Vector2<T> b) {
  return make_vector2(a.x + b.x, a.y + b.y);
}

template<typename T>
Vector2<T> vector2_mul(Vector2<T> a, Vector2<T> b) {
  return make_vector2(a.x * b.x, a.y * b.y);
}

template<typename T>
Vector2<T> vector2_mul(Vector2<T> a, T b) {
  return make_vector2(a.x * b, a.y * b);
}



template<typename T>
struct Vector3 {
  T x = {};
  T y = {};
  T z = {};
};

template<typename T>
Vector3<T> make_vector3(T x, T y, T z) {
  Vector3<T> result;
  result.x = x;
  result.y = y;
  result.z = z;
  return result;
}



template<typename T>
struct Vector4 {
  T x = {};
  T y = {};
  T z = {};
  T w = {};
};

template<typename T>
Vector4<T> make_vector4(T x, T y, T z, T w) {
  Vector4<T> result;
  result.x = x;
  result.y = y;
  result.z = z;
  result.w = w;
  return result;
}

