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
				//uto item(new QListWidgetItem(QString::number(e.mapID) + " - " + QString::fromUtf8(e.name.c_str()),type_to_table[e.areaType]));
				//tem->setData(Qt::UserRole, QVariant(e.mapID));


				stringList << QString::fromStdString(dirEntry.path().filename().generic_string());
			}

			return stringList;
		}
	};
}