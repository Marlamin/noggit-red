#include <noggit/application/Configuration/NoggitApplicationConfigurationWriter.hpp>
#include <QJsonObject>
#include <QJsonDocument>

namespace Noggit::Application {

    void NoggitApplicationConfigurationWriter::PersistDefaultConfigurationState(QFile& outputFile)
    {
        auto noggitApplicationConfiguration = NoggitApplicationConfiguration();
        noggitApplicationConfiguration.ApplicationProjectPath = std::string("projects");
        noggitApplicationConfiguration.ApplicationThemePath = std::string("themes");
        noggitApplicationConfiguration.ApplicationDatabaseDefinitionsPath = std::string("definitions");
        noggitApplicationConfiguration.ApplicationNoggitDefinitionsPath = std::string("noggit-definitions");
        noggitApplicationConfiguration.ApplicationListFilePath = std::string("listfile.csv");

        noggitApplicationConfiguration.GraphicsConfiguration = NoggitApplicationGraphicsConfiguration();
        noggitApplicationConfiguration.GraphicsConfiguration.SwapChainDepth = QSurfaceFormat::TripleBuffer;
        noggitApplicationConfiguration.GraphicsConfiguration.DepthBufferSize = 24;
        noggitApplicationConfiguration.GraphicsConfiguration.SwapChainInternal = 0;
        noggitApplicationConfiguration.GraphicsConfiguration.SamplesCount = 0;

        noggitApplicationConfiguration.LoggingConfiguration = NoggitApplicationLoggingConfiguration();
        noggitApplicationConfiguration.LoggingConfiguration.EnableConsoleLogging = false;
        noggitApplicationConfiguration.LoggingConfiguration.ApplicationLoggingPath = "log.txt";

        PersistConfigurationState(outputFile, noggitApplicationConfiguration);
    }

    void NoggitApplicationConfigurationWriter::PersistConfigurationState(QFile& outputFile, const NoggitApplicationConfiguration& configuration)
    {
        outputFile.open(QIODevice::WriteOnly);

        auto document = QJsonDocument();
        auto root = QJsonObject();
        auto rootConfiguration = QJsonObject();

        auto swapChainDepth = std::string();
        if (configuration.GraphicsConfiguration.SwapChainDepth == QSurfaceFormat::DefaultSwapBehavior)
            swapChainDepth = std::string("DEFAULT");
        if (configuration.GraphicsConfiguration.SwapChainDepth == QSurfaceFormat::SingleBuffer)
            swapChainDepth = std::string("SINGLE");
        if (configuration.GraphicsConfiguration.SwapChainDepth == QSurfaceFormat::DoubleBuffer)
            swapChainDepth = std::string("DOUBLE");
        if (configuration.GraphicsConfiguration.SwapChainDepth == QSurfaceFormat::TripleBuffer)
            swapChainDepth = std::string("TRIPLE");

        auto graphicsConfiguration = QJsonObject();
        graphicsConfiguration.insert("SwapChainDepth", swapChainDepth.c_str());
        graphicsConfiguration.insert("DepthBufferSize", configuration.GraphicsConfiguration.DepthBufferSize);
        graphicsConfiguration.insert("SwapChainInternal", configuration.GraphicsConfiguration.SwapChainInternal);
        graphicsConfiguration.insert("SamplesCount", configuration.GraphicsConfiguration.SamplesCount);

        auto loggingConfiguration = QJsonObject();
        loggingConfiguration.insert("EnableConsoleLogging", configuration.LoggingConfiguration.EnableConsoleLogging);
        loggingConfiguration.insert("ApplicationLoggingPath", configuration.LoggingConfiguration.ApplicationLoggingPath.c_str());

        rootConfiguration.insert("ApplicationProjectPath", configuration.ApplicationProjectPath.c_str());
        rootConfiguration.insert("ApplicationThemePath", configuration.ApplicationThemePath.c_str());
        rootConfiguration.insert("ApplicationDatabaseDefinitionsPath", configuration.ApplicationDatabaseDefinitionsPath.c_str());
        rootConfiguration.insert("ApplicationNoggitDefinitionsPath", configuration.ApplicationNoggitDefinitionsPath.c_str());
        rootConfiguration.insert("ApplicationListFilePath", configuration.ApplicationListFilePath.c_str());
        rootConfiguration.insert("GraphicsConfiguration", graphicsConfiguration);
        rootConfiguration.insert("LoggingConfiguration", loggingConfiguration);

        root.insert("Noggit", rootConfiguration);
        document.setObject(root);

        outputFile.write(document.toJson(QJsonDocument::Indented));
    };
}