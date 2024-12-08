#include <sierrachart.h>

#define persistent_pointer_id 0
#define maximum_snake_length 1024
#define tile_scale_factor 200

static const char* welcome_banner = R"(
           /^\/^\
         _|_O|  O|           "yeah, but can your
\/     /~     \_/ \        trading platform do this?"
 \____|__________/  \
        \_______      \
                `\     \                 \
                  |     |                  \
                 /      /                    \
                /     /     sierra snake      \\
              /      /                         \ \
             /     /                            \  \
           /     /             _----_            \   \
          /     /           _-~      ~-_         |   |
         (      (        _-~    _--_    ~-_     _/   |
          \      ~-____-~    _-~    ~-_    ~-_-~    /
            ~-_           _-~          ~-_       _-~
               ~--______-~                ~-___-~


                     movement: w, s, a, d
)";

static const char* game_over_banner = R"(
                                                       _
                                                      | |
      __ _  __ _ _ __ ___   ___    _____   _____ _ __ | |
     / _` |/ _` | '_ ` _ \ / _ \  / _ \ \ / / _ \ '__|| |
    | (_| | (_| | | | | | |  __/ | (_) \ V /  __/ |   |_|
     \__, |\__,_|_| |_| |_|\___|  \___/ \_/ \___|_|   (_)
      __/ |
     |___/

                   "dude, the market makers
                        hunted my stop!"


                      score: %d
)";

struct point
{
	int x, y;
};

struct chart_colors
{
	n_ACSIL::s_GraphicsColor text;
	n_ACSIL::s_GraphicsColor background;
	n_ACSIL::s_GraphicsColor snake;
	n_ACSIL::s_GraphicsColor food;
};

struct chart_dimensions
{
	int left, right, top, bottom;
	int width, height;
	struct point center;
	int tile_size;
};

enum game_phase
{
	game_phase_init,
	game_phase_welcome,
	game_phase_playing,
	game_phase_game_over
};

enum snake_direction
{
	snake_direction_left,
	snake_direction_right,
	snake_direction_up,
	snake_direction_down
};

struct persistent
{
	enum game_phase game_phase;

	n_ACSIL::s_GraphicsFont monospace_font;
	struct chart_colors chart_colors;
	struct chart_dimensions chart_dimensions;

	struct point snake[maximum_snake_length];
	enum snake_direction snake_direction;

	struct point food_location;

	int speed;
	int score;
};

n_ACSIL::s_GraphicsFont create_monospace_font ()
{
	n_ACSIL::s_GraphicsFont monospace_font;

	monospace_font.m_FaceName = "Consolas";
	monospace_font.m_Height = 12;
	monospace_font.m_Weight = FW_HEAVY;

	return monospace_font;
}

struct chart_colors get_chart_colors (SCStudyInterfaceRef sc)
{
	struct chart_colors chart_colors;

	uint32_t text;
	uint32_t background;
	uint32_t snake;
	uint32_t food;

	uint32_t unused_line_width;
	SubgraphLineStyles unused_line_style;

	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CHART_TEXT, text, unused_line_width, unused_line_style);
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CHART_BACKGROUND, background, unused_line_width, unused_line_style);
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_VALUE_CHANGE_UP, snake, unused_line_width, unused_line_style);
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_VALUE_CHANGE_DOWN, food, unused_line_width, unused_line_style);

	chart_colors.text.SetColorValue(text);
	chart_colors.background.SetColorValue(background);
	chart_colors.snake.SetColorValue(snake);
	chart_colors.food.SetColorValue(food);

	return chart_colors;
}

struct chart_dimensions get_chart_dimensions (SCStudyInterfaceRef sc)
{
	struct chart_dimensions chart_dimensions;

	chart_dimensions.left = sc.StudyRegionLeftCoordinate;
	chart_dimensions.right = sc.StudyRegionRightCoordinate;
	chart_dimensions.top = sc.StudyRegionTopCoordinate;
	chart_dimensions.bottom = sc.StudyRegionBottomCoordinate;

	chart_dimensions.width = chart_dimensions.right - chart_dimensions.left;
	chart_dimensions.height = chart_dimensions.bottom - chart_dimensions.top;

	chart_dimensions.center.x = chart_dimensions.right / 2;
	chart_dimensions.center.y = chart_dimensions.bottom / 2;

	int width_height_average = (chart_dimensions.width + chart_dimensions.height) / 2;
	chart_dimensions.tile_size = width_height_average / tile_scale_factor;

	return chart_dimensions;
}

struct persistent *get_persistent (SCStudyInterfaceRef sc) {
	struct persistent *persistent = (struct persistent *) sc.GetPersistentPointer(persistent_pointer_id);

	if (!persistent)
	{
		persistent = (struct persistent *) sc.AllocateMemory(sizeof(struct persistent));
		sc.SetPersistentPointer(persistent_pointer_id, persistent);
	}

	return persistent;
}

void free_persistent (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	if (persistent)
		sc.FreeMemory(persistent);

	sc.SetPersistentPointer(persistent_pointer_id, nullptr);
}

bool is_valid_keyboard_input (int key)
{
	if (!key)
		return false;

	int valid_keys[] = {'w', 's', 'a', 'd', 'r'};
	int valid_keys_length = sizeof(valid_keys) / sizeof(valid_keys[0]);

	for (int i = 0; i < valid_keys_length; i++)
		if (key == valid_keys[i])
			return true;

	return false;
}

void handle_keyboard_input (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	if (!is_valid_keyboard_input(sc.CharacterEventCode))
		return;

	switch (persistent->game_phase)
	{
		case game_phase_welcome:
			persistent->game_phase = game_phase_playing;
			break;

		case game_phase_playing:
			persistent->game_phase = game_phase_game_over;
			break;

		case game_phase_game_over:
			persistent->game_phase = game_phase_init;
			break;

		case game_phase_init:
		default:
			break;
        }

	switch (sc.CharacterEventCode)
	{
		case 'w':
			persistent->snake_direction = snake_direction_up;
			break;

		case 's':
			persistent->snake_direction = snake_direction_down;
			break;

		case 'a':
			persistent->snake_direction = snake_direction_left;
			break;

		case 'd':
			persistent->snake_direction = snake_direction_right;
			break;

		case 'r':
			persistent->game_phase = game_phase_init;
			break;

		default:
			break;
	}
}

void draw_banner(SCStudyInterfaceRef sc, const char *banner)
{
	struct persistent *persistent = get_persistent(sc);

	sc.Graphics.SetTextAlign(0);
	sc.Graphics.SetTextFont(persistent->monospace_font);
	sc.Graphics.SetTextColor(persistent->chart_colors.text);
	sc.Graphics.SetBackgroundColor(persistent->chart_colors.background);

	n_ACSIL::s_GraphicsRectangle banner_rect;
	sc.Graphics.DrawTextA(banner, banner_rect, DT_CALCRECT);

	int banner_width = banner_rect.Right - banner_rect.Left;
	int banner_height = banner_rect.Bottom - banner_rect.Top;

	int x = (persistent->chart_dimensions.right - banner_width) / 2;
	int y = (persistent->chart_dimensions.bottom - banner_height) / 2;

	banner_rect.Left = x;
	banner_rect.Right = x + banner_width;
	banner_rect.Top = y;
	banner_rect.Bottom = y + banner_height;

	sc.Graphics.DrawTextA(banner, banner_rect, 0);
}

void draw_welcome (HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	draw_banner(sc, welcome_banner);
}

void draw_playing (HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	// struct persistent *persistent = get_persistent(sc);
}

void draw_game_over (HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	draw_banner(sc, game_over_banner);
}

void set_defaults (SCStudyInterfaceRef sc)
{
	sc.GraphName = "sierrasnake";
	sc.GraphRegion = 0;

	sc.DisplayAsMainPriceGraph = 1;
	sc.DisplayStudyName = 0;

	sc.ReceiveCharacterEvents = 1;

	sc.UpdateAlways = 1;
}

SCDLLName("sierrasnake")

SCSFExport scsf_sierrasnake (SCStudyInterfaceRef sc)
{
	if (sc.SetDefaults)
		return set_defaults(sc);

	if (sc.HideStudy)
		return;

	struct persistent *persistent = get_persistent(sc);
	if (sc.LastCallToFunction)
		return free_persistent(sc, persistent);

	handle_keyboard_input(sc, persistent);

	switch (persistent->game_phase)
	{
		case game_phase_init:

			memset(persistent, 0, sizeof(*persistent));

			persistent->monospace_font = create_monospace_font();
			persistent->chart_colors = get_chart_colors(sc);
			persistent->chart_dimensions = get_chart_dimensions(sc);

			persistent->game_phase = game_phase_welcome;

			break;

		case game_phase_welcome:
			sc.p_GDIFunction = draw_welcome;
			break;

		case game_phase_playing:

			sc.p_GDIFunction = draw_playing;

			break;

		case game_phase_game_over:
			sc.p_GDIFunction = draw_game_over;
			break;

		default:
			persistent->game_phase = game_phase_init;
			break;
        }
}
