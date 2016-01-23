/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fsifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
 /*
  * Author: Diakonov Igor, 01.2016
  * */
#include "cli_parser.hpp"
#include <stdio.h>
#include <unistd.h>
#include "directories.h"
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include "config.h"
#include <sys/stat.h>
#include <inttypes.h>

#define WRONG_OPTIONS       1 ///<возвращается, если опции указаны некорректно
#define HELP_OPTION         2 ///<возвращается, если была передана опция help
#define DONT_DO_ANYTHING    3 ///<возвращается, если невозможно выполнить запрошенную операцию

struct keys{
    const char* key;
    bool needValue;
};

/**
 * \brief Список опций
 * 
 * Список опций в формате "Имя_опции", "Требуется ли опции аргумент"
 * */
const struct keys options[]=
    {
        {"load",true},
        {"reload",false},
        {"restart",false},
        {"stop",false},
        {"status",false},
        {"help",false},
        {"",false}
    };

/**
 * \brief Краткая справка, которая выдается, если опции были переданы неверно
 * 
 * Выводит список допустимых опций и формат их использования
 * */
inline void false_parameters()
{
    printf("%s\n",PACKAGE_STRING);
    printf("Usage: %s  <option>\n\n",PACKAGE_NAME);
    printf("Options:\n");
    uint8_t i=0;
    while(options[i].key[0]!='\0')
    {
        printf("\t%s",options[i].key);
        if(options[i].needValue)
        {
            printf(" [filename.cfg]\n");
        }
        else
        {
            printf(" \r\n");
        }
        ++i;
    }
}

/**
 * \brief Функция проверяет, возможно ли открыть файл конфигурации
 * и, в случае успеха, записывает во вспомогательный файл путь до файла конфигурации
 * 
 * \return В случае успеха возвращает 0, в случае ошибки - errno
 * */
int option_load()
{
    FILE* conf;
    conf=fopen((std::string(get_current_dir_name())+"/"+CONFIG_FILE).c_str(),"r");
    //проверяем, достаточный ли это путь
    if(conf==NULL)
    {
        //не достаточный. может в /еtc лежит?
        CONFIG_FILE=std::string(ETC_CONF_DIR)+"/"+CONFIG_FILE;
        conf=fopen(CONFIG_FILE.c_str(),"r");
        if(conf==NULL)
        {
            fprintf(stderr,"Error: Failed to open config file: %m\n");
            return errno;
        }
    }
    else
    {
        CONFIG_FILE=std::string(get_current_dir_name())+"/"+CONFIG_FILE;
    }
    fclose(conf);
    
    //пишем в run/conf-файл путь к конфиг-файлу
    FILE* config_load_desc=fopen(CONFIG_LOAD.c_str(),"w");
    if(config_load_desc==NULL)
    {
        if(errno==ENOENT)
        {//no such file
            struct stat statbuf;
            if(stat(RUN_CONF_DIR.c_str(),&statbuf)==-1)
            {
                if(mkdir(RUN_CONF_DIR.c_str(),01777)==-1)
                {
                    fprintf(stderr,"Error: Failed to create directory %s%s: %m\n", 
                            RUN_CONF_DIR.c_str(), CONFIG_LOAD.c_str());
                    return errno;
                }
            }
            config_load_desc=fopen(CONFIG_LOAD.c_str(),"w");
            if(config_load_desc==NULL)
            {
                fprintf(stderr,"Error: %s - %m\n", 
                        CONFIG_LOAD.c_str());
                return errno;
            }
        }
        else
        {
            fprintf(stderr,"Error: Failed to open directory %s%s: %m\n", 
                            RUN_CONF_DIR.c_str(), CONFIG_LOAD.c_str());
            return errno;
        }
    }
    fprintf(config_load_desc, "%s\n",CONFIG_FILE.c_str());
    fflush(config_load_desc);
    fclose(config_load_desc);
    return 0;
}


/**
 * \brief Выводит справку по программе
 * */
inline void help()
{
    printf("%s - cross-platform UNIX daemonized server.\r\n",PACKAGE_STRING);
    printf("License GNU GPLv2 <http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>\r\n");
    printf("This program can be used for development your own daemon.\r\n");
    printf("This is free software: you are free to change and redistribute it.\r\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\r\n");
    printf("Written by Igor Diakonov. Compiled 21.01.2016.\r\n\r\n");
    printf("Usage: %s <option>\r\n\r\n",PACKAGE_NAME);
    printf("Options:\r\n");
    printf("\thelp                  show this message\r\n");
    printf("\tload [filename.cfg]   run daemon with configuration loaded from filename.cfg.\r\n");
    printf("\t                      Note: It can be only one copy of the daemon run in the system.\r\n");
    printf("\treload                reload configuration file without stopping daemon.\r\n");
    printf("\tstop                  terminate daemon.\r\n");
    printf("\trestart               restart daemon with the same configuration.\r\n");
    printf("\tstatus                show status of the daemon. If daemon is run you will see its PID.\r\n");
}

/**
 * \brief Функция, которая вызывается, если процесс демона не найден, 
 * но указанные пользователем опции требуют запущенного процесса и их обработать нужно
 * 
 * \param [in] argc - количество аргументов командной строки
 * \param [in] argv - указатель на массив аргументов командной строки
 * \return DONT_DO_ANYTHING или WRONG_OPTIONS
 * */
int no_daemon_process(int argc, char**argv)
{
    //только при 3 опциях нам не нужно дальнейшее выполнение
    //при всех остальных - просто запуск демона
    if(strcmp(argv[1],options[3].key)==0)
    {//stop - выходим
        return DONT_DO_ANYTHING;
    }
    else if(strcmp(argv[1],options[4].key)==0)
    {//status
        printf("%s stopped\r\n", PACKAGE_NAME);
        return DONT_DO_ANYTHING;
    }
    else if(strcmp(argv[1],options[0].key)==0 || 
            strcmp(argv[1],options[1].key)==0 || 
            strcmp(argv[1],options[2].key)==0)//load|reload|restart
    {
        //Задаем путь к конфигу
        if(argc==3)
        {
            CONFIG_FILE=argv[2];
        }
        else if(argc==2)
        {
            //если хотим использовать дефолтный конфиг
        }
        else
        {
            printf("Usage: %s load [filename.cfg]\r\n", PACKAGE_NAME);
            return WRONG_OPTIONS;
        }
        
        int ret=option_load();
        return ret;
    }
    else
    {//В противном случае просто не знаем, что нам делать
        printf("Undefined option\n%s\r\n", argv[1]);
        false_parameters();
        return WRONG_OPTIONS;
    }
}

int parse_options(int argc, char**argv)
{
    // если количество параметров командной строки не равно двум или трем, 
    // то покажем как использовать демона
    if (!(argc == 2 || argc==3))
    {
        false_parameters();
        return WRONG_OPTIONS;
    }
    
    if(strcmp(argv[1],options[5].key)==0)//help
    {
        help();
        return HELP_OPTION;
    }
    
    //индикатор того, что PID-файл найден.
    bool pid_found=false;
    uint16_t pid=0;
    //проверяем, существует ли PID-файл и открываем его на чтение
    FILE *exist=fopen(PID_FILE.c_str(),"r");
    if(exist!=NULL)
    {//вроде существует. ошибок при открытии не выявлено.
        uint16_t pid_exists=0;
        //читаем PID процесса
        int words_read=fscanf(exist,"%" SCNu16,&pid_exists);
        fclose(exist);
        if(words_read<1)
        {//не умеем читать
            fprintf(stderr,"Error: Failed to read PID-file: %m words read %i\n", 
                    words_read);
            return errno;
        }
        DIR *dfd;
        struct dirent *dp;
        dfd=opendir("/proc");
        //проверяем, есть ли процесс с нужным нам  PID-ом
        while( (dp=readdir(dfd)) != NULL )
        {
            if (isdigit(dp->d_name[0]) && atoi (dp->d_name)==pid_exists)
            {//Процесс существует. можно прерываться и смотреть опции
                pid=pid_exists;
                pid_found=true;
                break;
            }
        }
        closedir(dfd);
        if(!pid_found)
        {//Процесса такого нет, файл ложный
         //удаляем этот файл
            if(unlink(PID_FILE.c_str())==-1)
            {
                fprintf(stderr, "Can't remove PID-file: %m\n");
                return errno;
            }
            return no_daemon_process(argc, argv);
        }
    }
    else
    {//файл не существует. значит, процесс точно упал. 
        return no_daemon_process(argc,argv);
    }
    
    //------------------------------------------------------------------
    //В этой секции pid-файл верен, процесс существует
    //------------------------------------------------------------------
    if(strcmp(argv[1],options[4].key)==0)//status
    {
        printf("%s run. PID: %i\r\n",PACKAGE_NAME,pid);
        
        return DONT_DO_ANYTHING;
    }
    
    //проверяем, можем ли открыть конфиг
    FILE *exist_conf=fopen(CONFIG_LOAD.c_str(),"r");
    if(exist_conf!=NULL)
    {
        char tmp[256];
        bzero(tmp,256);
        int words_read=fscanf(exist_conf,"%s",tmp);
        std::string s=CONFIG_FILE;//for resque defaults
        CONFIG_FILE=tmp;
        fclose(exist_conf);
        if(words_read<1)
        {//не умеем читать
            exist_conf=fopen(CONFIG_LOAD.c_str(),"w");
            fprintf(exist_conf, "%s",s.c_str());
            fflush(exist_conf);
            fclose(exist_conf);
            
            fprintf(stderr,"Error: Failed to read %s: %m, fscanf returned %i\n", 
                    CONFIG_LOAD.c_str(), words_read);
            return errno;
        }
    }
    else
    {
        if(errno==ENOENT)
        {//no such file
            struct stat statbuf;
            if(stat(RUN_CONF_DIR.c_str(),&statbuf)==-1)
            {
                if(mkdir(RUN_CONF_DIR.c_str(),01777)==-1)
                {
                    fprintf(stderr,"Error: Failed to create directory %s%s: %m\n", 
                            RUN_CONF_DIR.c_str(), CONFIG_LOAD.c_str());
                    return errno;
                }
            }
            exist_conf=fopen(CONFIG_LOAD.c_str(),"w");
            if(exist_conf==NULL)
            {
                fprintf(stderr,"Error: I don't understand, why file %s couldn't be created...: %m\n", 
                CONFIG_LOAD.c_str());
                return errno;
            }
            fprintf(exist_conf, "%s\n",
                    (ETC_CONF_DIR+"/"+PACKAGE_NAME+"/"+CONFIG_LOAD).c_str());
            fflush(exist_conf);
            fclose(exist_conf);
        }
        else
        {
            fprintf(stderr,"Error: Failed to open directory %s%s: %m\n", 
                            RUN_CONF_DIR.c_str(), CONFIG_LOAD.c_str());
            return errno;
        }
    }

    if(strcmp(argv[1],options[0].key)==0)//load
    {
        //В случае, когда демон уже запущен, 
        //эта команда приказывает заменить конфиг
        
        //Задаем путь к конфигу
        if(argc==3)
        {
            CONFIG_FILE=argv[2];
        }
        else
        {
            printf("Usage: %s load [filename.cfg]\n",PACKAGE_NAME);
            return WRONG_OPTIONS;
        }
        int ret=option_load();
        if(ret!=0)
        {
            return ret;
        }
        //даем команду обновить конфиг
        ret=kill(pid,SIGHUP);
        if(ret)
        {
            fprintf(stderr, "Can't load config: %m\n");
            return errno;
        }
        return DONT_DO_ANYTHING;
    }
    else if(strcmp(argv[1],options[1].key)==0)//reload
    {
        //процесс уже существует.
        //проверки на существование конфига не нужны 
        //(точнее, они не здесь, а в парсере)
        //просто обновляем конфиг
        int ret=kill(pid,SIGHUP);
        if(ret==-1)
        {
            fprintf(stderr, "Can't reload %s: %m\n", PACKAGE_NAME);
            return errno;
        }
        return DONT_DO_ANYTHING;
    }
    else if(strcmp(argv[1],options[2].key)==0)//restart
    {
        //даем сигнал умереть, после выхода из if-else запускаемся
        printf("Current config: %s\n", CONFIG_FILE.c_str());
        int ret=kill(pid,SIGTERM);
        if(ret==-1)
        {
            fprintf(stderr, "Can't restart %s: %m\n", PACKAGE_NAME);
            return errno;
        }
        return ret;
    }
    else if(strcmp(argv[1],options[3].key)==0)//stop
    {
        //просто даем сигнал умереть
        int ret=kill(pid,SIGTERM);
        if(ret==-1)
        {//не убился:
            fprintf(stderr, "Can't stop %s: %m\n",PACKAGE_NAME);
            return errno;
        }
        else
        {
            printf("%s stopped: %i\n", PACKAGE_NAME,pid);
            return DONT_DO_ANYTHING;
        }
    }
    else
    {
        printf("Undefined option\n%s\n",argv[1]);
        false_parameters();
        return WRONG_OPTIONS;
    }
    return 0; 
}
