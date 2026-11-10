#include <stdlib.h>
#define NEEDLE_LENGTH 300
#define NEEDLE_CENTER_X 150
#define NEEDLE_CENTER_Y 150
#define NEEDLE_WIDTH 3

// Global variables
lv_obj_t *rpm_needle_line;
lv_point_precise_t rpm_needle_points[2];// = malloc(2 * sizeof(lv_point_precise_t));
int rpm_x, rpm_y;
//static lv_point_precise_t points[20][2];

void update_rpm_needle_angle(int16_t angle)
{
    // Convert angle to radians
    float angle_rad = (-angle + 45) * 3.14159f / 180.0f; // -90 to start from right (0°)
    
    // Calculate end point
    rpm_needle_points[0].x = rpm_x;//NEEDLE_CENTER_X;
    rpm_needle_points[0].y = rpm_y;//NEEDLE_CENTER_Y;
    rpm_needle_points[1].x = rpm_x - (int16_t)(NEEDLE_LENGTH * cosf(angle_rad));
    //rpm_needle_points[1].x = NEEDLE_CENTER_X + (int16_t)(NEEDLE_LENGTH * cosf(angle_rad));
    rpm_needle_points[1].y = rpm_y + (int16_t)(NEEDLE_LENGTH * sinf(angle_rad));
    //rpm_needle_points[1].y = NEEDLE_CENTER_Y + (int16_t)(NEEDLE_LENGTH * sinf(angle_rad));
    
    // Update line points
    lv_line_set_points(rpm_needle_line, rpm_needle_points, 2);
}

/**
 * Animation callback to rotate the needle
 */
static void rpm_needle_anim_cb(void *var, int32_t value)
{
    update_rpm_needle_angle(value);
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);

    //printf("LVGL heap: used=%d free=%d frag=%d%%\n",mon.total_size - mon.free_size,mon.free_size,mon.frag_pct);
}

void create_rpm_gauge_needle(lv_obj_t *parent)
{
    // Create the line object for the needle
    rpm_needle_line = lv_line_create(parent);
    lv_coord_t w = lv_obj_get_width(parent);
    lv_coord_t h = lv_obj_get_height(parent);
    lv_coord_t x = lv_obj_get_x(parent);
    lv_coord_t y = lv_obj_get_y(parent);
    printf("Width = %d, Heig = %d, x = %d, y = %d\n", w, h, x, y);
    //int gauge_mid[2] = {(y+w/2), (x+h/2)};
    rpm_x = w/2;
    rpm_y = h/2;
    printf("Cx = %d, Cy = %d\n", rpm_x, rpm_y);

    // Set initial points (pointing to 0 degrees)
    //rpm_needle_points[0].x = rpm_x;//NEEDLE_CENTER_X;
    //rpm_needle_points[0].y = rpm_y;//NEEDLE_CENTER_Y;
    //rpm_needle_points[1].x = rpm_x + NEEDLE_LENGTH;//NEEDLE_CENTER_X + NEEDLE_LENGTH;
    //rpm_needle_points[1].y = rpm_y + NEEDLE_LENGTH;//NEEDLE_CENTER_Y;
    
    //lv_line_set_points(rpm_needle_line, rpm_needle_points, 2);
    
    // Style the needle
    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, NEEDLE_WIDTH);
    lv_style_set_line_color(&style_line, lv_color_hex(0xFF0000)); // Red needle
    lv_style_set_line_rounded(&style_line, true);
    
    lv_obj_add_style(rpm_needle_line, &style_line, 0);
    
    // Optional: Create a center point/circle for the needle pivot
    lv_obj_t *center = lv_obj_create(parent);
    lv_obj_set_size(center, 10, 10);
    lv_obj_set_pos(center, rpm_x - 5, rpm_y - 5);
    lv_obj_set_style_radius(center, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center, lv_color_hex(0x003300), 0);
    lv_obj_clear_flag(center, LV_OBJ_FLAG_SCROLLABLE);
    
    // Create animation for the needle
    /*lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, rpm_needle_line);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, rpm_needle_anim_cb);
    lv_anim_set_values(&a, 0, 360);  // Rotate from 0 to 360 degrees
    lv_anim_set_time(&a, 6000);       // 3 seconds for full rotation
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); // Repeat forever
    lv_anim_start(&a); */
}
void create_scale_marks(lv_obj_t *parent, int num) {
    int w = lv_obj_get_width(parent);
    int h = lv_obj_get_height(parent);
    
    int32_t radius_outer = 10*w/20;
    int32_t radius_inner = 9*w/20;  // Inner end of the tick mark
    
    int idx = 0;
    for (int i = 0; i <= 245; i += 40 ) {
        // Create a line for the tick mark
        lv_obj_t *tick = lv_line_create(parent);
        
        //static lv_point_precise_t points[2];
	lv_point_precise_t *points = lv_malloc(sizeof(lv_point_precise_t) * 2);
        int32_t angle = 135 + (i * 270) / 240;
        
        // Outer point
        points[0].x = w/2 + (radius_outer * lv_trigo_cos(angle)) / LV_TRIGO_SIN_MAX;
        points[0].y = h/2 + (radius_outer * lv_trigo_sin(angle)) / LV_TRIGO_SIN_MAX;
        
        // Inner point
        points[1].x = w/2 + (radius_inner * lv_trigo_cos(angle)) / LV_TRIGO_SIN_MAX;
        points[1].y = h/2 + (radius_inner * lv_trigo_sin(angle)) / LV_TRIGO_SIN_MAX;
        //printf("0 = (%d, %d), 1 = (%d, %d)\n", points[0].x, points[0].y, points[1].x, points[1].y); 
        lv_line_set_points(tick, points, 2);
        
        // Style the tick
        lv_obj_set_style_line_width(tick, 2, 0);
        lv_obj_set_style_line_color(tick, lv_color_hex(0x666666), 0);
	idx++;
    }
}

void create_label(lv_obj_t *parent) {
    int w = lv_obj_get_width(parent);
    int h = lv_obj_get_height(parent);
    
    int32_t radius = 8*w/20;
    
    // Only create labels for major marks (every 40 or 60)
    for (int i = 0; i <= 240; i += 40) {  // Only 5 labels instead of 13
        lv_obj_t *mark = lv_label_create(parent);
        lv_label_set_text_fmt(mark, "%d", i);
        lv_obj_set_style_text_font(mark, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(mark, lv_color_hex(0x666666), 0);
        
        int32_t angle = 135 + (i * 270) / 240;
        int32_t x = w/2 + (radius * lv_trigo_cos(angle)) / LV_TRIGO_SIN_MAX - 10;
        int32_t y = h/2 + (radius * lv_trigo_sin(angle)) / LV_TRIGO_SIN_MAX - 10;
        
        lv_obj_set_pos(mark, x, y);
    }
}
