#pragma once

class QFile;

namespace Noggit::Application
{
  struct NoggitApplicationConfiguration;

    class NoggitApplicationConfigurationWriter
    {
    public:
        void PersistDefaultConfigurationState(QFile& outputFile);
        void PersistConfigurationState(QFile& outputFile, const NoggitApplicationConfiguration& configuration);
    };
}
