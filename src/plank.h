#ifndef __PLANK_H__
#define __PLANK_H__

#define PERSIST_LOG_MAX 8

enum{
  NUM_PERSIST_SETS =1,
  NUM_PERSIST_TIME1,
  NUM_PERSIST_REST,
  NUM_PERSIST_TIC,
  NUM_PERSIST_LOG_CRT,
  NUM_PERSIST_LOG,
  NUM_PERSIST_ADV = NUM_PERSIST_LOG+PERSIST_LOG_MAX,
  NUM_PERSIST_TIME2,
  NUM_PERSIST_TIME3
};

#define PERSIST_LOG_LEN 30

typedef struct _log_t{
  int ver;
  int year;
  int mon;
  int day;
  int hour;
  int min;
  int sec;
  int sets;
  int time[3];
  int rest;
}log_t;

extern int cfg_sets;
extern int cfg_time[3];
extern int cfg_rest;
extern bool cfg_tic;
extern bool cfg_adv;

void open_go_window_layer(void);
void open_log_window_layer(void);
void open_about_window_layer(void);

void close_go_window_layer(void);
void close_log_window_layer(void);
void close_about_window_layer(void);

#endif /* __PLANK_H__ */
