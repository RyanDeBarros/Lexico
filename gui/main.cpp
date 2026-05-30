#include "state.h"
#include "actions.h"

#include <iostream>
#include <optional>

static const char* DOCKSPACE_UID = "MyDockSpace";
static const char* INPUT_WINDOW = "Input";
static const char* OUTPUT_WINDOW = "Output";
static const char* SCRIPT_WINDOW = "Script";
static const char* LOG_WINDOW = "Log";

enum class Channel
{
    Highlight,
    Main,
    _Count
};

static std::array<const char*, lx::color_count()> COLOR_NAMES = {
    "Yellow",
    "Red",
    "Green",
    "Blue",
    "Light",
    "Dark",
    "Purple",
    "Orange",
};

static ImU32 mapped_color(lx::HighlightColor c)
{
    ImVec4 color = GUI.highlight_colors[static_cast<size_t>(c)];
    return IM_COL32(
        static_cast<unsigned int>(roundf(color.x * 255)),
        static_cast<unsigned int>(roundf(color.y * 255)),
        static_cast<unsigned int>(roundf(color.z * 255)),
        static_cast<unsigned int>(roundf(color.w * 255))
    );
}

static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

static void build_dockspace_layout()
{
    ImGuiID dockspace_id = ImGui::GetID(DOCKSPACE_UID);
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGuiID dock_main = dockspace_id;

    ImGuiID dock_top, dock_bottom;
    dock_top = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, 0.5f, nullptr, &dock_bottom);

    ImGuiID dock_top_left, dock_top_right;
    dock_top_left = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.5f, nullptr, &dock_top_right);

    ImGuiID dock_bottom_left, dock_bottom_right;
    dock_bottom_left = ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Left, 0.5f, nullptr, &dock_bottom_right);

    ImGui::DockBuilderDockWindow(INPUT_WINDOW, dock_top_left);
    ImGui::DockBuilderDockWindow(OUTPUT_WINDOW, dock_top_right);
    ImGui::DockBuilderDockWindow(SCRIPT_WINDOW, dock_bottom_left);
    ImGui::DockBuilderDockWindow(LOG_WINDOW, dock_bottom_right);

    ImGui::DockBuilderFinish(dockspace_id);
}

static void draw_dockspace()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("DockSpace Window", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID(DOCKSPACE_UID);
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    static bool first_time = true;
    if (first_time)
    {
        first_time = false;
        build_dockspace_layout();
    }

    ImGui::End();
}

static int resize_buffer_callback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        auto* str = (std::string*)data->UserData;
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }

    return 0;
}

static void draw_input_buffer(std::string& buffer)
{
    ImGui::InputTextMultiline(
        "##input",
        buffer.data(),
        buffer.capacity() + 1,
        ImVec2(-FLT_MIN, -FLT_MIN),
        ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_WordWrap,
        resize_buffer_callback,
        &buffer
    );
}

static void draw_output_buffer(std::string& buffer, std::optional<ImU32> text_color)
{
    if (text_color.has_value())
        ImGui::PushStyleColor(ImGuiCol_Text, *text_color);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
    ImGui::TextUnformatted(buffer.data());
    ImGui::PopStyleColor(3);

    if (text_color.has_value())
        ImGui::PopStyleColor();
}

static void draw_highlight_rect(ImVec2 origin, ImRect rect, ImU32 color)
{
    rect.Min.x += origin.x;
    rect.Min.y += origin.y;
    rect.Max.x += origin.x;
    rect.Max.y += origin.y;

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->ChannelsSetCurrent(static_cast<int>(Channel::Highlight));
    dl->AddRectFilled(rect.Min, rect.Max, color);
    dl->ChannelsSetCurrent(static_cast<int>(Channel::Main));
}

static void setup_window_channels()
{
    ImGui::GetWindowDrawList()->ChannelsSplit(static_cast<int>(Channel::_Count));
}

static void draw_highlight_modal()
{
    setup_window_channels();

    if (ImGui::Button("Select All"))
        std::fill(STATE.show_highlights.begin(), STATE.show_highlights.end(), true);
    ImGui::SameLine();

    if (ImGui::Button("Deselect All"))
        std::fill(STATE.show_highlights.begin(), STATE.show_highlights.end(), false);
    ImGui::Separator();

    if (ImGui::BeginTable("##selection-table", 2, ImGuiTableFlags_SizingFixedFit))
    {
        for (size_t i = 0; i < lx::color_count(); ++i)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Checkbox(COLOR_NAMES[i], STATE.show_highlights.data() + i);

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            ImGui::ColorEdit4("", &GUI.highlight_colors[i].x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}

struct WrappedLine
{
    size_t start_idx;
    size_t end_idx;
    float y;
};

static std::vector<WrappedLine> build_wrapped_lines(const float wrap_width)
{
    const char* text_begin = STATE.output.c_str();
    const char* text_end = text_begin + STATE.output.size();

    ImFont* font = ImGui::GetFont();
    const float font_size = ImGui::GetFontSize();

    const float line_h = ImGui::GetTextLineHeight();

    std::vector<WrappedLine> wrapped_lines;

    const char* p = text_begin;
    float y = 0.f;
    while (p < text_end)
    {
        const char* line_end = font->CalcWordWrapPosition(font_size, p, text_end, wrap_width);
        if (line_end > p)
        {
            size_t start_idx = p - text_begin;
            size_t end_idx = line_end - text_begin;

            wrapped_lines.push_back({ .start_idx = start_idx, .end_idx = end_idx, .y = y });
        }

        y += line_h;

        p = line_end;
        if (p < text_end)
        {
            if (*p == '\n')
                ++p;
            else if (*p == '\r')
            {
                ++p;
                if (p < text_end && *p == '\n')
                    ++p;
            }
        }
    }

    return wrapped_lines;
}

static void compute_wrapped_highlight_rects(std::vector<ImRect>& rects, lx::HighlightColor color, const std::vector<WrappedLine>& wrapped_lines)
{
    rects.clear();

    ImFont* font = ImGui::GetFont();
    const float font_size = ImGui::GetFontSize();
    const float line_h = ImGui::GetTextLineHeight();

    const char* text_begin = STATE.output.c_str();
    size_t wrapped_line_idx = 0;

    STATE.highlight_map.visit(color, [&](const lx::HighlightVisit& visit) {
        const size_t start = visit.highlight.start;
        const size_t end = visit.highlight.end();

        while (wrapped_line_idx < wrapped_lines.size())
        {
            const WrappedLine line = wrapped_lines[wrapped_line_idx];
            if (line.end_idx <= start)
            {
                ++wrapped_line_idx;
                continue;
            }
            else if (line.start_idx >= end)
                break;

            const size_t a = std::max(line.start_idx, start);
            const size_t b = std::min(line.end_idx, end);

            const float x0 = font->CalcTextSizeA(font_size, FLT_MAX, 0, text_begin + line.start_idx, text_begin + a).x;
            const float x1 = font->CalcTextSizeA(font_size, FLT_MAX, 0, text_begin + line.start_idx, text_begin + b).x;
            rects.push_back(ImRect(ImVec2(x0, line.y), ImVec2(x1, line.y + line_h)));

            const size_t next_line_idx = wrapped_line_idx + 1;
            if (next_line_idx < wrapped_lines.size() && wrapped_lines[next_line_idx].start_idx < end)
                ++wrapped_line_idx;
            else
                break;
        }
    });
}

static void compute_wrapped_highlight_rects(const float wrap_width)
{
    const std::vector<WrappedLine> wrapped_lines = build_wrapped_lines(wrap_width);
    for (size_t i = 0; i < lx::color_count(); ++i)
        compute_wrapped_highlight_rects(STATE.highlight_rects[i], static_cast<lx::HighlightColor>(i), wrapped_lines);
}

static void draw_highlights(ImVec2 origin, lx::HighlightColor color)
{
    for (ImRect rect : STATE.highlight_rects[lx::color_idx(color)])
        draw_highlight_rect(origin, rect, mapped_color(color));
}

static void draw_input_window()
{
    ImGui::Begin(INPUT_WINDOW, nullptr, ImGuiWindowFlags_MenuBar);

    static int focus_frames = 2;
    if (focus_frames > 0)
    {
        --focus_frames;
        ImGui::SetWindowFocus();
        ImGui::SetKeyboardFocusHere();
    }

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
                dialogs::INPUT_FILE.Open();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load Input From File (Ctrl+O)");

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
    {
        for (std::string& path : DROPPED_PATHS)
            open_input_file(std::move(path));

        DROPPED_PATHS.clear();
    }

    draw_input_buffer(STATE.input);
    ImGui::End();
}

static void draw_all_highlights(float wrap_width, ImVec2 origin)
{
    compute_wrapped_highlight_rects(wrap_width);

    for (size_t i = 0; i < lx::color_count(); ++i)
        if (STATE.show_highlights[i])
            draw_highlights(origin, static_cast<lx::HighlightColor>(i));
}

static void draw_output_area()
{
    ImGui::BeginChild("##output", ImVec2(), ImGuiChildFlags_Borders);
    setup_window_channels();

    float wrap_width = ImGui::GetContentRegionAvail().x;
    ImVec2 origin = ImGui::GetCursorScreenPos();
    draw_output_buffer(STATE.output, std::nullopt);

    ImVec2 clip_min = ImGui::GetWindowContentRegionMin();
    ImVec2 clip_max = ImGui::GetWindowContentRegionMax();
    clip_min.x += ImGui::GetWindowPos().x + ImGui::GetScrollX();
    clip_min.y += ImGui::GetWindowPos().y + ImGui::GetScrollY();
    clip_max.x += ImGui::GetWindowPos().x + ImGui::GetScrollX();
    clip_max.y += ImGui::GetWindowPos().y + ImGui::GetScrollY();
    ImGui::GetWindowDrawList()->PushClipRect(clip_min, clip_max);
    draw_all_highlights(wrap_width, origin);

    ImGui::EndChild();
}

static void draw_output_window()
{
    ImGui::Begin(OUTPUT_WINDOW, nullptr, ImGuiWindowFlags_MenuBar);
    setup_window_channels();

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
                dialogs::OUTPUT_FILE.Open();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save Output To File (Ctrl+S)");

            // TODO save input file + output file + highlight metadata into special file, then ability to load that here (sort of a project file *.lxproj)

            ImGui::EndMenu();
        }

        if (ImGui::Button("Highlight"))
        {
            GUI.show_highlight_modal = true;
            ImGui::OpenPopup("Highlights");
        }

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal("Highlights", &GUI.show_highlight_modal))
        {
            draw_highlight_modal();
            ImGui::EndPopup();
        }

        ImGui::EndMenuBar();
    }

    draw_output_area();
    ImGui::End();
}

static void draw_script_window()
{
    ImGui::Begin(SCRIPT_WINDOW, nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("Run"))
            run_script();

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Run Script (F5)");

        ImGui::EndMenuBar();
    }

    draw_input_buffer(STATE.script);
    ImGui::End();
}

static void draw_log_window()
{
    ImGui::Begin(LOG_WINDOW, nullptr, ImGuiWindowFlags_MenuBar);
    ImGui::BeginChild("##output", ImVec2(), ImGuiChildFlags_Borders);
    draw_output_buffer(STATE.log, STATE.success ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
    ImGui::EndChild();
    ImGui::End();
}

static void draw_frame()
{
    draw_dockspace();
    
    draw_input_window();
    draw_script_window();

    draw_output_window();
    draw_log_window();
}

static void handle_shortcuts()
{
    if (ImGui::Shortcut(ImGuiKey_F5, ImGuiInputFlags_RouteGlobal))
        run_script();

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_RouteGlobal))
        dialogs::INPUT_FILE.Open();

    if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_S, ImGuiInputFlags_RouteGlobal))
        dialogs::OUTPUT_FILE.Open();
}

static void glfw_drop_callback(GLFWwindow* window, int count, const char** paths)
{
    for (int i = 0; i < count; i++)
        DROPPED_PATHS.emplace_back(paths[i]);
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Lexico Desktop", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetDropCallback(window, glfw_drop_callback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(main_scale);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = main_scale;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    init_state();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        handle_shortcuts();
        draw_frame();
        process_file_dialogs();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
