#include "state.h"

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

    dialogs::INPUT_FILE.SetTitle("Open Input File");
    dialogs::INPUT_FILE.SetTypeFilters({ ".txt", ".*" });
}
