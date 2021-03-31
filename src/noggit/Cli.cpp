#include <iostream>
#include <optional>
#include "Cli.inl"

using namespace noggit;

template < typename Ty >
concept CharOrCStr = std::is_same_v<Ty, char>
|| std::is_same_v<Ty, char const*>;

template < CharOrCStr Str >
struct ControllerException
{
  explicit constexpr
  ControllerException ( Str arg )
  : arg{arg}
  { }

  Str arg;
};

Cli::Cli
(
  std::size_t argC,
  char const* const* argV
)
: _args(_countArgs(argC, argV))
{
  try
  {
    std::size_t counter{};
    auto const findHandlerOrThrow{
    [ this ]
    < CharOrCStr Str >
    ( Str str )
    -> ControllerHandler
    {
      auto const result{CliAssister::_findControllerInfo(str, _argMapping)};
      return result ? throw ControllerException{str} : result->second;
    }};

    for(std::size_t i{}; i < argC; ++i)
    {
      if(argV[i][0] == '-')
      {
        if(argV[i][1] == '\0')
          throw "-";

        if(argV[i][1] == '-')
        {
          if(argV[i][2] == '\0')
            throw "--";
          else
          {
            _args[i] = findHandlerOrThrow(&argV[i][2]);
            ++counter;
            continue;
          }

          for(std::size_t j{1}; argV[i][j]; ++j, ++counter)
            _args[counter] = findHandlerOrThrow(argV[i][j]);
        }
      }

      _args[counter] = (argV[i]);
      ++counter;
    }
  }
  catch ( ControllerException<char> e )
  {
    std::cerr << "\nE: Unknown control flag: '" << e.arg << '\'';
    throw;
  }
  catch ( ControllerException<char const*> e )
  {
    std::cerr << "\nE: Unknown control statement: '"  << e.arg << '\'';
    throw;
  }
  catch ( char const* str )
  {
    std::cerr << "\nE: Unknown parameter: '" << str << '\'';
    throw;
  }
  catch ( ... )
  {
    std::exit(EXIT_FAILURE);
  }
}

struct ControllerNotSatisfied
{
  std::string_view name;
};

auto Cli::exec ( )
-> void
{
  StateMachine state{};

  try
  {
    for(auto arg{_args.cbegin()}; arg != _args.cend(); )
    {
      if(ControllerHandler const* handler{std::get_if<ControllerHandler>(&*arg)}
      ; handler)
      {
        if(state.curHandler)
          throw ControllerNotSatisfied{*CliAssister::_findControllerInfo
            (state.curHandler, _nameMapping)};

        (*handler)({}) || (state.curHandler = *handler);
        ++arg;
        continue;
      }


    }
  }
  catch ( ControllerNotSatisfied e )
  {
    std::cerr << "\nE: Controller '" << e.name << "' expected an argument.";
    throw;
  }
  catch ( ... )
  {
    std::exit(EXIT_FAILURE);
  }
}

Cli::StateMachine::StateMachine ( )
: curHandler{}, flags{Flags::doContinueExecution}
{ }

constexpr
auto Cli::_countArgs
(
  std::size_t argC,
  char const* const* argV
)
-> std::size_t
{
  std::size_t result{};

  for(std::size_t i{}; i < argC; ++i)
  {
    if(argV[i][0] == '-' && argV[i][1] != '-' && argV[i][1] != '\0')
    {
      for(std::size_t j{1}; argV[i][j]; ++j, ++result);

      continue;
    }

    ++result;
  }

  return result;
}
