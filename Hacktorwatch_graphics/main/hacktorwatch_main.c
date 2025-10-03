#include <nuttx/config.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/boardctl.h>
#include <math.h>
#ifdef CONFIG_GRAPHICS_LVGL
#include <lvgl/lvgl.h>
#endif

/****************************************************************************
 * Private Variables
 ****************************************************************************/
#ifdef CONFIG_GRAPHICS_LVGL
static lv_obj_t *clock_face;
static lv_obj_t *hour_hand;
static lv_obj_t *minute_hand;
static lv_obj_t *second_hand;
static lv_obj_t *digital_time_label;
static lv_obj_t *date_label;
static lv_obj_t *center_dot;
static lv_timer_t *clock_timer;
#endif

int x_offset = 80; // sau altă valoare pozitivă/negativă
int y_offset =-60; // exemplu: mutăm puțin în jos

/****************************************************************************
 * Private Functions
 ****************************************************************************/
#ifdef CONFIG_GRAPHICS_LVGL
static int lvgl_handler(int argc, char *argv[])
{
    while (1) {
        lv_timer_handler();
        usleep(20000);
    }
    return EXIT_FAILURE;
}

/* Creează marcatorii pentru ore */
static void create_hour_markers(lv_obj_t *parent)
{
    int cx = 100 ;
    int cy = 100;
    int radius = (cx < cy ? cx : cy) - 3; /* margine 30 pixeli, ajustează după gust */

    for (int i = 0; i < 12; i++) {
        const int marker_w = 4;
        const int marker_h = 20;
        lv_obj_t *marker = lv_obj_create(parent);
        lv_obj_set_size(marker, marker_w, marker_h);
        lv_obj_set_style_bg_color(marker, lv_color_hex(0x00ccff), 0);
        lv_obj_set_style_border_width(marker, 0, 0);
        lv_obj_set_style_radius(marker, 2, 0);

        /* pivot la centru marker (rotim corect) */
        lv_obj_set_style_transform_pivot_x(marker, marker_w / 2, 0);
        lv_obj_set_style_transform_pivot_y(marker, marker_h / 2, 0);

        /* Calculează poziția marcatorului (folosim round pentru a evita tăierile) */
        int angle = (i * 30) - 90;
        double rad = angle * M_PI / 180.0;
        int x = (int)round(cx + cos(rad) * radius - (marker_w / 2));
        int y = (int)round(cy + sin(rad) * radius - (marker_h / 2));

        lv_obj_set_pos(marker, x, y);
        lv_obj_set_style_transform_angle(marker, (angle + 90) * 10, 0);
    }
}

/* Creează marcatorii pentru minute */
static void create_minute_markers(lv_obj_t *parent)
{
    int cx = 100;
    int cy = 100;
    int radius = (cx < cy ? cx : cy); /* puțin mai spre exterior decât ora */

    for (int i = 0; i < 60; i++) {
        if (i % 5 != 0) { /* skip hour markers */
            const int marker_w = 1;
            const int marker_h = 8;
            lv_obj_t *marker = lv_obj_create(parent);
            lv_obj_set_size(marker, marker_w, marker_h);
            lv_obj_set_style_bg_color(marker, lv_color_hex(0x666666), 0);
            lv_obj_set_style_border_width(marker, 0, 0);
            lv_obj_set_style_radius(marker, 1, 0);

            /* pivot la centru marker */
            lv_obj_set_style_transform_pivot_x(marker, marker_w / 2, 0);
            lv_obj_set_style_transform_pivot_y(marker, marker_h / 2, 0);

            int angle = (i * 6) - 90;
            double rad = angle * M_PI / 180.0;
            int x = (int)round(cx + cos(rad) * radius - (marker_w / 2));
            int y = (int)round(cy + sin(rad) * radius - (marker_h / 2));

            lv_obj_set_pos(marker, x, y);
            lv_obj_set_style_transform_angle(marker, (angle + 90) * 10, 0);
        }
    }
}

/* Creează o acă a ceasului */
static lv_obj_t* create_clock_hand(lv_obj_t *parent, int width, int height, lv_color_t color)
{
    lv_obj_t *hand = lv_obj_create(parent);
    lv_obj_set_size(hand, width, height);
    lv_obj_set_style_bg_color(hand, color, 0);
    lv_obj_set_style_border_width(hand, 0, 0);
    lv_obj_set_style_radius(hand, width/2, 0);
    
    // Setează punctul de pivotare la baza acului
    lv_obj_set_style_transform_pivot_x(hand, width/2, 0);
    lv_obj_set_style_transform_pivot_y(hand, height, 0);
    
    // Poziționează acul în centru
    lv_obj_align(hand, LV_ALIGN_CENTER, 0, -height/2);
    
    return hand;
}

/* Actualizează poziția acelor și afișajul digital */
static void update_clock_display(lv_timer_t *timer)
{
    time_t rawtime;
    struct tm *timeinfo;
    char time_str[20];
    char date_str[50];
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    // Calculează unghiurile pentru ace
    int hour_angle = (timeinfo->tm_hour % 12) * 300 + timeinfo->tm_min * 5 - 1800; // *30*10 - 90*10
    int minute_angle = timeinfo->tm_min * 60 + timeinfo->tm_sec - 1800; // *6*10 - 90*10
    int second_angle = timeinfo->tm_sec * 60 - 1800; // *6*10 - 90*10
    
    // Actualizează acele
    lv_obj_set_style_transform_angle(hour_hand, hour_angle, 0);
    lv_obj_set_style_transform_angle(minute_hand, minute_angle, 0);
    lv_obj_set_style_transform_angle(second_hand, second_angle, 0);
    
    // Actualizează timpul digital
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", 
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    lv_label_set_text(digital_time_label, time_str);
    
    // Actualizează data
    const char* months[] = {"Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
                           "Iul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char* weekdays[] = {"Dum", "Lun", "Mar", "Mie", "Joi", "Vin", "Sam"};
    
    snprintf(date_str, sizeof(date_str), "%s, %d %s %d", 
             weekdays[timeinfo->tm_wday], 
             timeinfo->tm_mday,
             months[timeinfo->tm_mon],
             timeinfo->tm_year + 1900);
    lv_label_set_text(date_label, date_str);
}
#endif

/* Creează UI-ul pentru cadranul de ceas */
static void hacktorwatch_create_clock(void)
{
#ifdef CONFIG_GRAPHICS_LVGL
    // Creează cadranul principal
    clock_face = lv_obj_create(lv_screen_active());
    lv_obj_set_size(clock_face, 230, 230);
    lv_obj_set_style_radius(clock_face, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(clock_face, lv_color_hex(0x2c5f7a), 0);
    lv_obj_set_style_border_color(clock_face, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_border_width(clock_face, 3, 0);
    lv_obj_set_style_shadow_width(clock_face, 20, 0);
    lv_obj_set_style_shadow_color(clock_face, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_shadow_opa(clock_face, LV_OPA_30, 0);
    lv_obj_align(clock_face, LV_ALIGN_CENTER, 0, 0);
    
    // Creează marcatorii
    create_hour_markers(clock_face); 
    create_minute_markers(clock_face);
    
    // Creează acele ceasului
    hour_hand = create_clock_hand(clock_face, 4, 60, lv_color_hex(0xffffff));
    minute_hand = create_clock_hand(clock_face, 3, 80, lv_color_hex(0x00ccff));
    second_hand = create_clock_hand(clock_face, 2, 95, lv_color_hex(0xff6b6b));
    
    // Creează punctul central
    center_dot = lv_obj_create(clock_face);
    lv_obj_set_size(center_dot, 8, 8);
    lv_obj_set_style_radius(center_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_dot, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_border_width(center_dot, 0, 0);
    lv_obj_align(center_dot, LV_ALIGN_CENTER, 0, 0);
    
    // Creează afișajul digital pentru timp
    digital_time_label = lv_label_create(clock_face);
    lv_obj_set_style_text_color(digital_time_label, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_text_font(digital_time_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(digital_time_label, "00:00:00");
    lv_obj_align(digital_time_label, LV_ALIGN_CENTER, 0, 50);

    lv_obj_set_style_transform_pivot_x(digital_time_label, lv_obj_get_width(digital_time_label)/2, 0);
    lv_obj_set_style_transform_pivot_y(digital_time_label, lv_obj_get_height(digital_time_label)/2, 0);


    lv_obj_set_style_transform_angle(digital_time_label, 1800, 0); // 1800 = 180° * 10

    lv_obj_align(digital_time_label, LV_ALIGN_CENTER, 50, -40);

    
    // Creează o fundal semitransparent pentru timpul digital
    lv_obj_t *time_bg = lv_obj_create(clock_face);
    lv_obj_set_size(time_bg, 80, 25);
    lv_obj_set_style_bg_color(time_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(time_bg, LV_OPA_50, 0);
    lv_obj_set_style_border_width(time_bg, 1, 0);
    lv_obj_set_style_border_color(time_bg, lv_color_hex(0x00ccff), 0);
    lv_obj_set_style_radius(time_bg, 12, 0);
    lv_obj_align(time_bg, LV_ALIGN_CENTER, 0, 50);

    lv_obj_set_style_transform_pivot_x(time_bg, lv_obj_get_width(time_bg)/2, 0);
    lv_obj_set_style_transform_pivot_y(time_bg, lv_obj_get_height(time_bg)/2, 0);

    lv_obj_align(time_bg, LV_ALIGN_CENTER, -3, -55);

    
    // Mută eticheta timpului deasupra fundalului
    lv_obj_move_to_index(digital_time_label, -1);
    
    // Creează afișajul pentru dată
    date_label = lv_label_create(clock_face);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(date_label, "Lun, 1 Ian 2024");
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 75);

    lv_obj_set_style_transform_pivot_x(date_label, lv_obj_get_width(date_label)/2, 0);
    lv_obj_set_style_transform_pivot_y(date_label, lv_obj_get_height(date_label)/2, 0);

    lv_obj_set_style_transform_angle(date_label, 1800, 0); // 1800 = 180° * 10

    lv_obj_align(date_label, LV_ALIGN_CENTER, 90, -10);

    lv_obj_update_layout(clock_face);
    
    // Creează timer-ul care actualizează ceasul la fiecare secundă
    clock_timer = lv_timer_create(update_clock_display, 1000, NULL);
    
    // Actualizează imediat
    update_clock_display(clock_timer);
#endif
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
int hacktorwatch_main(int argc, FAR char *argv[])
{
#ifdef CONFIG_GRAPHICS_LVGL
    lv_nuttx_dsc_t info;
    lv_nuttx_result_t result;
#endif
    int ret;

#ifdef CONFIG_GRAPHICS_LVGL
    lv_init();
    lv_nuttx_dsc_init(&info);
    
#ifdef CONFIG_LV_USE_NUTTX_LCD
    info.fb_path = "/dev/lcd0";
#endif

#ifdef CONFIG_INPUT_TOUCHSCREEN
    info.input_path = "/dev/input0";
#endif

    lv_nuttx_init(&info, &result);
    if (result.disp == NULL) {
        LV_LOG_ERROR("lv_demos initialization failure!");
        return 1;
    }

    /* Creează ecranul principal */
    lv_obj_t *screen = lv_obj_create(NULL);
    lv_scr_load(screen);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57), LV_PART_MAIN);

    /* Creează UI pentru ceas */
    hacktorwatch_create_clock();


    /* Creează task-ul pentru handler-ul LVGL */
    ret = task_create("lvgl_handler", 110, 4096, lvgl_handler, NULL);
    if (ret < 0) {
        printf("Error creating lvgl_handler task\n");
        
    }
#endif

    while (1) {
        usleep(100000);
    }

#ifdef CONFIG_GRAPHICS_LVGL
    lv_disp_remove(result.disp);
    lv_deinit();
#endif

    return 0;
}
