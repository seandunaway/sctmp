#include <sierrachart.h>

SCDLLName("papertape")

struct calculations {int margin, bottom, left, right, top, height, width, rows, columns, stop, start;};
static void calculate(SCStudyInterfaceRef sc, n_ACSIL::s_GraphicsSize size, struct calculations &c) {
	c.margin = size.Height * 2;

	c.bottom = sc.StudyRegionBottomCoordinate;
	c.left = sc.StudyRegionLeftCoordinate;
	c.right = sc.StudyRegionRightCoordinate;
	c.top = sc.StudyRegionTopCoordinate + c.margin;

	c.height = c.bottom - c.top;
	c.width = c.right - c.left;

	c.columns = c.width / size.Width;
	c.rows = c.height / size.Height;

	c.stop = sc.IndexOfLastVisibleBar / c.rows;
	c.start = max(0, c.stop - c.columns);
}

static void format(SCStudyInterfaceRef sc, int index, SCString &text) {
	int year, month, day, hour, minute, second;
	sc.BaseDateTimeIn[index].GetDateTimeYMDHMS(year, month, day, hour, minute, second);

	float last = sc.BaseDataIn[SC_LAST][index];

	text.Format("%02d:%02d %.2f", hour, minute, (double) last);
	text.Append("  ");
}

static void settings(SCStudyInterfaceRef sc, n_ACSIL::s_GraphicsFont &font, n_ACSIL::s_GraphicsColor &color, n_ACSIL::s_GraphicsColor &background) {
	int32_t italic;
	int32_t underline;
	sc.GetChartFontProperties(font.m_FaceName, font.m_Height, font.m_Weight, italic, underline);
	font.m_IsItalic = (bool) italic;
	font.m_IsUnderline = (bool) underline;

	uint32_t dummy_color;
	uint32_t dummy_background;
	uint32_t dummy_line_width;
	SubgraphLineStyles dummy_line_style;
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CHART_TEXT, dummy_color, dummy_line_width, dummy_line_style);
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CHART_BACKGROUND, dummy_background, dummy_line_width, dummy_line_style);
	color.SetColorValue(dummy_color);
	background.SetColorValue(dummy_background);

	sc.Graphics.SetTextAlign(TA_NOUPDATECP);
	sc.Graphics.SetTextFont(font);
	sc.Graphics.SetTextColor(color);
	sc.Graphics.SetBackgroundColor(background);
}

static void draw(HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc) {
	n_ACSIL::s_GraphicsFont font;
	n_ACSIL::s_GraphicsColor color;
	n_ACSIL::s_GraphicsColor background;
	settings(sc, font, color, background);

	SCString dummy_text;
	format(sc, 0, dummy_text);
	n_ACSIL::s_GraphicsSize size;
	sc.Graphics.GetTextSizeWithFont(dummy_text, font, size);

	struct calculations c;
	calculate(sc, size, c);

	for (int column = c.start; column <= c.stop; column++) {
		int x = c.right - (c.stop - column + 1) * size.Width;

		for (int row = 0; row < c.rows; row++) {
			int index = column * c.rows + row;
			if (index >= sc.ArraySize) return;

			int y = c.top + (row * size.Height);

			SCString text;
			format(sc, index, text);
			sc.Graphics.DrawTextAt(text, x, y);
		}
	}
}

SCSFExport scsf_papertape(SCStudyInterfaceRef sc) {
	if (sc.SetDefaults) {
		sc.DisplayStudyName = 0;
		sc.GraphName = "papertape";
		sc.GraphRegion = 0;
		return;
	}

	if (sc.HideStudy) return;

	sc.p_GDIFunction = draw;
}
