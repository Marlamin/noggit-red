#pragma once

#include <noggit/Sky.h>
#include <noggit/MapView.h>
#include <QApplication>
#include <QMouseEvent>
#include <QPainterPath>
#include <QFrame>
#include <QEvent>
#include <QLayout>
#include <QLabel>
#include <QPixmap>
#include "DynamicMouseWidget.h"

#define MAX_TIME_VALUE  2880

#define LIGHT_VIEW_PREVIEW_WIDTH   340
#define LIGHT_VIEW_PREVIEW_HEIGHT  20

#define LIGHT_VIEW_PREVIEW_LINE_SIZE            1
#define LIGHT_VIEW_PREVIEW_LINE_COLOR           0xFFFFFF
#define LIGHT_VIEW_PREVIEW_LINE_DAY_CYCLE_COLOR 0x000000

static std::map <int, std::string> SkyColorNameByIndex = {
        {0	, "Direct"},
        {1	, "Ambiant"},
        {2	, "Sky Top"},
        {3	, "Sky Midle"},
        {4	, "Sky Band 1"},
        {5	, "Sky Band 2"},
        {6	, "Sky Smog"},
        {7	, "Sky Fog"},
        {8	, "Sun"},
        {9	, "Cloud Sun"},
        {10	, "Cloud Emissive"},
        {11	, "Cloud Layer 1 Ambian"},
        {12	, "Cloud Layer 2 Ambiant"},
        {13	, "Unknown/Unused"},
        {14	, "Ocean Close"},
        {15	, "Ocean Far"},
        {16	, "River Close"},
        {17	, "River Far"}
};

/*
* PIXMAP
*/
class LightViewPixmap : public QLabel
{
    Q_OBJECT

public:
    LightViewPixmap(QSize Size = QSize(LIGHT_VIEW_PREVIEW_WIDTH, LIGHT_VIEW_PREVIEW_HEIGHT), QWidget* parent = nullptr);
    void ShowLines(bool show);
    void EnableEvent() { bEvent = true; };
    void DisableEvent() { bEvent = false; };
    void SetPreview(std::vector<SkyColor>& data, int Index = -1);

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void UpdatePixmap(const QPixmap Updated);
    void CreateValue(int Time);

private:
    bool bShowLines = false;
    bool bEvent = false;
    std::vector<SkyColor> Data;

    bool DrawLines();

    QImage Image;
    QPixmap Base;
    QSize PixmapSize;
};

/*
* ARROW
*/
class LightViewArrow : public QLabel
{
    Q_OBJECT
        
public:
    LightViewArrow(QSize Size = QSize(LIGHT_VIEW_PREVIEW_WIDTH, LIGHT_VIEW_PREVIEW_HEIGHT), QWidget* parent = nullptr);
    void UpdateData(std::vector<SkyColor>& data, bool UpdateDataColor = true);
    int CurrentHover = -1;
    int CurrentSelected = -1;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void DeleteValue(int Index);
    void SelectValue(int Index);
    void ChangeTime(int Index, int Time);
    void ReleaseArrow();

private:
    std::vector<SkyColor> Data;

    QPixmap Base;
    QSize PixmapSize;
    QColor ColorFromStyleSheet;

    bool bDragging = false;
    bool bUpdateMouse = false;
};

/*
* DAY INDICATOR
*/
class LightViewDayIndicator : public QLabel
{
public:
    LightViewDayIndicator(QSize Size = QSize(LIGHT_VIEW_PREVIEW_WIDTH, LIGHT_VIEW_PREVIEW_HEIGHT), QWidget* parent = nullptr);
    void UpdateWorldTime(int Iime);

private:
    QPixmap Base;
    QSize PixmapSize;
    QColor ColorFromStyleSheet;
};

/*
* EDITOR
*/
class LightViewEditor : public QWidget
{
    Q_OBJECT

public:
    LightViewEditor(MapView* Map, SkyParam* Sky, SkyColorNames eSkyColor, QWidget* parent = nullptr);
    void UpdateWorldTime();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void UpdateWidgetEdition(int Index);
    void DeleteValue(int Index);
    void CreateValue(int Time = 0);

private:
    void UpdateSkyColorRowsValue();
    bool ValidCurrentIndex();

signals:
    void Delete(LightViewEditor* self);
    void UpdatePixmap(const QPixmap Updated);

private:
    World* _World;
    MapView* _Map;
    SkyParam* _Sky;
    SkyColorNames _eSkyColorIndex;
    std::string _ColorName;

    QVBoxLayout* MainLayout;

    int CurrentIndex = -1;

    LightViewDayIndicator* Indicator;
    LightViewPixmap* Preview;
    LightViewArrow* Arrow;

    QPushButton* Add;
    QSpinBox* AddHour;
    QSpinBox* AddMin;

    QPushButton* Remove;

    color_widgets::ColorSelector* ColorPicker;
    QSpinBox* TimeSelectorHour;
    QSpinBox* TimeSelectorMin;
};

/*
* PREVIEW
*/
class LightViewPreview : public DynamicMouseWidget
{
    Q_OBJECT

public:
    LightViewPreview(QSize Size = QSize(LIGHT_VIEW_PREVIEW_WIDTH, LIGHT_VIEW_PREVIEW_HEIGHT), QWidget* parent = nullptr);
    LightViewPreview(QString LightName, QSize Size = QSize(LIGHT_VIEW_PREVIEW_WIDTH, LIGHT_VIEW_PREVIEW_HEIGHT), QWidget* parent = nullptr);
    void ShowLines(bool show);
    void SetPreview(std::vector<SkyColor>& data);
    void UpdatePixmap(const QPixmap Updated);

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void LeftClicked();

private:
    QVBoxLayout* MainLayout;

    QLabel* Name;
    LightViewPixmap* Preview;
};

/*
* WIDGET BASE
*/
class LightViewWidget : public QWidget
{
    Q_OBJECT

public:
    LightViewWidget(QWidget* parent = nullptr);

    static QColor InterpolateColor(QColor& a, QColor& b, float t);
    static QImage FillImagePart(QImage Image, int X, QColor Color);
    static inline int ClampColor(int Value);
    static QColor GetColorFromStyleSheet();
    static void SortSkyColorVector(std::vector<SkyColor>& Vector);
};

