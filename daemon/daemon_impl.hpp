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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */
 /*
 * Author: Diakonov Igor, 01.2016
 * */
#include <errno.h>
//for transforming to lower case
#include <algorithm>
#include <iostream>
#include <regex>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include "definitions.hpp"
#include "directories.h"

template<class... slave>
robot::daemon<slave...>::daemon(const std::string & config_name,
                const std::string & pid_dir) :
                     robot::loader("Daemon"),
                     pid_file(pid_dir+"/"+PACKAGE+".pid"),
                     config(config_name)
{
    int ret=daemon_load_config<slave...>(config);
    //грузим конфиги и включаем логгер
    #ifdef TRACE
        log->trace("daemon_load_config returned {}", ret);
    #endif
    if(ret)
    {
        log->error("Some errors was found in slave sections of the config file");
        exit(ret);
    }
    #ifdef TRACE
        log->trace("Slave configs loaded");
    #endif
    //демонизируемся
    if(daemonize())
    {
        exit(EXIT_FAILURE);
    }
    log->info("Parent Daemon run");
}

template<class... slave>
robot::daemon<slave...>::~daemon()
{
    log->info("Daemon exited");
    // удалим файл с PID'ом
    unlink(pid_file.c_str());
}

template<class... slave> 
int robot::daemon<slave...>::daemonize()
{
    pid = fork();
    if (pid == -1) 
    {   
		// если не удалось запустить потомка
        // выведем на экран ошибку и её описание
        std::cerr<<tag<<" Start Daemon failed: "<<strerror(errno)<<"\n";
        return errno;
    }
    else if (!pid) // если это потомок.
    {
        // разрешаем выставлять все биты прав на создаваемые файлы, 
        // иначе у нас могут быть проблемы с правами доступа
        umask(0);
        
        // создаём новый сеанс, чтобы не зависеть от родителя
        if(setsid()==((pid_t)-1))
        {
            std::cerr<<tag<<" Failed to create new session: "<<strerror(errno)<<"\n";
            return errno;
        }
        
        // переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
        // к примеру с размонтированием дисков
        if(chdir("/")==-1)
        {
            std::cerr<<tag<<" Failed to set default directory: "<<strerror(errno);
            return errno;
        }
        // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
        if(close(STDIN_FILENO)==-1)
        {
            std::cerr<<tag<<"Failed to close stdin descriptor: "<<strerror(errno);
            return errno;
        }
        if(close(STDOUT_FILENO)==-1)
        {
            std::cerr<<tag<<" Failed to close stdout descriptor: "<<strerror(errno);
            return errno;
        }
        if(close(STDERR_FILENO)==-1)
        {
            std::cerr<<tag<<" Failed to close stderr descriptor: "<<strerror(errno);
            return errno;
        }
        set_pid_file();
        return 0;
    }
    else 
    {   // если это родитель
        // завершим процесc, т.к. основную свою задачу (запуск демона) мы выполнили
        exit(EXIT_SUCCESS);
        return 0;//до сюда все равно не доходит
    }
}

template<class... slave> 
void robot::daemon<slave...>::set_pid_file()
{
    #ifdef TRACE
        log->trace("Entered to robot::daemon::set_pid_file");
    #endif
    FILE *f, *exist;
    exist=fopen(pid_file.c_str(),"r");
    //если существует - убиваем
    if(exist!=NULL)
    {
        unlink(pid_file.c_str());
        fclose(exist);
        #ifdef TRACE
            log->trace("Existing pid-file deleted");
        #endif
    }
    //и создаем новый
    f = fopen(pid_file.c_str(), "w+");
    if (f)
    {
        fprintf(f, "%u\n", getpid());
        #ifdef TRACE
            log->trace("New pid-file created");
        #endif
        fclose(f);
    }
    else
    {
        log->error("Failed to set PID file: {}", strerror(errno));
        exit(EXIT_FAILURE);
    }
    #ifdef TRACE
        log->trace("Exiting from robot::daemon::set_pid_file");
    #endif
}

template<class... slave> 
int robot::daemon<slave...>::reload_config()
{
    //extern std::string CONFIG_LOAD;
    #ifdef TRACE
        log->trace("Entering to robot::daemon::reload_config");
    #endif
    std::filebuf fb;
    fb.open (CONFIG_LOAD,std::ios::in);
    std::istream file(&fb);
    std::string str;
    std::getline(file, str);
    #ifdef TRACE
        log->trace("Path to config file restored: {}", str);
    #endif
    //здесь мы уже пишем ошибки в файл, поэтому true
    int result= daemon_load_config<slave...>(str);
    #ifdef TRACE
        log->trace("Exiting from robot::daemon::reload_config");
    #endif
    return result;
}

template<class... slave> 
int robot::daemon<slave...>::monitor_proc() throw()
{
    #ifdef TRACE
        log->trace("Entering to robot::daemon::monitor_proc");
    #endif
    int      need_start = 1;
    sigset_t sigset;
    siginfo_t siginfo;
    // настраиваем сигналы которые будем обрабатывать
    {
        if(sigemptyset(&sigset)==-1)
        {
            log->error("sigemptyset error occured ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        // сигнал остановки процесса пользователем
        if(sigaddset(&sigset, SIGQUIT)==-1)
        {
            log->error("sigaddset error occured on SIGQUIT ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        // сигнал для остановки процесса пользователем с терминала
        if(sigaddset(&sigset, SIGINT)==-1)
        {
            log->error("sigaddset error occured on SIGINT ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        // сигнал запроса завершения процесса
        if(sigaddset(&sigset, SIGTERM)==-1)
        {
            log->error("sigaddset error occured on SIGTERM ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        // сигнал посылаемый при изменении статуса дочернего процесса
        if(sigaddset(&sigset, SIGCHLD)==-1)
        {
            log->error("sigaddset error occured on SIGCHLD ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        // пользовательский сигнал который мы будем использовать для обновления конфига
        if(sigaddset(&sigset, SIGHUP)==-1)
        {
            log->error("sigaddset error occured on SIGHUP ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        if(sigprocmask(SIG_BLOCK, &sigset, NULL)==-1)
        {
            log->error("sigprocmask error occured ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    #ifdef TRACE
        log->trace("All signals are registered");
    #endif
    // бесконечный цикл работы
    for (;;)
    {
        // если необходимо создать потомка
        if (need_start)
        {
            // создаём потомка
            pid = fork();
        }
        
        need_start = 1;
        
        if (pid == -1) // если произошла ошибка
        {
            // запишем в лог сообщение об этом
            log->error("Fork failed ({})",strerror(errno));
            exit(EXIT_FAILURE);
        }
        else if (!pid) // если мы потомок
        {
            #ifdef TRACE
                log->trace("Child process created");
            #endif
            // данный код выполняется в потомке
            // запустим функцию отвечающую за работу демона
            master<slave...> slaves("Master");
            #ifdef TRACE
                log->trace("\'Master\' class created");
            #endif
            status = slaves.work_proc();
            // завершим процесс
            exit(status);
        }
        else // если мы родитель
        {
            // данный код выполняется в родителе
            #ifdef TRACE
                log->trace("Parent process continued");
            #endif
            // ожидаем поступление сигнала
            if(sigwaitinfo(&sigset, &siginfo)==-1)
            {
                log->error("sigwaitinfo error occured ({})",strerror(errno));
                
            }
            // если пришел сигнал от потомка
            if (siginfo.si_signo == SIGCHLD)
            {
                // получаем статус завершение
                wait(&status);
                
                // преобразуем статус в нормальный вид
                status = WEXITSTATUS(status);
                #ifdef TRACE
                    log->trace("\'Master\' process returned {}",status);
                #endif
                 // если потомок завершил работу с кодом говорящем о том, что нет нужды дальше работать
                if (status == CHILD_NEED_TERMINATE)
                {
                    // запишем в лог сообщени об этом 
                    log->info("Master stopped");
                    // прервем цикл
                    break;
                }
                else if (status == CHILD_NEED_WORK) // если требуется перезапустить потомка
                {
                    // запишем в лог данное событие
                    log->info("Master restart");
                }
            }
            else if (siginfo.si_signo == SIGHUP) // если пришел сигнал что необходимо перезагрузить конфиг
            {
                //перезагрузим конфиг
                if(reload_config()!=0)
                {
                    log->error("Reload config failed: {}",strerror(errno));
                }
                else
                {
                    log->info("Reload config OK");
                }
                if(kill(pid, SIGHUP)==-1) // и перешлем его потомку
                {
                    log->warn("Couldn't send SIGHUP to child process");
                }
                need_start = 0; // установим флаг что нам не надо запускать потомка заново
            }
            else // если пришел какой-либо другой ожидаемый сигнал
            {
                // запишем в лог информацию о пришедшем сигнале
                log->info("Received signal {}", strsignal(siginfo.si_signo));
                // убьем потомка
                if(kill(pid, SIGTERM)==-1) // перешлем его потомку
                {
                    log->info("Couldn't send SIGTERM to child process");
                }
                status = 0;
                break;
            }
        }
    }

    // запишем в лог, что мы остановились
    log->info("Stop");
    #ifdef TRACE
        log->trace("Exiting from robot::daemon::monitor_proc");
    #endif
    return status;
}

