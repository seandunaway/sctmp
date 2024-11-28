#include <sierrachart.h>
#include <stdlib.h>
#include <time.h>

SCDLLName("walk")

SCSFExport scsf_walk(SCStudyInterfaceRef sc) {
	int input = -1;
	int persistent = -1;

	SCInputRef button = sc.Input[++input];
	SCInputRef delay = sc.Input[++input];
	SCInputRef delay_random = sc.Input[++input];
	SCInputRef probability = sc.Input[++input];
	SCInputRef reward = sc.Input[++input];
	SCInputRef risk = sc.Input[++input];
	SCInputRef tag = sc.Input[++input];

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
		delay.SetTimeAsSCDateTime(SCDateTime(0, 10, 0, 0));
		delay_random.Name = "delay random";
		delay_random.SetYesNo(1);
		probability.Name = "probability";
		probability.SetInt(50);
		probability.SetIntLimits(0, 100);
		reward.Name = "reward";
		reward.SetDouble(2);
		reward.SetDoubleLimits(0, 100);
		risk.Name = "risk";
		risk.SetDouble(20);
		risk.SetDoubleLimits(0, 100);
		tag.Name = "tag";
		tag.SetString("walk");

		return;
	}

	if (sc.UpdateStartIndex == 0) {
		sc.SetCustomStudyControlBarButtonHoverText(button.GetInt(), tag.GetString());
		sc.SetCustomStudyControlBarButtonShortCaption(button.GetInt(), tag.GetString());
		sc.SetCustomStudyControlBarButtonText(button.GetInt(), tag.GetString());

		srand((unsigned int) time(nullptr));

		return;
	}

	if (!sc.GetCustomStudyControlBarButtonEnableState(button.GetInt())) return;

	s_SCPositionData position;
	sc.GetTradePosition(position);
	if (position.PositionQuantity != 0.0 || position.WorkingOrdersExist) return;

	SCDateTime& timestamp = sc.GetPersistentSCDateTimeFast(++persistent);
	if (timestamp >= sc.LatestDateTimeForLastBar) return;

	SCDateTime& lastexit = sc.GetPersistentSCDateTimeFast(++persistent);
	if (lastexit < position.LastExitDateTime) {
		lastexit = position.LastExitDateTime;

		int timestamp_delay = delay.GetDateTime().GetTimeInSeconds();
		if (delay_random.GetYesNo()) timestamp_delay = rand() % timestamp_delay;

		timestamp = lastexit;
		timestamp.AddSeconds(timestamp_delay);

		return;
	}

	sc.SendOrdersToTradeService = !sc.GlobalTradeSimulationIsOn;

	s_SCNewOrder order;
	order.OrderQuantity = sc.TradeWindowOrderQuantity;
	order.OrderType = SCT_ORDERTYPE_MARKET;
	order.TimeInForce = SCT_TIF_GOOD_TILL_CANCELED;
	order.Target1Offset = sc.RoundToTickSize(reward.GetDouble());
	order.Stop1Offset = sc.RoundToTickSize(risk.GetDouble());
	order.TextTag = tag.GetString();

	bool coinflip = (rand() % 100) < probability.GetInt();
	if (coinflip) sc.BuyEntry(order);
	else sc.SellEntry(order);
}
