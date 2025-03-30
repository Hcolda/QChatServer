#ifndef COMMANDS_H
#define COMMANDS_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <shared_mutex>

#include <option.hpp>
#include "session.h"

namespace qls
{

class Command
{
public:
    Command() = default;
    virtual ~Command() noexcept = default;

    virtual opt::Option getOption() const = 0;
    virtual void execute(qls::Session&, const opt::Option&) = 0;
};

class HelpCommand;
class CommandManager final
{
public:
    CommandManager();
    ~CommandManager() noexcept = default;

    bool addCommand(std::string_view commandName, const std::shared_ptr<Command>& commandPtr);
    bool removeCommand(std::string_view commandName);
    bool canFindCommand(std::string_view commandName) const;
    std::shared_ptr<Command> getCommand(std::string_view commandName) const;

private:
    struct string_hash
    {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;
    
        std::size_t operator()(const char* str) const        { return hash_type{}(str); }
        std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
        std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
    };

    std::unordered_map<std::string, std::shared_ptr<Command>, string_hash, std::equal_to<>>
                                m_command_map;
    mutable std::shared_mutex   m_command_map_mutex;

    friend class HelpCommand;
};

} // namespace qls

#endif
