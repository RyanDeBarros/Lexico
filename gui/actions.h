#pragma once

#include <filesystem>
#include <optional>

extern void run_script();
extern void open_input_file(const std::filesystem::path& path);
extern void process_file_dialogs();
extern void close_file_dialogs();
