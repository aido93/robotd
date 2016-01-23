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
#include <sys/time.h>
#include <sys/resource.h>
#include "definitions.hpp"
#include <stdlib.h>
#include "directories.h"

template<class... slave> 
robot::master<slave...>* robot::master<slave...>::me=nullptr;

template<class... slave>
robot::master<slave...>::master(const std::string & name) : 
    robot::loader(name)
{
    log  = std::make_shared<::spdlog::logger>(tag+"_logger",sink);
    log->set_pattern(std::string("[%T] <%L> (")+tag+") %v");
    log->set_level(lev);
    me=this;
    #ifdef TRACE
        log->trace("Entering to constructor of the robot::master");
    #endif
    log->info("Run");
    //здесь должно все падать =)
    struct rlimit lim = {ramsize, ramsize};
    if (setrlimit(RLIMIT_STACK, &lim) == -1) 
    {
        log->warn("rlimit failed");
        exit(CHILD_NEED_WORK);
    }
}

template<class... slave>
template<std::size_t I>
    typename std::enable_if<I < sizeof...(slave), void>::type
    robot::master<slave...>::update_all_loggers()
{
    update_logger<I>();
    update_all_loggers<I+1>();
}

template<class... slave>
template<std::size_t I> void robot::master<slave...>::update_logger()
{
    {
        slaves[I]->set_level(lev);
    }
    #ifdef TRACE
        log->trace("Logger of {} is updated", I);
    #endif
}

template<class... slave>
void robot::master<slave...>::run_all_services()
{
    for (uint16_t i = 0; i < thread_count; ++i)
    {
        tg.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
        #ifdef TRACE
            log->trace("Thread {} is run", i);
        #endif
    }
}

template<class... slave>
template<std::size_t I, class T>
        void robot::master<slave...>::register_service()
{
    slaves[I]=std::make_unique <T>( lev,
                                    sink,
                                    io_service);
    slaves[I]->run();
    #ifdef TRACE
        log->trace("Service {} is registered", I);
    #endif
}

template<class... slave>
template<std::size_t I , class Head, class... Tail>
    typename std::enable_if<I < sizeof...(slave), void>::type
    robot::master<slave...>::register_all_services()
{
    register_service<I,Head>();
    register_all_services<I+1,Tail...>();
}

//выставляем максимальное число открытых дескрипторов
template<class... slave> 
int robot::master<slave...>::set_fd_limit(const uint16_t& max_fd) const throw()
{
    #ifdef TRACE
        log->trace("Entering to robot::master::set_fd_limit");
    #endif
    struct rlimit lim;
    // зададим текущий лимит на кол-во открытых дискриптеров
    lim.rlim_cur = max_fd;
    // зададим максимальный лимит на кол-во открытых дискриптеров
    lim.rlim_max = max_fd;
    // установим указанное кол-во
    #ifdef TRACE
        log->trace("Exiting from robot::master::set_fd_limit");
    #endif
    return setrlimit(RLIMIT_NOFILE, &lim);
}

#include <fstream>
template<class... slave> 
int robot::master<slave...>::reload_config()
{
    #ifdef TRACE
        log->trace("Entering to robot::master::reload_config");
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
    int result=daemon_load_config<slave...>(str);
    update_all_loggers();
    #ifdef TRACE
        log->trace("Exiting from robot::master::reload_config");
    #endif
    return result;
}

template<class... slave> 
int robot::master<slave...>::work_proc() throw()
{
    #ifdef TRACE
        log->trace("Entering to robot::master::work_proc");
    #endif
    struct sigaction sigact;
    sigset_t         sigset;
    int             signo;
    // сигналы об ошибках в программе будут обрататывать более тщательно
    // указываем что хотим получать расширенную информацию об ошибках
    sigact.sa_flags = SA_SIGINFO;
    // задаем функцию обработчик сигналов
    sigact.sa_sigaction = master::self_debug_wrapper;

    //Если любая из этих функций завершится с ошибкой - выходим
    //так как это происходит на ранних этапах загрузки, 
    //отладиться и настроить систему можно будет в самом начале.
    if(sigemptyset(&sigact.sa_mask)==-1)
    {
        log->error("sigemptyset sigact error occured ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }

    // установим наш обработчик на сигналы
    // ошибка FPU
    if(sigaction(SIGFPE, &sigact, 0)==-1)
    {
        log->error("sigaction error occured on SIGFPE ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    // ошибочная инструкция
    if(sigaction(SIGILL, &sigact, 0)==-1) 
    {
        log->error("sigaction error occured on SIGILL ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    // ошибка доступа к памяти
    if(sigaction(SIGSEGV,&sigact, 0)==-1)
    {
        log->error("sigaction error occured on SIGSEGV ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    // ошибка шины, при обращении к физической памяти
    if(sigaction(SIGBUS, &sigact, 0)==-1) 
    {
        log->error("sigaction error occured on SIGBUS ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    
    if(sigemptyset(&sigset)==-1)
    {
        log->error("sigemptyset sigset error occured ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    // блокируем сигналы которые будем ожидать
    // пользовательский сигнал который мы будем использовать для обновления конфига
    if(sigaddset(&sigset, SIGHUP)==-1)
    {
        log->error("sigaddset error occured on SIGHUP ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    // сигнал запроса завершения процесса
    if(sigaddset(&sigset, SIGTERM)==-1)
    {
        log->error("sigaddset error occured on SIGTERM ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    if(sigprocmask(SIG_BLOCK, &sigset, NULL)==-1)
    {
        log->error("sigprocmask error occured on sigset ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }

    // Установим максимальное кол-во дискрипторов которое можно открыть
    if(set_fd_limit(FD_LIMIT)==-1)
    {
        log->error("Setting max count of file descriptors failed ({})",strerror(errno));
        return CHILD_NEED_TERMINATE;
    }
    
    register_all_services<0,slave...>();
    run_all_services();
        // цикл ожидания сообщений
        while(1)
        {
            // ждем указанных сообщений
            sigwait(&sigset, &signo);
            switch(signo)
            {
                // если это сообщение обновления конфига
                case SIGHUP:
                    //suspend_all_threads();
                    if (reload_config() != 0)
                    {
                        log->warn("Reload config failed: {}",strerror(errno));
                    }
                    else
                    {
                        log->info("Reload config OK");
                    }
                    //resume_all_threads();
                break;
                
                case SIGTERM:
                default:
                    #ifdef TRACE
                        log->trace("Received signal {}",strsignal(signo));
                    #endif
                    //send_system_msg(INTERRUPT|ALL_WORKERS);
                    //interrupt_all_threads();
                    log->info("Stopped");
                    return CHILD_NEED_TERMINATE;
            }
        }
    tg.join_all();
    log->warn("Stopped");
    #ifdef TRACE
        log->trace("Exiting from robot::master::work_proc");
    #endif
    // вернем код не требующим перезапуска
    return CHILD_NEED_TERMINATE;
}

#include <execinfo.h>

template<class... slave> 
void robot::master<slave...>::self_debug_wrapper(int sig, siginfo_t *si, void *ptr)
{
    me->self_debug(sig, si, ptr);
}

/* demangle for GCC */
#include <cxxabi.h>
#include <sstream>
#include <iomanip>
template<class... slave> 
void robot::master<slave...>::self_debug(int sig, siginfo_t *si, void *ptr)
{
    void*  error_addr;
    void*  trace[16];
    int    trace_size;
    char** messages;

    // запишем в лог что за сигнал пришел
    unsigned long int addr=reinterpret_cast<unsigned long int>(si->si_addr);
    std::stringstream ss;
    ss<< std::setfill ('0')<<std::setw(16)<<std::hex<< addr;
    log->error("Signal: {}, Addr: 0x{}",strsignal(sig),ss.str());

    
    #if __WORDSIZE == 64 // если дело имеем с 64 битной ОС
        // получим адрес инструкции которая вызвала ошибку
        error_addr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_RIP];
    #else 
        // получим адрес инструкции которая вызвала ошибку
        error_addr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_EIP];
    #endif

    // произведем backtrace чтобы получить весь стек вызовов 
    trace_size = backtrace(trace, 16);
    if(trace_size==0)
    {
        log->error( "backtrace error");
        exit(CHILD_NEED_WORK);
    }
    trace[1] = error_addr;

    // получим расшифровку трасировки
    messages = backtrace_symbols(trace, trace_size);
    if (messages)
    {
        log->error("== Backtrace ==");
        // allocate string which will be filled with the demangled function name
        size_t funcnamesize = 256;
        char* funcname = (char*)malloc(funcnamesize);
        // запишем в лог
        for (int i = 1; i < trace_size; ++i)
        {
            char *begin_name = 0, *begin_offset = 0, *end_offset = 0,
                 *begin_addr = 0, *end_addr = 0;

            // find parentheses and +address offset surrounding the mangled name:
            // ./module(function+0x15c) [0x8048a6d]
            for (char *p = messages[i]; *p; ++p)
            {
                if (*p == '(')
                {
                    begin_name = p;
                }
                else if (*p == '+')
                {
                    begin_offset = p;
                }
                else if (*p == ')' && begin_offset) 
                {
                    end_offset = p;
                }
                else if (*p == '[') 
                {
                    begin_addr = p;
                }
                else if (*p == ']') 
                {
                    end_addr = p;
                    break;
                }
            }

            if (begin_name && begin_offset && end_offset
                && begin_name < begin_offset)
            {
                *begin_name++ = '\0';
                *begin_offset++ = '\0';
                *end_offset = '\0';
                *begin_addr++ = '\0';
                *end_addr = '\0';
                // mangled name is now in [begin_name, begin_offset) and caller
                // offset in [begin_offset, end_offset). now apply
                // __cxa_demangle():

                int status;
                char* ret = abi::__cxa_demangle(begin_name,
                                funcname, &funcnamesize, &status);
                if (status == 0) 
                {
                    funcname = ret; // use possibly realloc()-ed string
                    log->error()<<messages[i]<<": "<<funcname<<
                                        "()+"<<begin_offset<<
                                        " ["<<begin_addr<<"]";
                }
                else 
                {
                    // demangling failed. Output function name as a C function with
                    // no arguments.
                    log->error()<<messages[i]<<": "<<begin_name<<
                                        "()+"<<begin_offset<<
                                        " ["<<begin_addr<<"]";
                }
            }
            else
            {
                // couldn't parse the line? print the whole line.
                log->error()<< messages[i];
            }
            //log(logger::error, "%s", demangle(Messages[x]).c_str());
        }
        
        log->error("== End Backtrace ==");
        free(messages);
    }
    log->error("Stopped");
    
    // остановим все рабочие потоки и корректно закроем всё что надо
    //дописать!!!!
    
    // завершим процесс с кодом требующим перезапуска
    #ifdef DEBUG
        exit(CHILD_NEED_TERMINATE);
    #else
        //в надежде, что была случайность...
        exit(CHILD_NEED_WORK);
    #endif
}
