#include <sierrachart.h>

#undef SC_DLL_VERSION
#define SC_DLL_VERSION 2563

SCDLLName("pnl all accounts")

SCSFExport scsf_pnl_all_accounts(SCStudyInterfaceRef sc) {
	SCSubgraphRef subgraph = sc.Subgraph[0];
	SCInputRef x = sc.Input[0];
	SCInputRef y = sc.Input[1];
	SCInputRef bold = sc.Input[2];
	SCInputRef open = sc.Input[3];

	if (sc.SetDefaults) {
		sc.AutoLoop = 0;
		sc.DisplayStudyName = 0;
		sc.GraphName = "pnl all accounts";
		sc.GraphRegion = 0;

		subgraph.DrawStyle = DRAWSTYLE_CUSTOM_TEXT;
		subgraph.LineWidth = 24;
		subgraph.Name = "pnl all accounts";
		subgraph.PrimaryColor = RGB(0, 0, 0);
		subgraph.SecondaryColor = RGB(255, 255, 255);
		subgraph.SecondaryColorUsed = 1;

		int max_x = (int) CHART_DRAWING_MAX_HORIZONTAL_AXIS_RELATIVE_POSITION;
		x.Name.Format("x (1-%d)", max_x);
		x.SetInt(10);
		x.SetIntLimits(1, max_x);

		int max_y = (int) CHART_DRAWING_MAX_VERTICAL_AXIS_RELATIVE_POSITION;
		y.Name.Format("y (1-%d)", max_y);
		y.SetInt(90);
		y.SetIntLimits(1, max_y);

		bold.Name = "bold";
		bold.SetYesNo(false);

		open.Name = "open";
		open.SetYesNo(true);

		return;
	}

	int num_accounts = (int) sc.GetNumTradeAccounts();
	SCString account;
	n_ACSIL::s_TradeAccountDataFields account_data;
	double total = 0.00;

	for (int i = 0; i < num_accounts; i++) {
		sc.GetTradeAccountAtIndex(i, account);
		sc.GetTradeAccountData(account_data, account);

		total += account_data.m_DailyProfitLoss;
		if (open.GetBoolean()) {
			total += account_data.m_OpenPositionsProfitLoss;
		}
	}

	SCString output;
	output.Format("%.2f", total);

	sc.AddAndManageSingleTextDrawingForStudy(
		sc,
		false,
		x.GetInt(),
		y.GetInt(),
		subgraph,
		0,
		output,
		sc.DrawStudyUnderneathMainPriceGraph ? 0 : 1,
		(int) bold.GetBoolean()
	);
}
