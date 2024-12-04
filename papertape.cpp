#include <sierrachart.h>

SCDLLName("papertape")

struct bar
{
	int index;
	int year, month, day, week, hour, minute, second;
	float last_price;
};

struct colors
{
	n_ACSIL::s_GraphicsColor text;
	n_ACSIL::s_GraphicsColor background;
};

struct grid
{
	int margin_top;
	int left, right, top, bottom;
	int width, height;
	int cell_width, cell_height;
	int visible_columns, rows;
	int total_columns, last_visible_column, first_visible_column;
};

SCString bar_to_text (struct bar bar)
{
	SCString text;

	text.Format("%02d:%02d %.2f", bar.hour, bar.minute, (double) bar.last_price);
	text.Append("  ");

	return text;
}

struct bar get_bar (SCStudyInterfaceRef sc, int index)
{
	struct bar bar;

	bar.index = index;
	sc.BaseDateTimeIn[index].GetDateTimeYMDHMS(bar.year, bar.month, bar.day, bar.hour, bar.minute, bar.second);
	bar.last_price = sc.BaseDataIn[SC_LAST][index];

	return bar;
}

struct colors get_chart_colors (SCStudyInterfaceRef sc)
{
	struct colors colors;

	uint32_t text;
	uint32_t background;
	uint32_t unused_line_width;
	SubgraphLineStyles unused_line_style;

	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CHART_TEXT, text, unused_line_width, unused_line_style);
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CHART_BACKGROUND, background, unused_line_width, unused_line_style);

	colors.text.SetColorValue(text);
	colors.background.SetColorValue(background);

	return colors;
}

n_ACSIL::s_GraphicsFont get_chart_font (SCStudyInterfaceRef sc)
{
	n_ACSIL::s_GraphicsFont font;

	int32_t is_italic;
	int32_t is_underline;

	sc.GetChartFontProperties(font.m_FaceName, font.m_Height, font.m_Weight, is_italic, is_underline);

	font.m_IsItalic = (bool) is_italic;
	font.m_IsUnderline = (bool) is_underline;

	return font;
}

void set_draw_style (SCStudyInterfaceRef sc, n_ACSIL::s_GraphicsFont font, struct colors colors)
{
	sc.Graphics.SetTextAlign(TA_NOUPDATECP);
	sc.Graphics.SetTextFont(font);
	sc.Graphics.SetTextColor(colors.text);
	sc.Graphics.SetBackgroundColor(colors.background);
}

struct grid calculate_grid (SCStudyInterfaceRef sc)
{
	struct bar first_bar = get_bar(sc, 0);
	SCString first_text = bar_to_text(first_bar);

	n_ACSIL::s_GraphicsFont font = get_chart_font(sc);
	n_ACSIL::s_GraphicsSize text_dimensions;
	sc.Graphics.GetTextSizeWithFont(first_text, font, text_dimensions);

	struct grid grid;

	grid.margin_top = text_dimensions.Height * 2;

	grid.left = sc.StudyRegionLeftCoordinate;
	grid.right = sc.StudyRegionRightCoordinate;
	grid.top = sc.StudyRegionTopCoordinate + grid.margin_top;
	grid.bottom = sc.StudyRegionBottomCoordinate;

	grid.width = grid.right - grid.left;
	grid.height = grid.bottom - grid.top;

	grid.cell_width = text_dimensions.Width;
	grid.cell_height = text_dimensions.Height;

	grid.visible_columns = grid.width / grid.cell_width;
	grid.rows = grid.height / grid.cell_height;

	grid.total_columns = sc.ArraySize / grid.rows;
	grid.last_visible_column = sc.IndexOfLastVisibleBar / grid.rows;
	grid.first_visible_column = max(0, grid.last_visible_column - grid.visible_columns);

	return grid;
}

void draw (HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	n_ACSIL::s_GraphicsFont font = get_chart_font(sc);
	struct colors colors = get_chart_colors(sc);
	set_draw_style(sc, font, colors);

	struct grid grid = calculate_grid(sc);

	for (int column = grid.first_visible_column; column <= grid.last_visible_column; column++)
	{
		int columns_from_right = (grid.last_visible_column - column) + 1;
		int pixels_from_right = columns_from_right * grid.cell_width;
		int x = grid.right - pixels_from_right;

		for (int row = 0; row < grid.rows; row++)
		{
			int bar_index = (column * grid.rows) + row;
			if (bar_index >= sc.ArraySize)
				return;

			struct bar bar = get_bar(sc, bar_index);
			SCString text = bar_to_text(bar);

			int pixels_from_top = row * grid.cell_height;
			int y = grid.top + pixels_from_top;

			sc.Graphics.DrawTextAt(text, x, y);
		}
	}
}

SCSFExport scsf_papertape (SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
	{
		sc.DisplayStudyName = 0;
		sc.GraphName = "papertape";
		sc.GraphRegion = 0;
	}

	if (sc.HideStudy || sc.IsFullRecalculation || sc.DownloadingHistoricalData)
		return;

	sc.p_GDIFunction = draw;
}
