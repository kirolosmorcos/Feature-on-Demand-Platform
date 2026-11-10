//#include "oilc.c"
//#include "motorc.c"
extern lv_obj_t *oil_icon;
extern lv_obj_t *motor_icon;
extern lv_obj_t *brake_icon;
extern lv_obj_t *belt_icon;
extern lv_obj_t *right_icon;
extern lv_obj_t *left_icon;
extern lv_obj_t *right_bsd, *left_bsd;

extern int left_trigger;
extern int right_trigger;
extern int oil_t;
extern int engine_t;
extern int brake_t;
extern int belt_t;
extern int right_bsd_t, left_bsd_t;


void motor_cb(void *var, int32_t value){
	if(engine_t){
		lv_obj_set_style_img_opa((lv_obj_t*) var, 255,0);
	}
	else{
	
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50,0);
	}
}
void oil_cb(void *var, int32_t value){
	if(value){
	if(oil_t){
		lv_obj_set_style_img_opa((lv_obj_t*) var, 255,0);
	}
	else{
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50,0);
	}
}
}
void brake_cb(void *var, int32_t value){
	if(brake_t){
		lv_obj_set_style_img_opa((lv_obj_t*) var, 255,0);
}
	else{

		lv_obj_set_style_img_opa((lv_obj_t*) var, 50,0);
	}
}
void belt_cb(void *var, int32_t value){
	if(belt_t){
		lv_obj_set_style_img_opa((lv_obj_t*) var, 255, 0);
	}
	else{
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50,0);
	}
}

void oil(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(oilc);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &oilc);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        //lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
/*
	lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, oil_cb);
        lv_anim_set_values(&a, 0, 255); 
        lv_anim_set_time(&a, 1000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);*/
	oil_icon = img;
}void motor(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(motorc);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &motorc);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 100, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);

/*	lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, motor_cb);
        lv_anim_set_values(&a, 0, 255); 
        lv_anim_set_time(&a, 1000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
*/
	motor_icon = img;
}void brake(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(brakec);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &brakec);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 100, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
/*
	lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, brake_cb);
        lv_anim_set_values(&a, 0, 255); 
        lv_anim_set_time(&a, 1000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
*/
	brake_icon = img;
}void belt(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(beltc);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &beltc);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 100, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
/*
	lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, img);
        lv_anim_set_exec_cb(&a, belt_cb);
        lv_anim_set_values(&a, 0, 255); 
        lv_anim_set_time(&a, 1000);
        lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&a);
*/
	belt_icon = img;
}
void right_cb(void *var, int32_t value){
	if(right_trigger){
		if(value <= 128){
			lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
		}
		else{
			lv_obj_set_style_img_opa((lv_obj_t*) var, 255, 0);
		} 
	}else{
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
	}
}
void right(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(rightc);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &rightc);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 100, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
	// anim
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	//lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
	lv_anim_set_exec_cb(&a, right_cb);
	lv_anim_set_values(&a, 0, 255);  // Rotate from 0 to 360 degrees
	lv_anim_set_time(&a, 1000);       // 3 seconds for full rotation
	//lv_anim_set_delay(&a, 1000);       // 3 seconds for full rotation
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);
	//right_anim = a;

	right_icon = img;
}
void left_cb(void *var, int32_t value){
	if(left_trigger){
		if(value <= 128){
			lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
		}
		else{
			lv_obj_set_style_img_opa((lv_obj_t*) var, 255, 0);
		} 
	}else{
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
	}
}
void left(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(leftc);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &leftc);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 100, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
	// anim
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	//lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
	lv_anim_set_exec_cb(&a, left_cb);
	lv_anim_set_values(&a, 0, 255);  // Rotate from 0 to 360 degrees
	lv_anim_set_time(&a, 1000);       // 3 seconds for full rotation
	//lv_anim_set_delay(&a, 1000);       // 3 seconds for full rotation
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);
	left_icon = img;
}

void left_bsd_cb(void *var, int32_t value){
	if(left_bsd_t){
		if(value <= 128){
			lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
		}
		else{
			lv_obj_set_style_img_opa((lv_obj_t*) var, 255, 0);
		} 
	}else{
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
	}
}
void left_bsd_f(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(leftbsd);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &leftbsd);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        //lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 200, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
	// anim
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	//lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
	lv_anim_set_exec_cb(&a, left_bsd_cb);
	lv_anim_set_values(&a, 0, 255);  // Rotate from 0 to 360 degrees
	lv_anim_set_time(&a, 1000);       // 3 seconds for full rotation
	//lv_anim_set_delay(&a, 1000);       // 3 seconds for full rotation
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);
	left_icon = img;
}
void right_bsd_cb(void *var, int32_t value){
	if(right_bsd_t){
		if(value <= 128){
			lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
		}
		else{
			lv_obj_set_style_img_opa((lv_obj_t*) var, 255, 0);
		} 
	}else{
		lv_obj_set_style_img_opa((lv_obj_t*) var, 50, 0);
	}
}
void right_bsd_f(int w, int h, lv_obj_t *parent){
	LV_IMG_DECLARE(rightbsd);
        lv_obj_t * img = lv_img_create(parent);
        lv_img_set_src(img, &rightbsd);
        lv_obj_center(img);
        lv_img_set_zoom(img, 128);
        //lv_obj_align(img, LV_ALIGN_BOTTOM_MID, 100, -w/30);
        lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, 200);
        lv_obj_set_style_img_opa(img, 50, 0);
	// anim
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, img);
	//lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
	lv_anim_set_exec_cb(&a, right_bsd_cb);
	lv_anim_set_values(&a, 0, 255);  // Rotate from 0 to 360 degrees
	lv_anim_set_time(&a, 1000);       // 3 seconds for full rotation
	//lv_anim_set_delay(&a, 1000);       // 3 seconds for full rotation
	lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);
	//right_anim = a;

	right_icon = img;
}
