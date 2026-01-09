#include "external/slurp/include/box.h"
#include "external/slurp/include/sticker.h"
#include <MagickCore/magick-type.h>
#include <MagickWand/MagickWand.h>
#include <MagickWand/magick-image.h>
#include <cjson/cJSON.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define SCREENSHOT_PATH "/tmp/mscreenshot_tmp.png"

char *output_path = "/tmp/output.png";

char        *slurp_main(struct sticker *, size_t);
static char *read_config(char *, size_t *);
static void  image_composite(struct sticker *, size_t, char *);
static void  get_box_dimensions(char *, int *, int *);

int main(int argc, char *argv[]) {
    char *template_name = "sparkle";

    int opt;
    while ((opt = getopt(argc, argv, "s:o:")) != -1) {
        switch (opt) {
        case 's':
            template_name = optarg;
            break;
        case 'o':
            output_path = optarg;
            break;
        }
    }

    char *config_file_path = getenv("MEME_SCREENSHOT_CONFIG");

    size_t config_size;
    char  *config = read_config(config_file_path, &config_size);

    cJSON *config_json = cJSON_ParseWithLength(config, config_size);

    if (config_json != NULL) {
        if (munmap(config, config_size) == -1) {
            perror("munmap");
        }
    } else {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
            return 1;
        }
    }

    const cJSON *template_json = cJSON_GetObjectItemCaseSensitive(config_json, template_name);
    if (!cJSON_IsObject(template_json)) {
        fprintf(stderr, "Key \"%s\" not found in %s\n", template_name, config_file_path);
        return 1;
    }

    const cJSON *stickers_json = cJSON_GetObjectItemCaseSensitive(template_json, "stickers");
    if (!cJSON_IsArray(stickers_json)) {
        fprintf(stderr, "Key \"stickers\" not found in %s\n", config_file_path);
        return 1;
    }

    size_t         stickers_size = cJSON_GetArraySize(stickers_json);
    struct sticker stickers[stickers_size];

    const cJSON *sticker_json = NULL;
    int          i            = 0;
    cJSON_ArrayForEach(sticker_json, stickers_json) {
        const cJSON *path   = cJSON_GetObjectItemCaseSensitive(sticker_json, "path");
        const cJSON *scale  = cJSON_GetObjectItemCaseSensitive(sticker_json, "scale");
        const cJSON *pivot  = cJSON_GetObjectItemCaseSensitive(sticker_json, "pivot");
        const cJSON *anchor = cJSON_GetObjectItemCaseSensitive(sticker_json, "anchor");

        stickers[i].path   = path ? path->valuestring : NULL;
        stickers[i].scale  = scale ? scale->valuedouble : 1;
        stickers[i].pivot  = pivot ? pivot->valueint : 0;
        stickers[i].anchor = anchor ? anchor->valueint : 0;

        i += 1;
    }

    char *region = slurp_main(stickers, stickers_size);
    if (region == 0) {
        fprintf(stderr, "Slurp: unable to capture region");
    }
    region[strlen(region) - 1] = 0;

    pid_t pid = fork();
    if (pid == 0) {
        sleep(1);
        int ret = execlp("grim", "grim", "-g", region, "-t", "png", SCREENSHOT_PATH, NULL);
        if (ret == -1) {
            perror("execlp");
        }
    } else if (pid == -1) {
        perror("fork");
    }

    int status;
    waitpid(pid, &status, 0);
    if (status == 0) {
        image_composite(stickers, stickers_size, region);
    }

    // clean up
    cJSON_Delete(config_json);
    free(region);

    return 0;
}

static void image_composite(struct sticker *stickers, size_t stickers_size, char *region) {
    MagickWandGenesis();

    MagickWand *screenshot = NewMagickWand();

    if (MagickReadImage(screenshot, SCREENSHOT_PATH) == MagickFalse) {
        fprintf(stderr, "MagickReadImage failed to load image");
        return;
    }

    for (size_t i = 0; i < stickers_size; i += 1) {
        MagickWand *overlay = NewMagickWand();
        if (MagickReadImage(overlay, stickers[i].path) == MagickFalse) {
            fprintf(stderr, "MagickReadImage failed to load image");
            DestroyMagickWand(screenshot);
            return;
        }

        size_t w_overlay = MagickGetImageWidth(overlay);
        size_t h_overlay = MagickGetImageHeight(overlay);

        struct slurp_box box;
        get_box_dimensions(region, &box.width, &box.height);

        double x        = 0;
        double y        = 0;
        double scaled_w = 0;
        double scaled_h = 0;
        get_transformation(stickers + i, w_overlay, h_overlay, &box, &x, &y, &scaled_w, &scaled_h);

        MagickResizeImage(overlay, scaled_w * w_overlay, scaled_h * h_overlay, LanczosFilter);

        if (MagickCompositeImage(screenshot, overlay, OverCompositeOp, MagickTrue, x, y) == MagickFalse) {
            fprintf(stderr, "MagickReadImage failed to compose image");
            DestroyMagickWand(overlay);
            goto magick_cleanup;
        }

        DestroyMagickWand(overlay);
    }

    if (MagickWriteImage(screenshot, output_path) == MagickFalse) {
        fprintf(stderr, "MagickReadImage failed to write image");
        goto magick_cleanup;
    }

magick_cleanup:
    DestroyMagickWand(screenshot);
    MagickWandTerminus();
    return;
}

static char *read_config(char *path, size_t *file_size) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("stat");
        close(fd);
        exit(1);
    }
    *file_size = st.st_size;

    char *buf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buf == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    close(fd);

    return buf;
}

static void get_box_dimensions(char *region, int *w, int *h) {
    char *c = region;
    while (*c++ != 0x20)
        ;

    char *dimension = c;

    int digit_count = 1;
    while (*c++ != 'x')
        digit_count++;

    char w_str[digit_count];
    snprintf(w_str, digit_count, "%s", dimension);

    char *h_str = c;

    *w = atoi(w_str);
    *h = atoi(h_str);
}
