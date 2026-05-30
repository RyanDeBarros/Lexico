#pragma once

#include <filesystem>
#include <optional>

extern void run_script();
extern void open_input_file(const std::filesystem::path& path);
extern void save_output_file(const std::filesystem::path& path);
extern void process_file_dialogs();
