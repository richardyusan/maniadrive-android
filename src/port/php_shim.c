/*
 * ManiaDrive Android port - replacement for the embedded PHP interpreter.
 *
 * The PC version runs a few small PHP scripts through raydium_php_exec()
 * (story progress, track lists, music playlist, online scores). PHP is not
 * available on Android, so this file implements the same scripts in C.
 * Variables are exchanged exactly like with PHP: through the Raydium
 * "register" API (raydium_register_variable()).
 *
 * This file is #included by raydium/index.c when MANIA_PORT is defined.
 * GPL v2 or later, like ManiaDrive.
 */
#include <dirent.h>

/* ---------------------------------------------------------------- helpers */

static void *php_var(const char *name, int type)
{
    int i = raydium_register_find_name((char *)name);
    if (i < 0) return NULL;
    if (type && raydium_register_variable_type[i] != type) return NULL;
    return raydium_register_variable_addr[i];
}

static void php_set_str(const char *name, const char *value, size_t max)
{
    char *s = php_var(name, RAYDIUM_REGISTER_STR);
    if (!s) return;
    strncpy(s, value, max - 1);
    s[max - 1] = 0;
}

static void php_set_int(const char *name, int v)
{
    int i = raydium_register_find_name((char *)name);
    if (i < 0) return;
    switch (raydium_register_variable_type[i]) {
    case RAYDIUM_REGISTER_INT:   *(int *)raydium_register_variable_addr[i] = v; break;
    case RAYDIUM_REGISTER_SCHAR: *(signed char *)raydium_register_variable_addr[i] = (signed char)v; break;
    case RAYDIUM_REGISTER_FLOAT: *(float *)raydium_register_variable_addr[i] = (float)v; break;
    }
}

static void php_trim(char *s)
{
    size_t n;
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
        memmove(s, s + 1, strlen(s));
    n = strlen(s);
    while (n && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n'))
        s[--n] = 0;
}

static int php_strcmp(const void *a, const void *b)
{
    return strcmp(*(char *const *)a, *(char *const *)b);
}

/* ------------------------------------------------- mania_story.php */
/* "d name;author;gold_time;author_time;message" line of a .mni track */
static void story_track_data(const char *mni, char *name, float *gold)
{
    char line[1024];
    FILE *fp = fopen(mni, "rt");
    strcpy(name, mni);
    *gold = 0;
    if (!fp) return;
    while (fgets(line, sizeof line, fp)) {
        if (line[0] == 'd') {
            char *p = line + 2, *sc;
            sc = strchr(p, ';');
            if (sc) *sc = 0;
            strcpy(name, p);
            php_trim(name);
            if (sc) {
                p = sc + 1;               /* author */
                sc = strchr(p, ';');
                if (sc) *gold = (float)atof(sc + 1);
            }
            break;
        }
    }
    fclose(fp);
}

static float story_best(const char *mni)
{
    char line[1024];
    float best = 999999;
    FILE *fp = fopen(raydium_file_home_path("mania_drive.state"), "rt");
    if (!fp) return best;
    while (fgets(line, sizeof line, fp)) {
        char *sc = strchr(line, ';');
        if (!sc) continue;
        *sc = 0;
        if (!strcmp(line, mni)) {
            float t = (float)atof(sc + 1);
            if (t < best) best = t;
        }
    }
    fclose(fp);
    return best;
}

static void php_mania_story(void)
{
    char *story_file = php_var("story_file", RAYDIUM_REGISTER_STR);
    char ret[RAYDIUM_GUI_DATASIZE];
    char line[RAYDIUM_MAX_NAME_LEN];
    char col = '2';
    FILE *fp;

    ret[0] = 0;
    if (story_file && (fp = fopen(story_file, "rt"))) {
        while (fgets(line, sizeof line, fp)) {
            char name[RAYDIUM_MAX_NAME_LEN], entry[RAYDIUM_MAX_NAME_LEN * 2 + 16];
            float gold, best;
            php_trim(line);
            if (!line[0]) continue;
            story_track_data(line, name, &gold);
            best = story_best(line);
            snprintf(entry, sizeof entry, "%s;^%c%s  \n", line, col, name);
            if (strlen(ret) + strlen(entry) < sizeof ret)
                strcat(ret, entry);
            if (best > gold)
                col = '4';
        }
        fclose(fp);
    }
    php_set_str("ret", ret, RAYDIUM_GUI_DATASIZE);
    php_set_int("completed", col == '2');
}

/* ------------------------------------------- mania_localtracks.php */
static int list_has(char **list, int n, const char *s)
{
    int i;
    for (i = 0; i < n; i++) if (!strcmp(list[i], s)) return 1;
    return 0;
}

static void php_mania_localtracks(void)
{
    char *tracks[512];
    char *off[256];
    int nt = 0, no = 0, i;
    char dirs[2][RAYDIUM_MAX_DIR_LEN];
    char line[RAYDIUM_MAX_NAME_LEN];
    char list[RAYDIUM_GUI_DATASIZE];
    const char *story[2] = { "mania_drive.story.beg", "mania_drive.story.pro" };
    int d;

    for (i = 0; i < 2; i++) {
        FILE *fp = fopen(story[i], "rt");
        if (!fp) continue;
        while (fgets(line, sizeof line, fp) && no < 256) {
            php_trim(line);
            if (line[0]) off[no++] = strdup(line);
        }
        fclose(fp);
    }

    strcpy(dirs[0], ".");
    raydium_file_home_path_cpy("data", dirs[1]);
    for (d = 0; d < 2; d++) {
        DIR *dir = opendir(dirs[d]);
        struct dirent *e;
        if (!dir) continue;
        while ((e = readdir(dir)) && nt < 512) {
            size_t n = strlen(e->d_name);
            if (n < 5 || strcmp(e->d_name + n - 4, ".mni")) continue;
            if (!strcmp(e->d_name, "dyn_track.mni")) continue;
            if (list_has(off, no, e->d_name) || list_has(tracks, nt, e->d_name)) continue;
            tracks[nt++] = strdup(e->d_name);
        }
        closedir(dir);
    }
    qsort(tracks, nt, sizeof(char *), php_strcmp);

    list[0] = 0;
    for (i = 0; i < nt; i++) {
        if (strlen(list) + strlen(tracks[i]) + 2 < sizeof list) {
            if (i) strcat(list, "\n");
            strcat(list, tracks[i]);
        }
        free(tracks[i]);
    }
    for (i = 0; i < no; i++) free(off[i]);
    php_set_str("list", list, RAYDIUM_GUI_DATASIZE);
}

/* -------------------------------------------- mania_playlist.php */
static void php_mania_playlist(void)
{
    const char *rep = "mania_music/";
    char *files[64];
    int n = 0;
    char result[RAYDIUM_MAX_NAME_LEN];
    DIR *dir = opendir(rep);
    struct dirent *e;

    result[0] = 0;
    if (dir) {
        while ((e = readdir(dir)) && n < 64) {
            size_t l = strlen(e->d_name);
            if (l > 4 && !strcmp(e->d_name + l - 4, ".ogg"))
                files[n++] = strdup(e->d_name);
        }
        closedir(dir);
    }
    if (n) {
        int i, pick = rand() % n;
        snprintf(result, sizeof result, "%s%s", rep, files[pick]);
        raydium_log("Now playing %s ...", result);
        for (i = 0; i < n; i++) free(files[i]);
    } else
        raydium_log("Cannot find any music");
    php_set_str("raydium_ogg_file", result, RAYDIUM_MAX_NAME_LEN);
}

/* ------------------------------------------- mania_tracklist.php */
/* The CQFD Corp. track server is offline: always use the cached list. */
static void php_mania_tracklist(void)
{
    char list[RAYDIUM_GUI_DATASIZE];
    size_t n = 0;
    FILE *fp = fopen("mania_tracklist.txt", "rb");
    list[0] = 0;
    if (fp) {
        n = fread(list, 1, sizeof list - 1, fp);
        list[n] = 0;
        fclose(fp);
    }
    php_set_str("list", list, RAYDIUM_GUI_DATASIZE);
    php_set_int("ok", 0);
}

/* ------------------------------- mania_score.php / mania_score_get.php */
/* Online scores (SOAP web service) do not exist any more. */
static void php_mania_score(void)     { php_set_int("position", 0); }
static void php_mania_score_get(void) { /* keeps "unavailable" / 0 */ }

/* ---------------------------------------------------------- dispatcher */
int raydium_php_exec(char *name)
{
    char base[RAYDIUM_MAX_NAME_LEN];
    raydium_file_basename(base, name);

    if (!strcmp(base, "mania_story.php"))            php_mania_story();
    else if (!strcmp(base, "mania_localtracks.php")) php_mania_localtracks();
    else if (!strcmp(base, "mania_playlist.php"))    php_mania_playlist();
    else if (!strcmp(base, "mania_tracklist.php"))   php_mania_tracklist();
    else if (!strcmp(base, "mania_score.php"))       php_mania_score();
    else if (!strcmp(base, "mania_score_get.php"))   php_mania_score_get();
    else {
        raydium_log("php: script '%s' is not available in this port", name);
        return 0;
    }
    return 1;
}

/* R3S repository / HTTP helpers normally implemented in PHP (rayphp.c) */
int raydium_rayphp_repository_file_get(char *path)  { (void)path; return 0; }
int raydium_rayphp_repository_file_put(char *path, int depends) { (void)path; (void)depends; return 0; }
int raydium_rayphp_repository_file_list(char *filter) { (void)filter; return 0; }
signed char raydium_rayphp_http_test(void) { return 0; }
signed char raydium_rayphp_zip_extract(char *file, char *to) { (void)file; (void)to; return 0; }
