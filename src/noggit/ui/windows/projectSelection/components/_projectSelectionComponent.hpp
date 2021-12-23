#include <QStringList>

namespace Noggit::Ui::Component
{
	class ExistingProjectEnumerationComponent
	{
	public:
		QStringList EnumerateExistingProjects(std::filesystem::path projectDirectory)
		{
			auto stringList = QStringList();
			for (const auto& dirEntry : std::filesystem::directory_iterator(projectDirectory))
			{
				stringList << QString::fromStdString(dirEntry.path().filename().generic_string());
			}

			return stringList;
		}
	};
}