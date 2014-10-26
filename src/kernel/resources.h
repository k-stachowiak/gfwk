/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef RESOURCES_H
#define RESOURCES_H

void res_deinit(void);

void *res_load_bitmap(char *path);
void res_dispose_bitmap(void *bitmap);

void res_cut_frame_sheet(
        void *bitmap, int frame_w,
        void ***frames, int *frames_count);
void res_dispose_frame_sheet(void **frames, int frames_count);

void *res_load_font(char *path, int size);
void res_dispose_font(void *font);

void *res_load_sample(char *path);
void res_dispose_sample(void *sample);

#endif

