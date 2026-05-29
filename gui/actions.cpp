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

void open_input_file(const std::filesystem::path& path)
{
    std::ifstream f;
    f.open(path);

    size_t size = std::filesystem::file_size(path);
    STATE.input.resize(size);
    f.read(STATE.input.data(), static_cast<std::streamsize>(size));
}

static void process_input_file()
{
    dialogs::INPUT_FILE.Display();

    if (dialogs::INPUT_FILE.HasSelected())
    {
        open_input_file(dialogs::INPUT_FILE.GetSelected());
        dialogs::INPUT_FILE.ClearSelected();
    }
}

void process_file_dialogs()
{
    process_input_file();
}

void close_file_dialogs()
{
    dialogs::INPUT_FILE.Close();
}
