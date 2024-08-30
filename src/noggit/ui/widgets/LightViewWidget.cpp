#include "LightViewWidget.h"

LightViewWidget::LightViewWidget(QWidget* parent)
	: QWidget(parent)
{

}

QColor LightViewWidget::InterpolateColor(QColor& a, QColor& b, float t)
{
	QColor Color;

	Color.setRgb(
		ClampColor(a.red() * (1 - t) + b.red() * t),
		ClampColor(a.green() * (1 - t) + b.green() * t),
		ClampColor(a.blue() * (1 - t) + b.blue() * t)
	);

	return Color;
}

QImage LightViewWidget::FillImagePart(QImage Image, int X, QColor Color)
{
	if (X == Image.width() || X < 0)
		return Image;

	for (int Y = 0; Y < Image.height(); ++Y)
		Image.setPixelColor(QPoint(X, Y), Color);

	return Image;
}

int LightViewWidget::ClampColor(int Value)
{
	if (Value < 0) Value = 0;
	if (Value > 255) Value = 255;

	return Value;
}

QColor LightViewWidget::GetColorFromStyleSheet()
{
	const QString SearchPattern = ".*qmainwindow{background-color:(#[A-Fa-f0-9]{3-6});}.*";
	QString StyleSheet = qApp->styleSheet().simplified().toLower().remove(R"( )");

	QRegularExpression Regex = QRegularExpression(SearchPattern);
	auto Match = Regex.match(StyleSheet);
	QStringList HexColor = Match.capturedTexts();

	if (HexColor.size() > 0)
		return QColor(HexColor.at(0));

	return Qt::transparent;
}

void LightViewWidget::SortSkyColorVector(std::vector<SkyColor>& Vector)
{
	std::sort(Vector.begin(), Vector.end(), [](const auto& a, const auto& b)
		{
			return (a.time < b.time);
		});
}

// use for access to the editor and for view
LightViewPreview::LightViewPreview(QString LightName, QSize Size, QWidget* parent)
	: DynamicMouseWidget(parent), MainLayout(new QVBoxLayout(this)), Name(new QLabel(this)),
	Preview(new LightViewPixmap(Size, this))
{
	// Preview->ShowLines(true);

	Name->setText(LightName);

	MainLayout->addWidget(Name);
	MainLayout->addWidget(Preview);
	MainLayout->setSpacing(0);
	MainLayout->setMargin(0);
}

LightViewPreview::LightViewPreview(QSize Size, QWidget* parent)
	: DynamicMouseWidget(parent), MainLayout(new QVBoxLayout(this)), Name(new QLabel(this)),
	Preview(new LightViewPixmap(Size, this))
{
	MainLayout->addWidget(Preview);
	MainLayout->setSpacing(0);
	MainLayout->setMargin(0);
}

void LightViewPreview::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		emit LeftClicked();
}

void LightViewPreview::ShowLines(bool Show)
{
	Preview->ShowLines(Show);
}

void LightViewPreview::SetPreview(std::vector<SkyColor>& data)
{
	Preview->SetPreview(data);
}

void LightViewPreview::UpdatePixmap(const QPixmap Updated)
{
	Preview->setPixmap(Updated.scaled(Preview->pixmap(Qt::ReturnByValueConstant{}).size(), Qt::IgnoreAspectRatio));
}

LightViewPixmap::LightViewPixmap(QSize Size, QWidget* parent)
	: QLabel(parent), PixmapSize(Size)
{
	Image = QImage(PixmapSize, QImage::Format_ARGB32);
	Base = QPixmap(PixmapSize);
}

void LightViewPixmap::mousePressEvent(QMouseEvent* event)
{
	if (!bEvent)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	if (event->modifiers() & Qt::CTRL && event->button() == Qt::LeftButton)
	{
		int Time = event->pos().x() * MAX_TIME_VALUE / width();
		emit CreateValue(Time);
	}
}

void LightViewPixmap::ShowLines(bool Show)
{
	bShowLines = Show;
	SetPreview(Data);
}

void LightViewPixmap::SetPreview(std::vector<SkyColor>& data, int Index)
{
	Data = data;
	if (Data.size() == 0)
		return;

	for (int i = 0; i < Data.size(); ++i)
		Data[i].color *= 255.f;

	LightViewWidget::SortSkyColorVector(Data);

	if (Data.size() == 1)
	{
		Image.fill(qRgba(Data[0].color.r, Data[0].color.g, Data[0].color.b, 255));

		QPainter Painter(&Base);
		Painter.drawImage(QPoint(0, 0), Image);
		emit UpdatePixmap(Base);

		if (DrawLines())
			return;

		setPixmap(Base);
		return;
	}

	Image.fill(qRgba(0, 0, 0, 0));

	QPainter Painter(&Base);
	for (int i = 0; i < Data.size(); ++i)
	{
		QColor PreColor = nullptr;
		QColor CurrentColor(Data[i].color.r, Data[i].color.g, Data[i].color.b);
		QColor NextColor = nullptr;

		if (i == 0)
		{
			glm::vec3 Color = Data[Data.size() - 1].color;
			PreColor = QColor(Color.r, Color.g, Color.b);

			int LastTimeOffset = Data[Data.size() - 1].time - MAX_TIME_VALUE;
			int DistanceWithFirstTime = Data[i].time - LastTimeOffset;

			for (int time = 0; time <= Data[i].time; ++time)
			{
				float t = 1.f;
				if (DistanceWithFirstTime != 0)
					t = (float(time) + float(abs(LastTimeOffset))) / float(DistanceWithFirstTime);

				QColor Calculated = LightViewWidget::InterpolateColor(PreColor, CurrentColor, t);

				int X = time * Image.width() / MAX_TIME_VALUE;
				Image = LightViewWidget::FillImagePart(Image, X, Calculated);
			}
		}
		else if (i == Data.size() - 1)
		{
			glm::vec3 Color = Data[i - 1].color;
			PreColor = QColor(Color.r, Color.g, Color.b);

			Color = Data[0].color;
			NextColor = QColor(Color.r, Color.g, Color.b);

			int FirstTimeOffset = Data[0].time + MAX_TIME_VALUE;
			int DistanceWithFirstTime = FirstTimeOffset - Data[i].time;

			for (int time = Data[i].time; time <= MAX_TIME_VALUE; ++time)
			{
				float t = 1.f;
				if (DistanceWithFirstTime != 0)
					t = (float(time) - float(Data[i].time)) / float(DistanceWithFirstTime);

				QColor Calculated = LightViewWidget::InterpolateColor(CurrentColor, NextColor, t);

				int X = time * Image.width() / MAX_TIME_VALUE;
				Image = LightViewWidget::FillImagePart(Image, X, Calculated);
			}
			break;
		}

		glm::vec3 Color = Data[i + 1].color;
		NextColor = QColor(Color.r, Color.g, Color.b);

		for (int time = Data[i].time; time <= Data[i + 1].time; ++time)
		{
			float t = 1.f;
			if (Data[i + 1].time != Data[i].time)
				t = (float(time) - float(Data[i].time)) / (float(Data[i + 1].time) - float(Data[i].time));

			QColor Calculated = LightViewWidget::InterpolateColor(CurrentColor, NextColor, t);

			int X = time * Image.width() / MAX_TIME_VALUE;
			Image = LightViewWidget::FillImagePart(Image, X, Calculated);
		}
	}

	Painter.drawImage(QPoint(0, 0), Image);
	emit UpdatePixmap(Base);

	if (DrawLines())
		return;

	setPixmap(Base);
}

bool LightViewPixmap::DrawLines()
{
	if (!bShowLines) return bShowLines;

	QImage Final = Image;
	QPixmap ResultPixmap = QPixmap(Final.size());

	QPainter Painter(&ResultPixmap);

	const bool blend_lines = false;
	for (int i = 0; i < Data.size(); ++i)
	{
		for (int y = 0; y < LIGHT_VIEW_PREVIEW_LINE_SIZE; ++y)
		{
			int X = Data[i].time * Final.width() / MAX_TIME_VALUE - LIGHT_VIEW_PREVIEW_LINE_SIZE / 2 + y;
			if (!blend_lines)
			{
				Final = LightViewWidget::FillImagePart(Final, X, QColor(LIGHT_VIEW_PREVIEW_LINE_COLOR));
				continue;
			}

			// blend the line instead, it just looks better with red.
			if (X == Image.width() || X < 0)
				continue;

			QColor new_color(255, 0, 0, 150);

			for (int Y = 0; Y < Image.height(); ++Y)
			{
				QColor existingColor = Image.pixelColor(X, Y);
				QColor blendedColor;

				float alpha = new_color.alphaF();

				// Blend the colors
				int red = static_cast<int>(new_color.red() * alpha + existingColor.red() * (1.0 - alpha));
				int green = static_cast<int>(new_color.green() * alpha + existingColor.green() * (1.0 - alpha));
				int blue = static_cast<int>(new_color.blue() * alpha + existingColor.blue() * (1.0 - alpha));

				blendedColor.setRgb(red, green, blue);
				Image.setPixelColor(QPoint(X, Y), blendedColor);
			}
			Final = Image;
		}
	}

	Painter.drawImage(QPoint(0, 0), Final);
	setPixmap(ResultPixmap);
	return bShowLines;
}

LightViewEditor::LightViewEditor(MapView* Map, SkyParam* Sky, SkyColorNames eSkyColor, QWidget* parent)
	: QWidget(parent), _World(Map->getWorld()), _Map(Map), _Sky(Sky), _eSkyColorIndex(eSkyColor)
{
	setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

	MainLayout = new QVBoxLayout(this);
	MainLayout->setContentsMargins(0, 0, 0, 0);
	MainLayout->setSpacing(0);

	_ColorName = SkyColorNameByIndex.at(_eSkyColorIndex);
	setWindowTitle(QString(tr("Light Color Editor - %1 Color")).arg(_ColorName.c_str()));

	Indicator = new LightViewDayIndicator(QSize(LIGHT_VIEW_PREVIEW_WIDTH * 2 + 30, LIGHT_VIEW_PREVIEW_HEIGHT), this);

	QHBoxLayout* PreviewLayout = new QHBoxLayout(this);

	Preview = new LightViewPixmap(QSize(LIGHT_VIEW_PREVIEW_WIDTH * 2, LIGHT_VIEW_PREVIEW_HEIGHT * 2), this);
	Preview->ShowLines(true);
	Preview->EnableEvent();
	Preview->SetPreview(_Sky->colorRows[_eSkyColorIndex]);

	Arrow = new LightViewArrow(QSize(LIGHT_VIEW_PREVIEW_WIDTH * 2 + 30, LIGHT_VIEW_PREVIEW_HEIGHT * 2), this);
	Arrow->UpdateData(_Sky->colorRows[_eSkyColorIndex]);

	connect(Preview, &LightViewPixmap::UpdatePixmap, this, [=](const QPixmap Updated)
		{
			emit UpdatePixmap(Updated);
		});

	connect(Preview, SIGNAL(CreateValue(int)), this, SLOT(CreateValue(int)));

	connect(Arrow, &LightViewArrow::ChangeTime, this, [=](int Index, int Time)
		{
			if (Index > 0 && Index <= _Sky->colorRows[_eSkyColorIndex].size() - 1)
			{
				if (Time < _Sky->colorRows[_eSkyColorIndex][Index - 1].time)
				{
					Arrow->CurrentHover--;
					Arrow->CurrentSelected--;
				}
			}

			if (Index >= 0 && Index < _Sky->colorRows[_eSkyColorIndex].size() - 1)
			{
				if (Time > _Sky->colorRows[_eSkyColorIndex][Index + 1].time)
				{
					Arrow->CurrentHover++;
					Arrow->CurrentSelected++;
				}
			}

			_Sky->colorRows[_eSkyColorIndex][Index].time = Time;

			LightViewWidget::SortSkyColorVector(_Sky->colorRows[_eSkyColorIndex]);
			UpdateSkyColorRowsValue();
		});

	connect(Arrow, &LightViewArrow::ReleaseArrow, this, [=]()
		{ LightViewWidget::SortSkyColorVector(_Sky->colorRows[_eSkyColorIndex]); });

	connect(Arrow, SIGNAL(SelectValue(int)), this, SLOT(UpdateWidgetEdition(int)));
	connect(Arrow, SIGNAL(DeleteValue(int)), this, SLOT(DeleteValue(int)));

	PreviewLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	PreviewLayout->addWidget(Preview);
	PreviewLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

	QFrame* Line = new QFrame();
	Line->setFrameShape(QFrame::HLine);
	Line->setFrameShadow(QFrame::Sunken);

	// Other widget for display
	QHBoxLayout* SecLayout = new QHBoxLayout(this);
	{
		ColorPicker = new color_widgets::ColorSelector(this);
		ColorPicker->setDisplayMode(color_widgets::ColorSelector::NoAlpha);
		ColorPicker->setColor(Qt::gray);

		connect(ColorPicker, &color_widgets::ColorSelector::colorChanged, this, [=](QColor Changed)
			{
				if (!ValidCurrentIndex())
					return;

				_Sky->colorRows[_eSkyColorIndex][CurrentIndex].color = glm::vec3(Changed.redF(), Changed.greenF(), Changed.blueF());
				UpdateSkyColorRowsValue();
			});

		TimeSelectorHour = new QSpinBox(this);
		TimeSelectorHour->setMinimum(0);
		TimeSelectorHour->setMaximum(23);

		TimeSelectorMin = new QSpinBox(this);
		TimeSelectorMin->setMinimum(0);
		TimeSelectorMin->setMaximum(59);

		connect(TimeSelectorHour, &QSpinBox::textChanged, [=](QString)
			{
				if (!ValidCurrentIndex())
					return;

				int Time = ((TimeSelectorHour->value() * 60) + TimeSelectorMin->value()) * MAX_TIME_VALUE / (23 * 60 + 59);
				_Sky->colorRows[_eSkyColorIndex][CurrentIndex].time = Time;
				UpdateSkyColorRowsValue();
			});

		connect(TimeSelectorMin, &QSpinBox::textChanged, [=](QString)
			{
				if (!ValidCurrentIndex())
					return;

				int Time = ((TimeSelectorHour->value() * 60) + TimeSelectorMin->value()) * MAX_TIME_VALUE / (23 * 60 + 59);
				_Sky->colorRows[_eSkyColorIndex][CurrentIndex].time = Time;
				UpdateSkyColorRowsValue();
			});

		TimeSelectorHour->setEnabled(false);
		TimeSelectorMin->setEnabled(false);
		ColorPicker->setEnabled(false);

		SecLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		SecLayout->addWidget(new QLabel(tr("Color"), this));
		SecLayout->addWidget(ColorPicker);
		SecLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		SecLayout->addWidget(new QLabel(tr("Hours"), this));
		SecLayout->addWidget(TimeSelectorHour);
		SecLayout->addWidget(new QLabel(tr("Minutes"), this));
		SecLayout->addWidget(TimeSelectorMin);
		SecLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	}

	QHBoxLayout* ThirdLayout = new QHBoxLayout(this);
	{
		Remove = new QPushButton(tr("Delete Selected"), this);
		connect(Remove, &QPushButton::clicked, this, [=]()
			{
				DeleteValue(CurrentIndex);
			});

		ThirdLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		ThirdLayout->addWidget(Remove);
		ThirdLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

		Remove->setEnabled(false);
	}

	QHBoxLayout* PreLayout = new QHBoxLayout(this);
	{
		Add = new QPushButton(tr("Insert Value"), this);

		AddHour = new QSpinBox(this);
		AddHour->setMinimum(0);
		AddHour->setMaximum(23);

		AddMin = new QSpinBox(this);
		AddMin->setMinimum(0);
		AddMin->setMaximum(59);

		connect(Add, &QPushButton::clicked, this, [=]()
			{
				int Time = ((AddHour->value() * 60) + AddMin->value()) * MAX_TIME_VALUE / (24 * 60);
				CreateValue(Time);
			});

		PreLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		PreLayout->addWidget(Add);
		PreLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
		PreLayout->addWidget(new QLabel(tr("Time"), this));
		PreLayout->addWidget(AddHour);
		PreLayout->addWidget(new QLabel(tr("Minutes"), this));
		PreLayout->addWidget(AddMin);
		PreLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	}

	MainLayout->addSpacerItem(new QSpacerItem(0, 8, QSizePolicy::Expanding, QSizePolicy::Fixed));
	MainLayout->addLayout(PreLayout);
	MainLayout->addSpacerItem(new QSpacerItem(0, 8, QSizePolicy::Expanding, QSizePolicy::Fixed));
	MainLayout->addWidget(Indicator);
	MainLayout->addLayout(PreviewLayout);
	MainLayout->addWidget(Arrow);
	MainLayout->addSpacerItem(new QSpacerItem(0, 16, QSizePolicy::Expanding, QSizePolicy::Fixed));
	MainLayout->addWidget(Line);
	MainLayout->addSpacerItem(new QSpacerItem(0, 16, QSizePolicy::Expanding, QSizePolicy::Fixed));
	MainLayout->addLayout(SecLayout);
	MainLayout->addSpacerItem(new QSpacerItem(0, 8, QSizePolicy::Expanding, QSizePolicy::Fixed));
	MainLayout->addLayout(ThirdLayout);
	MainLayout->addSpacerItem(new QSpacerItem(0, 16, QSizePolicy::Expanding, QSizePolicy::Fixed));

	setFixedSize(sizeHint());
}

void LightViewEditor::closeEvent(QCloseEvent* event)
{
	emit Delete(this);
	event->accept();
}

void LightViewEditor::UpdateSkyColorRowsValue()
{
	UpdateWidgetEdition(CurrentIndex);
	_World->renderer()->skies()->force_update();

	Arrow->UpdateData(_Sky->colorRows[_eSkyColorIndex]);
	Preview->SetPreview(_Sky->colorRows[_eSkyColorIndex]);
}

bool LightViewEditor::ValidCurrentIndex()
{
	return (CurrentIndex >= 0 && CurrentIndex < _Sky->colorRows[_eSkyColorIndex].size());
}

void LightViewEditor::UpdateWorldTime()
{
	Indicator->UpdateWorldTime(static_cast<int>(_World->time) % MAX_TIME_VALUE);
}

void LightViewEditor::UpdateWidgetEdition(int Index)
{
	CurrentIndex = Index;
	if (CurrentIndex < 0 || CurrentIndex >= _Sky->colorRows[_eSkyColorIndex].size())
	{
		TimeSelectorHour->setEnabled(false);
		TimeSelectorMin->setEnabled(false);
		ColorPicker->setEnabled(false);
		Remove->setEnabled(false);

		QSignalBlocker TimeHourBlocker(TimeSelectorHour);
		QSignalBlocker TimeMinBlocker(TimeSelectorMin);
		QSignalBlocker ColorBlocker(ColorPicker);

		TimeSelectorHour->setValue(0);
		TimeSelectorMin->setValue(0);
		ColorPicker->setColor(Qt::gray);

		TimeHourBlocker.unblock();
		TimeMinBlocker.unblock();
		ColorBlocker.unblock();
		return;
	}

	TimeSelectorHour->setEnabled(true);
	TimeSelectorMin->setEnabled(true);
	ColorPicker->setEnabled(true);
	Remove->setEnabled(true);

	QSignalBlocker TimeHourBlocker(TimeSelectorHour);
	QSignalBlocker TimeMinBlocker(TimeSelectorMin);
	QSignalBlocker ColorBlocker(ColorPicker);

	int Time = _Sky->colorRows[_eSkyColorIndex][CurrentIndex].time;
	int ConvertedTime = Time * (24 * 60) / MAX_TIME_VALUE;

	int Hour = floor(ConvertedTime / 60);
	int Min = ConvertedTime % 60;

	TimeSelectorHour->setValue(Hour);
	TimeSelectorMin->setValue(Min);

	auto Color = _Sky->colorRows[_eSkyColorIndex][CurrentIndex].color * 255.f;
	ColorPicker->setColor(QColor(Color.r, Color.g, Color.b));

	TimeHourBlocker.unblock();
	TimeMinBlocker.unblock();
	ColorBlocker.unblock();
}

void LightViewEditor::CreateValue(int Time)
{
	if (_Sky->colorRows[_eSkyColorIndex].size() == 16)
		return;

	SkyColor NewValue(Time, 0);
	_Sky->colorRows[_eSkyColorIndex].push_back(NewValue);
	LightViewWidget::SortSkyColorVector(_Sky->colorRows[_eSkyColorIndex]);

	for (int i = 0; i < _Sky->colorRows[_eSkyColorIndex].size(); ++i)
	{
		if (_Sky->colorRows[_eSkyColorIndex][i].time == Time)
		{
			auto Vec = _Sky->colorRows[_eSkyColorIndex];

			if (Vec.size() == 1)
			{
				Vec[i].color = glm::vec3(.2f, .2f, .2f);
				UpdateSkyColorRowsValue();
				return;
			}

			if (Vec.size() == 2)
			{
				glm::vec3 Color = Vec[0].color * 255.f;

				if (i == 0)
					Color = Vec[i + 1].color * 255.f;

				Vec[i].color = Color;
				UpdateSkyColorRowsValue();
				return;
			}

			QColor Color;
			if (i == 0)
			{
				glm::vec3 Vec3A = Vec[Vec.size() - 1].color * 255.f;
				glm::vec3 Vec3B = Vec[i + 1].color * 255.f;

				QColor ColorA(Vec3A.r, Vec3A.g, Vec3A.b);
				QColor ColorB(Vec3B.r, Vec3B.g, Vec3B.b);

				int OffsetTime = abs(Vec[Vec.size() - 1].time - MAX_TIME_VALUE);
				int TimeRef = Vec[i + 1].time + OffsetTime;

				float t = float(Time + OffsetTime) / float(TimeRef);
				Color = LightViewWidget::InterpolateColor(ColorA, ColorB, t);
			}
			else if (i == _Sky->colorRows[_eSkyColorIndex].size() - 1)
			{
				glm::vec3 Vec3A = Vec[i - 1].color * 255.f;
				glm::vec3 Vec3B = Vec[0].color * 255.f;

				QColor ColorA(Vec3A.r, Vec3A.g, Vec3A.b);
				QColor ColorB(Vec3B.r, Vec3B.g, Vec3B.b);

				int OffsetTime = abs(Vec[i - 1].time - MAX_TIME_VALUE);
				int TimeRef = Vec[0].time + OffsetTime;

				float t = float(Time - Vec[i - 1].time) / float(TimeRef);
				Color = LightViewWidget::InterpolateColor(ColorA, ColorB, t);
			}
			else
			{
				glm::vec3 Vec3A = Vec[i - 1].color * 255.f;
				glm::vec3 Vec3B = Vec[i + 1].color * 255.f;

				QColor ColorA(Vec3A.r, Vec3A.g, Vec3A.b);
				QColor ColorB(Vec3B.r, Vec3B.g, Vec3B.b);

				int OffsetTime = Vec[i - 1].time;
				int TimeRef = Vec[i + 1].time - OffsetTime;

				float t = float(Time - OffsetTime) / float(TimeRef);
				Color = LightViewWidget::InterpolateColor(ColorA, ColorB, t);
			}

			_Sky->colorRows[_eSkyColorIndex][i].color = glm::vec3(Color.redF(), Color.greenF(), Color.blueF());
			Arrow->CurrentSelected = i;
			CurrentIndex = i;
			UpdateSkyColorRowsValue();
			return;
		}
	}
}

void LightViewEditor::DeleteValue(int Index)
{
	if (_Sky->colorRows[_eSkyColorIndex].size() == 1)
		return;

	QSignalBlocker Blocker(Arrow);
	if (Index >= 0 && Index < _Sky->colorRows[_eSkyColorIndex].size())
	{
		_Sky->colorRows[_eSkyColorIndex].erase(_Sky->colorRows[_eSkyColorIndex].begin() + Index);

		Arrow->CurrentHover = -1;
		Arrow->CurrentSelected = -1;
		CurrentIndex = -1;
		UpdateSkyColorRowsValue();
		Blocker.unblock();
		return;
	}
	Blocker.unblock();
}

LightViewDayIndicator::LightViewDayIndicator(QSize Size, QWidget* parent)
	: QLabel(parent), PixmapSize(Size)
{
	ColorFromStyleSheet = LightViewWidget::GetColorFromStyleSheet();
}

void LightViewDayIndicator::UpdateWorldTime(int Time)
{
	Base = QPixmap(PixmapSize);
	Base.fill(ColorFromStyleSheet);
	QPainter Painter(&Base);

	const int PixelPointing = Time * (PixmapSize.width() - 30) / MAX_TIME_VALUE + 15;

	QPainterPath Triangle;
	Triangle.moveTo(QPoint(PixelPointing, PixmapSize.height() - 5));
	Triangle.lineTo(QPoint(PixelPointing + 10, PixmapSize.height() - 15));
	Triangle.lineTo(QPoint(PixelPointing - 10, PixmapSize.height() - 15));
	Triangle.lineTo(QPoint(PixelPointing, PixmapSize.height() - 5));

	Painter.setPen(Qt::NoPen);
	Painter.fillPath(Triangle, QBrush(Qt::white));

	setPixmap(Base);
}

LightViewArrow::LightViewArrow(QSize Size, QWidget* parent)
	: QLabel(parent), PixmapSize(Size)
{
	setMouseTracking(true);
	ColorFromStyleSheet = LightViewWidget::GetColorFromStyleSheet();
}

void LightViewArrow::UpdateData(std::vector<SkyColor>& data, bool UpdateDataColor)
{
	Data = data;
	if (Data.size() == 0)
		return;

	if (UpdateDataColor)
		for (int i = 0; i < Data.size(); ++i)
			Data[i].color *= 255.f;

	LightViewWidget::SortSkyColorVector(Data);

	Base = QPixmap(PixmapSize);
	Base.fill(ColorFromStyleSheet);
	QPainter Painter(&Base);

	for (int i = 0; i < Data.size(); ++i)
	{
		// this is where the value is exactly on the Pixmap
		const int PixelPointing = Data[i].time * (PixmapSize.width() - 30) / MAX_TIME_VALUE + 15;
		QColor ColorCadre = (i == CurrentSelected) ? Qt::black : Qt::white;

		// draw triangle pointing exactly where it's needed
		{
			QPainterPath Triangle;
			Triangle.moveTo(QPoint(PixelPointing, 5));
			Triangle.lineTo(QPoint(PixelPointing + 10, 15));
			Triangle.lineTo(QPoint(PixelPointing - 10, 15));
			Triangle.lineTo(QPoint(PixelPointing, 5));

			Painter.setPen(Qt::NoPen);
			Painter.fillPath(Triangle, QBrush(ColorCadre));
		}

		// draw rectangle preview with correct color
		{
			QPainterPath Cadre;
			Cadre.moveTo(QPoint(PixelPointing - 10, 15));
			Cadre.lineTo(QPoint(PixelPointing + 10, 15));
			Cadre.lineTo(QPoint(PixelPointing + 10, 35));
			Cadre.lineTo(QPoint(PixelPointing - 10, 35));
			Cadre.lineTo(QPoint(PixelPointing - 10, 15));

			Painter.setPen(Qt::NoPen);
			Painter.fillPath(Cadre, QBrush(ColorCadre));

			QPainterPath Color;
			Color.moveTo(QPoint(PixelPointing - 8, 17));
			Color.lineTo(QPoint(PixelPointing + 8, 17));
			Color.lineTo(QPoint(PixelPointing + 8, 33));
			Color.lineTo(QPoint(PixelPointing - 8, 33));
			Color.lineTo(QPoint(PixelPointing - 8, 17));

			Painter.setPen(Qt::NoPen);
			Painter.fillPath(Color, QColor(Data[i].color.r, Data[i].color.g, Data[i].color.b));
		}
	}

	setPixmap(Base);
}

void LightViewArrow::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		if (CurrentHover < 0)
			return;

		CurrentSelected = CurrentHover;
		UpdateData(Data, false);
		emit SelectValue(CurrentSelected);

		bDragging = true;
		QApplication::setOverrideCursor(Qt::ClosedHandCursor);
	}

	if (event->modifiers() & Qt::CTRL && event->button() == Qt::RightButton)
	{
		if (CurrentHover < 0)
			return;

		emit DeleteValue(CurrentHover);
		QApplication::restoreOverrideCursor();
	}
}

void LightViewArrow::mouseReleaseEvent(QMouseEvent* event)
{
	if (!bDragging)
		return;

	CurrentHover = -1;
	bDragging = false;
	QApplication::restoreOverrideCursor();
	emit ReleaseArrow();
}

void LightViewArrow::mouseMoveEvent(QMouseEvent* event)
{
	if (!bDragging)
	{
		QApplication::restoreOverrideCursor();
		CurrentHover = -1;

		if (event->pos().y() >= 10 && event->pos().y() <= 30)
		{
			for (int i = 0; i < Data.size(); ++i)
			{
				const int PixelPointing = Data[i].time * (PixmapSize.width() - 30) / MAX_TIME_VALUE + 20;

				if (event->pos().x() >= PixelPointing - 10 && event->pos().x() <= PixelPointing + 10)
				{
					QApplication::setOverrideCursor(Qt::PointingHandCursor);
					CurrentHover = i;
					return;
				}
			}
		}
	}
	else
	{
		if (event->pos().x() < 15)
		{
			emit ChangeTime(CurrentHover, 0);
			return;
		}

		if (event->pos().x() > PixmapSize.width() - 15)
		{
			emit ChangeTime(CurrentHover, MAX_TIME_VALUE);
			return;
		}

		int Time = (event->pos().x() - 15) * MAX_TIME_VALUE / (PixmapSize.width() - 30);
		emit ChangeTime(CurrentHover, Time);
	}
}