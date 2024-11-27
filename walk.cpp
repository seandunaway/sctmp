#include <sierrachart.h>
#include <stdlib.h>
#include <time.h>

SCDLLName("walk")

SCSFExport scsf_walk(SCStudyInterfaceRef sc) {
	int input = -1;
	SCInputRef button = sc.Input[++input];
	SCInputRef delay = sc.Input[++input];
	SCInputRef probability = sc.Input[++input];
	SCInputRef reward = sc.Input[++input];
	SCInputRef risk = sc.Input[++input];

	if (sc.SetDefaults) {
		sc.AllowOnlyOneTradePerBar = 0;
		sc.DisplayStudyName = 0;
		sc.GraphName = "walk";
		sc.GraphRegion = 0;
		sc.MaintainTradeStatisticsAndTradesData = 1;

		button.Name = "button";
		button.SetInt(1);
		button.SetIntLimits(1, MAX_ACS_CONTROL_BAR_BUTTONS);
		delay.Name = "delay";
		delay.SetTimeAsSCDateTime(SCDateTime(0, 5, 0, 0));
		probability.Name = "probability";
		probability.SetInt(50);
		probability.SetIntLimits(0, 100);
		reward.Name = "reward";
		reward.SetDouble(2);
		reward.SetDoubleLimits(0, 100);
		risk.Name = "risk";
		risk.SetDouble(20);
		risk.SetDoubleLimits(0, 100);

		return;
	}

	if (sc.UpdateStartIndex == 0) {
		sc.SetCustomStudyControlBarButtonHoverText(button.GetInt(), "walk");
		sc.SetCustomStudyControlBarButtonShortCaption(button.GetInt(), "walk");
		sc.SetCustomStudyControlBarButtonText(button.GetInt(), "walk");

		srand((unsigned int) time(nullptr));

		return;
	}

	if (!sc.GetCustomStudyControlBarButtonEnableState(button.GetInt())) return;

	s_SCPositionData position;
	sc.GetTradePosition(position);
	if (position.PositionQuantity != 0.0 || position.WorkingOrdersExist) return;

	SCDateTime& timestamp = sc.GetPersistentSCDateTimeFast(0);
	if (timestamp + delay.GetDateTime() > sc.LatestDateTimeForLastBar) return;
	timestamp = sc.LatestDateTimeForLastBar;

	sc.SendOrdersToTradeService = !sc.GlobalTradeSimulationIsOn;

	s_SCNewOrder order;
	order.OrderQuantity = sc.TradeWindowOrderQuantity;
	order.OrderType = SCT_ORDERTYPE_MARKET;
	order.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
	order.Target1Offset = sc.RoundToTickSize(reward.GetDouble());
	order.Stop1Offset = sc.RoundToTickSize(risk.GetDouble());
	order.TextTag = "walk";

	bool coinflip = (rand() % 100) < probability.GetInt();
	if (coinflip) sc.BuyEntry(order);
	else sc.SellEntry(order);
}
