
#pragma once



#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/color.h>

const auto log_error_color = fmt::color::deep_pink;
const auto log_warning_color = fmt::color::orange;
const auto log_note_color = fmt::color::light_steel_blue;
const auto log_info_color = fmt::color::plum;

void log_print(const char* file, int line, fmt::string_view format, fmt::format_args args);
void vlog_print(fmt::string_view format, fmt::format_args args);

template <typename S, typename... Args>
void log(const char* file, int line, const S& format, Args&&... args) {
  vlog_print(file, line, format, fmt::make_format_args(args...));
}



template <typename S, typename... Args>
void log(const S& format, Args&&... args) {
  vlog_print(format, fmt::make_format_args(args...));
}



template <typename S, typename... Args>
void log_fatal(const S& message, Args &&...args) {
  fmt::print(fmt::emphasis::bold | fmt::fg(log_error_color),    "[FATAL ERROR] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
  exit(1);
}



template <typename S, typename... Args>
void log_assert(bool condition, const S& message, Args &&...args) {
  if (condition)  return;

  fmt::print(fmt::emphasis::bold | fmt::fg(log_error_color),  " [assert] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
  exit(1);
}




template <typename S, typename... Args>
void log_error(const S& message, Args &&...args) {
  fmt::print(fmt::emphasis::bold | fmt::fg(log_error_color),    "  [error] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
}



template <typename S, typename... Args>
void log_note(const S& message, Args &&...args) {
  fmt::print(fmt::emphasis::bold | fmt::fg(log_note_color), "   [note] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
}



template <typename S, typename... Args>
void log_warning(const S& message, Args &&...args) {
  fmt::print(fmt::emphasis::bold | fmt::fg(log_warning_color), "[warning] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
}



template <typename S, typename... Args>
void log_info(const S& message, Args &&...args) {
  fmt::print(fmt::emphasis::bold | fmt::fg(log_info_color),  "   [info] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
}



template <typename S, typename... Args>
void log_command(const S& message, Args &&...args) {
  fmt::print(fmt::emphasis::bold | fmt::fg(log_info_color),  " [system] ");
  fmt::vprint(message, fmt::make_format_args(args...));
  fmt::print("\n");
}


