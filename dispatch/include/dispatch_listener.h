#ifndef DISPATCH_LISTENER_H
#define DISPATCH_LISTENER_H

struct sensor_sample;

typedef void (*sensor_on_sample_fn)(void *context, const struct sensor_sample *sample);

typedef struct {
    void *context;
    sensor_on_sample_fn on_sample;
} sensor_event_listener_t;

#endif
