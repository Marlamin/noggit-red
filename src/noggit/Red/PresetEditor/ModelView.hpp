#ifndef NOGGIT_MODELVIEW_HPP
#define NOGGIT_MODELVIEW_HPP

#include <noggit/Red/AssetBrowser/ModelView.hpp>
#include <noggit/World.h>


namespace noggit
{
  namespace Red::PresetEditor
  {
    class ModelViewer : public Red::AssetBrowser::ModelViewer
    {
    public:
        explicit ModelViewer(QWidget* parent = nullptr);

    private:
        std::unique_ptr<World> _world;

        void paintGL() override;

    };


  }
}


#endif //NOGGIT_MODELVIEW_HPP
