#include <sierrachart.h>

SCDLLName("full width line")

SCSFExport scsf_full_width_line(SCStudyInterfaceRef sc) {
	int i = -1;
	SCInputRef anchor1_x = sc.Input[++i];
	SCInputRef anchor1_y = sc.Input[++i];
	SCInputRef anchor2_x = sc.Input[++i];
	SCInputRef anchor2_y = sc.Input[++i];
	SCInputRef color = sc.Input[++i];
	SCInputRef width = sc.Input[++i];

	if (sc.SetDefaults) {
		sc.AutoLoop = 0;
		sc.DisplayStudyName = 0;
		sc.GraphName = "full width line";
		sc.GraphRegion = 0;
		sc.UpdateAlways = 1;

		int x_max = (int) CHART_DRAWING_MAX_HORIZONTAL_AXIS_RELATIVE_POSITION;
		int y_max = (int) CHART_DRAWING_MAX_VERTICAL_AXIS_RELATIVE_POSITION;

		anchor1_x.Name.Format("anchor1_x (1-%d)", x_max);
		anchor1_x.SetInt(1);
		anchor1_x.SetIntLimits(1, x_max);
		anchor1_y.Name.Format("anchor1_y (1-%d)", y_max);
		anchor1_y.SetInt(30);
		anchor1_y.SetIntLimits(1, y_max);

		anchor2_x.Name.Format("anchor2_x (1-%d)", x_max);
		anchor2_x.SetInt(150);
		anchor2_x.SetIntLimits(1, x_max);
		anchor2_y.Name.Format("anchor2_y (1-%d)", y_max);
		anchor2_y.SetInt(70);
		anchor2_y.SetIntLimits(1, y_max);

		color.Name = "color";
		color.SetColor(128, 0, 255);

		width.Name = "width";
		width.SetInt(10);
		width.SetIntLimits(1, 100);

		return;
	}

	if (sc.HideStudy) return;

	s_UseTool tool;
	tool.Clear();
	tool.AddMethod = UTAM_ADD_OR_ADJUST;
	tool.BeginDateTime = anchor1_x.GetInt();
	tool.BeginValue = (float) anchor1_y.GetInt();
	tool.Color = color.GetColor();
	tool.DrawingType = DRAWING_LINE;
	tool.EndDateTime = anchor2_x.GetInt();
	tool.EndValue = (float) anchor2_y.GetInt();
	tool.LineWidth = (unsigned short) width.GetInt();
	tool.UseRelativeVerticalValues = 1;

	int& line = sc.GetPersistentIntFast(0);
	if (line != 0) tool.LineNumber = line;
	sc.UseTool(tool);
	line = tool.LineNumber;
}
