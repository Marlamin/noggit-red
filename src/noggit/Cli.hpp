#ifndef NOGGIT_SRC_NOGGIT_CLI_HPP
#define NOGGIT_SRC_NOGGIT_CLI_HPP

#include <array>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace noggit
{
  using namespace std::literals;

  class Cli
  {
    public:
      explicit
      Cli
      (
        std::size_t argC,
        char const* const* argV
      );
      auto exec ( )
      -> void;
    private:
      static constexpr
      auto _countArgs
      (
        std::size_t argC,
        char const* const* argV
      )
      -> std::size_t;

      typedef std::vector<std::variant<std::size_t, std::string>> ArgContainer;

      struct ParamInfo
      {
        typedef auto(HandlerSignature)
        (
          ArgContainer::const_iterator begin,
          ArgContainer::const_iterator end
        )
        -> bool;

        explicit constexpr
        ParamInfo
        (
          std::initializer_list<std::string_view> aliases,
          std::string_view name,
          HandlerSignature* handler
        );

        std::vector<std::string_view> aliases;
        std::string_view name;
        HandlerSignature* handler;
      };

      struct StateMachine
      {
        struct Flags
        {
          enum : std::size_t
          {
            IsControllerSatisfied = 0x1,
            doContinueExecution = 0x2
          };
        };

        StateMachine ( );

        std::size_t curParamIdx;
        std::size_t flags;
      };

      static constexpr
      std::array paramInfos
      {
        ParamInfo{{"r", "recovery"}, "Recovery", nullptr}
      };
      ArgContainer _args;
  };

  template < typename Ty >
  concept ControllerInfo = std::is_same_v<Ty, std::string_view>
  || std::is_same_v<Ty, Cli::ControllerHandler>;

  class CliAssister
  {
    private:
      friend class Cli;

      template
      <
        ControllerInfo Key,
        ControllerInfo Value,
        std::size_t n
      >
      requires ( !std::is_same_v<Key, Value> )
      static constexpr
      auto _findControllerInfo
      (
        Key key,
        std::array<std::pair<Key, Value>, n> const& info
      )
      -> std::optional<Value>;
  };
}

#endif //NOGGIT_SRC_NOGGIT_CLI_HPP
