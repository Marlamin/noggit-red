// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#pragma once

namespace noggit {
  namespace scripting {
    class scripting_tool;
    class script_context;
    void register_global(script_context * state);
  }
}