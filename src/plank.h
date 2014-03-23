#ifndef __PLANK_H__
#define __PLANK_H__

enum{
  NUM_PERSIST_SETS =1,
  NUM_PERSIST_TIME,
  NUM_PERSIST_REST,
  NUM_PERSIST_TIC,
  /* add new items here before the logs , if need */
  NUM_PERSIST_LOG_CRT,
  NUM_PERSIST_LOG
};

#define PERSIST_LOG_MAX 8
#define PERSIST_LOG_LEN 30

typedef struct _log_t{
  int year;
  int mon;
  int day;
  int hour;
  int min;
  int sec;
  int sets;
  int time;
  int rest;
}log_t;

extern int cfg_sets;
extern int cfg_time;
extern int cfg_rest;
extern bool cfg_tic;

void open_go_window_layer(void);
void open_log_window_layer(void);
void open_about_window_layer(void);

#endif /* __PLANK_H__ */
