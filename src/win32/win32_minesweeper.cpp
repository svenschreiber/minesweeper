#include <windows.h>

static Platform_State global_platform_state;
static HDC            global_window_dc;
static HGLRC          global_opengl_rc;
static Vec2i          global_mouse_pos;

#include "win32/win32_opengl.cpp"

void
platform_log(char *format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 1024, format, args);
    OutputDebugStringA(buffer);
    va_end(args);
}

b32
platform_read_file(Mem_Arena *arena, char *filename, Platform_File *file_result) {
    HANDLE file_handle = CreateFileA(filename, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file_handle == INVALID_HANDLE_VALUE) return 0;
    LARGE_INTEGER file_size;
    GetFileSizeEx(file_handle, &file_size);
    file_result->size = file_size.QuadPart;
    file_result->data = PushData(arena, u8, file_result->size);
    DWORD bytes_read = 0;
    ReadFile(file_handle, file_result->data, (u32)file_result->size, &bytes_read, 0);
    CloseHandle(file_handle);
    if (file_result->size == bytes_read) {
        return 1;
    } else {
        mem_arena_pop(arena, file_result->size);
        return 0;
    }
}

void *
platform_reserve_memory(u64 size) {
    void *mem = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    return mem;
}

void
platform_commit_memory(void *mem, u64 size) {
    VirtualAlloc(mem, size, MEM_COMMIT, PAGE_READWRITE);
}

void
platform_release_memory(void *mem) {
    VirtualFree(mem, 0, MEM_RELEASE);
}

void
platform_decommit_memory(void *mem, u64 size) {
    VirtualFree(mem, size, MEM_DECOMMIT);
}

static void
win32_process_pending_messages() {
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

static Vec2i
win32_get_mouse_pos(HWND window)
{
    Vec2i result = {};
    POINT mouse_pos;
    GetCursorPos(&mouse_pos);
    ScreenToClient(window, &mouse_pos);
    result.x = mouse_pos.x;
    result.y = mouse_pos.y;
    return result;
}

LRESULT CALLBACK
win32_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;

    s32 key_modifiers = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        key_modifiers |= KEY_MODIFIER_CTRL;
    else if (GetKeyState(VK_SHIFT) & 0x8000)
        key_modifiers |= KEY_MODIFIER_SHIFT;
    else if (GetKeyState(VK_MENU) & 0x8000)
        key_modifiers |= KEY_MODIFIER_ALT;

    switch (message) {
        case WM_SIZE: {
            platform_state->window_width  = LOWORD(lparam);
            platform_state->window_height = HIWORD(lparam);
            glViewport(0, 0, platform_state->window_width, platform_state->window_height);
            PostMessage(window, WM_PAINT, 0, 0);
        } break;

        case WM_QUIT:
        case WM_DESTROY:
        case WM_CLOSE: {
            platform_state->running = false;
        } break;

        case WM_LBUTTONDOWN: {
            Platform_Event event = {};
            {
                event.type          = PLATFORM_EVENT_TYPE_MOUSE_PRESS;
                event.key_index     = KEY_MOUSE_BUTTON_LEFT;
                event.key_modifiers = key_modifiers;
            }
            platform_push_event(event);
        } break;

        case WM_LBUTTONUP: {
            Platform_Event event = {};
            {
                event.type          = PLATFORM_EVENT_TYPE_MOUSE_RELEASE;
                event.key_index     = KEY_MOUSE_BUTTON_LEFT;
                event.key_modifiers = key_modifiers;
            }
            platform_push_event(event);
        } break;

        case WM_RBUTTONDOWN: {
            Platform_Event event = {};
            {
                event.type          = PLATFORM_EVENT_TYPE_MOUSE_PRESS;
                event.key_index     = KEY_MOUSE_BUTTON_RIGHT;
                event.key_modifiers = key_modifiers;
            }
            platform_push_event(event);
        } break;

        case WM_RBUTTONUP: {
            Platform_Event event = {};
            {
                event.type          = PLATFORM_EVENT_TYPE_MOUSE_RELEASE;
                event.key_index     = KEY_MOUSE_BUTTON_RIGHT;
                event.key_modifiers = key_modifiers;
            }
            platform_push_event(event);
        } break;

        case WM_MOUSEMOVE: {
            platform_state->mouse_pos = win32_get_mouse_pos(window);

            Platform_Event event = {};
            {
                event.type          = PLATFORM_EVENT_TYPE_MOUSE_MOVE;
            }
            platform_push_event(event);
            
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            s32 vk_code = (s32)wparam;
            b32 was_down = (lparam & (1 << 30)) == 1;
            b32 is_down = (lparam & (1 << 31)) == 0;

            s32 key_input = 0;
            if (vk_code >= 'A' && vk_code <= 'Z') {
                key_input = KEY_A + (vk_code - 'A');
            } else if (vk_code >= '0' && vk_code <= '9') {
                key_input = KEY_0 + (vk_code - '0');
            } else {
                if (vk_code == VK_BACK) {
                    key_input = KEY_BACKSPACE;
                } else if (vk_code == VK_TAB) {
                    key_input = KEY_TAB;
                } else if (vk_code == VK_RETURN) {
                    key_input = KEY_ENTER;
                } else if (vk_code == VK_SHIFT) {
                    key_input = KEY_SHIFT;
                } else if (vk_code == VK_CONTROL) {
                    key_input = KEY_CTRL;
                    key_modifiers &= ~KEY_MODIFIER_CTRL;
                } else if (vk_code == VK_MENU) {
                    key_input = KEY_ALT;
                    key_modifiers &= ~KEY_MODIFIER_ALT;
                } else if (vk_code == VK_PAUSE) {
                    key_input = KEY_PAUSE;
                } else if (vk_code == VK_CAPITAL) {
                    key_input = KEY_CAPS_LOCK;
                } else if (vk_code == VK_ESCAPE) {
                    key_input = KEY_ESCAPE;
                } else if (vk_code == VK_SPACE) {
                    key_input = KEY_SPACE;
                } else if (vk_code == VK_PRIOR) {
                    key_input = KEY_PAGE_UP;
                } else if (vk_code == VK_NEXT) {
                    key_input = KEY_PAGE_DOWN;
                } else if (vk_code == VK_END) {
                    key_input = KEY_END;
                } else if (vk_code == VK_HOME) {
                    key_input = KEY_HOME;
                } else if (vk_code == VK_LEFT) {
                    key_input = KEY_LEFT;
                } else if (vk_code == VK_RIGHT) {
                    key_input = KEY_RIGHT;
                } else if (vk_code == VK_UP) {
                    key_input = KEY_UP;
                } else if (vk_code == VK_DOWN) {
                    key_input = KEY_DOWN;
                } else if (vk_code == VK_SNAPSHOT) {
                    key_input = KEY_PRINT_SCREEN;
                } else if (vk_code == VK_INSERT) {
                    key_input = KEY_INSERT;
                } else if (vk_code == VK_DELETE) {
                    key_input = KEY_DELETE;
                }  else if (vk_code >= VK_F1 && vk_code <= VK_F12) {
                    key_input = KEY_F1 + (vk_code - VK_F1);
                } else if (vk_code == VK_SCROLL) {
                    key_input = KEY_SCROLL_LOCK;
                } else if (vk_code == VK_OEM_1) {
                    key_input = KEY_SEMICOLON;
                } else if (vk_code == VK_OEM_PLUS) {
                    key_input = KEY_PLUS;
                } else if (vk_code == VK_OEM_MINUS) {
                    key_input = KEY_MINUS;
                } else if (vk_code == VK_OEM_PERIOD) {
                    key_input = KEY_PERIOD;
                } else if (vk_code == VK_OEM_2) {
                    key_input = KEY_SLASH;
                } else if (vk_code == VK_OEM_3) {
                    key_input = KEY_GRAVE_ACCENT;
                } else if (vk_code == VK_OEM_4) {
                    key_input = KEY_LEFT_BRACKET;
                } else if (vk_code == VK_OEM_5) {
                    key_input = KEY_BACKSLASH;
                } else if (vk_code == VK_OEM_6) {
                    key_input = KEY_RIGHT_BRACKET;
                } else if (vk_code == VK_OEM_7) {
                    key_input = KEY_QUOTE;
                }
            }

            if (is_down) {
                Platform_Event event = {};
                {
                    event.type          = PLATFORM_EVENT_TYPE_KEY_PRESS;
                    event.key_index     = key_input;
                    event.key_modifiers = key_modifiers;
                }
                platform_push_event(event);
            } else {
                Platform_Event event = {};
                {
                    event.type          = PLATFORM_EVENT_TYPE_KEY_RELEASE;
                    event.key_index     = key_input;
                    event.key_modifiers = key_modifiers;
                }
                platform_push_event(event);
            }
            
            result = DefWindowProc(window, message, wparam, lparam);
            
        } break;

        case WM_CHAR: {
            s32 character = (s32)wparam; // wParam is u64, maybe not safe?
            if (character >= 32) {
                Platform_Event event = {};
                {
                    event.type          = PLATFORM_EVENT_TYPE_CHARACTER_INPUT;
                    event.character     = character;
                    event.key_modifiers = key_modifiers;
                }
                platform_push_event(event);
            }
        } break;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(window, &ps);
            EndPaint(window, &ps);
        } break;

        default: {
            result = DefWindowProc(window, message, wparam, lparam);
        } break;
    }

    return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
    
    global_mouse_pos = {};
    
    platform_state = &global_platform_state;
    {
        platform_state->window_width  = PLATFORM_DEFAULT_WINDOW_WIDTH;
        platform_state->window_height = PLATFORM_DEFAULT_WINDOW_HEIGHT;
        platform_state->running       = true;
        platform_state->event_count   = 0;
    }

    WNDCLASSA window_class = {};
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        window_class.lpfnWndProc = win32_window_proc;
        window_class.hInstance = instance;
        window_class.hbrBackground = CreateSolidBrush(RGB(51, 51, 64));
        window_class.hCursor = LoadCursor(0, IDC_ARROW);
        window_class.lpszClassName = "MinesweeperWindowClass";
    }

    if (RegisterClass(&window_class) == 0)
        return -1;

    HWND window = CreateWindowA(window_class.lpszClassName,
                                "Minesweeper",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                platform_state->window_width, platform_state->window_height,
                                0, 0, instance, 0);

    if (window == 0)
        return -1;
    
    global_window_dc = GetDC(window);
			
    win32_create_opengl_context(&global_opengl_rc, global_window_dc);

    game_init();
    ShowWindow(window, show_code);
    UpdateWindow(window);

    while (platform_state->running) {
        
        win32_process_pending_messages();

        game_update();

        wglSwapLayerBuffers(global_window_dc, WGL_SWAP_MAIN_PLANE);
    }

    wglMakeCurrent(global_window_dc, 0);
    wglDeleteContext(global_opengl_rc);

    return 0;
}
