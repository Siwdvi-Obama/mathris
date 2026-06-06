int GRID_WIDTH = 20, GRID_HEIGHT = 10, MIN_GRID_HEIGHT = 20, INTERVAL = 30, GRID_SIZE = 32, SAVE_TYPE = 1;
bool SAVING = 0;

bool DEBUG_ = true;

#include "raylib.h"
#include "siwlib.hpp"
#include "constants.h"

Font font;

struct TextBox
{
	int MAX_INPUT_CHARS;
	string name = "";
    Rectangle textBox;
    int letterCount = 0;
    bool mouseOnText = false, active = false;
	int framesCounter = 0;
	TextBox();
	TextBox(int chars){MAX_INPUT_CHARS = chars;}
	TextBox(Rectangle rec, int chars);
	const char* data(){
		return name.data();
	}
	bool is_empty(){
		return letterCount == 0;
	}
    void update(Camera2D cam=Camera2D{0, 0, 0, 1}){
        if (CheckCollisionPointRec(GetMousePositionCam(cam), textBox)) mouseOnText = true;
        else mouseOnText = false;

		if (mouseOnText){
			SetMouseCursor(MOUSE_CURSOR_IBEAM);
		}

		if (mouseOnText && IsMouseButtonPressed(0)){
			active = true;
		}
		else if (IsMouseButtonPressed(0)){
			active = false;
		}

        if (active)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 48) && (key <= 57) && (letterCount < MAX_INPUT_CHARS))
                {
                    name[letterCount] = (char)key;
                    name[letterCount+1] = '\0';
                    letterCount++;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE))
            {
                letterCount--;
                if (letterCount < 0) letterCount = 0;
                name[letterCount] = '\0';
            }
        }

        if (active) framesCounter++;
        else framesCounter = 0;
	};
	void draw(){
        DrawRectangleRec(textBox, LIGHTGRAY);
        if (active) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, RED);
        else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
        DrawTextEx(font, name.data(), Vector2{textBox.x + 2, textBox.y - 1}, 32, 2, BLACK);

        if (active)
        {
            if (letterCount < MAX_INPUT_CHARS)
            {
                if (((framesCounter/20)%2) == 0) DrawTextEx(font, "_", Vector2{textBox.x + 5 + MeasureTextEx(font, name.data(), 32, 2).x, textBox.y + 8}, 20, 2, BLACK);
            }
        }
    };
};
TextBox::TextBox()
{
	MAX_INPUT_CHARS = 9;
    textBox = { GetScreenWidth()/2.0f - 100, 180, 225, 50 };
}
TextBox::TextBox(Rectangle rec, int chars)
{
	MAX_INPUT_CHARS = chars;
    textBox = rec;
}

bool IsAnyNumberPressed()
{
    bool keyPressed = false;
    int key = GetKeyPressed();

    if ((key >= 48) && (key <= 57)) keyPressed = true;

    return keyPressed;
}

struct Button
{
	Rectangle rec;
	Texture2D texture;
	Color col;
	Color line_col;
	float line_thick;
	float roundness;
	bool active = 0;

	bool down = 0, released = 0;
	int animation = 0;

	Button(){};
	Button(Rectangle r, Texture2D tex, Color c=RAYWHITE, Color lc=BLACK, float lt=1, float ro=0)
	{
		rec = r;
		texture = tex;
		col = c;
		line_col = lc;
		line_thick = lt;
		roundness = ro;
	}
	bool is_pressed()
	{
		return released;
	}
	void update(Camera2D cam=Camera2D{0, 0, 0, 1})
	{
		if (released) released = 0;
		bool mouse_on_button = 0;
		if (CheckCollisionPointRec({GetMousePosition().x + cam.target.x, GetMousePosition().y + cam.target.y}, {rec.x - rec.width / 2, rec.y - rec.height / 2, rec.width, rec.height})){
			mouse_on_button = 1;
		}
		if (down && IsMouseButtonReleased(0) && !IsKeyDown(KEY_SPACE) || active && IsKeyReleased(KEY_SPACE) && (!mouse_on_button || !IsMouseButtonDown(0))){
			down = 0;
			released = 1;
			animation = 11;
		}
		if (!mouse_on_button && (!active || !IsKeyDown(KEY_SPACE))){
			down = 0;
			if (animation > 0 && animation < 11){
				animation = 11;
			}
		}
		if ((mouse_on_button && IsMouseButtonPressed(0) || active && IsKeyPressed(KEY_SPACE)) && !down){
			down = 1;
			animation = 1;
			active = 1;
		}
		if (!mouse_on_button && IsMouseButtonPressed(0)){
			active = 0;
		}
		if (animation != 0 && animation != 10){
			animation++;
		}
		if (animation == 50){
			animation = 0;
		}
	}
	void draw()
	{
		float scale = 1;
		if (animation < 11 && animation > 0){
			scale = EaseSineOut(animation - 1, 1, -0.2, 10);
		}
		else if (animation > 10){
			scale = EaseElasticOut(animation - 11, 0.8, 0.2, 40);
		}
		Rectangle draw_rec = {rec.x - rec.width * scale / 2, rec.y - rec.height * scale / 2, rec.width * scale, rec.height * scale};
		DrawRectangleRounded(draw_rec, roundness, min(roundness, 10.0f), col);
		DrawTexturePro(texture, {0, 0, float(texture.width), float(texture.height)}, draw_rec, {0, 0}, 0, WHITE);
		DrawRectangleRoundedLinesEx(draw_rec, roundness, min(roundness, 10.0f), line_thick, (active ? RED : line_col));
	}
};

float next_option_draw_y = 0;
int next_option_id = 0;
int current_option_id = -1;
int switch_to_option_id = -1;

class Op_Separator
{
	private:
		float height;
		Color tint;
		float self_y;
	public:
		Op_Separator(float h, Color t){height = h; tint = t;}
		void update(Camera2D cam=Camera2D{0, 0, 0, 1})
		{
			self_y = next_option_draw_y;
			next_option_draw_y += height;
		}
		void draw()
		{
			DrawRectangle(0, self_y, float(GetScreenWidth()), height, tint);
		}
};
class Op_Textbox
{
	private:
		string name;
		float self_y;
		int self_id;
		bool highlighted = 0;
		int highlight_frame = 0;
		Color highlight_color;
	public:
		TextBox textbox;
		Op_Textbox(string s, int chars){name = s; textbox = TextBox(chars);}
		void update(Camera2D cam=Camera2D{0, 0, 0, 1})
		{
			textbox.textBox = {float(GetScreenWidth() - textbox.MAX_INPUT_CHARS * 16 - 3), next_option_draw_y + 2, float(textbox.MAX_INPUT_CHARS * 16), 32.0f};
			textbox.update(cam);
			self_y = next_option_draw_y;
			self_id = next_option_id;
			next_option_draw_y += 36;
			next_option_id++;
			if (switch_to_option_id != -1){
				if (switch_to_option_id == self_id){
					textbox.active = 1;
				}
				else{
					textbox.active = 0;
				}
			}
			if (textbox.active){
				current_option_id = self_id;
			}
			highlight_frame++;
			if (CheckCollisionPointRec(GetMousePositionCam(cam), {0, self_y, float(GetScreenWidth()), 36})) highlighted = 0;
		}
		void draw()
		{
			DrawRectangle(0, self_y, float(GetScreenWidth()), 36, (highlighted && highlight_frame % 40 < 20 ? highlight_color : (self_id % 2 == 0 ? RAYWHITE : LIGHTERGRAY)));
			DrawRectangleLines(0, self_y, float(GetScreenWidth()), 36, BLACK);
			DrawTextEx(font, name.data(), {3.0f, self_y + 2}, 32, 0, BLACK);
			textbox.draw();
		}
		void highlight(Color tint)
		{
			highlighted = 1;
			highlight_frame = 0;
			highlight_color = tint;
		}
		const char* data()
		{
			return textbox.data();
		}
		bool is_empty()
		{
			return textbox.is_empty();
		}
};
Texture2D Check;
Texture2D Nothing;
class Op_Toggle
{
	private:
		string name;
		bool toggle = 0;
		float self_y;
		int self_id;
		bool highlighted = 0;
		int highlight_frame = 0;
		Color highlight_color;
	public:
		Button button;
		Op_Toggle(string s, bool toggld=0){name = s; button = Button({0, 0, 0, 0}, (toggld ? Check : Nothing)); toggle = toggld;}
		void update(Camera2D cam=Camera2D{0, 0, 0, 1})
		{
			button.rec = {float(GetScreenWidth() - 18), next_option_draw_y + 18, 28.0f, 28.0f};
			button.update(cam);
			if (button.is_pressed()){
				toggle = !toggle;
				button.texture = (toggle ? Check : Nothing);
			}
			self_y = next_option_draw_y;
			self_id = next_option_id;
			next_option_draw_y += 36;
			next_option_id++;
			if (switch_to_option_id != -1){
				if (switch_to_option_id == self_id){
					button.active = 1;
				}
				else{
					button.active = 0;
				}
			}
			if (button.active){
				current_option_id = self_id;
			}
			highlight_frame++;
			if (CheckCollisionPointRec(GetMousePositionCam(cam), {0, self_y, float(GetScreenWidth()), 36})) highlighted = 0;
		}
		void draw()
		{
			DrawRectangle(0, self_y, float(GetScreenWidth()), 36, (highlighted && highlight_frame % 40 < 20 ? highlight_color : (self_id % 2 == 0 ? RAYWHITE : LIGHTERGRAY)));
			DrawRectangleLines(0, self_y, float(GetScreenWidth()), 36, BLACK);
			DrawTextEx(font, name.data(), {3.0f, self_y + 2}, 32, 0, BLACK);
			button.draw();
		}
		void highlight(Color tint)
		{
			highlighted = 1;
			highlight_frame = 0;
			highlight_color = tint;
		}
		bool toggled()
		{
			return toggle;
		}
};
class Op_Choose
{
	private:
		vector <string> names;
		int _chosen = 0;
		float self_y;
	public:
		Op_Choose(vector <string> s){names = s;}
		void update(Camera2D cam=Camera2D{0, 0, 0, 1})
		{
			for (int i = 0; i < names.size(); i++){
				if (IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePositionCam(cam), {float(i * GetScreenWidth() / names.size()), next_option_draw_y, float(GetScreenWidth() / names.size()), 36.0f})){
					_chosen = i;
				}
			}
			self_y = next_option_draw_y;
			next_option_draw_y += 36;
		}
		void draw()
		{
			for (int i = 0; i < names.size(); i++){
				Rectangle rec = {float(i * GetScreenWidth() / names.size()), self_y, float(GetScreenWidth() / names.size()), 36.0f};
				DrawRectangleRec(rec, (i == _chosen ? SKYBLUE : RAYWHITE));
				DrawRectangleLinesEx(rec, 1, BLACK);
				DrawTextEx(font, names[i].data(), {rec.x + rec.width / 2 - MeasureTextEx(font, names[i].data(), 32, 0).x / 2, self_y + 2}, 32, 0, BLACK);
			}
		}
		int chosen()
		{
			return _chosen;
		}
};
class Op_Button
{
	private:
		float height;
		Texture2D texture;
		float self_y;
		bool pressed;
	public:
		Op_Button(float h, Texture2D tex){height = h; texture = tex;}
		void update(Camera2D cam=Camera2D{0, 0, 0, 1})
		{
			pressed = 0;
			if (CheckCollisionPointRec(GetMousePositionCam(cam), {0, next_option_draw_y, float(GetScreenWidth()), height})){
				SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
				if (IsMouseButtonPressed(0)){
					pressed = 1;
				}
			}
			self_y = next_option_draw_y;
			next_option_draw_y += height;
		}
		void draw()
		{
			DrawRectangle(0, self_y, GetScreenWidth(), height, RAYWHITE);
			DrawRectangleLinesEx({0, self_y, float(GetScreenWidth()), height}, 2, BLACK);
			bool squish_hor = GetScreenWidth() / height < texture.width / texture.height;
			float scale = (squish_hor ? GetScreenWidth() / float(texture.width) : height / texture.height);
			DrawTexturePro(texture, {0, 0, float(texture.width), float(texture.height)}, 
						   {GetScreenWidth() / 2.0f - texture.width * scale / 2, self_y + height / 2 - texture.height * scale / 2, texture.width * scale, texture.height * scale}, 
						   {0, 0}, 0, WHITE);
		}
		bool is_pressed()
		{
			return pressed;
		}
};

// datetime vibecodeeeeeee
bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
int days_in_month(int year, int month) {
    const int month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) return 29;
    return month_days[month - 1];
}
struct DateTime {
    int year, month, day;
    int hour, minute, second;
};
DateTime seconds_to_datetime(long long seconds_since_epoch) {
    // Step 1: extract whole days and remaining seconds
    long long days = seconds_since_epoch / 86400;
    int seconds_in_day = seconds_since_epoch % 86400;
    if (seconds_in_day < 0) { // handle negative input (optional)
        seconds_in_day += 86400;
        days -= 1;
    }

    // Step 2: compute year, month, day from days
    int year = 1970;
    while (true) {
        int days_this_year = is_leap_year(year) ? 366 : 365;
        if (days < days_this_year) break;
        days -= days_this_year;
        ++year;
    }

    // days now is 0?based day?of?year (0 = Jan 1)
    int month = 1;
    while (true) {
        int days_this_month = days_in_month(year, month);
        if (days < days_this_month) break;
        days -= days_this_month;
        ++month;
    }
    int day_of_month = days + 1;  // convert to 1?based

    // Step 3: extract hour, minute, second
    int hour = seconds_in_day / 3600;
    int minute = (seconds_in_day % 3600) / 60;
    int second = seconds_in_day % 60;

    return {year, month, day_of_month, hour, minute, second};
}
string format_datetime(const DateTime& dt) {
    ostringstream oss;
    oss << setw(2) << setfill('0') << dt.day << "."
        << setw(2) << setfill('0') << dt.month << "."
        << dt.year << " "
        << setw(2) << setfill('0') << dt.hour << ":"
        << setw(2) << setfill('0') << dt.minute << ":"
        << setw(2) << setfill('0') << dt.second;
    return oss.str();
}

int main()
{
	SetTraceLogLevel(LOG_WARNING);
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	InitWindow(800, 600, "Mathris");
	InitAudioDevice();

	int state = STATE_MAIN_MENU;

	int game_mode = 0;
	vector <int> punish(10, 0);
	vector <float> prev_punish(10, 0);
	int punishment = 4;
	bool warning = 0;
	float gameover_tint = 0;

	int start_time = 60;
	int game_time = 60;
	int time_frame = 0;

	int correct_score = 1;
	int wrong_score = 5;
	int victory_score = 50;
	int score = 0;

	font = LoadFont("resources/font.ttf");

	Texture2D Times = LoadTexture("resources/times.png");
	Texture2D Plus = LoadTexture("resources/plus.png");
	Texture2D Minus = LoadTexture("resources/minus.png");
	Texture2D Divide = LoadTexture("resources/divide.png");
	Texture2D Equals = LoadTexture("resources/equals.png");
	Check = LoadTexture("resources/check.png");

	Texture2D Title = LoadTexture("resources/title.png");
	Texture2D TitleWrong = LoadTexture("resources/title_wrong.png");
	Texture2D TitleWrong2 = LoadTexture("resources/title_wrong2.png");

	Texture2D GameOver = LoadTexture("resources/gameover.png");

	Texture2D ResultsScreen = LoadTexture("resources/results_screen.png");
	Texture2D ResultsTimed = LoadTexture("resources/results_timed.png");
	Texture2D ResultsNotTimed = LoadTexture("resources/results_not_timed.png");

	Texture2D PauseScreen = LoadTexture("resources/pause_screen.png");

	Texture2D ResultsButton = LoadTexture("resources/results_button.png");
	Texture2D BackButton = LoadTexture("resources/back_button.png");

	Sound CorrectSnd[8] = {LoadSound("resources/correct1.mp3"), LoadSound("resources/correct2.mp3"), LoadSound("resources/correct3.mp3"), LoadSound("resources/correct4.mp3"), LoadSound("resources/correct5.mp3"), LoadSound("resources/correct6.mp3"), LoadSound("resources/correct7.mp3"), LoadSound("resources/correct8.mp3")};
	Sound WrongSnd = LoadSound("resources/wrong.mp3");
	Sound LandSnd = LoadSound("resources/land.mp3");

	bool que_or_ans = 0;
	int min_num = 1, max_num = 10, min_mul = 1, max_mul = 10, min_ans = 1, max_ans = 100;
	Op_Choose que_or_ans_ch({"que", "ans"});
	Op_Textbox min_num_box("min num", 9),
			   max_num_box("max num", 9),
			   min_mul_box("min mul", 9),
			   max_mul_box("max mul", 9),
			   min_ans_box("min ans", 9),
			   max_ans_box("max ans", 9);
	Op_Separator GRID_HEIGHT_sep(3, GRAY);
	Op_Textbox GRID_HEIGHT_box("GRID HEIGHT", 2),
			   corrects_to_narrow_box("corrects to narrow", 3);
	Op_Toggle test_plus_but("test plus"),
		   	  test_minus_but("test minus"),
		      test_times_but("test times", 1),
		      test_divide_but("test divide");
	Op_Choose game_mode_ch({"till gameover", "timed", "score"});
	Op_Textbox punishment_box("punishment", 2),
			   corrects_to_recover_box("corrects_to_recover", 2),
			   game_time_box("game time", 5),
			   correct_score_box("correct score", 4),
			   wrong_score_box("wrong score", 4),
			   victory_score_box("victory score", 8);
	Op_Button start_game_but(200, LoadTexture("resources/start_game.png"));
	bool test_plus = 0, test_minus = 0, test_times = 1, test_divide = 0;
	int current_test = 0;
	int corrects_to_narrow = 0;
	int corrects_to_recover = 0;
	long long wrong_counter = 120;
	long long wrong2_counter = 120;
	Vector2 title_wrong_pos;

	int first_num = 1;
	int prev_first_num = 1;

	Camera2D cam = {0};
	cam.zoom = 1;
	cam.target.x = -(800 - (GRID_WIDTH + 2) * GRID_SIZE) / 2;
	cam.target.y = -(600 - GRID_HEIGHT * GRID_SIZE) / 2;

	Camera2D title_cam = {0};
	title_cam.zoom = 1;

	long long move_counter = 0;
	int block_x = 2, block_y = GRID_HEIGHT / 2;
	float display_x = -1, display_y = GRID_HEIGHT / 2;
	float display_GRID_WIDTH = 20;

	int number = GetRandomValue(1, 10);
	int multiplier = GetRandomValue(1, 10);
	bool reverse = false;

	long long impact_frame = 1000;
	long long transition_frame = 1000;
	long long gameover_frame = 1000;
	bool gameover = 0;
	int prev_test;
	int prev_number;
	int prev_multiplier;
	int prev_block_x;
	int prev_block_y;
	int prev_GRID_WIDTH = 20;
	int prev_reverse;
	bool is_correct;
	int corrects_in_a_row = 0;

	int correct = 0, wrong = 0;
	vector <vector<unsigned short>> corrects;
	vector <vector<unsigned short>> wrongs;
	vector <int> recovers;

	bool results_view = 0;
	Camera2D results_cam = {0};
	results_cam.zoom = 1;
	int cur_results = 0;
	int cur_time = 0;

	auto LOAD_DATA = [&] ()
	{
				ifstream fin("resources/results.txt");
				char data[22];
				char* data2;
				int it = 0;
				for (int i = 0; i <= cur_results; i++){
					fin.read(data, 22);
					it = 6;
					min_num = data[it++] << 24;
					min_num += data[it++] << 16;
					min_num += data[it++] << 8;
					min_num += data[it++];
					max_num = data[it++] << 24;
					max_num += data[it++] << 16;
					max_num += data[it++] << 8;
					max_num += data[it++];
					min_mul = data[it++] << 24;
					min_mul += data[it++] << 16;
					min_mul += data[it++] << 8;
					min_mul += data[it++];
					max_mul = data[it++] << 24;
					max_mul += data[it++] << 16;
					max_mul += data[it++] << 8;
					max_mul += data[it++];
					data2 = new char[(max_num - min_num + 1) * (max_mul - min_mul + 1) * 4];
					fin.read(data2, (max_num - min_num + 1) * (max_mul - min_mul + 1) * 4);
				}
				if (fin.eof()){
					cur_results--;
					return;
				}
				it = 0;
				cur_time = data[it++] << 24;
				cur_time += data[it++] << 16;
				cur_time += data[it++] << 8;
				cur_time += data[it++];
				cur_time += 10800;
				if (data[it] & char(1 << 7)){
					game_mode = 1;
					unsigned short temp = 0;
					temp += (data[it++] & (char(1 << 7) - 1)) << 8;
					temp += data[it++];
					start_time = temp;
				}
				else{
					game_mode = 0;
					unsigned short temp = 0;
					temp += data[it++] << 8;
					temp += data[it++];
					punishment = temp;
				}
				vector <vector<unsigned short>> temp1(max_num - min_num + 1, vector<unsigned short>(max_mul - min_mul + 1));
				vector <vector<unsigned short>> temp2(max_num - min_num + 1, vector<unsigned short>(max_mul - min_mul + 1));
				corrects = temp1;
				wrongs = temp2;
				it = 0;
				for (int i = 0; i <= max_num - min_num; i++){
					for (int j = 0; j <= max_mul - min_mul; j++){
						corrects[i][j] = data2[it++] << 8;
						corrects[i][j] += data2[it++];
						wrongs[i][j] = data2[it++] << 8;
						wrongs[i][j] += data2[it++];
					}
				}
			};
	auto LAND_BLOCK = [&] ()
	{
				move_counter = 0;
				prev_test = current_test;
				prev_number = number;
				prev_multiplier = multiplier;
				prev_first_num = first_num;
				prev_block_x = block_x;
				prev_block_y = block_y;
				prev_GRID_WIDTH = GRID_WIDTH;
				prev_reverse = reverse;
				if (current_test == 0){
					is_correct = block_y == (number + multiplier) - first_num;
				}
				else if (current_test == 1){
					is_correct = block_y == (number - multiplier) - first_num;
				}
				else if (current_test == 2){
					is_correct = block_y == number - first_num;
				}
				else if (current_test == 3){
					is_correct = block_y == (number / multiplier) - first_num;
				}
				if (is_correct){
					correct++;
					corrects_in_a_row++;
					if (!que_or_ans && SAVING){
						corrects[number - min_num][multiplier - min_mul]++;
					}
					if (corrects_to_narrow != 0){
						GRID_WIDTH = max(10, 20 - correct / corrects_to_narrow);
						if (game_mode == 0){
							if (punish[block_y] >= GRID_WIDTH * 2){
								warning = true;
							}
							if (punish[block_y] >= GRID_WIDTH * 4 - 12){
								gameover = 1;
								gameover_frame = 0;
							}
						}
					}
					if (corrects_to_recover != 0 && punish[block_y] > 0){
						recovers[block_y]++;
						if (recovers[block_y] == corrects_to_recover){
							punish[block_y] -= 4;
							punish[block_y] = max(punish[block_y], 0);
							recovers[block_y] = 0;
						}
					}
					if (game_mode == 2){
						score += correct_score;
						if (score >= victory_score){
							gameover = 1;
							gameover_frame = 0;
						}
					}
					PlaySound(CorrectSnd[min(corrects_in_a_row - 1, 7)]);
				}
				else{
					wrong++;
					corrects_in_a_row = 0;
					recovers[block_y] = 0;
					if (!que_or_ans && SAVING){
						wrongs[number - min_num][multiplier - min_mul]++;
					}
					if (game_mode == 0){
						punish[block_y] += punishment;
						if (punish[block_y] >= GRID_WIDTH * 2){
							warning = true;
						}
						if (punish[block_y] >= GRID_WIDTH * 4 - 12){
							gameover = 1;
							gameover_frame = 0;
						}
					}
					else if (game_mode == 2){
						score -= wrong_score;
					}
					PlaySound(WrongSnd);
				}
				impact_frame = 0;
				block_x = 2;
				display_x = -1;
				display_y = block_y;
				vector <int> tests;
				if (test_plus) tests.push_back(0);
				if (test_minus) tests.push_back(1);
				if (test_times) tests.push_back(2);
				if (test_divide) tests.push_back(3);
				current_test = tests[GetRandomValue(0, tests.size() - 1)];
				if (que_or_ans){
					if (current_test == 0){
						int ans = GetRandomValue(min_ans, max_ans);
						number = GetRandomValue(1, ans - 1);
						multiplier = ans - number;
					}
					else if (current_test == 1){
						int ans = GetRandomValue(min_ans, max_ans);
						number = GetRandomValue(ans + 1, ans * 2);
						multiplier = number - ans;
					}
					else if (current_test == 2){
						int ans = GetRandomValue(min_ans, max_ans);
						number = GetRandomValue(2, ans / 2);
						number = max(1, number);
						multiplier = ans / number;
					}
					else if (current_test == 3){
						int ans = GetRandomValue(min_ans, max_ans);
						multiplier = GetRandomValue(2, max_ans);
						number = ans * multiplier;
					}
				}
				else{
					if (current_test == 3){
						multiplier = GetRandomValue(min_mul, max_mul);
						number = GetRandomValue((min_num + multiplier - 1) / multiplier, max_num / multiplier) * multiplier;
					}
					else if (current_test == 1){
						multiplier = GetRandomValue(min_mul, max_mul);
						number = GetRandomValue(max(multiplier, min_num), max_num);
					}
					else{
						number = GetRandomValue(min_num, max_num);
						multiplier = GetRandomValue(min_mul, max_mul);
					}
				}

				if (current_test == 0 || current_test == 2){
					reverse = GetRandomValue(0, 1);
				}
				else{
					reverse = 0;
				}
				if (que_or_ans){
					if (current_test == 0){
						first_num = max(1, min(max_ans - GRID_HEIGHT + 1, GetRandomValue((number + multiplier) - GRID_HEIGHT + 1, number + multiplier)));
					}
					else if (current_test == 1){
						first_num = max(0, min(max_ans - GRID_HEIGHT + 1, GetRandomValue((number - multiplier) - GRID_HEIGHT + 1, number - multiplier)));
					}
					else if (current_test == 2){
						first_num = max(1, min(max_ans - GRID_HEIGHT + 1, GetRandomValue(number - GRID_HEIGHT + 1, number)));
					}
					else if (current_test == 3){
						first_num = max(1, min(max_ans - GRID_HEIGHT + 1, GetRandomValue((number / multiplier) - GRID_HEIGHT + 1, number / multiplier)));
					}
				}
				else{
					if (current_test == 0){
						first_num = max(1, min(max_num + max_mul - GRID_HEIGHT + 1, GetRandomValue((number + multiplier) - GRID_HEIGHT + 1, number + multiplier)));
					}
					else if (current_test == 1){
						first_num = max(0, min(max_num - min_mul - GRID_HEIGHT + 1, GetRandomValue((number - multiplier) - GRID_HEIGHT + 1, number - multiplier)));
					}
					else if (current_test == 2){
						first_num = max(1, min(max_num - GRID_HEIGHT + 1, GetRandomValue(number - GRID_HEIGHT + 1, number)));
					}
					else if (current_test == 3){
						first_num = max(1, min(max_num / min_mul - GRID_HEIGHT + 1, GetRandomValue((number / multiplier) - GRID_HEIGHT + 1, number / multiplier)));
					}
				}
				PlaySound(LandSnd);
	};
	auto START_GAME = [&] ()
	{
					que_or_ans = que_or_ans_ch.chosen();
					min_num = TextToInteger(min_num_box.data());
					max_num = TextToInteger(max_num_box.data());
					min_mul = TextToInteger(min_mul_box.data());
					max_mul = TextToInteger(max_mul_box.data());
					min_ans = TextToInteger(min_ans_box.data());
					max_ans = TextToInteger(max_ans_box.data());
					if (game_mode == 2){
						correct_score = TextToInteger(correct_score_box.data());
						wrong_score = TextToInteger(wrong_score_box.data());
						victory_score = TextToInteger(victory_score_box.data());
						score = 0;
					}
					GRID_WIDTH = 20;
					prev_GRID_WIDTH = 20;
					if  (GRID_HEIGHT_box.is_empty()){
						if (que_or_ans){
							GRID_HEIGHT = min(MIN_GRID_HEIGHT, max_ans - min_ans + 1);
						}
						else{
							GRID_HEIGHT = min(MIN_GRID_HEIGHT, max_num - min_num + 1);
						}
					}
					else{
						GRID_HEIGHT = TextToInteger(GRID_HEIGHT_box.data());
					}
					if  (punishment_box.is_empty()){
						punishment = 2;
					}
					else{
						punishment = TextToInteger(punishment_box.data());
					}
					if  (corrects_to_narrow_box.is_empty()){
						corrects_to_narrow = 0;
					}
					else{
						corrects_to_narrow = TextToInteger(corrects_to_narrow_box.data());
					}
					if  (corrects_to_recover_box.is_empty()){
						corrects_to_recover = 0;
					}
					else{
						corrects_to_recover = TextToInteger(corrects_to_recover_box.data());
					}
					if (game_time_box.is_empty()){
						start_time = 60;
						game_time = 60;
					}
					else{
						start_time = TextToInteger(game_time_box.data());
						game_time = TextToInteger(game_time_box.data());
					}
					test_plus = test_plus_but.toggled();
					test_minus = test_minus_but.toggled();
					test_times = test_times_but.toggled();
					test_divide = test_divide_but.toggled();
					punish.clear();
					punish.reserve(GRID_HEIGHT);
					prev_punish.clear();
					prev_punish.reserve(GRID_HEIGHT);
					recovers.clear();
					recovers.reserve(GRID_HEIGHT);
					for (int i = 0; i < GRID_HEIGHT; i++){
						punish[i] = 0;
						prev_punish[i] = 0;
						recovers[i] = 0;
					}
					block_y = GetRandomValue(0, GRID_HEIGHT - 1);
					display_y = block_y;
					vector <int> tests;
					if (test_plus) tests.push_back(0);
					if (test_minus) tests.push_back(1);
					if (test_times) tests.push_back(2);
					if (test_divide) tests.push_back(3);
					current_test = tests[GetRandomValue(0, tests.size() - 1)];
					if (que_or_ans){
						if (current_test == 0){
							int ans = GetRandomValue(min_ans, max_ans);
							number = GetRandomValue(1, ans - 1);
							multiplier = ans - number;
						}
						else if (current_test == 1){
							int ans = GetRandomValue(min_ans, max_ans);
							number = GetRandomValue(ans + 1, ans * 2);
							multiplier = number - ans;
						}
						else if (current_test == 2){
							int ans = GetRandomValue(min_ans, max_ans);
							number = GetRandomValue(2, ans / 2);
							multiplier = ans / number;
						}
						else if (current_test == 3){
							int ans = GetRandomValue(min_ans, max_ans);
							multiplier = GetRandomValue(2, max_ans);
							number = ans * multiplier;
						}
					}
					else{
						if (current_test == 3){
							multiplier = GetRandomValue(min_mul, max_mul);
							number = GetRandomValue((min_num + multiplier - 1) / multiplier, max_num / multiplier) * multiplier;
						}
						else if (current_test == 1){
							multiplier = GetRandomValue(min_mul, max_mul);
							number = GetRandomValue(max(multiplier, min_num), max_num);
						}
						else{
							number = GetRandomValue(min_num, max_num);
							multiplier = GetRandomValue(min_mul, max_mul);
						}
					}
					
					if (current_test == 0 || current_test == 2){
						reverse = GetRandomValue(0, 1);
					}
					else{
						reverse = 0;
					}
					if (que_or_ans){
						if (current_test == 0){
							first_num = max(1, min(max_ans - GRID_HEIGHT + 1, GetRandomValue((number + multiplier) - GRID_HEIGHT + 1, number + multiplier)));
						}
						else if (current_test == 1){
							first_num = max(0, min(max_ans - GRID_HEIGHT + 1, GetRandomValue((number - multiplier) - GRID_HEIGHT + 1, number - multiplier)));
						}
						else if (current_test == 2){
							first_num = max(1, min(max_ans - GRID_HEIGHT + 1, GetRandomValue(number - GRID_HEIGHT + 1, number)));
						}
						else if (current_test == 3){
							first_num = max(1, min(max_ans - GRID_HEIGHT + 1, GetRandomValue((number / multiplier) - GRID_HEIGHT + 1, number / multiplier)));
						}
					}
					else{
						if (current_test == 0){
							first_num = max(1, min(max_num + max_mul - GRID_HEIGHT + 1, GetRandomValue((number + multiplier) - GRID_HEIGHT + 1, number + multiplier)));
						}
						else if (current_test == 1){
							first_num = max(0, min(max_num - min_mul - GRID_HEIGHT + 1, GetRandomValue((number - multiplier) - GRID_HEIGHT + 1, number - multiplier)));
						}
						else if (current_test == 2){
							first_num = max(1, min(max_num - GRID_HEIGHT + 1, GetRandomValue(number - GRID_HEIGHT + 1, number)));
						}
						else if (current_test == 3){
							first_num = max(1, min(max_num / min_mul - GRID_HEIGHT + 1, GetRandomValue((number / multiplier) - GRID_HEIGHT + 1, number / multiplier)));
						}
					}
					transition_frame = 0;
					if (!que_or_ans && SAVING){
						vector <vector<unsigned short>> temp1(max_num - min_num + 1, vector<unsigned short>(max_mul - min_mul + 1, 0));
						vector <vector<unsigned short>> temp2(max_num - min_num + 1, vector<unsigned short>(max_mul - min_mul + 1, 0));
						corrects = temp1;
						wrongs = temp2;
					}
	};

	while (!WindowShouldClose()){
		if (state == STATE_GAME){
			if (IsKeyPressed(KEY_P)){
				state = STATE_PAUSE;
			}

			display_x = (display_x * 2 + min(float(block_x), GRID_WIDTH - float(punish[block_y]) / 4 - 1)) / 3;
			display_y = (display_y * 2 + block_y) / 3;
			if (game_mode == 0){
				for (int i = 0; i < GRID_HEIGHT; i++){
					prev_punish[i] = (prev_punish[i] + punish[i] * 2) / 3;
				}
			}
			
			if ((IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressedRepeat(KEY_UP) || IsKeyPressedRepeat(KEY_W)) && block_y > 0 && !gameover){
				block_y--;
				if (block_x >= GRID_WIDTH - punish[block_y] / 4){
					LAND_BLOCK();
				}
			}
			if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressedRepeat(KEY_S)) && block_y < GRID_HEIGHT - 1 && !gameover){
				block_y++;
				if (block_x >= GRID_WIDTH - punish[block_y] / 4){
					LAND_BLOCK();
				}
			}
			if (IsKeyPressed(KEY_SPACE) && !gameover){
				LAND_BLOCK();
			}

			if (move_counter == INTERVAL && !gameover){
				move_counter = 0;
				block_x++;
				if (block_x == GRID_WIDTH - punish[block_y] / 4){
					LAND_BLOCK();
				}
			}

			gameover_tint = min(gameover_tint + 0.001f * warning, 0.2f);

			if (game_mode == 1 && game_time == 0 && !gameover){
				gameover = 1;
				gameover_frame = 0;
			}

			cam.target.x = -(GetScreenWidth() - (display_GRID_WIDTH + 2) * GRID_SIZE) / 2;
			cam.target.y = -(GetScreenHeight() - GRID_HEIGHT * GRID_SIZE) / 2;
			cam.offset.x = 0;
			if (impact_frame < 10){
				cam.offset.x = EaseSineOut(impact_frame, 5, -5, 10);
			}
			
			// DRAW
			BeginDrawing();
				BeginMode2D(cam);
				ClearBackground(WHITE);

				if (impact_frame < 30){
					display_GRID_WIDTH = EaseSineOut(impact_frame, prev_GRID_WIDTH, GRID_WIDTH - prev_GRID_WIDTH, 29);
					cam.target.x = -(GetScreenWidth() - (display_GRID_WIDTH + 2) * GRID_SIZE) / 2;
					float x = EaseSineIn(impact_frame, display_GRID_WIDTH - (1 + prev_punish[prev_block_y] / 4), 5, 30);
					if (is_correct){
						DrawRectangle(int(x * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GREEN);
						DrawRectangle(int((x - 1) * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GREEN);
						DrawRectangle(int((x - 2) * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GREEN);
					}
					else{
						DrawRectangle(int(x * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, RED);
						DrawRectangle(int((x - 1) * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, RED);
						DrawRectangle(int((x - 2) * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, RED);
					}
					DrawRectangleLines(int(x * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
					DrawRectangleLines(int((x - 1) * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
					DrawRectangleLines(int((x - 2) * GRID_SIZE), prev_block_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
					if (prev_reverse){
						const char* prev_number_t = TextFormat("%01i", prev_multiplier);
						DrawText(prev_number_t, int((x - 2) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(prev_number_t, 20) / 2), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 10, 20, BLACK);
						const char* prev_multipiler_t = TextFormat("%01i", prev_number);
						DrawText(prev_multipiler_t, int(x * GRID_SIZE + GRID_SIZE / 2 - MeasureText(prev_multipiler_t, 20) / 2), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 10, 20, BLACK);
					}
					else{
						const char* prev_number_t = TextFormat("%01i", prev_number);
						DrawText(prev_number_t, int((x - 2) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(prev_number_t, 20) / 2), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 10, 20, BLACK);
						const char* prev_multipiler_t = TextFormat("%01i", prev_multiplier);
						DrawText(prev_multipiler_t, int(x * GRID_SIZE + GRID_SIZE / 2 - MeasureText(prev_multipiler_t, 20) / 2), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 10, 20, BLACK);
					}
					if (prev_test == 0){
						DrawTexture(Plus, int((x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 8, WHITE);
					}
					else if (prev_test == 1){
						DrawTexture(Minus, int((x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 8, WHITE);
					}
					else if (prev_test == 2){
						DrawTexture(Times, int((x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 8, WHITE);
					}
					else if (prev_test == 3){
						DrawTexture(Divide, int((x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), prev_block_y * GRID_SIZE + GRID_SIZE / 2 - 8, WHITE);
					}
				}

				for (int i = 0; i < GRID_HEIGHT; i++){
					DrawRectangle((display_GRID_WIDTH + 2) * GRID_SIZE - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, int(prev_punish[i] * GRID_SIZE / 4), GRID_SIZE, BLACK);
					if (corrects_to_recover > 0){
						DrawRectangle((display_GRID_WIDTH + 2) * GRID_SIZE - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, int(recovers[i] * GRID_SIZE / corrects_to_recover), GOLD);
					}
					DrawRectangle(display_GRID_WIDTH * GRID_SIZE - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, GRID_SIZE, WHITE);
					DrawRectangle((display_GRID_WIDTH + 1) * GRID_SIZE - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, GRID_SIZE, WHITE);
					DrawRectangleLines(display_GRID_WIDTH * GRID_SIZE - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
					DrawRectangleLines((display_GRID_WIDTH + 1) * GRID_SIZE - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
					DrawTexture(Equals, display_GRID_WIDTH * GRID_SIZE + GRID_SIZE / 2 - 7 - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE + GRID_SIZE / 2 - 8, WHITE);
					if (current_test == 2){
						const char* equal_t = TextFormat("%01i", (i + first_num) * multiplier);
						int text_size = 20, text_y = i * GRID_SIZE + GRID_SIZE / 2 - 10;
						if (MeasureText(equal_t, 20) > GRID_SIZE){
							text_size = 10;
							text_y = i * GRID_SIZE + GRID_SIZE / 2 - 5;
						}
						DrawText(equal_t, (display_GRID_WIDTH + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(equal_t, text_size) / 2 - int(prev_punish[i] * GRID_SIZE / 4), text_y, text_size, BLACK);
					}
					else{
						const char* equal_t = TextFormat("%01i", i + first_num);
						int text_size = 20, text_y = i * GRID_SIZE + GRID_SIZE / 2 - 10;
						if (MeasureText(equal_t, 20) > GRID_SIZE){
							text_size = 10;
							text_y = i * GRID_SIZE + GRID_SIZE / 2 - 5;
						}
						DrawText(equal_t, (display_GRID_WIDTH + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(equal_t, text_size) / 2 - int(prev_punish[i] * GRID_SIZE / 4), text_y, text_size, BLACK);
					}
				}
				if (impact_frame < 15){
					bool change = 0;
					if (prev_test == 2){
						change = prev_first_num != first_num || prev_multiplier != multiplier || current_test != 2;
					}
					else{
						change = prev_first_num != first_num || current_test == 2;
					}
					if (change){
						for (int i = 0; i < GRID_HEIGHT; i++){
							int x = (int)EaseSineIn(impact_frame, 0, GRID_SIZE * (prev_punish[i] / 4 + 2), 15);
							DrawRectangle((display_GRID_WIDTH + 1) * GRID_SIZE + x - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, GRID_SIZE, WHITE);
							DrawRectangleLines((display_GRID_WIDTH + 1) * GRID_SIZE + x - int(prev_punish[i] * GRID_SIZE / 4), i * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
							if (prev_test == 2){
								const char* equal_t = TextFormat("%01i", (i + prev_first_num) * prev_multiplier);
								int text_size = 20, text_y = i * GRID_SIZE + GRID_SIZE / 2 - 10;
								if (MeasureText(equal_t, 20) > GRID_SIZE){
									text_size = 10;
									text_y = i * GRID_SIZE + GRID_SIZE / 2 - 5;
								}
								DrawText(equal_t, (display_GRID_WIDTH + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(equal_t, text_size) / 2 + x - int(prev_punish[i] * GRID_SIZE / 4), text_y, text_size, BLACK);
							}
							else{
								const char* equal_t = TextFormat("%01i", i + prev_first_num);
								int text_size = 20, text_y = i * GRID_SIZE + GRID_SIZE / 2 - 10;
								if (MeasureText(equal_t, 20) > GRID_SIZE){
									text_size = 10;
									text_y = i * GRID_SIZE + GRID_SIZE / 2 - 5;
								}
								DrawText(equal_t, (display_GRID_WIDTH + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(equal_t, text_size) / 2 + x - int(prev_punish[i] * GRID_SIZE / 4), text_y, text_size, BLACK);
							}
						}
					}
				}

				if (!gameover){
				DrawRectangle(int(display_x * GRID_SIZE), int(display_y * GRID_SIZE), GRID_SIZE, GRID_SIZE, WHITE);
				DrawRectangle(int((display_x - 1) * GRID_SIZE), display_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, WHITE);
				DrawRectangle(int((display_x - 2) * GRID_SIZE), display_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, WHITE);
				DrawRectangleLines(int(display_x * GRID_SIZE), display_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
				DrawRectangleLines(int((display_x - 1) * GRID_SIZE), display_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
				DrawRectangleLines(int((display_x - 2) * GRID_SIZE), display_y * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
					if (reverse){
						const char* number_t = TextFormat("%01i", multiplier);
						int text_size = 20, text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 10;
						if (MeasureText(number_t, 20) > GRID_SIZE){
							text_size = 10;
							text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 5;
						}
						DrawText(number_t, int((display_x - 2) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(number_t, text_size) / 2), text_y, text_size, BLACK);
						const char* multipiler_t = TextFormat("%01i", number);
						text_size = 20; text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 10;
						if (MeasureText(multipiler_t, 20) > GRID_SIZE){
							text_size = 10;
							text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 5;
						}
						DrawText(multipiler_t, int(display_x * GRID_SIZE + GRID_SIZE / 2 - MeasureText(multipiler_t, text_size) / 2), text_y, text_size, BLACK);
					}
					else{
						const char* number_t = TextFormat("%01i", number);
						int text_size = 20, text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 10;
						if (MeasureText(number_t, 20) > GRID_SIZE){
							text_size = 10;
							text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 5;
						}
						DrawText(number_t, int((display_x - 2) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(number_t, text_size) / 2), text_y, text_size, BLACK);
						const char* multipiler_t = TextFormat("%01i", multiplier);
						text_size = 20; text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 10;
						if (MeasureText(multipiler_t, 20) > GRID_SIZE){
							text_size = 10;
							text_y = display_y * GRID_SIZE + GRID_SIZE / 2 - 5;
						}
						DrawText(multipiler_t, int(display_x * GRID_SIZE + GRID_SIZE / 2 - MeasureText(multipiler_t, text_size) / 2), text_y, text_size, BLACK);
					}
					if (current_test == 0){
						DrawTexture(Plus, int((display_x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), int(display_y * GRID_SIZE + GRID_SIZE / 2 - 8), WHITE);
					}
					else if (current_test == 1){
						DrawTexture(Minus, int((display_x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), int(display_y * GRID_SIZE + GRID_SIZE / 2 - 8), WHITE);
					}
					else if (current_test == 2){
						DrawTexture(Times, int((display_x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), int(display_y * GRID_SIZE + GRID_SIZE / 2 - 8), WHITE);
					}
					else if (current_test == 3){
						DrawTexture(Divide, int((display_x - 1) * GRID_SIZE + GRID_SIZE / 2 - 7), int(display_y * GRID_SIZE + GRID_SIZE / 2 - 8), WHITE);
					}
					DrawRectangleGradientV(-5, int(display_y * GRID_SIZE), (display_GRID_WIDTH + 2) * GRID_SIZE + 5, GRID_SIZE / 4, ColorAlpha(YELLOW, 0.3), BLANK);
					DrawRectangleGradientV(-5, int(display_y * GRID_SIZE + GRID_SIZE * 3 / 4), (display_GRID_WIDTH + 2) * GRID_SIZE + 5, GRID_SIZE / 4, BLANK, ColorAlpha(YELLOW, 0.3));
				}
				
				DrawRectangle(0, 0, GRID_SIZE * 3, GRID_SIZE * GRID_HEIGHT, ColorAlpha(RED, gameover_tint));
				DrawRectangleGradientH(GRID_SIZE * 3, 0, GRID_SIZE / 2, GRID_SIZE * GRID_HEIGHT, ColorAlpha(RED, gameover_tint), BLANK);
				
				if (impact_frame < 6){
					if (is_correct){
						DrawRectangleGradientH(prev_block_x * GRID_SIZE, prev_block_y * GRID_SIZE, (display_GRID_WIDTH - prev_punish[prev_block_y] / 4 - prev_block_x) * GRID_SIZE, GRID_SIZE, BLANK, ColorAlpha(GREEN, 0.3 - impact_frame / 20.0));
					}
					else{
						DrawRectangleGradientH(prev_block_x * GRID_SIZE, prev_block_y * GRID_SIZE, (display_GRID_WIDTH - prev_punish[prev_block_y] / 4 - prev_block_x) * GRID_SIZE, GRID_SIZE, BLANK, ColorAlpha(RED, 0.3 - impact_frame / 20.0));
					}
				}

				DrawRectangleV(cam.target, Vector2{GetScreenWidth(), -cam.target.y}, RAYWHITE);
				DrawRectangleV(Vector2{cam.target.x - 5, cam.target.y}, Vector2{-cam.target.x + 5, GetScreenHeight()}, RAYWHITE);
				DrawRectangleV(Vector2{cam.target.x, float(GRID_HEIGHT * GRID_SIZE)}, Vector2{GetScreenWidth(), -cam.target.y}, RAYWHITE);
				DrawRectangleV(Vector2{(display_GRID_WIDTH + 2) * GRID_SIZE, cam.target.y}, Vector2{-cam.target.x, GetScreenHeight()}, RAYWHITE);
				DrawRectangleLines(0, 0, (display_GRID_WIDTH + 2) * GRID_SIZE, GRID_HEIGHT * GRID_SIZE, BLACK);
				DrawText(TextFormat("Correct: %01i", correct), 4, 4, 30, ColorAlpha(GREEN, max(0, (120 - impact_frame) / 120.0)));
				DrawText(TextFormat("Wrong: %01i", wrong), 4, 44, 30, ColorAlpha(RED, max(0, (120 - impact_frame) / 120.0)));

				if (game_mode == 1){
					DrawText(TextFormat("%02i:%02i:%02i", game_time / 3600, game_time / 60 % 60, game_time % 60), (display_GRID_WIDTH + 2) * GRID_SIZE - MeasureText(TextFormat("%02i:%02i:%02i", game_time / 3600, game_time / 60 % 60, game_time % 60), 30), -30, 30, BLACK);
				}
				else if (game_mode == 2){
					DrawText(TextFormat("Score: %01i", score), (display_GRID_WIDTH + 2) * GRID_SIZE - MeasureText(TextFormat("Score: %01i", score), 30), -30, 30, BLACK);
				}

				if (impact_frame < 60){
					if (is_correct){
						DrawRectangle(-5000, -5000, 10000, 10000, ColorAlpha(GREEN, max(0, 0.2 - (float)impact_frame / 300)));
					}
					else{
						DrawRectangle(-5000, -5000, 10000, 10000, ColorAlpha(RED, max(0, 0.2 - (float)impact_frame / 300)));
					}
				}


				if (transition_frame < 30){
					DrawCircle(cam.target.x, cam.target.y + GetScreenHeight() / 2, EaseSineOut(transition_frame - 15, 1000, -1000, 15), BLACK);
				}
				
				EndMode2D();

			if (gameover){
				if (game_mode == 0){
					DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 80) / 2, GetScreenHeight() / 2 - 40, 80, RED);
				}
				else if (game_mode == 1){
					DrawText("TIME UP", GetScreenWidth() / 2 - MeasureText("TIME UP", 80) / 2, GetScreenHeight() / 2 - 40, 80, RED);
				}
				else if (game_mode == 2){
					DrawText("VICTORY", GetScreenWidth() / 2 - MeasureText("VICTORY", 80) / 2, GetScreenHeight() / 2 - 40, 80, GREEN);
				}
			}
			EndDrawing();

			move_counter++;
			impact_frame++;
			transition_frame++;
			gameover_frame++;
			if (gameover_frame == 120){
				if (!que_or_ans && SAVING){
					if (SAVE_TYPE == 0){
						string data;
						if (GetFileLength("resources/results.txt") != 0){
							data = string(LoadFileText("resources/results.txt"));
						}
						if (game_mode == 1){
							data += to_string(start_time) + " seconds\n";
						}
						else{
							data += "endless\n";
						}
						data += "Correct: " + to_string(correct) + "\nWrong: " + to_string(wrong) + "\n\n";
						SaveFileText("resources/results.txt", data.data());
					}
					else if (SAVE_TYPE == 1){
						ofstream fout("resources/results.txt", ios::app);
						char data[22];
						int it = 0;
						cur_time = time(nullptr);
						data[it++] = char(cur_time >> 24);
						data[it++] = char(cur_time >> 16);
						data[it++] = char(cur_time >> 8);
						data[it++] = char(cur_time);
						if (game_mode == 1){
							unsigned short temp = (1 << 15) | start_time;
							data[it++] = char(temp >> 8);
							data[it++] = char(temp);
						}
						else{
							unsigned short temp = punishment;
							data[it++] = char(temp >> 8);
							data[it++] = char(temp);
						}
						data[it++] = char(min_num >> 24);
						data[it++] = char(min_num >> 16);
						data[it++] = char(min_num >> 8);
						data[it++] = char(min_num);
						data[it++] = char(max_num >> 24);
						data[it++] = char(max_num >> 16);
						data[it++] = char(max_num >> 8);
						data[it++] = char(max_num);
						data[it++] = char(min_mul >> 24);
						data[it++] = char(min_mul >> 16);
						data[it++] = char(min_mul >> 8);
						data[it++] = char(min_mul);
						data[it++] = char(max_mul >> 24);
						data[it++] = char(max_mul >> 16);
						data[it++] = char(max_mul >> 8);
						data[it++] = char(max_mul);
						fout.write(data, 22);
						char data2[(max_num - min_num + 1) * (max_mul - min_mul + 1) * 4];
						it = 0;
						for (int i = 0; i <= max_num - min_num; i++){
							for (int j = 0; j <= max_mul - min_mul; j++){
								data2[it++] = char(corrects[i][j] >> 8);
								data2[it++] = char(corrects[i][j]);
								data2[it++] = char(wrongs[i][j] >> 8);
								data2[it++] = char(wrongs[i][j]);
							}
						}
						fout.write(data2, (max_num - min_num + 1) * (max_mul - min_mul + 1) * 4);
					}
				}
				state = STATE_GAME_OVER;
			}
			if (!gameover) time_frame++;
			if (game_mode == 1 && !gameover && time_frame % 60 == 0){
				game_time--;
			}
		}
		else if (state == STATE_MAIN_MENU){
			next_option_draw_y = 0;
			next_option_id = 0;

			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			que_or_ans_ch.update(title_cam);
			if (que_or_ans_ch.chosen() == 0){
				min_num_box.update(title_cam);
				max_num_box.update(title_cam);
				min_mul_box.update(title_cam);
				max_mul_box.update(title_cam);
			}
			else{
				min_ans_box.update(title_cam);
				max_ans_box.update(title_cam);
			}
			GRID_HEIGHT_sep.update();
			GRID_HEIGHT_box.update(title_cam);
			corrects_to_narrow_box.update(title_cam);
			test_plus_but.update(title_cam);
			test_minus_but.update(title_cam);
			test_times_but.update(title_cam);
			test_divide_but.update(title_cam);
			game_mode_ch.update(title_cam);
			if (game_mode_ch.chosen() == 0){
				punishment_box.update(title_cam);
				corrects_to_recover_box.update(title_cam);
			}
			else if (game_mode_ch.chosen() == 1){
				game_time_box.update(title_cam);
			}
			else if (game_mode_ch.chosen() == 2){
				correct_score_box.update(title_cam);
				wrong_score_box.update(title_cam);
				victory_score_box.update(title_cam);
			}
			start_game_but.update(title_cam);
			switch_to_option_id = -1;

			if (current_option_id > 0 && (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))){
				switch_to_option_id = current_option_id - 1;
			}
			if (current_option_id != -1 && current_option_id != next_option_id - 1 && (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))){
				switch_to_option_id = current_option_id + 1;
			}
			current_option_id = -1;
			
			if (start_game_but.is_pressed()){
				wrong_counter = 120;
				wrong2_counter = 120;
				title_wrong_pos = GetMousePositionCam(title_cam);
				if (que_or_ans_ch.chosen() == 0){
					if (min_num_box.is_empty()){
						wrong_counter = 0;
						min_num_box.highlight(RED);
					}
					if (max_num_box.is_empty()){
						wrong_counter = 0;
						max_num_box.highlight(RED);
					}
					if (min_mul_box.is_empty()){
						wrong_counter = 0;
						min_mul_box.highlight(RED);
					}
					if (max_mul_box.is_empty()){
						wrong_counter = 0;
						max_mul_box.highlight(RED);
					}
				}
				else{
					if (min_ans_box.is_empty()){
						wrong_counter = 0;
						min_ans_box.highlight(RED);
					}
					if (max_ans_box.is_empty()){
						wrong_counter = 0;
						max_ans_box.highlight(RED);
					}
				}
				if (!test_plus_but.toggled() && !test_minus_but.toggled() && !test_times_but.toggled() && !test_divide_but.toggled()){
					wrong_counter = 0;
					test_plus_but.highlight(RED);
					test_minus_but.highlight(RED);
					test_times_but.highlight(RED);
					test_divide_but.highlight(RED);
				}
				if (wrong_counter == 120){
					if (que_or_ans_ch.chosen() == 0){
						if (TextToInteger(min_num_box.data()) > TextToInteger(max_num_box.data())){
							wrong2_counter = 0;
							min_num_box.highlight(RED);
							max_num_box.highlight(RED);
						}
						if (TextToInteger(min_mul_box.data()) > TextToInteger(max_mul_box.data())){
							wrong2_counter = 0;
							min_mul_box.highlight(RED);
							max_mul_box.highlight(RED);
						}
					}
					else{
						if (TextToInteger(min_ans_box.data()) > TextToInteger(max_ans_box.data())){
							wrong2_counter = 0;
							min_ans_box.highlight(RED);
							max_ans_box.highlight(RED);
						}
					}
					if (!GRID_HEIGHT_box.is_empty() && TextToInteger(GRID_HEIGHT_box.data()) < 2){
						wrong2_counter = 0;
						GRID_HEIGHT_box.highlight(RED);
					}
					if (wrong2_counter == 120){
						game_mode = game_mode_ch.chosen();
						START_GAME();
					}
				}
			}
				
			if (transition_frame == 15){
				corrects_in_a_row = 0;
				state = STATE_GAME;
			}
/*
			if (CheckCollisionPointRec(GetMousePosition(), {0, 0, 100, 100}) && IsMouseButtonPressed(0)){
				state = STATE_SAVES;
				results_cam.target = {-150, -150};
				LOAD_DATA();
			}
*/
			title_cam.target.y -= GetMouseWheelMove() * 20;
			title_cam.target.y = int(title_cam.target.y);
			title_cam.target.y = min(title_cam.target.y, next_option_draw_y - GetScreenHeight());
			title_cam.target.y = max(title_cam.target.y, 0);

			BeginDrawing();
				BeginMode2D(title_cam);
					ClearBackground(RAYWHITE);

					que_or_ans_ch.draw();
					if (que_or_ans_ch.chosen() == 0){
						min_num_box.draw();
						max_num_box.draw();
						min_mul_box.draw();
						max_mul_box.draw();
					}
					else{
						min_ans_box.draw();
						max_ans_box.draw();
					}
					GRID_HEIGHT_sep.draw();
					GRID_HEIGHT_box.draw();
					corrects_to_narrow_box.draw();
					test_plus_but.draw();
					test_minus_but.draw();
					test_times_but.draw();
					test_divide_but.draw();
					game_mode_ch.draw();
					if (game_mode_ch.chosen() == 0){
						punishment_box.draw();
						corrects_to_recover_box.draw();
					}
					else if (game_mode_ch.chosen() == 1){
						game_time_box.draw();
					}
					else if (game_mode_ch.chosen() == 2){
						correct_score_box.draw();
						wrong_score_box.draw();
						victory_score_box.draw();
					}
					start_game_but.draw();

					if (wrong_counter < 120){
						DrawTexture(TitleWrong, title_wrong_pos.x - TitleWrong.width / 2, title_wrong_pos.y - TitleWrong.height / 2 - wrong_counter, ColorAlpha(WHITE, (120.0f - wrong_counter) / 120));
					}
					if (wrong2_counter < 120){
						DrawTexture(TitleWrong2, title_wrong_pos.x - TitleWrong2.width / 2, title_wrong_pos.y - TitleWrong2.height / 2 - wrong2_counter, ColorAlpha(WHITE, (120.0f - wrong2_counter) / 120));
					}

				EndMode2D();
				//DrawTexture(ResultsButton, 0, 0, WHITE);

				if (transition_frame <= 15){
					DrawCircle(800, 300, EaseSineIn(transition_frame, 0, 1000, 15), BLACK);
				}
			EndDrawing();
			wrong_counter++;
			wrong2_counter++;
			transition_frame++;
		}
		else if (state == STATE_GAME_OVER){
			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			if (CheckCollisionPointRec(GetMousePosition(), {200, 350, 400, 200})){
				SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
				if (IsMouseButtonPressed(0)){
					gameover = 0;
					correct = 0;
					wrong = 0;
					state = 0;
					gameover_tint = 0;
					warning = 0;
					move_counter = 0;
				}
			}

			BeginDrawing();
				DrawTexture(GameOver, 0, 0, WHITE);
				DrawText(TextFormat("%01i", correct), 500, 170, 40, GREEN);
				DrawText(TextFormat("%01i", wrong), 520, 260, 40, RED);
			EndDrawing();
		}
		else if (state == STATE_SAVES){
			if (IsKeyPressed(KEY_P)){
				results_view = 1 - results_view;
			}

			if (!IsKeyDown(KEY_LEFT_SHIFT)){
				results_cam.target.y -= GetMouseWheelMove() * 20;
			}
			else{
				results_cam.target.x -= GetMouseWheelMove() * 20;
			}

			if (IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), {0, 150, 50, 300}) && cur_results > 0){
				cur_results--;
				LOAD_DATA();
			}
			if (IsMouseButtonPressed(0) && CheckCollisionPointRec(GetMousePosition(), {750, 150, 50, 300})){
				cur_results++;
				LOAD_DATA();
			}
			if (CheckCollisionPointRec(GetMousePosition(), {0, 0, 100, 100}) && IsMouseButtonPressed(0)){
				state = STATE_MAIN_MENU;
			}

			BeginDrawing();
				BeginMode2D(results_cam);
					ClearBackground(RAYWHITE);

					for (int i = 0; i <= max_num - min_num; i++){
						DrawRectangleLines((i + 1) * GRID_SIZE, 0, GRID_SIZE, GRID_SIZE, BLACK);
						DrawText(TextFormat("%01i", (i + min_num)), (i + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(TextFormat("%01i", i + min_num), 20) / 2, 0, 20, BLACK);
					}
					for (int i = 0; i <= max_mul - min_mul; i++){
						DrawRectangleLines(0, (i + 1) * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
						DrawText(TextFormat("%01i", (i + min_mul)), GRID_SIZE / 2 - MeasureText(TextFormat("%01i", i + min_mul), 20) / 2, (i + 1) * GRID_SIZE, 20, BLACK);
					}
					if (!results_view){
						for (int i = 0; i <= max_num - min_num; i++){
							for (int j = 0; j <= max_mul - min_mul; j++){
								DrawRectangleLines((i + 1) * GRID_SIZE, (j + 1) * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
								DrawText(TextFormat("%01i", corrects[i][j]), (i + 1) * GRID_SIZE + GRID_SIZE * 3 / 4 - MeasureText(TextFormat("%01i", corrects[i][j]), 10) / 2, (j + 1) * GRID_SIZE, 10, GREEN);
								DrawText(TextFormat("%01i", wrongs[i][j]), (i + 1) * GRID_SIZE + GRID_SIZE / 4 - MeasureText(TextFormat("%01i", wrongs[i][j]), 10) / 2, (j + 1) * GRID_SIZE + GRID_SIZE / 2, 10, RED);
							}
						}
					}
					else{
						for (int i = 0; i <= max_num - min_num; i++){
							for (int j = 0; j <= max_mul - min_mul; j++){
								DrawRectangleLines((i + 1) * GRID_SIZE, (j + 1) * GRID_SIZE, GRID_SIZE, GRID_SIZE, BLACK);
								if (corrects[i][j] + wrongs[i][j] == 0){
									DrawText("-%", (i + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText("-%", 10) / 2, (j + 1) * GRID_SIZE + GRID_SIZE / 2 - 5, 10, BLACK);
								}
								else{
									DrawText(concatenate(TextFormat("%01i", 100 * corrects[i][j] / (corrects[i][j] + wrongs[i][j])), "%"), 
											(i + 1) * GRID_SIZE + GRID_SIZE / 2 - MeasureText(concatenate(TextFormat("%01i", 100 * corrects[i][j] / (corrects[i][j] + wrongs[i][j])), "%"), 10) / 2, 
											(j + 1) * GRID_SIZE + GRID_SIZE / 2 - 5, 10, 
											{min(255, 510 * wrongs[i][j] / (corrects[i][j] + wrongs[i][j])), 
											 min(255, 510 * corrects[i][j] / (corrects[i][j] + wrongs[i][j])), 0, 255});
								}
							}
						}
					}
				EndMode2D();

				DrawTexture(ResultsScreen, 0, 0, WHITE);
				DrawTexture(BackButton, 0, 0, WHITE);
				if (game_mode == 1){
					DrawTexture(ResultsTimed, 0, 0, WHITE);
					DrawText(TextFormat("%01i", start_time), 440, 100, 20, BLACK);
				}
				else{
					DrawTexture(ResultsNotTimed, 0, 0, WHITE);
					DrawText(TextFormat("%01i", punishment), 480, 100, 20, BLACK);
				}
				DrawText(format_datetime(seconds_to_datetime(cur_time)).data(), 100, 585, 10, BLACK);
				
			EndDrawing();
		}
		else if (state == STATE_PAUSE){
			if (IsKeyPressed(KEY_P)){
				state = STATE_GAME;
			}

			SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			if (CheckCollisionPointRec(GetMousePosition(), {50, 250, 300, 150}) || CheckCollisionPointRec(GetMousePosition(), {450, 250, 300, 150})){
				SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
			}
			if (CheckCollisionPointRec(GetMousePosition(), {50, 250, 300, 150}) && IsMouseButtonPressed(0)){
				if (!que_or_ans && SAVING){
					if (SAVE_TYPE == 0){
						string data;
						if (GetFileLength("resources/results.txt") != 0){
							data = string(LoadFileText("resources/results.txt"));
						}
						if (game_mode == 1){
							data += to_string(start_time) + " seconds\n";
						}
						else{
							data += "endless\n";
						}
						data += "Correct: " + to_string(correct) + "\nWrong: " + to_string(wrong) + "\n\n";
						SaveFileText("resources/results.txt", data.data());
					}
					else if (SAVE_TYPE == 1){
						ofstream fout("resources/results.txt", ios::app);
						char data[22];
						int it = 0;
						cur_time = time(nullptr);
						data[it++] = char(cur_time >> 24);
						data[it++] = char(cur_time >> 16);
						data[it++] = char(cur_time >> 8);
						data[it++] = char(cur_time);
						if (game_mode == 1){
							unsigned short temp = (1 << 15) | start_time;
							data[it++] = char(temp >> 8);
							data[it++] = char(temp);
						}
						else{
							unsigned short temp = punishment;
							data[it++] = char(temp >> 8);
							data[it++] = char(temp);
						}
						data[it++] = char(min_num >> 24);
						data[it++] = char(min_num >> 16);
						data[it++] = char(min_num >> 8);
						data[it++] = char(min_num);
						data[it++] = char(max_num >> 24);
						data[it++] = char(max_num >> 16);
						data[it++] = char(max_num >> 8);
						data[it++] = char(max_num);
						data[it++] = char(min_mul >> 24);
						data[it++] = char(min_mul >> 16);
						data[it++] = char(min_mul >> 8);
						data[it++] = char(min_mul);
						data[it++] = char(max_mul >> 24);
						data[it++] = char(max_mul >> 16);
						data[it++] = char(max_mul >> 8);
						data[it++] = char(max_mul);
						fout.write(data, 22);
						char data2[(max_num - min_num + 1) * (max_mul - min_mul + 1) * 4];
						it = 0;
						for (int i = 0; i <= max_num - min_num; i++){
							for (int j = 0; j <= max_mul - min_mul; j++){
								data2[it++] = char(corrects[i][j] >> 8);
								data2[it++] = char(corrects[i][j]);
								data2[it++] = char(wrongs[i][j] >> 8);
								data2[it++] = char(wrongs[i][j]);
							}
						}
						fout.write(data2, (max_num - min_num + 1) * (max_mul - min_mul + 1) * 4);
					}
				}
				state = STATE_GAME_OVER;
			}
			else if (CheckCollisionPointRec(GetMousePosition(), {450, 250, 300, 150}) && IsMouseButtonPressed(0)){
				state = STATE_GAME;
			}

			BeginDrawing();
				DrawTexture(PauseScreen, 0, 0, WHITE);
				DrawText(TextFormat("%01i", correct), 500, 490, 30, GREEN);
				DrawText(TextFormat("%01i", wrong), 520, 545, 30, RED);
			EndDrawing();
		}
	}
	CloseWindow();
}
