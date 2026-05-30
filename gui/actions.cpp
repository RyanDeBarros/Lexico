#include "actions.h"

#include "state.h"

#include <fstream>

void run_script()
{
    auto res = lx::execute({ .script = STATE.script, .input = STATE.input });
    STATE.output = res.output;
    STATE.log = res.log;
    STATE.highlight_map = std::move(res.highlights);
    STATE.success = res.success;
}

// TODO handle file io errors

void open_input_file(const std::filesystem::path& path)
{
    std::ifstream f(path);
    size_t size = std::filesystem::file_size(path);
    STATE.input.resize(size);
    f.read(STATE.input.data(), static_cast<std::streamsize>(size));
}

void save_output_file(const std::filesystem::path& path)
{
    std::ofstream f(path);
    f << STATE.output;
}

void open_script_file(const std::filesystem::path& path)
{
    std::ifstream f(path);
    size_t size = std::filesystem::file_size(path);
    STATE.script.resize(size);
    f.read(STATE.script.data(), static_cast<std::streamsize>(size));
}

void save_script_file(const std::filesystem::path& path)
{
    std::ofstream f(path);
    f << STATE.script;
}

void process_file_dialogs()
{
    dialogs::INPUT_FILE.Display();

    if (dialogs::INPUT_FILE.HasSelected())
    {
        open_input_file(dialogs::INPUT_FILE.GetSelected());
        dialogs::INPUT_FILE.ClearSelected();
    }

    dialogs::OUTPUT_FILE.Display();

    if (dialogs::OUTPUT_FILE.HasSelected())
    {
        save_output_file(dialogs::OUTPUT_FILE.GetSelected());
        dialogs::OUTPUT_FILE.ClearSelected();
    }

    dialogs::SCRIPT_OPEN_FILE.Display();

    if (dialogs::SCRIPT_OPEN_FILE.HasSelected())
    {
        open_script_file(dialogs::SCRIPT_OPEN_FILE.GetSelected());
        dialogs::SCRIPT_OPEN_FILE.ClearSelected();
    }

    dialogs::SCRIPT_SAVE_FILE.Display();

    if (dialogs::SCRIPT_SAVE_FILE.HasSelected())
    {
        save_script_file(dialogs::SCRIPT_SAVE_FILE.GetSelected());
        dialogs::SCRIPT_SAVE_FILE.ClearSelected();
    }
}
