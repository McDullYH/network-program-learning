#ifndef __LOG_H__
#define __LOG_H__



#ifdef OPEN_LOG

#include <stdio.h>

#ifdef CLOSE_COLOR


#define SET_FORGROUND_COLOR(color)	 (void(0));				
#define SET_DEFAULT    (void(0));


#else

#ifdef WIN32
#include <windows.h>
#define RED FOREGROUND_RED|FOREGROUND_INTENSITY 
#define BLUE FOREGROUND_BLUE|FOREGROUND_INTENSITY 
#define GREEN FOREGROUND_GREEN|FOREGROUND_INTENSITY 

#define YELLOW FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY 
#define WHITE FOREGROUND_RED | FOREGROUND_GREEN |  FOREGROUND_BLUE  

#define HIGH_LIGHT FOREGROUND_INTENSITY 

#define SET_FORGROUND_COLOR(color)													\
{																						\
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);				\
}

#define SET_DEFAULT  SET_FORGROUND_COLOR(WHITE);
#else

#define RED 31
#define YELLOW 33
#define BLUE 34
#define GREEN 32

#define SET_FORGROUND_COLOR(color)													\
{																					\
	printf("\033[0;%d;1m",color);  														\
}
#define SET_DEFAULT printf("\033[0m");
#endif 


#endif //CLOSE_COLOR




#ifdef LOG_DETAIL
#define PRINT_FILE_FUNC_LINE                                  \
{                                                             \
  printf("in %s ,%s, line:%d--",__FILE__,__FUNCTION__,__LINE__);        \
}
#else
#define PRINT_FILE_FUNC_LINE (void(0));
#endif


#define LOG(LOG_TAG,...)  \
{                         \
  printf("%-10s",LOG_TAG);   \
  PRINT_FILE_FUNC_LINE    \
  printf(__VA_ARGS__);    \
}


#ifdef LOG_ERROR
#define LOGE(...)           \
SET_FORGROUND_COLOR(RED)	  \
LOG("error",__VA_ARGS__)    \
SET_DEFAULT
#else
#define LOGE(...) (void(0)); 
#endif

#ifdef LOG_WARN
#define LOGW(...)           \
SET_FORGROUND_COLOR(YELLOW)	  \
LOG("warning",__VA_ARGS__)  \
SET_DEFAULT
#else
#define LOGW(...) (void(0));
#endif

#ifdef LOG_INFO
#define LOGI(...) LOG("info",__VA_ARGS__)
#else
#define LOGI(...) (void(0));
#endif


#ifdef LOG_DEBUG
#define LOGD(...)           \
SET_FORGROUND_COLOR(GREEN)	  \
LOG("debug",__VA_ARGS__)  \
SET_DEFAULT
#else
#define LOGD(...) (void(0));
#endif



#else
#define LOGI(...) (void(0));
#define LOGW(...) (void(0));
#define LOGE(...) (void(0));
#define LOGD(...) (void(0));


#endif  //OPEN_LOG
#endif  //__LOG_H__


