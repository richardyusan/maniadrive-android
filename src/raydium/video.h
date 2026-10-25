#ifndef RAY_VIDEO_H
#define RAY_VIDEO_H


typedef struct raydium_video_Video
    {
    signed char state;
    char name[RAYDIUM_MAX_NAME_LEN];
    FILE *fp;
    int sizex;
    int sizey;
    float fps;
    int frames_total;
    int live_id;
    float elapsed;
    unsigned char *data;
    long start;
    long *offsets;
    int last_decoded;
    signed char loop;
    signed char playing;
    } raydium_video_Video;

__global raydium_video_Video raydium_video_video[RAYDIUM_MAX_VIDEOS];

#endif
