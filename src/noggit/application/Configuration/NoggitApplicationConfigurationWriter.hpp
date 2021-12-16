#ifndef NOGGIT_APPLICATION_CONFIGURATION_WRITER_HPP
#define NOGGIT_APPLICATION_CONFIGURATION_WRITER_HPP

#include <QFile>
#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>

namespace Noggit::Application {

    class NoggitApplicationConfigurationWriter
    {
    public:
        void PersistDefaultConfigurationState(QFile& outputFile);
        void PersistConfigurationState(QFile& outputFile, const NoggitApplicationConfiguration& configuration);
    };
}
#endif NOGGIT_APPLICATION_CONFIGURATION_WRITER_HPP