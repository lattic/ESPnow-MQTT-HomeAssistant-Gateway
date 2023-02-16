The universal code implementing both: environmental device (env) with temperature, humidity and light sensors as well as motion only device (motion), without env sensors AND mix of both (env+mot).

Function timing:
19:08:23.431 > [read_saved_sleeping_time]: took 483us
19:08:23.442 > [readFile]: took 3485us
19:08:23.448 > [writeFile]: took 6000us
19:08:23.448 > [update_bootCount]: took 6190us
19:08:23.448 > [readFile]: took 3619us
19:08:23.453 > [load_ontime]: took 3951us
19:08:23.453 > [charging_state]: took 55us
19:08:23.544 > [gather_data]: took 76078us
19:08:23.585 > [writeFile]: took 6560us
19:08:23.585 > [save_ontime]: Program finished after 218ms.
19:08:23.591 > [save_ontime]: took 6808us

// littlefs:
2x write = 2x 6ms   = 12ms
2x read  = 2x 3.5ms =  7ms
total:              = 19ms 


// enc littlefs:
load: 19ms
save: 14ms
total: 33ms
