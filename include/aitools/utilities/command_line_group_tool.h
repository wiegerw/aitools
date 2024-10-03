// Copyright: Wieger Wesselink 2021
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file aitools/utilities/command_line_group_tool.h
/// \brief add your file description here.

#ifndef AITOOLS_UTILITIES_COMMAND_LINE_GROUP_TOOL_H
#define AITOOLS_UTILITIES_COMMAND_LINE_GROUP_TOOL_H

#include <cstdlib>
#include <utility>
#include <lyra/lyra.hpp>
#include "aitools/utilities/logger.h"

namespace aitools::utilities {

class sub_command
{
  friend class command_line_group_tool;

  private:
    std::string m_name;
    bool m_show_help = false;
    lyra::command m_command;
    bool m_result;

    virtual void add_options(lyra::command& cmd)
    {
    }

    virtual bool run() = 0;

    void execute(const lyra::group& g)
    {
      if (m_show_help)
      {
        std::cout << g;
        m_result = true;
      }
      else
      {
        m_result = run();
      }
    }

  public:
    sub_command(const std::string& name, const std::string& description = "")
      : m_name(name), m_command(name, [this](const lyra::group& g) { this->execute(g); })
    {
      m_command.add_argument(lyra::help(m_show_help).description(description));
    }

    bool result() const
    {
      return m_result;
    }

    const std::string& name() const
    {
      return m_name;
    }

    const lyra::command& command() const
    {
      return m_command;
    }
};

class command_line_group_tool
{
  protected:
    bool m_show_help = false;
    lyra::cli m_cli{};
    std::vector<std::string> m_names;
    std::string m_description;

  public:
    explicit command_line_group_tool(std::string description = "")
      : m_description(std::move(description))
    {
      // TODO: Unfortunately it is currently not possible to set the log level using a global --verbose option.
      log::aitools_logger::set_reporting_level(log::verbose);
    }

    void add_command(sub_command& cmd)
    {
      m_names.push_back(cmd.name());
      cmd.add_options(cmd.m_command);
      m_cli.add_argument(cmd.command());
    }

    int execute(int argc, const char** argv)
    {
      m_cli.add_argument(lyra::help(m_show_help).description(m_description));
      try
      {
        auto parse_result = m_cli.parse({argc, argv});
        if (m_show_help)
        {
          std::cout
            << "Usage: groups <subcommand> [options] [args]\n"
            << "Type 'groups <subcommand> -h' for help on a specific subcommand.\n"
            << "\n"
            << "Available subcommands:\n";
          for (const auto& name: m_names)
          {
            std::cout << "   " << name << '\n';
          }
          std::cout << std::flush;
          return EXIT_SUCCESS;
        }
        if (!parse_result)
        {
          std::cerr << parse_result.message() << "\n";
        }
      }
      catch (const std::exception& e)
      {
        std::cout << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
      }
      return EXIT_SUCCESS;
    }
};

} // namespace aitools::utilities

#endif // AITOOLS_UTILITIES_COMMAND_LINE_GROUP_TOOL_H
