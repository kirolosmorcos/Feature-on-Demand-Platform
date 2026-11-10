#include <lvgl.h>
#include <math.h>
#include <examples/lv_examples.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "rpm_needle.c"
#include "speed_needle.c"
#include "fuelbar.c"
#include "waterbar.c"
#include "rightbsd.c"
#include "leftbsd.c"
#include "icons.c"

#define LV_USE_QNX 1
#define LV_COLOR_DEPTH 32
#define LV_QNX_BUF_COUNT 2
#define WIDTH 1920
#define HEIGHT 1080
#define BUF 1024
#define PORT 3000
#define PORT2 3001

#define COLOR_PRIMARY lv_color_hex(0x00A8E8)
#define COLOR_DANGER lv_color_hex(0xFF3838)
#define COLOR_WARNING lv_color_hex(0xFFB800)
#define COLOR_SUCCESS lv_color_hex(0x00D26A)
#define COLOR_BG lv_color_hex(0x1A1A1A)
#define COLOR_PANEL lv_color_hex(0x2A2A2A)
#include "oilc.c"
#include "motorc.c"
#include "brakec.c"
#include "beltc.c"
#include "rightc.c"
#include "leftc.c"


static void value_changed_event_cb(lv_event_t * e);

volatile int update_needed = 1;
volatile int update2_needed = 1;
// Dashboard objects
static lv_obj_t *main_screen;
static lv_obj_t *speed_label;
static lv_obj_t *rpm_arc;
static lv_obj_t *speed_arc;
//static lv_obj_t *fuel_bar;
static lv_obj_t *temp_bar;
static lv_obj_t *gear_label;
static lv_obj_t *odo_label;
static lv_obj_t *trip_label;
static lv_obj_t *time_label;
static lv_obj_t *fuelbar;
static lv_obj_t *waterbar;
static lv_obj_t *bottom_bar;
lv_obj_t *oil_icon;
lv_obj_t *motor_icon;
lv_obj_t *brake_icon;
lv_obj_t *belt_icon;
lv_obj_t *right_icon;
lv_obj_t *left_icon;
lv_obj_t *right_bsd, *left_bsd;
extern lv_obj_t* rpm_needle_line;

//triggers
int right_trigger = 0;
int left_trigger = 0;
int oil_t = 1;
int engine_t = 1;
int brake_t = 1;
int belt_t = 1;
int left_bsd_t = 0, right_bsd_t = 0;
int rpm_angle = 0;
int speed_angle = 0;
int fuel_lvl = 50;
int water_lvl = 50;

// Simulation data
static int current_speed = 0;
static int current_rpm = 0;
static int current_gear = 0;
static int fuel_level = 75;
static int temp_level = 45;
static float odometer = 45678.9;
static float trip = 123.4;

void lv_example_arc_1(void)
{
    lv_obj_t * label = lv_label_create(lv_screen_active());

    /*Create an Arc*/
    lv_obj_t * arc = lv_arc_create(lv_screen_active());
    lv_obj_set_size(arc, 150, 150);
    lv_arc_set_rotation(arc, 135);
    lv_arc_set_bg_angles(arc, 0, 270);
    lv_arc_set_value(arc, 10);
    lv_obj_center(arc);
    lv_obj_add_event_cb(arc, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, label);

    /*Manually update the label for the first time*/
    lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, NULL);
}

static void value_changed_event_cb(lv_event_t * e)
{
    lv_obj_t * arc = lv_event_get_target_obj(e);
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);

    lv_label_set_text_fmt(label, "%" LV_PRId32 "%%", lv_arc_get_value(arc));

    /*Rotate the label to the current position of the arc*/
    lv_arc_rotate_obj_to_angle(arc, label, 25);
}


static void create_speedometer(int w, int h){
    // Speed arc background
    lv_obj_t *arc_bg = lv_arc_create(lv_screen_active());
    lv_obj_set_size(arc_bg, WIDTH/3, WIDTH/3);
    lv_obj_set_pos(arc_bg, WIDTH/12, (HEIGHT/2)-(WIDTH/6));
    lv_arc_set_rotation(arc_bg, 135);
    lv_arc_set_bg_angles(arc_bg, 0, 270);
    lv_arc_set_value(arc_bg, 0);
    lv_obj_remove_style(arc_bg, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc_bg, 15, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc_bg, COLOR_PANEL, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc_bg, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc_bg, COLOR_PRIMARY, LV_PART_INDICATOR);
    lv_obj_clear_flag(arc_bg, LV_OBJ_FLAG_CLICKABLE);

    speed_arc = arc_bg;

    }


// Create RPM gauge
static void create_rpm_gauge(int w, int h){
    // RPM arc
    lv_obj_t *rpm_bg = lv_arc_create(lv_screen_active());
    lv_obj_set_size(rpm_bg, w/3, w/3);
    lv_obj_set_pos(rpm_bg, 7*w/12, h/2 - w/6);
    lv_arc_set_rotation(rpm_bg, 135);
    lv_arc_set_bg_angles(rpm_bg, 0, 270);
    lv_arc_set_value(rpm_bg, 0);
    lv_obj_remove_style(rpm_bg, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(rpm_bg, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(rpm_bg, COLOR_PANEL, LV_PART_MAIN);
    lv_obj_set_style_arc_width(rpm_bg, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(rpm_bg, COLOR_WARNING, LV_PART_INDICATOR);
    lv_obj_clear_flag(rpm_bg, LV_OBJ_FLAG_CLICKABLE);

    rpm_arc = rpm_bg; 
}

void rpm_label(lv_obj_t *parent){
	// RPM label
    int w = lv_obj_get_width(parent);
    int h = lv_obj_get_height(parent);
    printf("w = %d, h = %d\n", w, h);
    lv_obj_t *rpm_value = lv_label_create(parent);
    lv_label_set_text(rpm_value, "0 x1000 RPM");
    lv_obj_set_style_text_font(rpm_value, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(rpm_value, lv_color_hex(0x888888), 0);
    lv_obj_set_pos(rpm_value, w/2, 4*h/5);
    lv_obj_align(rpm_value, LV_ALIGN_BOTTOM_MID, 0, -80);
    /*// "RPM x1000" label
    lv_obj_t *rpm_unit = lv_label_create(parent);
    lv_label_set_text(rpm_unit, "x1000 RPM");
    lv_obj_set_style_text_color(rpm_unit, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(rpm_unit, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(rpm_unit, w/2 + 22 , 4*h/5);*/
}
void speedo_label(lv_obj_t *parent){
	// RPM label
    int w = lv_obj_get_width(parent);
    int h = lv_obj_get_height(parent);
    printf("w = %d, h = %d\n", w, h);
    lv_obj_t *speed_value = lv_label_create(parent);
    lv_label_set_text(speed_value, "0 KM/h");
    lv_obj_set_style_text_font(speed_value, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(speed_value, lv_color_hex(0x888888), 0);
    lv_obj_set_pos(speed_value, w/2, 4*h/5);
    lv_obj_align(speed_value, LV_ALIGN_BOTTOM_MID, 0, -80);
    /*// "RPM x1000" label
    lv_obj_t *rpm_unit = lv_label_create(parent);
    lv_label_set_text(rpm_unit, "x1000 RPM");
    lv_obj_set_style_text_color(rpm_unit, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(rpm_unit, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(rpm_unit, w/2 + 22 , 4*h/5);*/
}

static void rpm_ready_cb(lv_event_t *e){
	lv_obj_t * parent = lv_event_get_target(e);
	create_rpm_gauge_needle(parent);
	rpm_label(parent);
	create_scale_marks(parent, 10);
   	create_label(parent);
	update_rpm_needle_angle(rpm_angle);

}
static void speedo_ready_cb(lv_event_t * e)
{
    lv_obj_t * parent = lv_event_get_target(e);
    create_rpm_gauge_needle(parent);
    speedo_label(parent);
    create_speed_scale_marks(parent, 14);
    create_speed_label(parent);
    //update_needle_angle(speed_angle);
}

void create_labels(lv_obj_t *parent){
	int w = lv_obj_get_width(parent);
	int h = lv_obj_get_height(parent);
	printf("RPM w = %d, RPM h = %d\n", w, h);
	for (int i = 0; i <= 240; i += 20) {
		lv_obj_t *mark = lv_label_create(parent);
		lv_label_set_text_fmt(mark, "%d", i);
		lv_obj_set_style_text_font(mark, &lv_font_montserrat_20, 0);
		lv_obj_set_style_text_color(mark, lv_color_hex(0x666666), 0);

		int32_t angle = 135 + (i * 270) / 240;   // degrees
		int32_t radius = 8*w/20;

		int32_t x = w/2 + (radius * lv_trigo_cos(angle)) / LV_TRIGO_SIN_MAX - 10;
		int32_t y = h/2 + (radius * lv_trigo_sin(angle)) / LV_TRIGO_SIN_MAX - 10;
		printf("RPM: angle = %d, radius = %d, x = %d, y = %d\n", angle, radius, x, y);
		lv_obj_set_pos(mark, x, y);
		}

}

// Create gear indicator
static void create_gear_indicator(int w, int h, lv_obj_t *parent){
    lv_obj_t *gear_panel = lv_obj_create(parent);
    lv_obj_set_size(gear_panel, 100, 100);
    //lv_obj_set_pos(gear_panel, 560, 110);
    lv_obj_set_style_bg_color(gear_panel, COLOR_PANEL, 0);
    lv_obj_set_style_border_width(gear_panel, 2, 0);
    lv_obj_set_style_border_color(gear_panel, COLOR_PRIMARY, 0);
    lv_obj_set_style_radius(gear_panel, 10, 0);
    lv_obj_clear_flag(gear_panel, LV_OBJ_FLAG_SCROLLABLE);

    gear_label = lv_label_create(gear_panel);
    lv_label_set_text(gear_label, "N");
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(gear_label, COLOR_PRIMARY, 0);
    lv_obj_center(gear_label);
    }

void *thread(void *i){
	int fd;
	char buf[BUF];
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	inet_pton(AF_INET, "0.0.0.0", (void *)&addr.sin_addr);

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	bind(fd, (struct sockaddr *) &addr, sizeof(addr));

	//recv(fd, buf, BUF, 0);
	//printf("%s", buf);
	while(1){
		//scanf("%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &left_trigger, &right_trigger, &oil_t, &engine_t, &brake_t, &belt_t, &rpm_angle, &speed_angle, &water_lvl, &fuel_lvl);
		recv(fd, buf, BUF, 0);
		printf("%s", buf);
		sscanf(buf, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d", &left_trigger, &right_trigger, &oil_t, &engine_t, &brake_t, &belt_t, &rpm_angle, &speed_angle, &water_lvl, &fuel_lvl);
		printf("%d-%d-%d-%d-%d-%d-%d-%d-%d-%d\n", left_trigger, right_trigger, oil_t, engine_t, brake_t, belt_t, rpm_angle, speed_angle, water_lvl, fuel_lvl);
		rpm_angle = (rpm_angle*270) / 240;
		speed_angle = (speed_angle*270) / 240;
		update_needed = 1;
	}
}
void thread2(void *i){
	int fd;
	char buf[BUF];
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT2);
	inet_pton(AF_INET, "0.0.0.0", (void *)&addr.sin_addr);

	fd = socket(AF_INET, SOCK_STREAM, 0);

	bind(fd, (struct sockaddr *) &addr, sizeof(addr));
	listen(fd, 5);

	while(1){
		int ecu = accept(fd, NULL, NULL);
		
		while(recv(ecu, buf, BUF, 0) != 0){
			printf("%s", buf);
			sscanf(buf, "%c-%c\n", &left_bsd_t, &right_bsd_t);
			printf("%c-%c\n", left_bsd_t, right_bsd_t);
			left_bsd_t = (left_bsd_t == '0') ? 0 : 1;
			right_bsd_t = (right_bsd_t == '0') ? 0 : 1;
		}
		close(ecu);
	}

}
void update_icons_timer(lv_timer_t *timer) {
    if(update_needed) {
        lv_obj_set_style_img_opa(oil_icon, oil_t ? 255 : 50, 0);
        lv_obj_set_style_img_opa(motor_icon, engine_t ? 255 : 50, 0);
        lv_obj_set_style_img_opa(brake_icon, brake_t ? 255 : 50, 0);
        lv_obj_set_style_img_opa(belt_icon, belt_t ? 255 : 50, 0);
	update_rpm_needle_angle(rpm_angle);
	update_needle_angle(speed_angle);
	lv_bar_set_value(fuelbar, fuel_lvl, LV_ANIM_ON);
	lv_bar_set_value(waterbar, water_lvl, LV_ANIM_ON);
        
        update_needed = 0;
    }
}

int main(int argc, char **argv)
{
	/* Initialize the library. */
	lv_init();

	lv_timer_create(update_icons_timer, 500, NULL);
	pthread_t mythread;
	pthread_create(&mythread, NULL, thread, NULL);
	pthread_t mythread2;
	pthread_create(&mythread2, NULL, thread2, NULL);

	lv_display_t *disp = lv_qnx_window_create(WIDTH, HEIGHT);
	lv_qnx_window_set_title(disp, "LVGL Dashboard");

	lv_coord_t w = lv_disp_get_hor_res(NULL);
	lv_coord_t h = lv_disp_get_ver_res(NULL);

	printf("w = %d, h= %d\n", w, h);
	/* Add keyboard and mouse devices. */
	//lv_qnx_add_keyboard_device(disp);
	//lv_qnx_add_pointer_device(disp);

	/* Generate the UI. */
	lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x303030), LV_PART_MAIN);

	create_speedometer(WIDTH, HEIGHT);
	lv_obj_add_event_cb(speed_arc, speedo_ready_cb, LV_EVENT_SIZE_CHANGED, NULL);

	create_rpm_gauge(WIDTH, HEIGHT);
	lv_obj_add_event_cb(rpm_arc, rpm_ready_cb, LV_EVENT_SIZE_CHANGED, NULL);
	//create_labels(rpm_arc);
	//lv_obj_add_event_cb(rpm_arc, arc_ready_cb, LV_EVENT_SIZE_CHANGED, NULL);

	//create_secondary_gauges(WIDTH, HEIGHT);


	/*Create an Arc*//*
	lv_obj_t * label = lv_label_create(lv_screen_active());
	lv_obj_t * arc = lv_arc_create(lv_screen_active());
	lv_obj_set_size(arc, WIDTH/3, WIDTH/3);
	lv_arc_set_rotation(arc, 135);
	lv_arc_set_bg_angles(arc, 0, 270);
	lv_arc_set_value(arc, 40);
	lv_obj_set_x(arc, WIDTH/12);//lv_obj_center(arc);
	lv_obj_set_y(arc, (HEIGHT/2)-(WIDTH/6));//lv_obj_center(arc);
	lv_obj_add_event_cb(arc, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, label);
*/
	/*Manually update the label for the first time*/
	//lv_obj_send_event(arc, LV_EVENT_VALUE_CHANGED, NULL);

	/*lv_obj_t * label2 = lv_label_create(lv_screen_active());
	lv_obj_t * arc2 = lv_arc_create(lv_screen_active());
	lv_obj_set_size(arc2, WIDTH/3, WIDTH/3);
	lv_arc_set_rotation(arc2, 135);
	lv_arc_set_bg_angles(arc2, 0, 270);
	lv_arc_set_value(arc2, 70);
	lv_obj_set_x(arc2, (7*WIDTH/12));//lv_obj_center(arc);
	lv_obj_set_y(arc2, (HEIGHT/2)-(WIDTH/6));//lv_obj_center(arc);
	lv_obj_add_event_cb(arc2, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, label2);

	//Manually update the label for the first time
	lv_obj_send_event(arc2, LV_EVENT_VALUE_CHANGED, NULL);*/

	fuel_bar(WIDTH, HEIGHT);
	water_bar(WIDTH, HEIGHT);
	
	// upper bar
	lv_obj_t * upper_bar = lv_obj_create(lv_scr_act());
        lv_obj_set_size(upper_bar, LV_PCT(100), 120);
        lv_obj_align(upper_bar, LV_ALIGN_TOP_MID, 0, WIDTH/60);
        lv_obj_set_style_bg_opa(upper_bar, LV_OPA_0, 0); 
        lv_obj_set_style_border_width(upper_bar, 0, 0);
        lv_obj_set_layout(upper_bar, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(upper_bar, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(
                upper_bar,
                LV_FLEX_ALIGN_CENTER,  // main axis (horizontal)
                LV_FLEX_ALIGN_CENTER,  // cross axis (vertical)
                LV_FLEX_ALIGN_CENTER   // track alignment
);	
	left_bsd_f(WIDTH, HEIGHT, upper_bar);
	left(WIDTH, HEIGHT, upper_bar);
	//create_gear_indicator(WIDTH, HEIGHT, upper_bar);
	right(WIDTH, HEIGHT, upper_bar);
	right_bsd_f(WIDTH, HEIGHT, upper_bar);

	// bottom bar	
	lv_obj_t * bottom_bar = lv_obj_create(lv_scr_act());
        lv_obj_set_size(bottom_bar, LV_PCT(100), 80);
        lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, -WIDTH/30);
        lv_obj_set_style_bg_opa(bottom_bar, LV_OPA_0, 0); 
        lv_obj_set_style_border_width(bottom_bar, 0, 0);
        lv_obj_set_layout(bottom_bar, LV_LAYOUT_FLEX);
        lv_obj_set_flex_flow(bottom_bar, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(
                bottom_bar,
                LV_FLEX_ALIGN_CENTER,  // main axis (horizontal)
                LV_FLEX_ALIGN_CENTER,  // cross axis (vertical)
                LV_FLEX_ALIGN_CENTER   // track alignment
);
	oil(WIDTH, HEIGHT, bottom_bar);		
	motor(WIDTH, HEIGHT, bottom_bar);		
	brake(WIDTH, HEIGHT, bottom_bar);		
	belt(WIDTH, HEIGHT, bottom_bar);		
	
	////lv_example_arc_1();
	/*lv_obj_t * Hellolabel = lv_label_create(lv_screen_active());
	lv_label_set_text(Hellolabel, "Hello world");
	lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0xffffff), LV_PART_MAIN);
	lv_obj_align(Hellolabel, LV_ALIGN_CENTER, 0, 0);*/
	create_gauge_needle(speed_arc);
	//create_gauge_example();

	/* Run the event loop until it exits. */
	return lv_qnx_event_loop(disp);
}
