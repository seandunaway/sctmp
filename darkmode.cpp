#include <dwmapi.h>
#include <sierrachart.h>

SCDLLName("darkmode")

SCSFExport scsf_darkmode(SCStudyInterfaceRef sc) {
	if (sc.SetDefaults) {
		sc.DisplayStudyName = 0;
		sc.GraphName = "darkmode";
		sc.GraphRegion = 0;
		return;
	}

	BOOL darkmode = TRUE;
	int& initialized = sc.GetPersistentIntFast(0);

	if (sc.LastCallToFunction) {
		darkmode = FALSE;
		initialized = 0;
	}

	if (initialized) return;

	HWND hwnd = sc.GetChartWindowHandle(sc.ChartNumber);
	DwmSetWindowAttribute(hwnd, 19, &darkmode, sizeof(darkmode));
	DwmSetWindowAttribute(hwnd, 20, &darkmode, sizeof(darkmode));

	sc.AddMessageToLog(darkmode ? "enabled" : "disabled", 0);
	initialized = 1;
}
