#include "state.h"

ImGui::FileBrowser dialogs::INPUT_FILE(
    ImGuiFileBrowserFlags_EditPathString |
    ImGuiFileBrowserFlags_CreateNewDir |
    ImGuiFileBrowserFlags_CloseOnEsc
);

ImGui::FileBrowser dialogs::OUTPUT_FILE(
    ImGuiFileBrowserFlags_EditPathString |
    ImGuiFileBrowserFlags_EnterNewFilename |
    ImGuiFileBrowserFlags_CreateNewDir |
    ImGuiFileBrowserFlags_CloseOnEsc
);

ImGui::FileBrowser dialogs::SCRIPT_OPEN_FILE(
    ImGuiFileBrowserFlags_EditPathString |
    ImGuiFileBrowserFlags_CreateNewDir |
    ImGuiFileBrowserFlags_CloseOnEsc
);

ImGui::FileBrowser dialogs::SCRIPT_SAVE_FILE(
    ImGuiFileBrowserFlags_EditPathString |
    ImGuiFileBrowserFlags_EnterNewFilename |
    ImGuiFileBrowserFlags_CreateNewDir |
    ImGuiFileBrowserFlags_CloseOnEsc
);

void init_state()
{
    std::fill(STATE.show_highlights.begin(), STATE.show_highlights.end(), true);

    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Yellow)] = ImVec4(1.0f, 1.0f, 0.0f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Red)] = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Green)] = ImVec4(0.0f, 1.0f, 0.0f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Blue)] = ImVec4(0.0f, 0.0f, 1.0f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Light)] = ImVec4(0.7f, 0.7f, 0.7f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Dark)] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Purple)] = ImVec4(1.0f, 0.0f, 1.0f, 0.5f);
    GUI.highlight_colors[lx::color_idx(lx::HighlightColor::Orange)] = ImVec4(1.0f, 0.6f, 0.0f, 0.5f);

    // TODO use user cache and SetDirectory() to load last opened directories from previous app instance. also cache the colors and user preferences

    dialogs::INPUT_FILE.SetTitle("Load Input From File");
    dialogs::INPUT_FILE.SetTypeFilters({ ".txt", ".*" });

    dialogs::OUTPUT_FILE.SetTitle("Save Output To File");
    dialogs::OUTPUT_FILE.SetTypeFilters({ ".txt", ".*" });

    dialogs::SCRIPT_OPEN_FILE.SetTitle("Open Script");
    dialogs::SCRIPT_OPEN_FILE.SetTypeFilters({ ".lx", ".*" });

    dialogs::SCRIPT_SAVE_FILE.SetTitle("Save Script");
    dialogs::SCRIPT_SAVE_FILE.SetTypeFilters({ ".lx", ".*" });
}
