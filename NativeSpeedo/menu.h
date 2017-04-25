#ifndef _MENU_
#define _MENU_

#ifndef MAX_CHILDREN
#define MAX_CHILDREN 10
#endif
#include <string>
enum menu_item_type { supermenu, passive, button, bar, selective, on_off };
enum menu_options_list { none,skin, units, red, night, first_person, scale, posx, posy, default, accuracy, display_speedo, reset_mileage, _counter, blinkers, shift_assist, shift_indicator, do_not_hide_car_name, start, stop, ready, open_board, reset_board, mileage_counter_option};

class Menu
{
public:
	//int is_active; //0->no 1->yes 2->don't care
	std::string title;
	char selected;
	void(*function)(char control, char selected);
	menu_options_list item_option;
	menu_item_type item_type;
	std::string data;
	std::string manual;
	Menu* parent;
	Menu* child[MAX_CHILDREN];
	float *precentage;
	float max_value;
	int *int_value;
	float *float_value;
	bool *bool_value;
	Menu::Menu(Menu* _parent, std::string _Title, menu_options_list _item_option, menu_item_type _item_type, std::string _data, std::string _manual);

	~Menu();
	void draw_menu() const;
	void register_action();

	bool menu_is_open;
};

void handle_menu();
void create_menu();
void draw_menu_text(float x, float y, float scale, int font,const std::string& text);
bool is_menu_open();
#endif