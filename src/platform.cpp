static void
platform_push_event(Platform_Event event) {
    // platform_state must be initalized before this!
    if (platform_state->event_count < ArrayCount(platform_state->events)) {
        platform_state->events[platform_state->event_count] = event;
        platform_state->event_count += 1;
    }
}
