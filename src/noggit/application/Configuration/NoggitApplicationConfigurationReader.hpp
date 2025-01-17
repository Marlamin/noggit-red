#ifndef NOGGIT_APPLICATION_CONFIGURATION_READER_HPP
#define NOGGIT_APPLICATION_CONFIGURATION_READER_HPP

#include <noggit/application/Configuration/NoggitApplicationConfiguration.hpp>

class QFile;

namespace Noggit::Application {

    class NoggitApplicationConfigurationReader
    {
    public:
        NoggitApplicationConfiguration ReadConfigurationState(QFile& inputFile);
    };
}
#endif //NOGGIT_APPLICATION_CONFIGURATION_READER_HPP
