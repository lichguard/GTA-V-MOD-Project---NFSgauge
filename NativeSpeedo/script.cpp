#include "script.h"

using namespace std;
#include <tchar.h>
#include <fstream>

#include "utils.h"
#include "resource.h"
#include "performance_timer.h"
#include "menu.h"

DWORD trainerResetTime = 0;

float speedoAlpha;

//float current_rpm, speed;
float current_rpm = 0;
Speedometer* active_speedo = nullptr;
Speedometer* air_speedo = nullptr;
Vector3 Player_loc;
std::string speed_string;
float mileage_counter = 0;

Settings local_settings;
MemoryAccess mem;
Vehicle veh;

bool blinker_right = false;
bool blinker_left = false;

void draw_text(const std::string& msg, float x, float y, float scalex, float scaley, int red, int green, int blue)
{
	UI::SET_TEXT_FONT(local_settings.font);
	UI::SET_TEXT_SCALE(scalex, scaley);
	UI::SET_TEXT_COLOUR(red, green, blue, static_cast<int>(255.0f * speedoAlpha));  // (int)(255.0f * speedoAlpha));
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(1);
	UI::SET_TEXT_DROPSHADOW(20, 20, 20, 20, 20);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING((char *)msg.c_str());
	UI::_DRAW_TEXT(x, y);
}

bool get_key_pressed(int nVirtKey)
{
	return (GetAsyncKeyState(nVirtKey) & 0x8000) != 0;
}
bool mod_switch_pressed(int nVirtKey)
{
	return ((GetTickCount() > trainerResetTime + 200) && get_key_pressed(nVirtKey));
}
void reset_mod_switch()
{
	trainerResetTime = GetTickCount();
}

HMODULE GetCurrentModuleHandle()
{
	HMODULE hMod = NULL;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&GetCurrentModuleHandle), &hMod);
	return hMod;
}
void Extract(const WORD& wResId, const LPSTR& lpszOutputPath)
{
	if (!local_settings.extract_imgs)
		return;

	HRSRC hrsrc = FindResource(GetCurrentModuleHandle(), MAKEINTRESOURCE(wResId), _T("png"));
	HGLOBAL hLoaded = LoadResource(GetCurrentModuleHandle(), hrsrc);
	LPVOID lpLock = LockResource(hLoaded);
	DWORD dwSize = SizeofResource(GetCurrentModuleHandle(), hrsrc);
	HANDLE hFile = CreateFile(lpszOutputPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwByteWritten;
	WriteFile(hFile, lpLock, dwSize, &dwByteWritten, NULL);
	CloseHandle(hFile);
	FreeResource(hLoaded);
}
bool exists(const std::string& name)
{
	ifstream file(name);
	return !!(file);
}
int create_texture(const std::string& name)
{
	if (!exists(name))
		return 0;

	std::string path = GetCurrentModulePath(); // includes trailing slash
	return createTexture((path + name).c_str());
}

uint8_t get_topgear()
{
	return mem.GetTopGear(veh);
}

int get_gear()
{
	//return 0x0000FFFF & mem.GetGear(veh);
	return mem.GetGear(veh);
}
std::string get_gear_string()
{

	if (VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(veh)) || VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(veh)))
		return "AIR";

	if (VEHICLE::_IS_VEHICLE_ENGINE_ON(veh) == 0)
		return "P";
	else
	{
		if (CONTROLS::GET_CONTROL_VALUE(2, 76) >= 200)
			return  "N";
		else
		{
			int current_gear = get_gear();
			//if (CONTROLS::GET_CONTROL_VALUE(2, 72) > 130 && !get_direction(veh))
			if (current_gear == 0 && !VEHICLE::IS_VEHICLE_STOPPED(veh))
				return "R";
			else if (current_gear == 0)
				return std::to_string(1);
			else if (current_gear > 15 || current_gear < 0)
				return "";
			else
				return std::to_string(current_gear);
		}
	}
}
void update_engine_health()
{
	float enginehealth = VEHICLE::GET_VEHICLE_ENGINE_HEALTH(PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID()));

	if (enginehealth > 850.0f) // 100
	{
		local_settings.health = 4;

		if (VEHICLE::GET_VEHICLE_BODY_HEALTH(veh) < 850)
			local_settings.health = 3;

		if (VEHICLE::GET_VEHICLE_BODY_HEALTH(veh) < 150)
			local_settings.health = 2;


		return;
	}
	else if (enginehealth > 550.0f) //75
	{
		local_settings.health = 3;

		if (VEHICLE::GET_VEHICLE_BODY_HEALTH(veh) < 150)
			local_settings.health = 2;
		return;
	}
	else if (enginehealth > 100.0f) // 50
	{
		local_settings.health = 2;
		return;
	}
	else
	{
		local_settings.health = 1;
		return;
	}

	local_settings.health = 0;
}

void load_settings()
{
	local_settings.speed_units.type = GetPrivateProfileInt("NFSgauge", "speed_units", 1, ".\\NFSgauge.ini");
	local_settings.enable_first_view = GetPrivateProfileInt("NFSgauge", "enable_first_view", 0, ".\\NFSgauge.ini");
	local_settings.wait_for_car_name = GetPrivateProfileInt("NFSgauge", "wait_for_car_name", 0, ".\\NFSgauge.ini");
	local_settings.speedoAlphaMax = static_cast<float>(GetPrivateProfileInt("NFSgauge", "transparency_percent", 80, ".\\NFSgauge.ini")) / 100.0f;
	local_settings.open_menu_key = GetPrivateProfileInt("NFSgauge", "open_menu_key", 0x6A, ".\\NFSgauge.ini");
	local_settings.posx = static_cast<float>(GetPrivateProfileInt("NFSgauge", "posx", 8500, ".\\NFSgauge.ini")) / 10000.0f;
	local_settings.posy = static_cast<float>(GetPrivateProfileInt("NFSgauge", "posy", 8500, ".\\NFSgauge.ini")) / 10000.0f;
	local_settings.scale = static_cast<float>(GetPrivateProfileInt("NFSgauge", "scale", 1000, ".\\NFSgauge.ini")) / 1000.0f;
	local_settings.enable_night_light = GetPrivateProfileInt("NFSgauge", "enable_night_light", 1, ".\\NFSgauge.ini");
	local_settings.skin_id = static_cast<speedo_type>(GetPrivateProfileInt("NFSgauge", "skin_id", 1, ".\\NFSgauge.ini"));
	local_settings.enable_max_red_speed = GetPrivateProfileInt("NFSgauge", "enable_max_red_speed", 1, ".\\NFSgauge.ini");
	local_settings.update_hertz = GetPrivateProfileInt("NFSgauge", "update_hertz", 28, ".\\NFSgauge.ini");
	local_settings.open_mileage_counter = GetPrivateProfileInt("NFSgauge", "counter_mileage", 0, ".\\NFSgauge.ini");
	local_settings.font = GetPrivateProfileInt("NFSgauge", "font", 0, ".\\NFSgauge.ini");
	local_settings.enable_bicycle = GetPrivateProfileInt("NFSgauge", "enable_bicycle", 1, ".\\NFSgauge.ini");
	local_settings.enable_bike = GetPrivateProfileInt("NFSgauge", "enable_bike", 1, ".\\NFSgauge.ini");
	local_settings.enable_boat = GetPrivateProfileInt("NFSgauge", "enable_boat", 1, ".\\NFSgauge.ini");
	local_settings.enable_car = GetPrivateProfileInt("NFSgauge", "enable_car", 1, ".\\NFSgauge.ini");
	local_settings.enable_heli = GetPrivateProfileInt("NFSgauge", "enable_heli", 1,		".\\NFSgauge.ini");
	local_settings.enable_plane = GetPrivateProfileInt("NFSgauge", "enable_plane", 1, ".\\NFSgauge.ini");
	local_settings.enable_quadbike = GetPrivateProfileInt("NFSgauge", "enable_quadbike", 1, ".\\NFSgauge.ini");
	local_settings.enable_train = GetPrivateProfileInt("NFSgauge", "enable_train", 1, ".\\NFSgauge.ini");
	local_settings.enable_submersible = GetPrivateProfileInt("NFSgauge", "enable_submersible", 1, ".\\NFSgauge.ini");
	local_settings.blinker_left_key = GetPrivateProfileInt("NFSgauge", "blinker_left_key", 0x25, ".\\NFSgauge.ini");
	local_settings.blinker_right_key = GetPrivateProfileInt("NFSgauge", "blinker_right_key", 0x27, ".\\NFSgauge.ini");
	local_settings.enable_blinkers = GetPrivateProfileInt("NFSgauge", "blinkers", 1, ".\\NFSgauge.ini");
	local_settings.enable_shift_assist = GetPrivateProfileInt("NFSgauge", "shift_assist", 0, ".\\NFSgauge.ini");
	local_settings.enable_shift_indicators = GetPrivateProfileInt("NFSgauge", "shift_indicators", 1, ".\\NFSgauge.ini");
	local_settings.enable_custom_bg = GetPrivateProfileInt("NFSgauge", "enable_custom_bg", 0, ".\\NFSgauge.ini");
	
	local_settings.extract_imgs = (GetPrivateProfileInt("NFSgauge", "extract_imgs", 1, ".\\NFSgauge.ini") == 1);
	if (local_settings.update_hertz > 28)
		local_settings.update_hertz = 28;
	if (local_settings.update_hertz < 0)
		local_settings.update_hertz = 0;
}
void save_settings()
{

#ifndef ToString
#define ToString(x) std::to_string(x).c_str()
#endif

	WritePrivateProfileString("NFSgauge", "skin_id", ToString(static_cast<int>(local_settings.skin_id)), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "speed_units", ToString(local_settings.speed_units.type), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "counter_mileage", ToString(local_settings.open_mileage_counter), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_max_red_speed", ToString(local_settings.enable_max_red_speed), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_night_light", ToString(local_settings.enable_night_light), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "update_hertz", ToString(local_settings.update_hertz), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_first_view", ToString(local_settings.enable_first_view), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_bicycle", ToString(local_settings.enable_bicycle), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_bike", ToString(local_settings.enable_bike), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_boat", ToString(local_settings.enable_boat), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_car", ToString(local_settings.enable_car), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_heli", ToString(local_settings.enable_heli), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_plane", ToString(local_settings.enable_plane), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_quadbike", ToString(local_settings.enable_quadbike), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_train", ToString(local_settings.enable_train), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "enable_submersible", ToString(local_settings.enable_submersible), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "scale", ToString(local_settings.scale*1000.0f), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "posy", ToString(local_settings.posy*10000.0f), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "posx", ToString(local_settings.posx*10000.0f), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "blinkers", ToString(local_settings.enable_blinkers), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "shift_assist", ToString(local_settings.enable_shift_assist), ".\\NFSgauge.ini");
	WritePrivateProfileString("NFSgauge", "shift_indicators", ToString(local_settings.enable_shift_indicators), ".\\NFSgauge.ini");

}

void handle_Speedo_skin()
{
	if (speedoAlpha != local_settings.speedoAlphaMax)
	{
		if (speedoAlpha < 0.0f) speedoAlpha = 0.0f;
		if (speedoAlpha < local_settings.speedoAlphaMax) speedoAlpha += 0.02f;
		if (speedoAlpha > local_settings.speedoAlphaMax) speedoAlpha = local_settings.speedoAlphaMax;
	}
	//Red speed max
	speed_string = std::to_string(static_cast<int>((ENTITY::GET_ENTITY_SPEED(veh) * local_settings.speed_units.get_multiplier())));
	if (ENTITY::GET_ENTITY_SPEED(veh) > VEHICLE::_GET_VEHICLE_MAX_SPEED(ENTITY::GET_ENTITY_MODEL(veh)) + 2.0f && local_settings.enable_max_red_speed)
		speed_string = "~r~" + speed_string;


	if (VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(veh)) || VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(veh)))
	{
		if (air_speedo == nullptr)
			air_speedo = new modern_speedometer();

		air_speedo->draw_speedo();
		return;
	}
	else if (active_speedo ==nullptr|| active_speedo->get_active_id() != local_settings.skin_id)
	{
		if (active_speedo != nullptr)
			delete active_speedo;

		switch (local_settings.skin_id)
		{
		case 1:
			active_speedo = new NFSHP_speedometer();
			break;
		case 2:
			active_speedo = new NFS2015_speedometer();
			break;
		case 3:
			active_speedo = new modern_speedometer();
			break;
		case 4:
			active_speedo = new custom_speedometer();
			break;
		case 5:
			active_speedo = new air_speedometer();
			break;
		default:
			active_speedo = nullptr;
			break;
		}
	}

	active_speedo->draw_speedo();
}
void handle_blinkers()
{
	if (!local_settings.enable_blinkers)
		return;
	if (mod_switch_pressed(local_settings.blinker_right_key))
	{
		reset_mod_switch();
		trainerResetTime = GetTickCount() + 100;
		blinker_right = !blinker_right;
		VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh, 0, blinker_right);
		reset_mod_switch();
	}

	if (mod_switch_pressed(local_settings.blinker_left_key))
	{
		reset_mod_switch();
		trainerResetTime = GetTickCount() + 100;
		blinker_left = !blinker_left;
		VEHICLE::SET_VEHICLE_INDICATOR_LIGHTS(veh, 1, blinker_left);
		reset_mod_switch();
	}
}
void handle_mileage()
{
	if (local_settings.open_mileage_counter)
	{
		Ped playerPed = PLAYER::PLAYER_PED_ID();
		if (Player_loc.x != 0 && Player_loc.y != 0 && Player_loc.z != 0)
			mileage_counter += GAMEPLAY::GET_DISTANCE_BETWEEN_COORDS(Player_loc.x, Player_loc.y, Player_loc.z, ENTITY::GET_ENTITY_COORDS(playerPed, 0).x, ENTITY::GET_ENTITY_COORDS(playerPed, 0).y, ENTITY::GET_ENTITY_COORDS(playerPed, 0).z, 0);
		if (local_settings.speed_units.type == 1)
			draw_text(std::to_string(mileage_counter * 0.001f).substr(0, 4) + " km", 0.85f, 0.93f, 0.5f, 0.5f, 255, 255, 255);
		else
			draw_text(std::to_string(mileage_counter * 0.000621371f).substr(0, 4) + " miles", 0.85f, 0.93f, 0.5f, 0.5f, 255, 255, 255);

		Player_loc = ENTITY::GET_ENTITY_COORDS(playerPed, 0);
	}
}

bool display_check()
{
	Player player = PLAYER::PLAYER_ID();
	Ped playerPed = PLAYER::PLAYER_PED_ID();


	if (local_settings.hide_speedo)
	{
		speedoAlpha = 0;
		return 0;
	}


	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player) || !ENTITY::DOES_ENTITY_EXIST(veh))
		return 0;


	if (veh < 1 || !PED::IS_PED_IN_VEHICLE(playerPed, veh, 0))
	{
		speedoAlpha = 0;
		return 0;
	}


	if (CAM::GET_FOLLOW_VEHICLE_CAM_VIEW_MODE() == 4 && local_settings.enable_first_view)
	{
		speedoAlpha = 0;
		return 0;
	}

	
	// check for player ped death and player arrest
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE))
		return 0;


	if (UI::IS_HUD_HIDDEN())
		return 0;

	
	if (PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(playerPed))
		return 0;


	if (UI::IS_HUD_COMPONENT_ACTIVE(3))
		return 0;

	// check if player is in a vehicle and vehicle name isn't being displayed

	if (local_settings.wait_for_car_name && UI::IS_HUD_COMPONENT_ACTIVE(6))
	{
		speedoAlpha = 5;
		return 0;
	}
	if ((VEHICLE::IS_THIS_MODEL_A_BICYCLE(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_bicycle) ||
		(VEHICLE::IS_THIS_MODEL_A_BIKE(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_bike) ||
		(VEHICLE::IS_THIS_MODEL_A_BOAT(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_boat) ||
		(VEHICLE::IS_THIS_MODEL_A_CAR(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_car) ||
		(VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_heli) ||
		(VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_plane) ||
		(VEHICLE::IS_THIS_MODEL_A_QUADBIKE(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_quadbike) ||
		(VEHICLE::IS_THIS_MODEL_A_TRAIN(ENTITY::GET_ENTITY_MODEL(veh)) && !local_settings.enable_train) ||
		(VEHICLE::_IS_THIS_MODEL_A_SUBMERSIBLE(ENTITY::GET_ENTITY_MODEL(veh)) && local_settings.enable_submersible)
		)
		return 0;

	return 1;
}
void update()
{
	
	veh = PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID());
	//speed = ENTITY::GET_ENTITY_SPEED(veh);
	handle_menu();

	if (!display_check())
		return;

	handle_blinkers();
	handle_timers();
	handle_mileage();
	handle_Speedo_skin();
	active_speedo->shift_assist_function();
}
void main()
{
	srand(GetTickCount());
	mem = MemoryAccess();
	local_settings = Settings();
	load_settings();
	create_menu();

	if (active_speedo != nullptr)
		delete active_speedo;

	while (true)
	{
		update();
		WAIT(MAX_ACCURACY - local_settings.update_hertz);
	}
}
void ScriptMain()
{
	main();
}

void menu_options_execute(menu_options_list option, char control, char selected)
{
	switch (option)
	{
	case menu_options_list::skin:
		if (control != 'c')
			return;
		local_settings.skin_id = static_cast<speedo_type>(selected + 1);
		break;
	case menu_options_list::units:

		if (control != 'c')
			return;

		if (selected == 0)
			local_settings.speed_units.type = 1;
		else if (selected == 1)
			local_settings.speed_units.type = 0;
		else if (selected == 2)
			local_settings.speed_units.type = 2;

		menu_options_execute(menu_options_list::start, 0, 0);
		menu_options_execute(menu_options_list::stop, 0, 0);
		//	menu_option_start(0, 0);
		//	menu_option_stop(0, 0);
		break;
	case menu_options_list::red:
		if (control != 'c')
			return;
		local_settings.enable_max_red_speed = !local_settings.enable_max_red_speed;
		break;
	case menu_options_list::night:
		if (control != 'c')
			return;
		local_settings.enable_night_light = !local_settings.enable_night_light;
		break;
	case menu_options_list::first_person:
		if (control != 'c')
			return;
		local_settings.enable_first_view = !local_settings.enable_first_view;
		break;
	case menu_options_list::scale:

		if (control == 'l')
		{
			local_settings.scale -= 0.007f;
			if (local_settings.scale < 0.01f)
				local_settings.scale = 0.01f;
		}
		if (control == 'r')
		{

			local_settings.scale += 0.007f;
			if (local_settings.scale > 3.0f)
				local_settings.scale = 3.0f;
			0;
		}

		break;
	case menu_options_list::posx:
		if (control == 'l')
		{
			local_settings.posx -= 0.007f;
			if (local_settings.posx < 0.01f)
				local_settings.posx = 0.01f;
		}
		if (control == 'r')
		{
			local_settings.posx += 0.007f;
			if (local_settings.posx > 1.0f)
				local_settings.posx = 1.0f;
		}
		break;
	case menu_options_list::posy:
		if (control == 'l')
		{
			local_settings.posy -= 0.007f;
			if (local_settings.posy < 0.01f)
				local_settings.posy = 0.01f;
		}
		if (control == 'r')
		{
			local_settings.posy += 0.007f;
			if (local_settings.posy > 1.0f)
				local_settings.posy = 1.0f;
		}
		break;
	case menu_options_list::default:
		if (control != 'c')
			return;
		local_settings.scale = 1.0f;
		local_settings.posy = 0.85f;
		local_settings.posx = 0.85f;
		local_settings.update_hertz = 28;
		break;
	case menu_options_list::accuracy:

		if (control == 'l')
		{
			local_settings.update_hertz--;
			if (local_settings.update_hertz < 1)
				local_settings.update_hertz = 1;
		}
		else if (control == 'r')
		{
			local_settings.update_hertz++;
			if (local_settings.update_hertz > MAX_ACCURACY)
				local_settings.update_hertz = MAX_ACCURACY;
		}
		break;
	case menu_options_list::display_speedo:

		if (control != 'c')
			return;

		switch (selected)
		{
		case 0:
			//local_settings. user_press_hide = 1;
			local_settings.hide_speedo = !local_settings.hide_speedo;
			break;
		case 1:
			local_settings.enable_bicycle = !local_settings.enable_bicycle;
			break;
		case 2:
			local_settings.enable_bike = !local_settings.enable_bike;
			break;
		case 3:
			local_settings.enable_boat = !local_settings.enable_boat;
			break;
		case 4:
			local_settings.enable_car = !local_settings.enable_car;
			break;
		case 5:
			local_settings.enable_heli = !local_settings.enable_heli;
			break;
		case 6:
			local_settings.enable_plane = !local_settings.enable_plane;
			break;
		case 7:
			local_settings.enable_quadbike = !local_settings.enable_quadbike;
			break;
		case 8:
			local_settings.enable_train = !local_settings.enable_train;
			break;
		case 9:
			local_settings.enable_submersible = !local_settings.enable_submersible;
			break;
		default:
			break;
		}
		break;
	case menu_options_list::reset_mileage:
		if (control != 'c')
			return;
		mileage_counter = 0;
		break;
	case menu_options_list::blinkers:
		if (control != 'c')
			return;
		local_settings.enable_blinkers = !local_settings.enable_blinkers;
		break;
	case menu_options_list::shift_assist:
		if (control != 'c')
			return;
		local_settings.enable_shift_assist = !local_settings.enable_shift_assist;
		break;
	case menu_options_list::shift_indicator:
		if (control != 'c')
			return;
		local_settings.enable_shift_indicators = !local_settings.enable_shift_indicators;
		break;
	case menu_options_list::do_not_hide_car_name:
		if (control != 'c')
			return;
		local_settings.wait_for_car_name = !local_settings.wait_for_car_name;
		break;
	case menu_options_list::start: case menu_options_list::stop: case menu_options_list::ready: case menu_options_list::open_board: case menu_options_list::reset_board: case menu_options_list::mileage_counter_option:
		menu_options_list_performance(option, control, selected);
	default:
		break;
	}
}

#pragma region SPEEDOMETER_FUNCTIONS
	
int textureid_assist_shift_up = 0;
int textureid_assist_shift_down = 0;
	void create_shift_assist_texutre()
{
	if (textureid_assist_shift_up != 0 || textureid_assist_shift_down != 0)
		return;

	CreateDirectory("custom_NFSgauge", NULL);
	Extract(IDB_PNG19, "custom_NFSgauge\\NFSgauge_rc19.temp");
	textureid_assist_shift_up = create_texture("custom_NFSgauge\\NFSgauge_rc19.temp");
	Extract(IDB_PNG20, "custom_NFSgauge\\NFSgauge_rc20.temp");
	textureid_assist_shift_down = create_texture("custom_NFSgauge\\NFSgauge_rc20.temp");

}
	void NFSHP_speedometer::create_textures()
	{
		CreateDirectory("custom_NFSgauge", NULL);
		Extract(IDB_PNG1, "custom_NFSgauge\\NFSgauge_rc1.temp");
		idTextureSpeedoBack = create_texture("custom_NFSgauge\\NFSgauge_rc1.temp");
		Extract(IDB_PNG2, "custom_NFSgauge\\NFSgauge_rc2.temp");
		idTextureSpeedoBackSkeleton = create_texture("custom_NFSgauge\\NFSgauge_rc2.temp");
		Extract(IDB_PNG3, "custom_NFSgauge\\NFSgauge_rc3.temp");
		id_back_textures[0] = create_texture("custom_NFSgauge\\NFSgauge_rc3.temp");
		Extract(IDB_PNG4, "custom_NFSgauge\\NFSgauge_rc4.temp");
		id_back_textures[1] = create_texture("custom_NFSgauge\\NFSgauge_rc4.temp");
		Extract(IDB_PNG5, "custom_NFSgauge\\NFSgauge_rc5.temp");
		id_back_textures[2] = create_texture("custom_NFSgauge\\NFSgauge_rc5.temp");
		Extract(IDB_PNG6, "custom_NFSgauge\\NFSgauge_rc6.temp");
		id_back_textures[3] = create_texture("custom_NFSgauge\\NFSgauge_rc6.temp");
		Extract(IDB_PNG7, "custom_NFSgauge\\NFSgauge_rc7.temp");
		id_back_textures[4] = create_texture("custom_NFSgauge\\NFSgauge_rc7.temp");
		Extract(IDB_PNG8, "custom_NFSgauge\\NFSgauge_rc8.temp");
		idTextureSpeedoArrow = create_texture("custom_NFSgauge\\NFSgauge_rc8.temp");
		Extract(IDB_PNG9, "custom_NFSgauge\\NFSgauge_rc9.temp");
		idTextureSpeedoBack_night = create_texture("custom_NFSgauge\\NFSgauge_rc9.temp");

		Extract(IDB_PNG17, "custom_NFSgauge\\NFSgauge_rc17.temp");
		idTextureShiftindicator = create_texture("custom_NFSgauge\\NFSgauge_rc17.temp");
		Extract(IDB_PNG18, "custom_NFSgauge\\NFSgauge_rc18.temp");
		idTextureBlinkerindicator1 = create_texture("custom_NFSgauge\\NFSgauge_rc18.temp");
		idTextureBlinkerindicator2 = create_texture("custom_NFSgauge\\NFSgauge_rc18.temp");
	}
	void NFS2015_speedometer::create_textures()
	{
		CreateDirectory("custom_NFSgauge", NULL);
		Extract(IDB_PNG10, "custom_NFSgauge\\NFSgauge_rc10.temp");
		idTextureSpeedoBackSkeleton = create_texture("custom_NFSgauge\\NFSgauge_rc10.temp");
		Extract(IDB_PNG11, "custom_NFSgauge\\NFSgauge_rc11.temp");
		idTextureSpeedoArrow = create_texture("custom_NFSgauge\\NFSgauge_rc11.temp");
		Extract(IDB_PNG12, "custom_NFSgauge\\NFSgauge_rc12.temp");
		idTextureSpeedoBackSkeleton_kmph = create_texture("custom_NFSgauge\\NFSgauge_rc12.temp");
		Extract(IDB_PNG17, "custom_NFSgauge\\NFSgauge_rc17.temp");
		idTextureShiftindicator = create_texture("custom_NFSgauge\\NFSgauge_rc17.temp");
		Extract(IDB_PNG18, "custom_NFSgauge\\NFSgauge_rc18.temp");
		idTextureBlinkerindicator1 = create_texture("custom_NFSgauge\\NFSgauge_rc18.temp");
		idTextureBlinkerindicator2 = create_texture("custom_NFSgauge\\NFSgauge_rc18.temp");
	}
	void modern_speedometer::create_textures()
	{
		CreateDirectory("custom_NFSgauge", NULL);
		Extract(IDB_PNG13, "custom_NFSgauge\\NFSgauge_rc13.temp");
		idTextureSpeedoBackSkeleton = create_texture("custom_NFSgauge\\NFSgauge_rc13.temp");
		Extract(IDB_PNG14, "custom_NFSgauge\\NFSgauge_rc14.temp");
		idTextureSpeedoArrow = create_texture("custom_NFSgauge\\NFSgauge_rc14.temp");
		Extract(IDB_PNG17, "custom_NFSgauge\\NFSgauge_rc17.temp");
		idTextureShiftindicator = create_texture("custom_NFSgauge\\NFSgauge_rc17.temp");
		Extract(IDB_PNG18, "custom_NFSgauge\\NFSgauge_rc18.temp");
		idTextureBlinkerindicator1 = create_texture("custom_NFSgauge\\NFSgauge_rc18.temp");
		idTextureBlinkerindicator2 = create_texture("custom_NFSgauge\\NFSgauge_rc18.temp");
	}
	void custom_speedometer::create_textures()
	{

		if (exists("custom_NFSgauge\\custom_background.png"))
			idTextureSpeedoBack = create_texture("custom_NFSgauge\\custom_background.png");
		else
			idTextureSpeedoBack = 0;

		if (exists("custom_NFSgauge\\custom_skeleton.png"))
			idTextureSpeedoBackSkeleton = create_texture("custom_NFSgauge\\custom_skeleton.png");
		else
			idTextureSpeedoBackSkeleton = 0;

		if (exists("custom_NFSgauge\\custom_health_0.png"))
			id_back_textures[0] = create_texture("custom_NFSgauge\\custom_health_0.png");
		else
			id_back_textures[0] = 0;

		if (exists("custom_NFSgauge\\custom_health_1.png"))
			id_back_textures[1] = create_texture("custom_NFSgauge\\custom_health_1.png");
		else
			id_back_textures[1] = 0;

		if (exists("custom_NFSgauge\\custom_health_2.png"))
			id_back_textures[2] = create_texture("custom_NFSgauge\\custom_health_2.png");
		else
			id_back_textures[2] = 0;

		if (exists("custom_NFSgauge\\custom_health_3.png"))
			id_back_textures[3] = create_texture("custom_NFSgauge\\custom_health_3.png");
		else
			id_back_textures[3] = 0;

		if (exists("custom_NFSgauge\\custom_health_4.png"))
			id_back_textures[4] = create_texture("custom_NFSgauge\\custom_health_4.png");
		else
			id_back_textures[4] = 0;

		if (exists("custom_NFSgauge\\custom_needle.png"))
			idTextureSpeedoArrow = create_texture("custom_NFSgauge\\custom_needle.png");
		else
			idTextureSpeedoArrow = 0;
		if (exists("custom_NFSgauge\\custom_background_night.png"))
			idTextureSpeedoBack_night = create_texture("custom_NFSgauge\\custom_background_night.png");
		else
			idTextureSpeedoBack_night = 0;

	}
	void air_speedometer::create_textures()
	{
		CreateDirectory("custom_NFSgauge", NULL);
		Extract(IDB_PNG15, "custom_NFSgauge\\NFSgauge_rc15.temp");
		idTextureSpeedoBack_airspeed = create_texture("custom_NFSgauge\\NFSgauge_rc15.temp");
		Extract(IDB_PNG16, "custom_NFSgauge\\NFSgauge_rc16.temp");
		idTextureSpeedoArrow_airspeed = create_texture("custom_NFSgauge\\NFSgauge_rc16.temp");
	}
	
	float pos_correction(float amount, bool x)
	{
		if (x)
			return local_settings.posx - amount * (local_settings.scale - 1);
		else
			return local_settings.posy - amount * (local_settings.scale - 1);
	}

	void NFSHP_speedometer::draw_speedo() const
	{
		update_engine_health();
		int lightsOn = 0;
		int highbeamsOn = 0;
		//Smooth RPM transition and correction
		current_rpm += 0.1924f * (get_RPM() * 100 - current_rpm) - 0.3f;

		if (VEHICLE::_IS_VEHICLE_ENGINE_ON(veh) == 0 || current_rpm < 10.8f)
			current_rpm = 10.8f;

		float rotation = current_rpm / 143.0f /*circle max*/ + 0.618f /*arrow initial rotation*/;
		float screencorrection = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(FALSE);
		// 0 rev with engine is off.

		GRAPHICS::DRAW_RECT(local_settings.posx, pos_correction(0.045f,false) - 0.05f , 0.023f * local_settings.scale, 0.035f * local_settings.scale, 0, 0, 0, static_cast<int>(230.0f * speedoAlpha));  //1

		draw_text("~r~" + get_gear_string(), local_settings.posx - 0.0007f, pos_correction(0.07f,false) - 0.069f, 0.5f * local_settings.scale, 0.5f * local_settings.scale);  //

			//shift indicator																													
		if (local_settings.enable_shift_indicators)
		{
			int current_gear = get_gear();
			if (mem.GetClutch(veh) >= 0.9f && current_gear >= 1)
			{
				float RPM = get_RPM();
				if (RPM > 0.9f && current_gear < get_topgear())
				{
					drawTexture(idTextureShiftindicator, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, pos_correction(-0.02f, true) + 0.019f, pos_correction(0.045f, false) - 0.045f, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha * ((GetTickCount() % 500) / 500.0f));  //3
				}
				if (RPM <= 0.4f && current_gear > 1)
				{
					drawTexture(idTextureShiftindicator, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, pos_correction(0.02f, true) - 0.019f, pos_correction(0.03f, false) - 0.045f, 0.5f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha * ((GetTickCount() % 500) / 500.0f));

				}
			}
		}
		
		//blinker
		if (local_settings.enable_blinkers)
		{
			if (blinker_right && GetTickCount() % 1000 < 500)
				drawTexture(idTextureBlinkerindicator1, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, pos_correction(-0.02f, true) + 0.022f, pos_correction(-0.04f, false) + 0.045f, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);

			//blinkerleft
			if (blinker_left && GetTickCount() % 1000 < 500)
				drawTexture(idTextureBlinkerindicator2, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, pos_correction(0.02f, true) - 0.022f, pos_correction(-0.04f, false) + 0.045f, 0.5f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);
		}																																																									// Night speedometer
		VEHICLE::GET_VEHICLE_LIGHTS_STATE(veh, &lightsOn, &highbeamsOn);
		if ((lightsOn != 0 || highbeamsOn != 0) && local_settings.enable_night_light)
			drawTexture(idTextureSpeedoBack_night, 0, 9996, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3
		else
			drawTexture(idTextureSpeedoBack, 0, 9996, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3


		drawTexture(idTextureSpeedoBackSkeleton, 0, 9997, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3
		drawTexture(idTextureSpeedoArrow, 0, 9999, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, rotation, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //4

		drawTexture(id_back_textures[local_settings.health], 0, 9998, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3

																																																													  //background for the speed meter
		GRAPHICS::DRAW_RECT(local_settings.posx, local_settings.posy, 0.065f * local_settings.scale, 0.059f * local_settings.scale, 0, 0, 0, static_cast<int>(230.0f * speedoAlpha));  //4

		draw_text(speed_string, local_settings.posx - 0.0011f, local_settings.posy - 0.03f - (0.03f *local_settings.scale - 0.03f), 0.85f * local_settings.scale, 0.85f * local_settings.scale); //6

		draw_text(local_settings.speed_units.get_string(), local_settings.posx + 0.0f, local_settings.posy + 0.03f + (0.03f *local_settings.scale - 0.03f), 0.4f * local_settings.scale, 0.4f * local_settings.scale); //7


	}
	void NFS2015_speedometer::draw_speedo() const
	{
		//Smooth RPM transition and correction
		current_rpm += 0.1924f * (get_RPM() * 100 - current_rpm) - 0.3f;

		if (VEHICLE::_IS_VEHICLE_ENGINE_ON(veh) == 0 || get_RPM() == 0)
			current_rpm = 11.2f;

		float rotation = current_rpm / 130.0f /*circle max*/ + 0.579f /*arrow initial rotation*/;
		float screencorrection = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(FALSE);
		// 0 rev with engine is off.

		//gear dispaly
		draw_text("~b~" + get_gear_string(), local_settings.posx - 0.0000f, local_settings.posy - 0.055f - (0.07f *local_settings.scale - 0.07f), 0.55f * local_settings.scale, 0.55f *local_settings.scale);  //

		if (local_settings.enable_shift_indicators)
		{
			int current_gear = get_gear();
			if (mem.GetClutch(veh) >= 0.9f && current_gear >= 1)
			{
				float RPM = get_RPM();
				if (RPM > 0.9f && current_gear < get_topgear())
				{
					drawTexture(idTextureShiftindicator, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, pos_correction(-0.02f, true) + 0.018f, pos_correction(0.07f, false) - 0.037f, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha * ((GetTickCount() % 500) / 500.0f));

				}
				if (RPM <= 0.4f && current_gear > 1)
				{
					drawTexture(idTextureShiftindicator, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, pos_correction(0.02f, true) - 0.018f, pos_correction(0.07f, false) - 0.037f, 0.5f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha * ((GetTickCount() % 500) / 500.0f));

				}
			}
		}

		drawTexture(idTextureSpeedoArrow, 0, 9999, 100, 0.17f * local_settings.scale, 0.17f *local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, rotation, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //4

																								
		draw_text(speed_string, local_settings.posx - 0.0011f, local_settings.posy + 0.061f + 0.06f * local_settings.scale - 0.06f, 0.85f * local_settings.scale, 0.85f * local_settings.scale); //6

		if (local_settings.speed_units.type)
			drawTexture(idTextureSpeedoBackSkeleton_kmph, 0, 9997, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3
		else
			drawTexture(idTextureSpeedoBackSkeleton, 0, 9997, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3

	}
	void modern_speedometer::draw_speedo() const
	{
		//Smooth RPM transition and correction
		current_rpm += 0.1924f * (get_RPM() * 100 - current_rpm) - 0.3f;

		if (VEHICLE::_IS_VEHICLE_ENGINE_ON(veh) == 0 || get_RPM() == 0)
			current_rpm = 10.5f;

		float rotation = current_rpm / 108.5f /*circle max*/ + 0.40f /*arrow initial rotation*/;
		float screencorrection = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(FALSE);
		// 0 rev with engine is off.

		draw_text("~o~" + get_gear_string(), local_settings.posx + 0.000f, pos_correction(0.02f, false) - 0.0245f, 0.7f *local_settings.scale + 0.2f, 0.7f * local_settings.scale + 0.2f, 250, 20, 100); //6

		//shift
		if (local_settings.enable_shift_indicators && mem.GetClutch(veh) >= 0.9f && get_gear() >= 1)
		{
			float RPM = get_RPM();
			if (RPM > 0.9f && get_gear() < get_topgear())
			{
				drawTexture(idTextureShiftindicator, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, local_settings.posx + 0.018f, local_settings.posy - 0.0f, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha * ((GetTickCount() % 500) / 500.0f));

			}
			if (RPM <= 0.4f && get_gear() > 1)
			{
				drawTexture(idTextureShiftindicator, 0, 9999, 100, 0.01f * local_settings.scale, 0.01f * local_settings.scale, 0.5f, 0.5f, local_settings.posx - 0.018f, local_settings.posy - 0.0f, 0.5f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha * ((GetTickCount() % 500) / 500.0f));

			}
		}

		float width_of_bar = 0.084f * get_RPM();
		if (!(VEHICLE::IS_THIS_MODEL_A_PLANE(ENTITY::GET_ENTITY_MODEL(veh)) || VEHICLE::IS_THIS_MODEL_A_HELI(ENTITY::GET_ENTITY_MODEL(veh))))
		{


			//pos_correction(-0.21f, true) + local_settings.scale * 0.0176f - (0.084f - width_of_bar) / 2
			//float custom_bar_x = pos_correction(-0.1f, true) + 0.8f;
			GRAPHICS::DRAW_RECT(
				local_settings.posx + 0.0176f - (0.084f - width_of_bar) / 2,
			//	pos_correction(-0.21f, true) + local_settings.scale * 0.0176f - (0.084f - width_of_bar) / 2,
				pos_correction(-0.060f, false) + 0.0627f,
				width_of_bar * local_settings.scale,
				0.025f * local_settings.scale,
				160, 5, 5, 255);

		}
		//engine health

		width_of_bar = 0.084f * (VEHICLE::GET_VEHICLE_ENGINE_HEALTH(PED::GET_VEHICLE_PED_IS_USING(PLAYER::PLAYER_PED_ID())) / 1000.0f);
		if (width_of_bar < 0)
			width_of_bar = 0;

		GRAPHICS::DRAW_RECT(local_settings.posx + 0.0176f - (0.084f - width_of_bar) / 2,
			pos_correction(0.035f, false) - 0.0445f,
			width_of_bar * local_settings.scale,
			0.010f * local_settings.scale,
			0, 0, 0, 220);


		//drawTexture(idTextureSpeedoArrow, 0, 9999, 100, 1.0f * local_settings.scale, 1.0f * local_settings.scale, 0.5f, 0.5f, local_settings.posx  + current_rpm * 0.5f, local_settings.posy, 0, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //4
		drawTexture(idTextureSpeedoBackSkeleton, 0, 0, 100, 0.12f * local_settings.scale, 0.12f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3


		draw_text(speed_string, pos_correction(0.07f, true) - 0.09f, pos_correction(0.05f,false) - 0.06f , 1.2f * local_settings.scale + 0.3f, 1.2f * local_settings.scale + 0.3f); //6

																 //draws units 
		draw_text(local_settings.speed_units.get_string(), local_settings.posx - 0.09f, pos_correction(0.12f, false) - 0.15f, 0.5f * local_settings.scale, 0.5f *local_settings.scale, 255, 255, 255); //7
	}
	void custom_speedometer::draw_speedo() const 
	{
		update_engine_health();
		int lightsOn = 0;
		int highbeamsOn = 0;

		if (idTextureSpeedoBackSkeleton + idTextureSpeedoArrow + idTextureSpeedoBack == 0)
			draw_text("~g~Custom skin can be found in 'custom' folder", local_settings.posx, local_settings.posy - 0.19f * local_settings.scale, 0.4f, 0.4f);

		//Smooth RPM transition and correction
		current_rpm += 0.1924f * (get_RPM() * 100 - current_rpm) - 0.3f;

		if (VEHICLE::_IS_VEHICLE_ENGINE_ON(veh) == 0 || get_RPM() == 0)
			current_rpm = 10.8f;

		float rotation = current_rpm / 143.0f /*circle max*/ + 0.618f /*arrow initial rotation*/;
		float screencorrection = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(FALSE);
		// 0 rev with engine is off.


		//GRAPHICS::DRAW_RECT(posx, posy - 0.05f - (0.055f *scale - 0.055f), 0.023f * scale, 0.035f * scale, 0, 0, 0, static_cast<int>(230.0f * speedoAlpha));  //1
		draw_text("~r~" + get_gear_string(), local_settings.posx - 0.0007f, local_settings.posy - 0.069f - (0.07f *local_settings.scale - 0.07f), 0.5f * local_settings.scale, 0.5f * local_settings.scale);  //
																																																   // Night speedometer
		VEHICLE::GET_VEHICLE_LIGHTS_STATE(veh, &lightsOn, &highbeamsOn);
		if ((lightsOn != 0 || highbeamsOn != 0) && local_settings.enable_night_light)
			drawTexture(idTextureSpeedoBack_night, 0, 9996, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3
		else
			drawTexture(idTextureSpeedoBack, 0, 9996, 100, 0.17f * local_settings.scale, 0.17f *local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3

		drawTexture(idTextureSpeedoBackSkeleton, 0, 9997, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3
		drawTexture(idTextureSpeedoArrow, 0, 9999, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, rotation, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //4
		drawTexture(id_back_textures[local_settings.health], 0, 9998, 100, 0.17f * local_settings.scale, 0.17f *local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3

		//show speed string
		draw_text(speed_string, local_settings.posx - 0.0011f, local_settings.posy - 0.03f - (0.03f *local_settings.scale - 0.03f), 0.85f * local_settings.scale, 0.85f * local_settings.scale); //6

	//show units_speed
			draw_text(local_settings.speed_units.get_string(), local_settings.posx - 0.002f, local_settings.posy + 0.03f + (0.03f *local_settings.scale - 0.03f), 0.4f  * local_settings.scale, 0.4f * local_settings.scale); //9

			if (local_settings.enable_custom_bg)
			{
				GRAPHICS::DRAW_RECT(local_settings.posx, pos_correction(0.045f, false) - 0.05f, 0.023f * local_settings.scale, 0.035f * local_settings.scale, 0, 0, 0, static_cast<int>(230.0f * speedoAlpha));  //1
				GRAPHICS::DRAW_RECT(local_settings.posx, local_settings.posy, 0.065f * local_settings.scale, 0.059f * local_settings.scale, 0, 0, 0, static_cast<int>(230.0f * speedoAlpha));  //4
			}

	}
	void air_speedometer::draw_speedo() const
	{
		//Smooth RPM transition and correction
		//speed += 0.1924f * (get_RPM() * 100 - speed) - 0.3f;

		float rotation = ENTITY::GET_ENTITY_SPEED(veh) / 110.0f /*circle max*/ + 0 /*arrow initial rotation*/;
		float screencorrection = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(FALSE);

		drawTexture(idTextureSpeedoBack_airspeed, 0, 9997, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, 0.0f, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //3
		drawTexture(idTextureSpeedoArrow_airspeed, 0, 9999, 100, 0.17f * local_settings.scale, 0.17f * local_settings.scale, 0.5f, 0.5f, local_settings.posx, local_settings.posy, rotation, screencorrection, 1.0f, 1.0f, 1.0f, speedoAlpha);  //4
	}

	void Speedometer::shift_assist_function() const
	{
		if (!local_settings.enable_shift_assist || ENTITY::IS_ENTITY_IN_AIR(veh))
			return;

		create_shift_assist_texutre();
		float screencorrection = GRAPHICS::_GET_SCREEN_ASPECT_RATIO(FALSE);
		int shift = 0;
		float RPM = get_RPM();


		if (mem.GetClutch(veh) >= 0.9f)
		{
			if (RPM > 0.85f && get_gear() < get_topgear() && get_gear() >= 1)
				shift = textureid_assist_shift_up;
			else if (RPM <= 0.5f && get_gear() > 1)
				shift = textureid_assist_shift_down;
		}
	
		if (shift)
		{
			int alpha = 0;
			if (RPM > 0.9f || RPM < 0.4)
				alpha = 200;
			else if (RPM > 0.85f || RPM < 0.5f)
				alpha = 35;

			drawTexture(shift, 0, 9999, 100, 0.04f, 0.04f, 0.5f, 0.5f, 0.5f, 0.1f, 0, screencorrection, 1.0f, 1.0f, 1.0f, static_cast<float>(alpha)/255.0f);
		}
	
	}
	float Speed_Units::get_multiplier() const
	{
		switch (type)
		{
		case 0:
			return 2.23694f;
		case 1:
			return 3.6f;
		case 2:
			return 1.0;
		default:
			return 0;
			break;
		}
	}
	std::string Speed_Units::get_string() const
	{
		switch (type)
		{
		case 0:
			return "mph";
		case 1:
			return "km/h";
		case 2:
			return "m/s";
		default:
			return 0;
			break;
		}
	}
	float Speedometer::get_RPM() const
	{
		float mem_rpm = (mem.GetVehicleRPM(veh));
		if (mem_rpm < 0.2f)
			mem_rpm = 0;
		else if (mem_rpm > 1.0f)
			mem_rpm = 1.0f;

		return mem_rpm;
	}
#pragma endregion



