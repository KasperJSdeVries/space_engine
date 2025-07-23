#include "camera.h"

#define YAW -90.0f
#define PITCH 0.0f
#define SPEED 2.5f
#define SENSITIVITY 0.1f
#define ZOOM 45.0f

static void update_vectors(Camera *self);

Camera camera_new(void) {
    Camera self = {
        .position = {{0.0f, 0.0f, 0.0f}},
        .up = {{0.0f, 1.0f, 0.0f}},
        .world_up = {{0.0f, 1.0f, 0.0f}},
        .yaw = YAW,
        .pitch = PITCH,
        .front = {{0.0f, 0.0f, -1.0f}},
        .movement_speed = SPEED,
        .mouse_sensitivity = SENSITIVITY,
        .zoom = ZOOM,
    };

    update_vectors(&self);

    return self;
}

mat4s camera_view(const Camera *self) {
    return glms_lookat(self->position, vec3_add(self->position, self->front), self->up);
}

mat4s camera_projection(const Camera *self, VkExtent2D extent) {
    mat4s projection;
    if (self->ortho) {
        projection = glms_ortho(0.0, extent.width, 0.0, extent.height, 0.1f, 10000.0f);
    } else {
        projection = glms_perspective(glm_rad(self->zoom), (f32)extent.width / (f32)extent.height, 0.1f, 10000.0f);
    }

    projection.m11 *= -1;

    return projection;
}

void camera_process_moving_inputs(Camera *self, u8 moving_flags, f32 delta_time) {
    f32 velocity = self->movement_speed * delta_time;
    if (moving_flags & MOVING_FORWARDS) {
        self->position = vec3_add(self->position, vec3_scale(self->front, velocity));
    }
    if (moving_flags & MOVING_BACKWARDS) {
        self->position = vec3_sub(self->position, vec3_scale(self->front, velocity));
    }
    if (moving_flags & MOVING_RIGHT) {
        self->position = vec3_add(self->position, vec3_scale(self->right, velocity));
    }
    if (moving_flags & MOVING_LEFT) {
        self->position = vec3_sub(self->position, vec3_scale(self->right, velocity));
    }
    if (moving_flags & MOVING_UP) {
        self->position = vec3_add(self->position, vec3_scale(self->up, velocity));
    }
    if (moving_flags & MOVING_DOWN) {
        self->position = vec3_sub(self->position, vec3_scale(self->up, velocity));
    }
}

void camera_process_looking(Camera *self, f32 xoffset, f32 yoffset) {
    xoffset *= self->mouse_sensitivity;
    yoffset *= self->mouse_sensitivity;

    self->yaw += xoffset;
    self->pitch += yoffset;

    self->pitch = CLAMP(self->pitch, -89.0f, 89.0f);

    update_vectors(self);
}

void camera_process_zoom(Camera *self, f32 yoffset) {
    self->zoom -= yoffset;
    self->zoom = CLAMP(self->zoom, 1.0f, 45.0f);
}

static void update_vectors(Camera *self) {
    vec3s front = {0};
    front.x = cosf(glm_rad(self->yaw)) * cosf(glm_rad(self->pitch));
    front.y = sinf(glm_rad(self->pitch));
    front.z = sinf(glm_rad(self->yaw)) * cosf(glm_rad(self->pitch));
    self->front = vec3_normalize(front);
    self->right = vec3_normalize(vec3_cross(self->front, self->world_up));
    self->up = vec3_normalize(vec3_cross(self->right, self->front));
}
