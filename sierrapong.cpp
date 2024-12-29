#include <sierrachart.h>

struct object
{
	int width;
	int height;
	int x;
	int y;
	int direction;
	int speed;
	n_ACSIL::s_GraphicsColor color;
};

struct persistent
{
	bool initialized;
	int region_width;
	int region_height;
	struct object paddle;
	struct object ball;
	int score;
	bool game_over;
};

void set_defaults (SCStudyInterfaceRef sc)
{
	sc.DisplayStudyName = 0;
	sc.GraphName = "sierrapong";
	sc.GraphRegion = 0;

	sc.ReceiveKeyboardKeyEvents = 1;
	sc.UpdateAlways = 1;
}

struct persistent* get_persistent (SCStudyInterfaceRef sc)
{
	struct persistent *p = (struct persistent *) sc.GetPersistentPointer(0);

	if (!p)
	{
		p = (struct persistent *) sc.AllocateMemory(sizeof(*p));
		sc.SetPersistentPointer(0, p);
	}

	return p;
}

void free_persistent (SCStudyInterfaceRef sc, struct persistent *p)
{
	if (!p)
		return;

	sc.FreeMemory(p);
	sc.SetPersistentPointer(0, nullptr);
}

void initialize (SCStudyInterfaceRef sc, struct persistent *p)
{
	memset(p, 0, sizeof(*p));

	p->initialized = true;

	p->region_width = sc.StudyRegionRightCoordinate;
	p->region_height = sc.StudyRegionBottomCoordinate;

	p->paddle.width = p->region_width / 5;
	p->paddle.height = p->region_width / 40;
	p->paddle.x = (p->region_width / 2) - (p->paddle.width / 2);
	p->paddle.y = p->region_height - (p->paddle.height * 2);
	p->paddle.speed = 10;
	p->paddle.color.SetRGB(98, 114, 164);

	p->ball.width = p->paddle.height;
	p->ball.height = p->ball.width;
	p->ball.x = p->paddle.x + (int) (p->paddle.width * 0.75);
	p->ball.y = p->paddle.y - p->ball.height;
	p->ball.speed = 10;
	p->ball.color.SetRGB(187, 147, 249);
}

void draw_paddle (HDC hdc, SCStudyInterfaceRef sc, struct persistent *p)
{
	n_ACSIL::s_GraphicsRectangle paddle;
	paddle.Left = p->paddle.x;
	paddle.Right = p->paddle.x + p->paddle.width;
	paddle.Top = p->paddle.y;
	paddle.Bottom = p->paddle.y + p->paddle.height;

        HBRUSH brush = CreateSolidBrush(p->paddle.color.Color.RawColor);
        HBRUSH old_brush = (HBRUSH) SelectObject(hdc, brush);

	sc.Graphics.DrawRectangle(paddle.Left, paddle.Top, paddle.Right, paddle.Bottom);

	SelectObject(hdc, old_brush);
}

void draw_ball (HDC hdc, SCStudyInterfaceRef sc, struct persistent *p)
{
	n_ACSIL::s_GraphicsRectangle ball;
	ball.Left = p->ball.x;
	ball.Right = p->ball.x + p->ball.width;
	ball.Top = p->ball.y;
	ball.Bottom = p->ball.y + p->ball.height;

        HBRUSH brush = CreateSolidBrush(p->ball.color.Color.RawColor);
        HBRUSH old_brush = (HBRUSH) SelectObject(hdc, brush);

	sc.Graphics.DrawEllipse(ball.Left, ball.Top, ball.Right, ball.Bottom);

	SelectObject(hdc, old_brush);
}

void draw_score (HDC hdc, SCStudyInterfaceRef sc, struct persistent *p)
{
	n_ACSIL::s_GraphicsFont font;
	font.m_FaceName = "Courier New";
	font.m_Height = p->paddle.height / 2;

	sc.Graphics.SetTextAlign(0);
	sc.Graphics.SetTextFont(font);

	SCString score;
	score.AppendFormat("score: %d", p->score);

	sc.Graphics.DrawTextAt(score, 0, font.m_Height);
}

void draw (HWND hwnd, HDC hdc, SCStudyInterfaceRef sc)
{
	struct persistent *p = get_persistent(sc);

	draw_paddle(hdc, sc, p);
	draw_ball(hdc, sc, p);
	draw_score(hdc, sc, p);
}

SCDLLName("sierrapong")
SCSFExport scsf_sierrapong (SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
		return set_defaults(sc);

	if (sc.HideStudy)
		return;

	struct persistent *p = get_persistent(sc);
	if (sc.LastCallToFunction)
		return free_persistent(sc, p);

	if (!p->initialized)
		initialize(sc, p);

	sc.p_GDIFunction = draw;
}
