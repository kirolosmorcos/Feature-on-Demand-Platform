#include "fuelc.c"
static lv_obj_t *fuelbar;
void fuel_bar(int w, int h)
{
    static lv_style_t style_bg;
    static lv_style_t style_indic;

    lv_style_init(&style_bg);
    lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&style_bg, 2);
    lv_style_set_pad_all(&style_bg, 6); /*To make the indicator smaller*/
    lv_style_set_radius(&style_bg, 6);
    lv_style_set_anim_duration(&style_bg, 1000);
    //lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_radius(&style_indic, 3);

    lv_obj_t * bar = lv_bar_create(lv_screen_active());
    lv_obj_remove_style_all(bar);  /*To have a clean start*/
    lv_obj_add_style(bar, &style_bg, 0);
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

    lv_obj_set_size(bar, w/20, w/3);
    lv_obj_align(bar, LV_ALIGN_CENTER, w/20, -w/30);
    lv_bar_set_value(bar, 100, LV_ANIM_ON);
	LV_IMG_DECLARE(fuelc);
	lv_obj_t * img = lv_img_create(bar);
	lv_img_set_src(img, &fuelc);
	lv_obj_center(img);
	lv_img_set_zoom(img, 64);
	lv_obj_align(img, LV_ALIGN_CENTER, 0, 20);
	lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
    	fuelbar = bar;
}
