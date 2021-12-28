#ifndef NOGGIT_COMPONENT_BUILD_MAP_LIST_HPP
#define NOGGIT_COMPONENT_BUILD_MAP_LIST_HPP

namespace Noggit::Ui::Windows
{
  class NoggitWindow;
}

namespace Noggit::Ui::Component
{
    class BuildMapListComponent
    {
         friend class Noggit::Ui::Windows::NoggitWindow;
    public:
        void BuildMapList(Noggit::Ui::Windows::NoggitWindow* parent);
    };
}
#endif //NOGGIT_COMPONENT_BUILD_MAP_LIST_HPP