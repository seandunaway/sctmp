#include <dwmapi.h>
#include <sierrachart.h>

SCDLLName("darkmode")

void set_color_mode (HWND hwnd, BOOL darkmode, COLORREF border_color)
{
	DwmSetWindowAttribute(hwnd, 19, &darkmode, sizeof(darkmode));
	DwmSetWindowAttribute(hwnd, 20, &darkmode, sizeof(darkmode));
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkmode, sizeof(darkmode));
	DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color));
}

SCSFExport scsf_darkmode(SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
	{
		sc.DisplayStudyName = 0;
		sc.GraphName = "darkmode";
		sc.GraphRegion = 0;
		return;
	}

	int persistent = -1;
	int& initialized = sc.GetPersistentInt(++persistent);
	int64_t& old_border_color = sc.GetPersistentInt64(++persistent);

	HWND hwnd = sc.GetChartWindowHandle(sc.ChartNumber);

	if (!initialized)
	{
		DwmGetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &old_border_color, sizeof(old_border_color));

		set_color_mode(hwnd, true, 0x000000);

		sc.AddMessageToLog("enabled", 0);

		initialized = 1;
		return;
	}

	if (sc.LastCallToFunction)
	{
		set_color_mode(hwnd, false, (COLORREF) old_border_color);

		sc.AddMessageToLog("disabled", 0);

		initialized = 0;
		return;
	}
}
