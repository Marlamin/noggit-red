#include <noggit/application/Configuration/NoggitApplicationConfigurationReader.hpp>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtCore/QSettings>

namespace Noggit::Application {

    NoggitApplicationConfiguration NoggitApplicationConfigurationReader::ReadConfigurationState(QFile& inputFile)
    {
        auto noggitApplicationConfiguration = NoggitApplicationConfiguration();

        // TODO move qsetting stuff to config file
        QSettings settings;
        noggitApplicationConfiguration.modern_features = settings.value("modern_features", false).toBool();

        settings.sync();
        //

        inputFile.open(QIODevice::ReadOnly);

        auto document = QJsonDocument().fromJson(inputFile.readAll());
        auto root = document.object();

        if (root.contains("Noggit") && root["Noggit"].isObject())
        {
            auto noggitConfiguration = root["Noggit"].toObject();
            if (noggitConfiguration.contains("ApplicationProjectPath"))
                noggitApplicationConfiguration.ApplicationProjectPath = noggitConfiguration["ApplicationProjectPath"].toString().toStdString();
            if (noggitConfiguration.contains("ApplicationThemePath"))
                noggitApplicationConfiguration.ApplicationThemePath = noggitConfiguration["ApplicationThemePath"].toString().toStdString();
            if (noggitConfiguration.contains("ApplicationDatabaseDefinitionsPath"))
                noggitApplicationConfiguration.ApplicationDatabaseDefinitionsPath = noggitConfiguration["ApplicationDatabaseDefinitionsPath"].toString().toStdString();
            if (noggitConfiguration.contains("ApplicationNoggitDefinitionsPath"))
            {
              noggitApplicationConfiguration.ApplicationNoggitDefinitionsPath = noggitConfiguration["ApplicationNoggitDefinitionsPath"].toString().toStdString();
            }
            else

            if (noggitConfiguration.contains("ApplicationListFilePath"))
                noggitApplicationConfiguration.ApplicationListFilePath = noggitConfiguration["ApplicationListFilePath"].toString().toStdString();

            if (noggitConfiguration.contains("GraphicsConfiguration") && noggitConfiguration["GraphicsConfiguration"].isObject())
            {
                auto noggitGraphicsConfiguration = noggitConfiguration["GraphicsConfiguration"].toObject();
                noggitApplicationConfiguration.GraphicsConfiguration = NoggitApplicationGraphicsConfiguration();

                if (noggitGraphicsConfiguration.contains("SwapChainDepth"))
                {
                    auto swapChainName = noggitGraphicsConfiguration["SwapChainDepth"].toString().toStdString();

                    auto swapChainDepth = QSurfaceFormat::DefaultSwapBehavior;
                    if (swapChainName == std::string("DEFAULT"))
                        swapChainDepth = QSurfaceFormat::DefaultSwapBehavior;
                    if (swapChainName == std::string("SINGLE"))
                        swapChainDepth = QSurfaceFormat::SingleBuffer;
                    if (swapChainName == std::string("DOUBLE"))
                        swapChainDepth = QSurfaceFormat::DoubleBuffer;
                    if (swapChainName == std::string("TRIPLE"))
                        swapChainDepth = QSurfaceFormat::TripleBuffer;

                    noggitApplicationConfiguration.GraphicsConfiguration.SwapChainDepth = swapChainDepth;
                }

                if (noggitGraphicsConfiguration.contains("SamplesCount"))
                    noggitApplicationConfiguration.GraphicsConfiguration.SamplesCount = noggitGraphicsConfiguration["SamplesCount"].toInt();
                if (noggitGraphicsConfiguration.contains("SwapChainInternal"))
                    noggitApplicationConfiguration.GraphicsConfiguration.SwapChainInternal = noggitGraphicsConfiguration["SwapChainInternal"].toInt();
                if (noggitGraphicsConfiguration.contains("DepthBufferSize"))
                    noggitApplicationConfiguration.GraphicsConfiguration.DepthBufferSize = noggitGraphicsConfiguration["DepthBufferSize"].toInt();
            }

            if (noggitConfiguration.contains("LoggingConfiguration") && noggitConfiguration["LoggingConfiguration"].isObject())
            {
                auto noggitLoggingConfiguration = noggitConfiguration["LoggingConfiguration"].toObject();
                noggitApplicationConfiguration.LoggingConfiguration = NoggitApplicationLoggingConfiguration();

                if (noggitLoggingConfiguration.contains("ApplicationLoggingPath"))
                    noggitApplicationConfiguration.LoggingConfiguration.ApplicationLoggingPath = noggitLoggingConfiguration["ApplicationLoggingPath"].toString().toStdString();
                if (noggitLoggingConfiguration.contains("EnableConsoleLogging"))
                    noggitApplicationConfiguration.LoggingConfiguration.EnableConsoleLogging = noggitLoggingConfiguration["EnableConsoleLogging"].toBool();
            }
        }

        return noggitApplicationConfiguration;
    }
}