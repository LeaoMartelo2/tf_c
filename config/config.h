#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "../libs/nuklear/nuklear.h"

#include <stdbool.h>

struct config
{
    int bunny_hop;
    struct
    {
        int aimbot_enabled;
        int draw_fov;
        float fov;
        struct nk_colorf fov_color;
        struct
        {
            int use_key;
            int code;
            int is_mouse_btn;
            bool is_pressed;
        } key;
    } aimbot;
    struct
    {
        // Player ESP
        int player_health_bar;
        int player_bounding_box;
        int player_name;
        // Other Entity ESP
        int ammo_hp_ents_name;
        int ammo_hp_ents_bounding_box;
        int sentry_name;
        int sentry_bounding_box;
        int teleporter_name;
        int teleporter_bounding_box;
        int dispenser_name;
        int dispenser_bounding_box;
        int friendly_dispenser_name;
        int friendly_dispenser_bounding_box;
    } esp;
};

extern struct config config;

bool init_config();
bool save_config();