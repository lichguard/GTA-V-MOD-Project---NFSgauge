#ifndef __PTIMER__
#define __PTIMER__

#include "performance_timer.h"
#include "menu.h"
Vehicle veh2;
float speed2;
int perforamce_state = 0;
DWORD performance_start_time = 0;
DWORD performance_finish_time = 0;

float performance_limit_start = 0.0f;
float performance_limit = 27.7778f;


extern Menu main_menu;
int open_performance_timer = 0;
Leaderboard leaderboard;

void handle_timers()
{
	if (open_performance_timer)
	{
		veh2 = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
		speed2 = ENTITY::GET_ENTITY_SPEED(veh2);
		set_performance_timer();
		draw_text(std::to_string((performance_finish_time - performance_start_time) / 1000.0f).substr(0, 5) + "s", 0.5, 0.9, 0.7f, 0.7f, 255, 255, 255);
	}

	if (leaderboard.open && is_menu_open())
		leaderboard.draw();
}
void set_performance_timer()
{
	bool brake = performance_limit < performance_limit_start;
	switch (perforamce_state)
	{
	case 0:
		draw_text("~r~SET", 0.5, 0.85, 0.7f, 0.7f, 255, 255, 255);
		if ((speed2 <= performance_limit_start && !brake) || (brake && speed2 >= performance_limit_start))
		{
			perforamce_state = 1;
			main_menu.menu_is_open = false;
			performance_start_time = GetTickCount();
			performance_finish_time = GetTickCount();
		}
		break;
	case 1:
		draw_text("~g~READY", 0.5, 0.85, 0.7f, 0.7f, 255, 255, 255);

		if ((!VEHICLE::IS_VEHICLE_STOPPED(veh2) && speed2 >= performance_limit_start && !brake) || (brake && speed2 <= performance_limit_start))
		{
			performance_start_time = GetTickCount();
			performance_finish_time = GetTickCount();
			perforamce_state = 2;
		}
		break;
	case 2:
		performance_finish_time = GetTickCount();
		if (((performance_finish_time - performance_start_time) > 200000) || (speed2 >= performance_limit && !brake) || (brake && speed2 <= performance_limit))
		{
			perforamce_state = 3;
			main_menu.menu_is_open = true;
			leaderboard.insert_entry();
		}
		break;
	case 3:
		draw_text("~b~TIME", 0.5, 0.85, 0.7f, 0.7f, 255, 255, 255);
		break;
	default:
		break;

	}
}
void draw_performance_text(float x, float y, float scale, int font, const std::string& text)//color: 0 - white, 1 - grey, 2 - green
{
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(scale, scale);
	UI::SET_TEXT_DROPSHADOW(50, 0, 0, 0, 0);
	UI::SET_TEXT_COLOUR(255, 255, 255, 255);
	/*switch (color)
	{
	//grey
	case 0:
	UI::SET_TEXT_COLOUR(150, 150, 150, 255);
	break;
	case 1:
	UI::SET_TEXT_COLOUR(255, 255, 255, 255);
	break;
	case 2:
	UI::SET_TEXT_COLOUR(0, 0, 0, 255);
	break;
	default:
	break;
	}*/

	UI::SET_TEXT_WRAP(0.8f, 1.0f);
	UI::SET_TEXT_CENTRE(1);
	UI::SET_TEXT_OUTLINE();
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text.c_str());
	UI::_DRAW_TEXT(x, y);
}

void Leaderboard::insert_entry()
{
	if (this->size > MAX_BOARD_TIMERS)
		this->delete_last_entry();

	float new_time = ((performance_finish_time - performance_start_time) / 1000.0f);
	Entry *current_link = this->first_entry;
	Entry* new_entry = new Entry();
	new_entry->time = new_time;
	new_entry->next = first_entry;
	new_entry->index = size;
	new_entry->start = static_cast<int>(performance_limit_start * local_settings.speed_units.get_multiplier());
	new_entry->stop = static_cast<int>(performance_limit* local_settings.speed_units.get_multiplier());
	new_entry->units = local_settings.speed_units.get_string();

	if (current_link == nullptr || current_link->time > new_time)
	{
		first_entry = new_entry;
		return;
	}

	highlighted_index = 0;
	while ((current_link->next != nullptr && current_link->next->time < new_time))
	{
		current_link = current_link->next;
		highlighted_index++;
	}
	highlighted_index++;

	new_entry->next = current_link->next;
	current_link->next = new_entry;
	++size;
}
void Leaderboard::draw() const
{
	Entry* draw_board = first_entry;
	draw_performance_text(0.85f, 0.25f, 1.0f, 1, "~b~Leaderboard");

	if (highlighted_index >= 0)
		GRAPHICS::DRAW_RECT(0.85f, 0.34f + 0.03f * highlighted_index, 0.19f, 0.03f, 200, 200, 200, 210);

	int i = 0;
	while (draw_board != nullptr)
	{
		draw_performance_text(0.85f, 0.32f + 0.03f * i, 0.5f, 1, ("#" + std::to_string(i + 1) + "~g~ Time: " + std::to_string(draw_board->time).substr(0, 5) + "s (" + std::to_string(draw_board->start).substr(0, 3) + "-" + std::to_string(draw_board->stop).substr(0, 3) + " " + draw_board->units + ")"));
		draw_board = draw_board->next;
		i++;
	}
}
void Leaderboard::delete_last_entry()
{
	if (first_entry == nullptr)
		return;

	if (first_entry->next == nullptr)
	{
		delete first_entry;
		first_entry = nullptr;
		highlighted_index = -1;
		return;
	}
	
	Entry* temp = first_entry;
	while (temp->next->next != nullptr)
		temp = temp->next;
	
	delete temp->next;
	temp->next = nullptr;
	size--;
	highlighted_index--;
}
void Leaderboard::reset_board()
{
	while (first_entry != nullptr)
		delete_last_entry();
}

void menu_options_list_performance(menu_options_list option, char control, char selected)
{
	switch (option)
	{
	case menu_options_list::start:
		if (control == 'l')
		{
			performance_limit_start -= 0.3f;

			if (performance_limit_start < 0)
				performance_limit_start = 0;
		}
		else if (control == 'r')
		{
			performance_limit_start += 0.3f;

			if (performance_limit_start > 250.0f)
				performance_limit_start = 250.0f;
		}

		main_menu.child[2]->child[0]->child[0]->data = ": << " + std::to_string(static_cast<int>(performance_limit_start * local_settings.speed_units.get_multiplier())) + " " + local_settings.speed_units.get_string() + ">>";

			break;
	case menu_options_list::stop: 
			if (control == 'l')
			{
				performance_limit -= 0.3f;
				if (performance_limit < 0)
					performance_limit = 0;
			}
			else if (control == 'r')
			{
				performance_limit += 0.3f;
				if (performance_limit > 250.0f)
					performance_limit = 0;
			}

			main_menu.child[2]->child[0]->child[1]->data = ": << " + std::to_string(static_cast<int>(performance_limit * local_settings.speed_units.get_multiplier())) + " " + local_settings.speed_units.get_string() + ">>";
			break;
	case menu_options_list::ready:

			if (control != 'c')
				return;


			open_performance_timer = !open_performance_timer;
			perforamce_state = 0;

			if (open_performance_timer && !leaderboard.open)
				leaderboard.open = !leaderboard.open;

			if (open_performance_timer)
				main_menu.child[2]->child[0]->child[2]->title = "~r~STOP";
			else
				main_menu.child[2]->child[0]->child[2]->title = "READY";

			break;
	case menu_options_list::open_board:
			if (control != 'c')
				return;
			leaderboard.open = !leaderboard.open;
			break;
	case menu_options_list::reset_board:

			if (control != 'c')
				return;

			leaderboard.reset_board();
			break;
	case menu_options_list::mileage_counter_option:

			if (control != 'c')
				return;

			local_settings.open_mileage_counter = !local_settings.open_mileage_counter;
			// = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0);
			//mileage_counter = 0;
			break;
	default:
		break;
	}
}

/*
void menu_option_start(char control, char selected) {

	if (control == 'l')
	{
		performance_limit_start -= 0.3f;

		if (performance_limit_start < 0)
			performance_limit_start = 0;
	}
	else if (control == 'r')
	{
		performance_limit_start += 0.3f;

		if (performance_limit_start > 250.0f)
			performance_limit_start = 250.0f;
	}

	main_menu.child[2]->child[0]->child[0]->data = ": << " + std::to_string(static_cast<int>(performance_limit_start * local_settings.speed_units.get_multiplier())) + " " + local_settings.speed_units.get_string() + ">>";

}
void menu_option_stop(char control, char selected) {
	if (control == 'l')
	{
		performance_limit -= 0.3f;
		if (performance_limit < 0)
			performance_limit = 0;
	}
	else if (control == 'r')
	{
		performance_limit += 0.3f;
		if (performance_limit > 250.0f)
			performance_limit = 0;
	}

	main_menu.child[2]->child[0]->child[1]->data = ": << " + std::to_string(static_cast<int>(performance_limit * local_settings.speed_units.get_multiplier())) + " " + local_settings.speed_units.get_string() + ">>";
}

void menu_option_ready(char control, char selected) {

	if (control != 'c')
		return;


	open_performance_timer = !open_performance_timer;
	perforamce_state = 0;

	if (open_performance_timer && !leaderboard.open)
		leaderboard.open = !leaderboard.open;

	if (open_performance_timer)
		main_menu.child[2]->child[0]->child[2]->title = "~r~STOP";
	else
		main_menu.child[2]->child[0]->child[2]->title = "READY";

}
void menu_option_open_board(char control, char selected) {
	if (control != 'c')
		return;
	leaderboard.open = !leaderboard.open;
}
void menu_option_reset_board(char control, char selected) {

	if (control != 'c')
		return;

	leaderboard.reset_board();
}
void menu_option_mileage_counter(char control, char selected) {

	if (control != 'c')
		return;

	local_settings.open_mileage_counter = !local_settings.open_mileage_counter;
	// = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), 0);
	//mileage_counter = 0;
}
*/
#endif