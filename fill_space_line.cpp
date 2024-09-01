#include "sierrachart.h"

SCDLLName("fill space line")

SCSFExport scsf_fill_space_line(SCStudyInterfaceRef sc) {
	if (sc.SetDefaults) {
		sc.AutoLoop = 0;
		sc.DisplayStudyName = 0;
		sc.GraphName = "fill space line";
		sc.GraphRegion = 0;
		sc.UpdateAlways = 1;
		return;
	}

	double last_price = sc.GetLastPriceForTrading();
	double price_begin = last_price - (last_price * 0.0025);
	double price_end = last_price + (last_price * 0.0025);

	s_UseTool tool;
	tool.Clear();
	tool.AddMethod = UTAM_ADD_OR_ADJUST;
	tool.BeginIndex = sc.IndexOfFirstVisibleBar;
	tool.BeginValue = (float) price_begin;
	tool.Color = 0xff0080;
	tool.DrawingType = DRAWING_LINE;
	tool.EndIndex = sc.IndexOfLastVisibleBar + (int) sc.NumFillSpaceBars;
	tool.EndValue = (float) price_end;
	tool.LineWidth = 10;

	int& line = sc.GetPersistentIntFast(0);
	if (line != 0) tool.LineNumber = line;
	sc.UseTool(tool);
	line = tool.LineNumber;
}
