#ifndef _SCRIPT_
#define _SCRIPT_

#include "..\..\inc\natives.h"
#include "..\..\inc\types.h"
#include "..\..\inc\enums.h"
#include "..\..\inc\main.h"

#include "Nativememory.h"
#include "menu.h"
#include <string>

#define MOD_VERSION "v2.63.2"
#define MAX_ACCURACY 28
#define MAX_SCALE 2.0f
#define MAX_POSX 1.5f
#define MAX_POSY 1.5f
#define MAX_START_TIME 300.0f
#define MAX_STOP_TIME 300.0f

//using namespace std;

enum speedo_type { NFSHP = 1, NFS2015, modern, custom, air };
struct Speed_Units
{
	int type = 0;
	float get_multiplier() const;
	std::string get_string() const;
};
struct Settings
{
	float speedoAlphaMax;
	int open_menu_key = 0;
	speedo_type skin_id = speedo_type::NFSHP;
	int enable_night_light, wait_for_car_name, enable_first_view, enable_max_red_speed, font = 0;
	float scale = 1.0f;
	float posx = 0.85f;
	float posy = 0.85f;
	Speed_Units speed_units;
	int update_hertz = 0;
	int open_mileage_counter = 0;
	int hide_speedo = 0;
	int enable_bicycle;
	int enable_bike;
	int enable_boat;
	int enable_car;
	int enable_heli;
	int enable_plane;
	int enable_quadbike;
	int enable_train;
	int enable_submersible;
	int enable_on_foot;
	int enable_blinkers;
	int enable_shift_assist;
	int enable_shift_indicators;
	int health = 4;
	int blinker_right_key = 0x27;
	int blinker_left_key = 0x25;
	bool extract_imgs = true;
	int enable_custom_bg = false;
};

#pragma region SCRIPT

extern Settings local_settings;

void draw_text(const std::string& msg, float x, float y, float scalex, float scaley, int red = 255, int green = 255, int blue = 255);
bool get_key_pressed(int nVirtKey);
bool mod_switch_pressed(int nVirtKey);
void reset_mod_switch();
bool exists(const std::string& name);
HMODULE GetCurrentModuleHandle();
void Extract(const WORD& wResId,const LPSTR& lpszOutputPath);
int create_texture(const std::string& name);
std::string get_gear_string();
void handle_Speedo_skin();
void load_settings();
void save_settings();
void update_engine_health();
bool display_check();
void update();
void main();
void ScriptMain();
int get_gear();
void menu_options_execute(menu_options_list option, char control, char selected);

class Speedometer
{
protected:

	int idTextureSpeedoBack, idTextureSpeedoBack_night, idTextureSpeedoBackSkeleton, idTextureSpeedoBackSkeleton_kmph, idTextureSpeedoArrow, id_back_textures[5];
	int idTextureSpeedoBack_airspeed = 0;
	int  idTextureSpeedoArrow_airspeed = 0;
	int idTextureShiftindicator = 0;
	int idTextureBlinkerindicator1 = 0;
	int idTextureBlinkerindicator2 = 0;
	//const static MemoryAccess mem;
public:
	virtual speedo_type get_active_id() const = 0;
	virtual void draw_speedo() const = 0;
	void shift_assist_function() const;
	float get_RPM() const;

};

class NFSHP_speedometer : public Speedometer
{
private:
	void create_textures();

public:
	speedo_type get_active_id() const { return speedo_type::NFSHP; }
	NFSHP_speedometer()
	{
		create_textures();
	}

	void draw_speedo() const;
};

class NFS2015_speedometer : public Speedometer
{
private:
	speedo_type get_active_id() const  { return speedo_type::NFS2015; }
	void create_textures();
public:

	NFS2015_speedometer()
	{
		create_textures();
	}

	void draw_speedo() const;
};

class modern_speedometer : public Speedometer
{
private:
	void create_textures();
	speedo_type get_active_id() const  { return speedo_type::modern; }

public:

	modern_speedometer()
	{
		this->create_textures();
	}


	void draw_speedo() const;
};

class custom_speedometer : public Speedometer
{
private:

	void create_textures();
	speedo_type get_active_id() const  { return speedo_type::custom; }
public:

	custom_speedometer()
	{
		create_textures();
	}

	void draw_speedo() const;
};

class air_speedometer : public Speedometer
{
private:
	void create_textures();
	speedo_type get_active_id() const  { return speedo_type::air; }
public:
	air_speedometer() {create_textures();}
	void draw_speedo() const;
};

#pragma endregion
#endif
























