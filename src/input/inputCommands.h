#ifndef INPUT_COMMANDS_H
#define INPUT_COMMANDS_H

#include <string>

#include "option.hpp"

namespace qls
{
    class Command;
    struct CommandInfo
    {
        opt::Option option;
        std::string description;
    };

    class Command
    {
    public:
        Command() = default;
        virtual ~Command() = default;
        virtual void setArguments(const opt::Option& options) {}
        virtual bool execute() { return true; }
        virtual CommandInfo registerCommand() { return {}; }
    };

    class stop_command: public Command
    {
    public:
        stop_command() = default;
        virtual bool execute();
    };

    class show_user_command: public Command
    {
    public:
        show_user_command() = default;
        virtual bool execute();
    };
}

#endif // !INPUT_COMMANDS_H