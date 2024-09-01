#include <sierrachart.h>

SCDLLName("pnl multiply")

SCSFExport scsf_pnl_multiply(SCStudyInterfaceRef sc) {
	int i = -1;
	SCInputRef multiply = sc.Input[++i];
	SCInputRef x = sc.Input[++i];
	SCInputRef y = sc.Input[++i];
	SCInputRef prefix = sc.Input[++i];
	SCInputRef suffix = sc.Input[++i];
	SCInputRef size = sc.Input[++i];
	SCInputRef background = sc.Input[++i];
	SCInputRef positive = sc.Input[++i];
	SCInputRef neutral = sc.Input[++i];
	SCInputRef negative = sc.Input[++i];
	SCInputRef bold = sc.Input[++i];
	SCInputRef format = sc.Input[++i];
	SCInputRef data = sc.Input[++i];

	if (sc.SetDefaults) {
		sc.AutoLoop = 0;
		sc.DisplayStudyName = 0;
		sc.GraphName = "pnl multiply";
		sc.GraphRegion = 0;

		multiply.Name = "multiply";
		multiply.SetDouble(20.0);
		multiply.SetDoubleLimits(0.0, 100.0);

		int x_max = (int) CHART_DRAWING_MAX_HORIZONTAL_AXIS_RELATIVE_POSITION;
		x.Name.Format("x (1-%d)", x_max);
		x.SetInt(10);
		x.SetIntLimits(1, x_max);

		int y_max = (int) CHART_DRAWING_MAX_VERTICAL_AXIS_RELATIVE_POSITION;
		y.Name.Format("y (1-%d)", y_max);
		y.SetInt(90);
		y.SetIntLimits(1, y_max);

		prefix.Name = "prefix";
		prefix.SetString("NPL: ");

		suffix.Name = "suffix";
		suffix.SetString("");

		size.Name = "size";
		size.SetInt(16);
		size.SetIntLimits(0, 100);

		background.Name = "background";
		background.SetColor(248, 248, 242);

		positive.Name = "positive";
		positive.SetColor(80, 250, 123);

		neutral.Name = "neutral";
		neutral.SetColor(40, 42, 54);

		negative.Name = "negative";
		negative.SetColor(255, 85, 85);

		bold.Name = "bold";
		bold.SetYesNo(0);

		format.Name = "format";
		format.SetCustomInputStrings("none;Currency Value;Points (P);Points - Ignore Quantity (p);Ticks (T);Ticks - Ignore Quantity (t);Currency Value & Points (P);Currency Value & Points - Ignore Quantity (p);Currency Value & Ticks (T);Currency Value & Ticks - Ignore Quantity (t)");
		format.SetCustomInputIndex(6);

		data.Name = "data";
		data.SetCustomInputStrings("all;today");
		data.SetCustomInputIndex(1);

		return;
	}

	if (sc.HideStudy) return;

	double pnl = sc.GetTotalNetProfitLossForAllSymbols((int) data.GetIndex());
	double pnl_multiply = pnl * multiply.GetDouble();

	SCString pnl_string;
	ProfitLossDisplayFormatEnum pnl_format = (ProfitLossDisplayFormatEnum) format.GetIndex();
	if (!pnl_format) pnl_string.Format("%.2f", pnl_multiply);
	else sc.CreateProfitLossDisplayString(pnl_multiply, 0, pnl_format, pnl_string);

	SCString output;
	output.Append(" ");
	output.Append(prefix.GetString());
	output.Append(pnl_string);
	output.Append(suffix.GetString());
	output.Append(" ");

	unsigned int color = neutral.GetColor();
	if (pnl_multiply > 0) color = positive.GetColor();
	if (pnl_multiply < 0) color = negative.GetColor();

	s_UseTool tool;
	tool.Clear();
	tool.AddMethod = UTAM_ADD_OR_ADJUST;
	tool.BeginDateTime = x.GetInt();
	tool.BeginValue = (float) y.GetInt();
	tool.ChartNumber = sc.ChartNumber;
	tool.Color = color;
	tool.DrawingType = DRAWING_TEXT;
	tool.FontBackColor = background.GetColor();
	tool.FontBold = (int) bold.GetBoolean();
	tool.FontSize = size.GetInt();
	tool.Region = sc.GraphRegion;
	tool.UseRelativeVerticalValues = true;
	tool.Text = output;

	int& line = sc.GetPersistentIntFast(0);
	if (line != 0) tool.LineNumber = line;
	sc.UseTool(tool);
	line = tool.LineNumber;
}
