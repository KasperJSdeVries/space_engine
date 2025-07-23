#ifndef CAMERA_H
#define CAMERA_H

#include "renderer/input.h"
#include "vulkan.h"

typedef struct {
    vec3s position;
    vec3s front;
    vec3s up;
    vec3s right;
    vec3s world_up;
    f32 yaw;
    f32 pitch;
    f32 movement_speed;
    f32 mouse_sensitivity;
    f32 zoom;
    b8 ortho;
} Camera;

Camera camera_new(void);

mat4s camera_view(const Camera *self);
mat4s camera_projection(const Camera *self, VkExtent2D extent);

void camera_process_moving_inputs(Camera *self, u8 moving_flags, f32 delta_time);
void camera_process_looking(Camera *self, f32 xoffset, f32 yoffset);
void camera_process_zoom(Camera *self, f32 yoffset);

#endif // CAMERA_H
