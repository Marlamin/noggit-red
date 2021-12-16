// This file is part of Noggit3, licensed under GNU General Public License (version 3).
#ifndef NOGGIT_APPLICATION_CONFIGURATION_HPP
#define NOGGIT_APPLICATION_CONFIGURATION_HPP

#include <string>
#include <QSurfaceFormat>

namespace Noggit::Application {

    struct NoggitApplicationLoggingConfiguration
    {
        std::string ApplicationLoggingPath;
        bool EnableConsoleLogging;
    };

    struct NoggitApplicationGraphicsConfiguration
    {
        QSurfaceFormat::SwapBehavior SwapChainDepth;
        char SwapChainInternal;
        char DepthBufferSize;
        char SamplesCount;
    };

    struct NoggitApplicationConfiguration
    {
        std::string ApplicationProjectPath;
        std::string ApplicationThemePath;
        NoggitApplicationGraphicsConfiguration GraphicsConfiguration;
        NoggitApplicationLoggingConfiguration LoggingConfiguration;
    };
}

#endif NOGGIT_APPLICATION_CONFIGURATION_HPP