#include <noggit/application/NoggitApplication.hpp>

#include "noggit/project/_Project.h"

namespace noggit::application {

    Noggit::Noggit(int argc, char* argv[])
	: fullscreen(false),
	doAntiAliasing(true)
    {
        InitLogging();
        assert(argc >= 1); (void)argc;
        Noggit::initPath(argv);

        Log << "Noggit Studio - " << STRPRODUCTVER << std::endl;


        QSettings settings;
        doAntiAliasing = settings.value("antialiasing", false).toBool();
        fullscreen = settings.value("fullscreen", false).toBool();


        srand(::time(nullptr));
        QDir path(settings.value("project/game_path").toString());

        wowpath = path.absolutePath().toStdString();

        Log << "Game path: " << wowpath << std::endl;

        project_path = settings.value("project/path", path.absolutePath()).toString().toStdString();
        settings.setValue("project/path", QString::fromStdString(project_path));

        Log << "Project path: " << project_path << std::endl;

        settings.setValue("project/game_path", path.absolutePath());
        settings.setValue("project/path", QString::fromStdString(project_path));

        if (!QGLFormat::hasOpenGL())
        {
            throw std::runtime_error("Your system does not support OpenGL. Sorry, this application can't run without it.");
        }

        QSurfaceFormat format;

        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setVersion(4, 1);
        format.setProfile(QSurfaceFormat::CoreProfile);
        //format.setOption(QSurfaceFormat::ResetNotification, true);
        format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
        format.setSwapInterval(0);
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setDepthBufferSize(16);
        format.setSamples(0);

        if (doAntiAliasing)
        {
            format.setSamples(4);
        }

        QSurfaceFormat::setDefaultFormat(format);

        QOpenGLContext context;
        context.create();
        QOffscreenSurface surface;
        surface.create();
        context.makeCurrent(&surface);

        opengl::context::scoped_setter const _(::gl, &context);

        LogDebug << "GL: Version: " << gl.getString(GL_VERSION) << std::endl;
        LogDebug << "GL: Vendor: " << gl.getString(GL_VENDOR) << std::endl;
        LogDebug << "GL: Renderer: " << gl.getString(GL_RENDERER) << std::endl;
    }

    void Noggit::initPath(char* argv[])
    {
        try
        {
            std::filesystem::path startupPath(argv[0]);
            startupPath.remove_filename();

            if (startupPath.is_relative())
            {
                std::filesystem::current_path(std::filesystem::current_path() / startupPath);
            }
            else
            {
                std::filesystem::current_path(startupPath);
            }
        }
        catch (const std::filesystem::filesystem_error& ex)
        {
            LogError << ex.what() << std::endl;
        }
    }

    void Noggit::start()
    {
        _client_data = std::make_unique<BlizzardArchive::ClientData>(wowpath.string(), BlizzardArchive::ClientVersion::WOTLK, BlizzardArchive::Locale::AUTO, project_path);

        OpenDBs();

        main_window = std::make_unique<noggit::ui::main_window>();

        if (fullscreen)
        {
            main_window->showFullScreen();
        }
        else
        {
            main_window->showMaximized();
        }
    }
}


/* I wonder if you would correctly guess the reason of this being here... */
template < typename Char >
requires (std::is_same_v<Char, wchar_t> || std::is_same_v<Char, char>)
auto convert
(
    Char const* src,
    std::string* dst
)
-> char const*
{
    if constexpr (std::is_same_v<Char, char>)
    {
        *dst = src;
        return dst->c_str();
    }

    std::string mbc(MB_CUR_MAX, '\0');
    dst->clear();

    while (*src)
    {
        std::wctomb(mbc.data(), *src++);
        dst->append(mbc.c_str());
    }

    return dst->c_str();
}