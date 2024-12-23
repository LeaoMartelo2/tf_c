#include "../../config/config.h"
#include "../../source_sdk/surface/surface.h"
#include "../../utils/utils.h"
#include "../hooks.h"
#include "menu.h"

#include <stdbool.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

// These defines need to be included everywhere nuklear's headers are included
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
// This should only be included in this file
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "../../libs/nuklear/nuklear.h"
#include "../../libs/nuklear/nuklear_sdl_gl3.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

static bool menu_focused = false;

void get_input(struct nk_context *ctx)
{
    if (nk_input_is_key_released(&ctx->input, NK_KEY_DEL))
    {
        menu_focused = !menu_focused;
        set_cursor_visible(menu_focused);
    }
}

void get_thirdperson_key(struct nk_context *ctx)
{
    if (nk_input_is_key_released(&ctx->input, NK_KEY_SHIFT))
    {
	config.misc.do_thirdperson = !config.misc.do_thirdperson;
    }
}

void set_style(struct nk_context *ctx)
{
    struct nk_color table[NK_COLOR_COUNT];
    memcpy(table, nk_default_color_style, sizeof(nk_default_color_style));

    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(207, 115, 54, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(207, 115, 54, 255 / 2);

    nk_style_from_table(ctx, table);
}

void swap_window_hook(void *window)
{
    static struct nk_context *ctx = NULL;
    static SDL_GLContext original_ctx, new_ctx;

    if (!ctx)
    {
        original_ctx = SDL_GL_GetCurrentContext();
        new_ctx = SDL_GL_CreateContext(window);

        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            log_msg("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
            swap_window_original(window);
            return;
        }

        ctx = nk_sdl_init(window);

        set_style(ctx);
        
        struct nk_font_atlas *atlas = NULL;
        nk_sdl_font_stash_begin(&atlas);
        nk_sdl_font_stash_end();
    }

    SDL_GL_MakeCurrent(window, new_ctx);
    
    get_input(ctx);
    get_thirdperson_key(ctx);
    watermark(ctx);

    if (menu_focused)
    {
        draw_menu(ctx);
    }

    nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

    SDL_GL_MakeCurrent(window, original_ctx);

    nk_input_begin(ctx);
    swap_window_original(window);
    nk_input_end(ctx);
}

int poll_event_hook(SDL_Event *event)
{
    int ret = poll_event_original(event);

    if (ret && nk_sdl_handle_event(event) && menu_focused)
    {
        // TBD: Make an array for keybinds
        if (config.aimbot.key.binding.editing)
        {
            const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);

            for (int i = 0; i < SDL_NUM_SCANCODES; i++)
            {
                if (keyboard_state[i])
                {
                    if (i == SDL_SCANCODE_ESCAPE)
                    {
                        config.aimbot.key.binding.code = 0;
                        config.aimbot.key.binding.type = INPUT_NONE;
                    }
                    else
                    {
                        config.aimbot.key.binding.code = i;
                        config.aimbot.key.binding.type = INPUT_KEY;
                    }
                    config.aimbot.key.binding.editing = 0;

                    // TBD: Use SDL_GetScancodeName(i) to pretty print the key in menu
                    break;
                }
            }
        }

        if (config.aimbot.key.binding.editing)
        {
            Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
            
            for (int i = SDL_BUTTON_LEFT; i <= SDL_BUTTON_X2; i++)
            {
                if (mouse_state & SDL_BUTTON(i))
                {
                    config.aimbot.key.binding.code = i;
                    config.aimbot.key.binding.type = INPUT_MOUSE;
                    config.aimbot.key.binding.editing = 0;
                }
            }
        }

        event->type = 0;

        return ret;
    }

    if (config.aimbot.key.binding.type == INPUT_MOUSE)
    {
        Uint32 mouse_state = SDL_GetMouseState(NULL, NULL);
        if (mouse_state & SDL_BUTTON(config.aimbot.key.binding.code))
        {
            config.aimbot.key.is_pressed = true;
        }
        else
        {
            config.aimbot.key.is_pressed = false;
        }
    }
    else if (config.aimbot.key.binding.type == INPUT_KEY)
    {
        const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
        if (keyboard_state && keyboard_state[config.aimbot.key.binding.code])
        {
            config.aimbot.key.is_pressed = true;
        }
        else
        {
            config.aimbot.key.is_pressed = false;
        }
    }
    else
    {
        config.aimbot.key.is_pressed = false;
    }

    return ret;
}
