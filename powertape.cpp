#include <sierrachart.h>

struct persistent
{
	bool initialized;
	uint32_t last_sequence;
	float last_price;
	uint64_t rs_volume;
	uint64_t is_volume;
	uint64_t ib_volume;
	uint64_t rb_volume;
	double total_volume;
	double rs_percent;
	double is_percent;
	double ib_percent;
	double rb_persent;
};

struct persistent *get_persistent (SCStudyInterfaceRef sc)
{
	struct persistent *persistent = (struct persistent *) sc.GetPersistentPointer(0);

	if (!persistent)
	{
		persistent = (struct persistent *) sc.AllocateMemory(sizeof(struct persistent));
		sc.SetPersistentPointer(0, persistent);
	}

	return persistent;
}

void free_persistent (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	if (persistent)
		sc.FreeMemory(persistent);

	sc.SetPersistentPointer(0, nullptr);
}

void set_defaults (SCStudyInterfaceRef sc)
{
	sc.DisplayStudyName = 0;
	sc.GraphName = "powertape";
	sc.GraphRegion = 0;
	sc.ReceiveCharacterEvents = 1;
	sc.UpdateAlways = 1;
}

void handle_input (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	switch (sc.CharacterEventCode)
	{
		case 'r':
			persistent->initialized = false;
			break;

		default:
			break;
	}
}

void init (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	memset(persistent, 0, sizeof(*persistent));
	persistent->initialized = true;
}

void update (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	c_SCTimeAndSalesArray tape_data;
	sc.GetTimeAndSales(tape_data);

	if (!tape_data.Size())
		return;

	if (!persistent->last_sequence)
		persistent->last_sequence = tape_data.GetSequenceNumberOfLastElement();

	if (persistent->last_price <= 0)
		persistent->last_price = sc.LastTradePrice;

	int next_record_index = (int) tape_data.GetRecordIndexAtGreaterThanSequenceNumber(persistent->last_sequence);
	int last_record_index = tape_data.Size() - 1;

	for (int i = next_record_index; i <= last_record_index; i++)
	{
		struct s_TimeAndSales record = tape_data[i];

		if (record.Sequence <= persistent->last_sequence)
			continue;

		switch (record.Type)
		{
			case SC_TS_ASK:

				persistent->ib_volume += record.Volume;

				if (!sc.FormattedEvaluate(record.Price, (unsigned int) sc.BaseGraphValueFormat, GREATER_OPERATOR, persistent->last_price, (unsigned int) sc.BaseGraphValueFormat))
					persistent->rs_volume += record.Volume;

				break;

			case SC_TS_BID:

				persistent->is_volume += record.Volume;

				if (!sc.FormattedEvaluate(record.Price, (unsigned int) sc.BaseGraphValueFormat, LESS_OPERATOR, persistent->last_price, (unsigned int) sc.BaseGraphValueFormat))
					persistent->rb_volume += record.Volume;

				break;

			default:

				// skip bid ask updates
				continue;
		}

		persistent->last_sequence = record.Sequence;
		persistent->last_price = record.Price;

		persistent->total_volume = (double) (persistent->rs_volume + persistent->is_volume + persistent->ib_volume + persistent->rb_volume);
		persistent->rs_percent = (double) persistent->rs_volume / persistent->total_volume;
		persistent->is_percent = (double) persistent->is_volume / persistent->total_volume;
		persistent->ib_percent = (double) persistent->ib_volume / persistent->total_volume;
		persistent->rb_persent = (double) persistent->rb_volume / persistent->total_volume;

		SCString log;
		log.AppendFormat
		(
			"#:%d type:%d price:%.2f | rs:%d is:%d ib:%d rb:%d | rs:%.4f is:%.4f ib:%.4f rb:%.4f",
			record.Sequence,
			record.Type,
			(double) record.Price,
			persistent->rs_volume,
			persistent->is_volume,
			persistent->ib_volume,
			persistent->rb_volume,
			persistent->rs_percent,
			persistent->is_percent,
			persistent->ib_percent,
			persistent->rb_persent
		);
		sc.AddMessageToLog(log, 1);
	}
}

void draw (HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	struct persistent *persistent = get_persistent(sc);

	int region_width = sc.StudyRegionRightCoordinate;
	int region_height = sc.StudyRegionBottomCoordinate;

	n_ACSIL::s_GraphicsColor rs_color;
	n_ACSIL::s_GraphicsColor is_color;
	n_ACSIL::s_GraphicsColor ib_color;
	n_ACSIL::s_GraphicsColor rb_color;

	rs_color.SetRGB(98, 114, 164);
	is_color.SetRGB(255, 85, 85);
	ib_color.SetRGB(80, 250, 123);
	rb_color.SetRGB(98, 114, 164);

	n_ACSIL::s_GraphicsRectangle rs;
	n_ACSIL::s_GraphicsRectangle is;
	n_ACSIL::s_GraphicsRectangle ib;
	n_ACSIL::s_GraphicsRectangle rb;

	rs.Left = 0;
	is.Left = 0;
	ib.Left = 0;
	rb.Left = 0;
	rs.Right = region_width;
	is.Right = region_width;
	ib.Right = region_width;
	rb.Right = region_width;

	rs.Top = 0;
	rs.Bottom = rs.Top + (int) (region_height * persistent->rs_percent);

	is.Top = rs.Bottom;
	is.Bottom = is.Top + (int) (region_height * persistent->is_percent);

	ib.Top = is.Bottom;
	ib.Bottom = ib.Top + (int) (region_height * persistent->ib_percent);

	rb.Top = ib.Bottom;
	rb.Bottom = rb.Top + (int) (region_height * persistent->rb_persent);

	sc.Graphics.FillRectangleWithColor(rs, rs_color);
	sc.Graphics.FillRectangleWithColor(is, is_color);
	sc.Graphics.FillRectangleWithColor(ib, ib_color);
	sc.Graphics.FillRectangleWithColor(rb, rb_color);

	n_ACSIL::s_GraphicsFont font;
	font.m_FaceName = "Courier New";
	font.m_Height = 12;
	sc.Graphics.SetTextAlign(0);
	sc.Graphics.SetTextFont(font);

	SCString rs_txt, is_txt, ib_txt, rb_txt;
	rs_txt.Format("%0.2f%%", persistent->rs_percent * 100);
	is_txt.Format("%0.2f%%", persistent->is_percent * 100);
	ib_txt.Format("%0.2f%%", persistent->ib_percent * 100);
	rb_txt.Format("%0.2f%%", persistent->rb_persent * 100);

	sc.Graphics.DrawTextAt(rs_txt, rs.Left, rs.Top + (rs.Bottom - rs.Top) / 2);
	sc.Graphics.DrawTextAt(is_txt, is.Left, is.Top + (is.Bottom - is.Top) / 2);
	sc.Graphics.DrawTextAt(ib_txt, ib.Left, ib.Top + (ib.Bottom - ib.Top) / 2);
	sc.Graphics.DrawTextAt(rb_txt, rb.Left, rb.Top + (rb.Bottom - rb.Top) / 2);
}

SCDLLName("powertape")
SCSFExport scsf_powertape (SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
		return set_defaults(sc);

	if (sc.IsFullRecalculation)
		return;

	struct persistent *persistent = get_persistent(sc);
	if (sc.LastCallToFunction)
		return free_persistent(sc, persistent);

	handle_input(sc, persistent);

	if (!persistent->initialized)
		init(sc, persistent);

	update(sc, persistent);

	sc.p_GDIFunction = draw;

	if (sc.IsNewBar(sc.IndexOfLastVisibleBar))
	{
		persistent->initialized = false;
		return;
	}
}
