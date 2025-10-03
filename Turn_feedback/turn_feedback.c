#include <nuttx/config.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#include <nuttx/sensors/lsm6dsl.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/input/ff.h>

#define HAPTIC_DEVICE "/dev/input_ff0"
#define THRESHOLD 20000 // prag pentru detectarea virajului

#define LEFT_EFFECT   0 // index efect stanga
#define RIGHT_EFFECT  1 // index efect dreapta

struct haptic_data_s {
    int fd;
    int n_effects;
};

static struct haptic_data_s g_haptic_data = { .fd = -1, .n_effects = 0 };

static int left_effect_id = -1;
static int right_effect_id = -1;

static void upload_rom_effect(int16_t number, uint16_t delay, int *effect_id)
{
    struct ff_effect effect;
    memset(&effect, 0, sizeof(effect));
    effect.type = FF_CONSTANT;      // constant vibration
    effect.id = -1;
    effect.u.constant.level = number;
    effect.replay.length = 1000;    // 1 secunda
    effect.replay.delay = delay;

    if (ioctl(g_haptic_data.fd, EVIOCSFF, &effect) < 0) {
        perror("Upload effect failed");
        *effect_id = -1;
        return;
    }
    *effect_id = effect.id;  // salveaza id-ul real
}

static void play_effect(int8_t number)
{
    struct ff_event_s play = {0};
    play.code = number;
    play.value = 1;
    if (write(g_haptic_data.fd, &play, sizeof(play)) < 0) {
        perror("Play effect failed");
    }
}

static void stop_effect(int8_t number)
{
    struct ff_event_s play = {0};
    play.code = number;
    play.value = 0;
    if (write(g_haptic_data.fd, &play, sizeof(play)) < 0) {
        perror("Stop effect failed");
    }
}

int main(int argc, FAR char *argv[])
{
    FILE *sensor;
    struct lsm6dsl_sensor_data_s sensor_data;
    int ret;
    int current_state = 0; // 0 = neutru, 1 = stanga, 2 = dreapta

    // deschide senzor accelerometru
    sensor = fopen("/dev/lsm6dsl0", "r");
    if (sensor == NULL) {
        printf("Unable to open sensor\n");
        return -ENOENT;
    }
    ret = ioctl(fileno(sensor), SNIOC_START, 0);
    if (ret < 0) {
        printf("SNIOC_START failed %d\n", ret);
    }

    // deschide motor haptic
    g_haptic_data.fd = open(HAPTIC_DEVICE, O_WRONLY);
    if (g_haptic_data.fd < 0) {
        perror("Open haptic device failed");
        return -errno;
    }

    // încarcă efecte
upload_rom_effect(0x7fff, 10, &left_effect_id);
upload_rom_effect(0x3fff, 10, &right_effect_id);

    for (;;) {
        ret = ioctl(fileno(sensor), SNIOC_LSM6DSLSENSORREAD, (unsigned long)&sensor_data);
        if (ret < 0) {
            printf("Read failed %d\n", ret);
        }

        printf("g_y_data = %d\n", sensor_data.g_y_data);

        if (sensor_data.g_y_data > THRESHOLD) {
            if (current_state != 2) {
                stop_effect(LEFT_EFFECT);
                play_effect(RIGHT_EFFECT);
                current_state = 2;
            }
        } else if (sensor_data.g_y_data < -THRESHOLD) {
            if (current_state != 1) {
                stop_effect(RIGHT_EFFECT);
                play_effect(LEFT_EFFECT);
                current_state = 1;
            }
        } else {
            if (current_state != 0) {
                stop_effect(LEFT_EFFECT);
                stop_effect(RIGHT_EFFECT);
                current_state = 0;
            }
        }

        usleep(50000); // 50ms pentru polling mai rapid
    }

    fclose(sensor);
    close(g_haptic_data.fd);
    return EXIT_SUCCESS;
}
