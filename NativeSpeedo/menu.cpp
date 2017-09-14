#include "menu.h"
#include "script.h"
#include "performance_timer.h"

using namespace std;
#ifndef BAR_WIDTH
#define BAR_WIDTH 0.13f
#endif

Menu main_menu = Menu(nullptr, "Main Menu", menu_options_list::none, menu_item_type::passive, "", "");
Menu *current_menu;

DWORD menu_time = 0;
float menu_physics = 0.2;

Menu::Menu(Menu* _parent, std::string _Title, menu_options_list _item_option, menu_item_type _item_type, std::string _data, std::string _manual)
{
	for (int i = 0; i < MAX_CHILDREN; i++)
		this->child[i] = nullptr;

	this->selected = 0;
	this->item_option = _item_option;
	//this->function = _function;
	this->parent = _parent;
	this->title = _Title;
	this->item_type = _item_type;
	this->data = _data;
	this->manual = _manual;
	this->precentage = NULL;
	this->int_value = NULL;
	this->float_value = NULL;
	this->bool_value = NULL;
	this->max_value = 1.0f;
}

void Menu::draw_menu() const
{

	//TITLE BACKGROUND
	GRAPHICS::DRAW_RECT(0.075f - menu_physics, 0.102f, 0.15f, 0.085f, 30, 30, 100, 220);
	draw_menu_text(0.075f - menu_physics, 0.088f, 0.4f, 0, "NFS:Speedometer");
	//creator
	draw_menu_text(0.075f - menu_physics, 0.1285f, 0.16f, 0, "by XMOD " + string(MOD_VERSION));

	//blue background for the sub title
	GRAPHICS::DRAW_RECT(0.075f - menu_physics, 0.166f, 0.15f, 0.0425f, 10, 10, 50, 200);
	draw_menu_text(0.075f - menu_physics, 0.155f, 0.35, 0, ((this->parent != nullptr) ? this->parent->title + "->" : "") + this->title + this->data);
	//--------------------------------------------------------------------------------------------------------------
	//MARKED
	GRAPHICS::DRAW_RECT(0.075f - menu_physics, 0.1662f + 0.0425f* (static_cast<float>(this->selected) + 1), 0.15f, 0.0425f, 250, 250, 250, 235);
	//-----------------------------------------------------------------------------------------------------------------
	int i = 0;
	for (i = 0; i < MAX_CHILDREN && this->child[i] != nullptr; ++i)
	{

		std::string display_child_text = child[i]->title + child[i]->data;

		if (this->child[i]->item_type == menu_item_type::on_off)
			if ((this->child[i]->bool_value != NULL && *this->child[i]->bool_value) || (this->child[i]->int_value != NULL && *this->child[i]->int_value))
				display_child_text += "~g~: ON";
			else
				display_child_text += "~r~: OFF";

		//draw the text of the child
		draw_menu_text(0.075f - menu_physics, 0.155f + 0.0425f*static_cast<float>(i + 1), 0.32f, 0, display_child_text);
		//black background
		if (i != this->selected)
			GRAPHICS::DRAW_RECT(0.075f - menu_physics, 0.16625f + 0.0425f*static_cast<float>(i + 1), 0.15f, 0.0425f, 0, 0, 0, 180);

		if (this->child[i]->item_type == menu_item_type::bar)
		{

			float width_of_bar = 0.13f;
			if (this->child[i]->float_value != NULL)
				width_of_bar *= (*this->child[i]->float_value / this->child[i]->max_value);
			else if (this->child[i]->int_value != NULL)
				width_of_bar *= (static_cast<float>(*this->child[i]->int_value) / this->child[i]->max_value);

			if (width_of_bar < 0.001f)
				width_of_bar = 0.001f;

			// bar indicator
			if (this->selected == i)
				GRAPHICS::DRAW_RECT(0.073f - menu_physics - (0.13f - width_of_bar) / 2, 0.167f + 0.0425f*static_cast<float>(i + 1), width_of_bar, 0.02f, 150, 150, 150, 205);
			else
				GRAPHICS::DRAW_RECT(0.073f - menu_physics - (0.13f - width_of_bar) / 2, 0.167f + 0.0425f*static_cast<float>(i + 1), width_of_bar, 0.02f, 90, 120, 180, 205);

		}
	}

	//show explain manual
	draw_menu_text(0.075f - menu_physics, 0.155f + 0.0425f*static_cast<float>(i + 1), 0.21f, 0, this->child[selected]->manual);
	//orange 
	GRAPHICS::DRAW_RECT(0.075f - menu_physics, 0.16625f + 0.0425f*static_cast<float>(i + 1), 0.15f, 0.0425f, 247, 140, 0, 180);

}

void Menu::register_action()
{
	if (mod_switch_pressed(VK_NUMPAD8))
	{
		reset_mod_switch();
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
		if (this->selected > 0)
			this->selected--;
		else
		{
			while (this->selected < MAX_CHILDREN - 1 && this->child[(this->selected) + 1] != nullptr)
				this->selected++;
		}

		reset_mod_switch();
	}

	if (mod_switch_pressed(VK_NUMPAD2))
	{
		reset_mod_switch();
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);

		if (this->selected < MAX_CHILDREN - 1 && this->child[(this->selected) + 1] != nullptr)
			this->selected++;
		else
			this->selected = 0;

		reset_mod_switch();
	}

	if (mod_switch_pressed(VK_NUMPAD0))
	{
		reset_mod_switch();
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);

		if (this->parent == nullptr)
		{
			main_menu.menu_is_open = !main_menu.menu_is_open;
			save_settings();
		}
		else
			current_menu = (this->parent);

		reset_mod_switch();
	}

	if (mod_switch_pressed(VK_NUMPAD5))
	{
		reset_mod_switch();
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);

		if (this->child[this->selected]->item_type == menu_item_type::supermenu)
			current_menu = this->child[this->selected];
		else if (this->child[this->selected]->item_type == menu_item_type::button || this->child[this->selected]->item_type == menu_item_type::on_off)
			menu_options_execute(this->child[this->selected]->item_option, 'c', this->selected);
		//	(*(this->child[this->selected]->function))('c', this->selected);
		reset_mod_switch();
	}

	if (mod_switch_pressed(VK_NUMPAD4))
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);


		//if (this->child[this->selected] != nullptr)
		switch (this->child[this->selected]->item_type)
		{
		case menu_item_type::bar:
		case menu_item_type::selective:
			menu_options_execute(this->child[this->selected]->item_option, 'l', -1);
			//(*(this->child[this->selected]->function))('l', -1);
		default:
			break;
		}
	}

	if (mod_switch_pressed(VK_NUMPAD6))
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, "NAV_UP_DOWN", "HUD_FRONTEND_DEFAULT_SOUNDSET", 0);
		//if (this->child[this->selected] != nullptr)

		switch (this->child[this->selected]->item_type)
		{
		case menu_item_type::bar:
		case menu_item_type::selective:
			menu_options_execute(this->child[this->selected]->item_option, 'r', -1);
			//(*(this->child[this->selected]->function))('r', -1);
		default:
			break;
		}
	}
}

void draw_menu_text(float x, float y, float scale, int font,const std::string& text)//color: 0 - white, 1 - grey, 2 - green
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

	UI::SET_TEXT_WRAP(0.0, 0.14f);
	UI::SET_TEXT_CENTRE(1);
	UI::SET_TEXT_OUTLINE();
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text.c_str());
	UI::_DRAW_TEXT(x, y);
}

Menu::~Menu()
{
	for (size_t i = 0; i < MAX_CHILDREN; i++)
	{
		if(this->child != nullptr)
		delete this->child[i];
	}
}
void create_menu()
{
	std::string temp_title = "";

	main_menu.child[0] = new Menu(&main_menu, "Skins", menu_options_list::none, menu_item_type::supermenu, "", "Select speedometer skin.");
	main_menu.child[0]->child[0] = new Menu(main_menu.child[0], "NFS:HP2010", menu_options_list::skin, menu_item_type::button, "", "Change to NFS: Hot pursuit 2 skin.");
	main_menu.child[0]->child[1] = new Menu(main_menu.child[0], "NFS:2015", menu_options_list::skin, menu_item_type::button, "", "Change to NFS: 2015 skin.");
	main_menu.child[0]->child[2] = new Menu(main_menu.child[0], "Modern", menu_options_list::skin, menu_item_type::button, "", "Change to modern skin.");
	main_menu.child[0]->child[3] = new Menu(main_menu.child[0], "Custom", menu_options_list::skin, menu_item_type::button, "", "Change to custom skin which can be found in the folder at the game directory.");


	main_menu.child[1] = new Menu(&main_menu, "Imperial/Metric", menu_options_list::none, menu_item_type::supermenu, "", "Choose desired SI unit, km/h, mph,m/s");
	main_menu.child[1]->child[0] = new Menu(main_menu.child[1], "km/h", menu_options_list::units, menu_item_type::button, "", "Change speed units to km/h");
	main_menu.child[1]->child[1] = new Menu(main_menu.child[1], "mph", menu_options_list::units, menu_item_type::button, "", "Change speed units to mph");
	main_menu.child[1]->child[2] = new Menu(main_menu.child[1], "m/s", menu_options_list::units, menu_item_type::button, "", "Change speed units to meter per second");


	main_menu.child[2] = new Menu(&main_menu, "Performance", menu_options_list::none, menu_item_type::supermenu, "", "Vehicle performance tools");
	main_menu.child[2]->child[0] = new Menu(main_menu.child[2], "Timers", menu_options_list::none, menu_item_type::supermenu, "", "Determine the time that takes the vehicle to accelerate/brake to various speeds.");
	main_menu.child[2]->child[0]->child[0] = new Menu(main_menu.child[2]->child[0], "Start", menu_options_list::start, menu_item_type::bar, "", "");
	main_menu.child[2]->child[0]->child[0]->float_value = &performance_limit_start;
	main_menu.child[2]->child[0]->child[0]->max_value = 250.0f;
	menu_options_execute(menu_options_list::start, 0, 0);
	//menu_option_start(0, 0);
	main_menu.child[2]->child[0]->child[1] = new Menu(main_menu.child[2]->child[0], "Stop", menu_options_list::stop, menu_item_type::bar, "", "");
	main_menu.child[2]->child[0]->child[1]->float_value = &performance_limit;
	main_menu.child[2]->child[0]->child[1]->max_value = 250.0f;
//	menu_option_stop(0, 0);
	menu_options_execute(menu_options_list::stop, 0, 0);
	main_menu.child[2]->child[0]->child[2] = new Menu(main_menu.child[2]->child[0], "READY", menu_options_list::ready, menu_item_type::button, "", "Lets you get ready for the timer.");
	main_menu.child[2]->child[0]->child[3] = new Menu(main_menu.child[2]->child[0], "Show leaderboard", menu_options_list::open_board, menu_item_type::on_off, "", "Open a board on the right side of the screen of every time you get.");
	main_menu.child[2]->child[0]->child[3]->int_value = &leaderboard.open;
	main_menu.child[2]->child[0]->child[4] = new Menu(main_menu.child[2]->child[0], "Reset leaderboard", menu_options_list::reset_board, menu_item_type::button, "", "Resets the board on the right.");
	main_menu.child[2]->child[1] = new Menu(main_menu.child[2]->child[0], "Mileage Counter", menu_options_list::mileage_counter_option, menu_item_type::on_off, "", "Can be found below the speedometer.");
	main_menu.child[2]->child[1]->int_value = &local_settings.open_mileage_counter;
	main_menu.child[2]->child[2] = new Menu(main_menu.child[2]->child[0], "Reset counter", menu_options_list::reset_mileage, menu_item_type::button, "", "Resets mileage counter to 0.");


	main_menu.child[3] = new Menu(&main_menu, "Settings", menu_options_list::none, menu_item_type::supermenu, "", "Graphical and visual settings.");
	main_menu.child[3]->child[0] = new Menu(main_menu.child[3], "Max speed in red", menu_options_list::red, menu_item_type::on_off, "", "When reaching the vehicle's top speed, the speedometer speed text will turn red.");
	main_menu.child[3]->child[0]->int_value = &local_settings.enable_max_red_speed;
	main_menu.child[3]->child[1] = new Menu(main_menu.child[3], "Night Light", menu_options_list::night, menu_item_type::on_off, "", "Enables the blue hue of the speedometer at night.");
	main_menu.child[3]->child[1]->int_value = &local_settings.enable_night_light;
	main_menu.child[3]->child[2] = new Menu(main_menu.child[3], "Hide in 1st person", menu_options_list::first_person, menu_item_type::on_off, "", "Disable the speedometer when in first person view.");
	main_menu.child[3]->child[2]->int_value = &local_settings.enable_first_view;
	main_menu.child[3]->child[3] = new Menu(main_menu.child[3], "Don't hide car name", menu_options_list::do_not_hide_car_name, menu_item_type::on_off, "", "Do not display the speedo while vehicle name is displayed.");
	main_menu.child[3]->child[3]->int_value = &local_settings.wait_for_car_name;
	main_menu.child[3]->child[4] = new Menu(main_menu.child[3], "Scale", menu_options_list::scale, menu_item_type::bar, "", "Set the scale of the speedoemter.");
	main_menu.child[3]->child[4]->float_value = &local_settings.scale;
	main_menu.child[3]->child[4]->max_value = 3.0f;
	main_menu.child[3]->child[5] = new Menu(main_menu.child[3], "PosX", menu_options_list::posx, menu_item_type::bar, "", "Set the X position.");
	main_menu.child[3]->child[5]->float_value = &local_settings.posx;
	main_menu.child[3]->child[6] = new Menu(main_menu.child[3], "PosY", menu_options_list::posy, menu_item_type::bar, "", "Set the Y position.");
	main_menu.child[3]->child[6]->float_value = &local_settings.posy;
	main_menu.child[3]->child[7] = new Menu(main_menu.child[3], "Set to Default", menu_options_list::default, menu_item_type::button, "", "Sets back to the default position and scale of the speedoemeter");


	//if (get_update_herz() == 0) temp_title = "Accuracy: High"; else if (get_update_herz() == 10) temp_title = "Accuracy: Medium"; else temp_title = "Accuracy: Low";
	main_menu.child[3]->child[8] = new Menu(main_menu.child[3], "Accuracy", menu_options_list::accuracy, menu_item_type::bar, "", "Lower value may improve FPS but also lowers speedo accurate readings.");
	main_menu.child[3]->child[8]->int_value = &local_settings.update_hertz;
	main_menu.child[3]->child[8]->max_value = 27.0f;

	main_menu.child[4] = new Menu(&main_menu, "HUD Settings", menu_options_list::none, menu_item_type::supermenu, "", "Lets you disable the speedometer in specific vehicle types like Planes,Bike, etc.");
	main_menu.child[4]->child[0] = new Menu(main_menu.child[5], "Hide speedo", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[0]->int_value = &local_settings.hide_speedo;
	main_menu.child[4]->child[1] = new Menu(main_menu.child[5], "Bicycle HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[1]->int_value = &local_settings.enable_bicycle;
	main_menu.child[4]->child[2] = new Menu(main_menu.child[5], "Bike HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[2]->int_value = &local_settings.enable_bike;
	main_menu.child[4]->child[3] = new Menu(main_menu.child[5], "Boat HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[3]->int_value = &local_settings.enable_boat;
	main_menu.child[4]->child[4] = new Menu(main_menu.child[5], "Car HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[4]->int_value = &local_settings.enable_car;
	main_menu.child[4]->child[5] = new Menu(main_menu.child[5], "Heli HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[5]->int_value = &local_settings.enable_heli;
	main_menu.child[4]->child[6] = new Menu(main_menu.child[5], "Plane HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[6]->int_value = &local_settings.enable_plane;
	main_menu.child[4]->child[7] = new Menu(main_menu.child[5], "Quadbike HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[7]->int_value = &local_settings.enable_quadbike;
	main_menu.child[4]->child[8] = new Menu(main_menu.child[5], "Train HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[8]->int_value = &local_settings.enable_train;
	main_menu.child[4]->child[9] = new Menu(main_menu.child[5], "submersible HUD", menu_options_list::display_speedo, menu_item_type::on_off, "", "");
	main_menu.child[4]->child[9]->int_value = &local_settings.enable_submersible;

	main_menu.child[5] = new Menu(&main_menu, "New features", menu_options_list::none, menu_item_type::supermenu, "", "Experimental settings.");
	main_menu.child[5]->child[0] = new Menu(main_menu.child[5], "Blinkers", menu_options_list::blinkers, menu_item_type::on_off, "", "Enables turn vehicle turn signals. (left & right arrows)");
	main_menu.child[5]->child[0]->int_value = &local_settings.enable_blinkers;
	main_menu.child[5]->child[1] = new Menu(main_menu.child[5], "Shift assist", menu_options_list::shift_assist, menu_item_type::on_off, "", "Display an indicator on the top of the screen to help shift gears in time.");
	main_menu.child[5]->child[1]->int_value = &local_settings.enable_shift_assist;

	main_menu.child[5]->child[2] = new Menu(main_menu.child[5], "Shift indicators", menu_options_list::shift_indicator, menu_item_type::on_off, "", "Display small gear shift indicators in the speedometers.");
	main_menu.child[5]->child[2]->int_value = &local_settings.enable_shift_indicators;
}

void handle_menu()
{
	if (mod_switch_pressed(local_settings.open_menu_key))
	{
		
		reset_mod_switch();
		menu_time = GetTickCount() + 1000;

		main_menu.menu_is_open = !main_menu.menu_is_open;
		current_menu = &main_menu;
		save_settings();
		load_settings();
		reset_mod_switch();
	}


	if (main_menu.menu_is_open)
	{
		if (menu_physics < 0.0020)
			menu_physics = 0;
		else
			menu_physics -= 0.02;

		current_menu->register_action();
		current_menu->draw_menu();
	}
	else
	{
		if (menu_physics >= 0.2f)
			menu_physics = 0.2f;
		else
		{
			menu_physics += 0.02;
			current_menu->draw_menu();
		}

	}
}

bool is_menu_open()
{
	return main_menu.menu_is_open;
}