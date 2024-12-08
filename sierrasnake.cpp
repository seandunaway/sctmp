#include <sierrachart.h>

#define persistent_pointer_id 0
#define maximum_banner_length 1024
#define maximum_snake_length 1024
#define tile_scale_factor 25
#define font_height_factor 4
#define snake_growth_rate 2

static const char welcome_banner[maximum_banner_length] = R"(
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
                     pause: p    reset: r
)";

static const char game_over_banner[maximum_banner_length] = R"(
          "dude, the market makers
               hunted my stop!"                        _
                                                      | |
      __ _  __ _ _ __ ___   ___    _____   _____ _ __ | |
     / _` |/ _` | '_ ` _ \ / _ \  / _ \ \ / / _ \ '__|| |
    | (_| | (_| | | | | | |  __/ | (_) \ V /  __/ |   |_|
     \__, |\__,_|_| |_| |_|\___|  \___/ \_/ \___|_|   (_)
      __/ |
     |___/
                          score: %d


                    press r to try again!
)";

struct tile
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
	int tile_size;
	int horizontal_tiles, vertical_tiles;
	struct tile center_tile;
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
	bool paused;

	n_ACSIL::s_GraphicsFont monospace_font;
	struct chart_colors chart_colors;
	struct chart_dimensions chart_dimensions;

	int snake_length;
	struct tile snake[maximum_snake_length];
	enum snake_direction snake_direction;

	struct tile food_location;
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
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CANDLESTICK_UP_FILL, snake, unused_line_width, unused_line_style);
	sc.GetGraphicsSetting(sc.ChartNumber, n_ACSIL::GRAPHICS_SETTING_CANDLESTICK_DOWN_FILL, food, unused_line_width, unused_line_style);

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

	int width_height_average = (chart_dimensions.width + chart_dimensions.height) / 2;
	chart_dimensions.tile_size = width_height_average / tile_scale_factor;

	chart_dimensions.horizontal_tiles = chart_dimensions.width / chart_dimensions.tile_size;
	chart_dimensions.vertical_tiles = chart_dimensions.height / chart_dimensions.tile_size;
	chart_dimensions.center_tile.x = chart_dimensions.horizontal_tiles / 2;
	chart_dimensions.center_tile.y = chart_dimensions.vertical_tiles / 2;

	return chart_dimensions;
}

n_ACSIL::s_GraphicsRectangle center_rectangle (struct chart_dimensions chart_dimensions, n_ACSIL::s_GraphicsRectangle rectangle)
{
	int width = rectangle.Right - rectangle.Left;
	int height = rectangle.Bottom - rectangle.Top;

	int x = (chart_dimensions.right - width) / 2;
	int y = (chart_dimensions.bottom - height) / 2;

	n_ACSIL::s_GraphicsRectangle centered_rectangle;

	centered_rectangle.Left = x;
	centered_rectangle.Right = x + width;
	centered_rectangle.Top = y;
	centered_rectangle.Bottom = y + height;

	return centered_rectangle;
}

n_ACSIL::s_GraphicsRectangle tile_to_rectangle (struct tile tile, int tile_size)
{
	n_ACSIL::s_GraphicsRectangle rectangle;

	rectangle.Left = tile.x * tile_size;
	rectangle.Right = rectangle.Left + tile_size;
	rectangle.Top = tile.y * tile_size;
	rectangle.Bottom = rectangle.Top + tile_size;

	return rectangle;
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

	int valid_keys[] = {'w', 's', 'a', 'd', 'p', 'r'};
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

		case game_phase_init:
		case game_phase_playing:
		case game_phase_game_over:
		default:
			break;
        }

	switch (sc.CharacterEventCode)
	{
		case 'w':
			if (persistent->snake_direction != snake_direction_down)
				persistent->snake_direction = snake_direction_up;
			break;

		case 's':
			if (persistent->snake_direction != snake_direction_up)
				persistent->snake_direction = snake_direction_down;
			break;

		case 'a':
			if (persistent->snake_direction != snake_direction_right)
				persistent->snake_direction = snake_direction_left;
			break;

		case 'd':
			if (persistent->snake_direction != snake_direction_left)
				persistent->snake_direction = snake_direction_right;
			break;

		case 'p':
			persistent->paused = !persistent->paused;
			break;

		case 'r':
			persistent->game_phase = game_phase_init;
			break;

		default:
			break;
	}
}

void move_food (struct persistent *persistent)
{
	persistent->food_location.x = rand() % persistent->chart_dimensions.horizontal_tiles;
	persistent->food_location.y = rand() % persistent->chart_dimensions.vertical_tiles;
}

void move_snake (struct persistent *persistent)
{
	for (int i = persistent->snake_length; i > 0; i--)
		persistent->snake[i] = persistent->snake[i - 1];

	switch (persistent->snake_direction)
	{
		case snake_direction_left:
			persistent->snake[0].x--;
			break;

		case snake_direction_right:
			persistent->snake[0].x++;
			break;

		case snake_direction_up:
			persistent->snake[0].y--;
			break;

		case snake_direction_down:
			persistent->snake[0].y++;
			break;

		default:
			break;
        }
}

void wrap_snake (struct persistent *persistent)
{
	if (persistent->snake[0].x >= persistent->chart_dimensions.horizontal_tiles)
		persistent->snake[0].x = 0;

	if (persistent->snake[0].x < 0)
		persistent->snake[0].x = persistent->chart_dimensions.horizontal_tiles - 1;

	if (persistent->snake[0].y >= persistent->chart_dimensions.vertical_tiles)
		persistent->snake[0].y = 0;

	if (persistent->snake[0].y < 0)
		persistent->snake[0].y = persistent->chart_dimensions.vertical_tiles - 1;
}

bool is_collision (struct tile a, struct tile b)
{
	return a.x == b.x && a.y == b.y;
}

void handle_food_collision (struct persistent *persistent)
{
	if (!is_collision(persistent->snake[0], persistent->food_location))
		return;

	if (persistent->snake_length > maximum_snake_length)
		return;

	persistent->snake_length += snake_growth_rate;
	persistent->score++;

	move_food(persistent);
}

void handle_snake_collision (struct persistent *persistent)
{
	for (int i = 1; i <= persistent->snake_length; i++)
	{
		if (!is_collision(persistent->snake[0], persistent->snake[i]))
			continue;

		persistent->game_phase = game_phase_game_over;
	}
}

void draw_banner (SCStudyInterfaceRef sc, struct persistent *persistent, const char *banner, ...)
{
	char banner_replaced[maximum_banner_length];

	va_list replacement_values;
	va_start(replacement_values, banner);
	vsnprintf(banner_replaced, sizeof(banner_replaced), banner, replacement_values);
	va_end(replacement_values);

	persistent->monospace_font.m_Height = persistent->chart_dimensions.tile_size / font_height_factor;

	sc.Graphics.SetTextAlign(0);
	sc.Graphics.SetTextFont(persistent->monospace_font);
	sc.Graphics.SetTextColor(persistent->chart_colors.text);
	sc.Graphics.SetBackgroundColor(persistent->chart_colors.background);

	n_ACSIL::s_GraphicsRectangle rectangle;
	sc.Graphics.DrawTextA(banner_replaced, rectangle, DT_CALCRECT);

	n_ACSIL::s_GraphicsRectangle centered_rectangle = center_rectangle(persistent->chart_dimensions, rectangle);
	sc.Graphics.FillRectangleWithColor(centered_rectangle, persistent->chart_colors.background);
	sc.Graphics.DrawTextA(banner_replaced, centered_rectangle, 0);
}

void draw_snake (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	for (int i = 0; i <= persistent->snake_length; i++)
	{
		n_ACSIL::s_GraphicsRectangle snake_segment = tile_to_rectangle(persistent->snake[i], persistent->chart_dimensions.tile_size);
		sc.Graphics.FillRectangleWithColor(snake_segment, persistent->chart_colors.snake);
	}
}

void draw_food (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	n_ACSIL::s_GraphicsRectangle food = tile_to_rectangle(persistent->food_location, persistent->chart_dimensions.tile_size);
	sc.Graphics.FillRectangleWithColor(food, persistent->chart_colors.food);
}

void draw (HWND WindowHandle, HDC DeviceContext, SCStudyInterfaceRef sc)
{
	struct persistent *persistent = get_persistent(sc);

	switch (persistent->game_phase)
	{
		case game_phase_welcome:
			draw_banner(sc, persistent, welcome_banner);
			break;

		case game_phase_playing:
			draw_snake(sc, persistent);
			draw_food(sc, persistent);
			break;

		case game_phase_game_over:
			draw_banner(sc, persistent, game_over_banner, persistent->score);
			break;

		case game_phase_init:
		default:
			break;
        }
}

void initialize (SCStudyInterfaceRef sc, struct persistent *persistent)
{
	memset(persistent, 0, sizeof(*persistent));

	persistent->monospace_font = create_monospace_font();
	persistent->chart_colors = get_chart_colors(sc);
	persistent->chart_dimensions = get_chart_dimensions(sc);

	memset(persistent->snake, -1, sizeof(persistent->snake));
	persistent->snake[0] = persistent->chart_dimensions.center_tile;

	srand((unsigned int) time(nullptr));
	move_food(persistent);

	persistent->game_phase = game_phase_welcome;
}

void set_defaults (SCStudyInterfaceRef sc)
{
	sc.GraphName = "sierrasnake";
	sc.GraphRegion = 0;
	// sc.DisplayAsMainPriceGraph = 1;
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

	if (persistent->game_phase == game_phase_init)
		initialize(sc, persistent);

	if (persistent->paused)
		return;

	move_snake(persistent);
	wrap_snake(persistent);

	handle_food_collision(persistent);
	handle_snake_collision(persistent);

	sc.p_GDIFunction = draw;
}
