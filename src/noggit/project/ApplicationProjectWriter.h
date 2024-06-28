#ifndef NOGGIT_APPLICATION_PROJECT_WRITER_HPP
#define NOGGIT_APPLICATION_PROJECT_WRITER_HPP

#include <filesystem>

namespace Noggit::Project
{
	class NoggitProject;

	class ApplicationProjectWriter
	{
	public:
		ApplicationProjectWriter() = default;

		void saveProject(NoggitProject* project, std::filesystem::path const& project_path);

		void savePalettes(NoggitProject* project, std::filesystem::path const& project_path);

		void saveObjectSelectionGroups(NoggitProject* project, std::filesystem::path const& project_path);
	};
}

#endif //NOGGIT_APPLICATION_PROJECT_WRITER_HPP