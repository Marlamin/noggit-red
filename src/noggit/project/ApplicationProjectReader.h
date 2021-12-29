#ifndef NOGGIT_APPLICATION_PROJECT_READER_HPP
#define NOGGIT_APPLICATION_PROJECT_READER_HPP

#include <filesystem>

namespace Noggit::Project
{
	class NoggitProject;

	class ApplicationProjectReader
	{
	public:
		ApplicationProjectReader() = default;

		std::optional<NoggitProject> ReadProject(std::filesystem::path const& projectPath);
	};
}

#endif //NOGGIT_APPLICATION_PROJECT_READER_HPP