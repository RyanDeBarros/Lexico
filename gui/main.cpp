#include <lang.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <imgui_internal.h>

#include <GLFW/glfw3.h>

#include <iostream>

static const char* DOCKSPACE_UID = "MyDockSpace";
static const char* INPUT_WINDOW = "Input";
static const char* OUTPUT_WINDOW = "Output";
static const char* SCRIPT_WINDOW = "Script";
static const char* LOG_WINDOW = "Log";

struct EditorState
{
    std::string input;
    std::string output;
    std::string script;
    std::string log;
};

static EditorState STATE{};

static void run_script()
{
    auto res = lx::execute({ .script = STATE.script, .input = STATE.input });
    STATE.output = res.output;
    STATE.log = res.log;
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

static void draw_output_buffer(std::string& buffer)
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
    ImGui::InputTextMultiline(
        "##output",
        buffer.data(),
        buffer.size() + 1,
        ImVec2(-FLT_MIN, -FLT_MIN),
        ImGuiInputTextFlags_WordWrap | ImGuiInputTextFlags_ReadOnly
    );
    ImGui::PopStyleColor(3);
}

static void draw_input_window()
{
    ImGui::Begin(INPUT_WINDOW);

    static int focus_frames = 2;
    if (focus_frames > 0)
    {
        --focus_frames;
        ImGui::SetWindowFocus();
        ImGui::SetKeyboardFocusHere();
    }

    draw_input_buffer(STATE.input);
    ImGui::End();
}

static void draw_output_window()
{
    ImGui::Begin(OUTPUT_WINDOW);
    draw_output_buffer(STATE.output);
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
    ImGui::Begin(LOG_WINDOW);
    draw_output_buffer(STATE.log);
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
    if (ImGui::IsKeyPressed(ImGuiKey_F5))
        run_script();
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(main_scale);

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = main_scale;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        handle_shortcuts();
        draw_frame();

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
