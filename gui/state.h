#pragma once

#include "vendor.h"

struct EditorState
{
    std::string input;
    std::string output;
    std::string script;
    std::string log;
    bool success;
    lx::HighlightMap highlight_map;
    std::array<std::vector<ImRect>, lx::color_count()> highlight_rects;
    std::array<bool, lx::color_count()> show_highlights;
};

inline EditorState STATE{};

struct GUIState
{
    bool show_highlight_modal = false;
    std::array<ImVec4, lx::color_count()> highlight_colors;
};

inline GUIState GUI{};

namespace dialogs
{
    extern ImGui::FileBrowser INPUT_FILE;
    extern ImGui::FileBrowser OUTPUT_FILE;
    extern ImGui::FileBrowser SCRIPT_OPEN_FILE;
    extern ImGui::FileBrowser SCRIPT_SAVE_FILE;
}

inline std::vector<std::string> DROPPED_PATHS;

extern void init_state();
