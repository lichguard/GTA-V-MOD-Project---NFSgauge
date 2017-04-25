#ifndef _PERFORMANCE_TIMER_
#define _PERFORMANCE_TIMER_ 
#include "script.h"
#include "menu.h"
#define MAX_BOARD_TIMERS 10


class Entry
{
public:
	int index;
	int start;
	int stop;
	std::string units;
	int timecode_hours;
	int timecode_minutes;
	float time;
	Entry* next;

	Entry::Entry() : index(0), next(nullptr), start(0), stop(0), time(0), units(""), timecode_hours(0), timecode_minutes(0)
	{
	}
};
class Leaderboard
{
public:
	int open;
	Entry* first_entry;
	int size;
	int highlighted_index;

	Leaderboard::Leaderboard() : open(0), first_entry(nullptr),size(0), highlighted_index(-1) {
	}
	void insert_entry();
	void draw() const;
	void delete_last_entry();
	void reset_board();
};

void handle_timers();
void draw_performance_text(float x, float y, float scale, int font, const std::string& text);
void set_performance_timer();

extern Leaderboard leaderboard;
extern float performance_limit_start;
extern float performance_limit;
extern int open_timers_board;
extern int open_performance_timer;

void menu_options_list_performance(menu_options_list option, char control, char selected);
/*
void menu_option_start(char control, char selected);
void menu_option_stop(char control, char selected);

void menu_option_ready(char control, char selected);
void menu_option_open_board(char control, char selected);
void menu_option_reset_board(char control, char selected);
void menu_option_mileage_counter(char control, char selected);
*/
#endif