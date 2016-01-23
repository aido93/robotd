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
#ifndef LOADER_IMPL_HPP
#define LOADER_IMPL_HPP

namespace spdlog
{
    namespace level
    {
        inline int from_str(const char* level_name,
                        ::spdlog::level::level_enum & level_parsed)
        {
            ::spdlog::level::level_enum l;
            for(l=::spdlog::level::trace;
                l<::spdlog::level::off;
                l=(::spdlog::level::level_enum)(l+1))
            {
                if(!strcmp(::spdlog::level::level_names[l],level_name))
                {
                    level_parsed=l;
                    return 0;
                }
            }
            //значение не найдено, парсинга не удался
            return -1;
        }
    }
}

#include <regex>
#include "definitions.hpp"

bool robot::loader::log_to_file=false;
uint16_t                   robot::loader::thread_count=2;
spdlog::level::level_enum  robot::loader::lev=spdlog::level::info;
std::shared_ptr<::spdlog::sinks::rotating_file_sink_mt> robot::loader::sink;
unsigned long robot::loader::ramsize=0;

//Рекурсивная загрузка
template <typename T1>
int robot::loader::load_slave_config(const libconfig::Setting& settings)
{
    return T1::load_config(settings, log, log_to_file);
}

template <typename T1, typename T2, typename... Args>
int robot::loader::load_slave_config(const libconfig::Setting& settings)
{
    int ret=T1::load_config(settings, log, log_to_file);
    if(ret!=0)
    {//если загрузка где-то не удалась - дальше не идем
        return ret;
    }
    else
    {
        return load_slave_config<T2, Args...>(settings);
    }
}

int robot::loader::size_parse(const std::string & size_s, 
                      unsigned long &logsize)
{
    std::smatch m;
    std::regex  e ("^([[:digit:]]*)([kKmM]?)$");
    std::string dummy;
    if(std::regex_search (size_s,m,e))
    {
        dummy=m[1];
        logsize=atoi(dummy.c_str());
        if(logsize!=0)
        {
            dummy=m[2];
            switch(dummy[0])
            {
                case 'k':
                case 'K':
                    logsize*=1024;
                break;
                case 'm':
                case 'M':
                    logsize*=(1024*1024);
                break;
                case '\0':
                
                break;
                default:
                    if(log_to_file)
                    {
                        log->warn("What's happened? regex lies ({})!",dummy);
                        return CHILD_NEED_WORK;
                    }
                    else
                    {
                        std::cerr<<"<"<<tag<<"> What's happened? regex lies! exit"<<std::endl;
                        return -1;
                    }
                break;
            }
        }
        else
        {
            if(log_to_file)
            {
                log->warn("Size must not be zero ({})!",dummy);
                return CHILD_NEED_WORK;
            }
            else
            {
                std::cerr<<"<"<<tag<<"> Size must not be zero! exit "<<std::endl;
                return -1;
            }
        }
        return 0;
    }
    else
    {
        if(log_to_file)
        {
            log->warn("Invalid Size value = {}!",size_s);
            return CHILD_NEED_WORK;
        }
        else
        {
            std::cerr<<"<"<<tag<<"> Invalid Size value = "<<size_s<<std::endl;
            return -1;
        }
    }
}

#include "directories.h"
       
template<class... slave> 
int robot::loader::daemon_load_config(const std::string & filename)
{
    libconfig::Config cfg;
    //стараемся загрузить конфиг
    try
    {
        cfg.readFile(filename.c_str());
    }
    catch(const libconfig::FileIOException &fioex)
    {
        if(log_to_file)
        {
            log->warn("I/O error while reading file {}",filename);
            return CHILD_NEED_WORK;
        }
        else
        {
            std::cerr<<tag<<"I/O error while reading file "<<filename<<std::endl;
            return -1;
        }
    }
    catch(const libconfig::ParseException &pex)
    {
        if(log_to_file)
        {
            log->warn()<<"Parse error at "<<pex.getFile()<<": "<<
                                            pex.getLine()<<" - "<<
                                            pex.getError();
            return CHILD_NEED_WORK;
        }
        else
        {
            std::cerr<<"<"<<tag<<"> "<<"Parse error at "<<pex.getFile()<<
                       " : "<<pex.getLine()<<
                       " - "<<pex.getError()<<std::endl;
            return -1;
        }
    }
    const libconfig::Setting& root = cfg.getRoot();
    const libconfig::Setting& daemon_section = root["Daemon"];
    
    //thread_count
    {
        int dummy=thread_count;
        uint16_t max_threads=boost::thread::hardware_concurrency();
        if(max_threads==0)
        {//может, придется переделать
            max_threads=sizeof...(slave);
        }
        daemon_section.lookupValue("ThreadCount", dummy);
        if(dummy<=max_threads && dummy>0 && dummy<65535)
        {
            thread_count=dummy;
        }
        else
        {
            if(log_to_file)
            {
                log->warn("ThreadCount number must be less then or equal to {}. ThreadCount={}",max_threads,dummy);
                return CHILD_NEED_WORK;
            }
            else
            {
                std::cerr<<"<"<<tag<<
                    "> ThreadCount number must be less then "<<max_threads<<". ThreadCount="<<
                    dummy<<std::endl;
                return -1;
            }
        }
    }
    //name
    std::string logname;
    //если имя объявлено, то заменяем стандартное:
    daemon_section.lookupValue("LogName", logname);
    if(logname=="")
    {
        if(log_to_file)
        {
            log->warn("Couldn't found Logname {}",
                       "in the configuration file or it's empty.");
            return CHILD_NEED_WORK;
        }
        else
        {
            std::cerr<<"Couldn't found Logname "<<
                        "in the configuration file or it's empty."<<std::endl;
            return -1;
        }
    }
    {
        FILE* dummy=fopen(logname.c_str(),"a");
        if(dummy==NULL)
        {
            if(log_to_file)
            {
                log->warn("Failed to open Logfile ({})",strerror(errno));
                return CHILD_NEED_WORK;
            }
            else
            {
                std::cerr<<"<"<<tag<<"> Failed to open logfile: "<<
                                        strerror(errno)<<std::endl;
                return -1;
            }
        }
        else
        {
            fclose(dummy);
        }
    }
    //level
    std::string loglevel;
    if(log_to_file)
    {
        loglevel=::spdlog::level::to_str(log->level());
    }
    else
    {//при первом запуске используем дефолтное значение info
        loglevel=::spdlog::level::to_str(::spdlog::level::info);
    }
    daemon_section.lookupValue("LogLevel", loglevel);
    std::transform(loglevel.begin(), loglevel.end(), loglevel.begin(), ::tolower);
    //дефолтное значение - информация без параноидального дебага
    int ret=::spdlog::level::from_str(loglevel.c_str(), lev);
    if(ret)
    {
        if(log_to_file)
        {
            log->warn("Invalid value of the loglevel ({})",loglevel);
            return CHILD_NEED_WORK;
        }
        else
        {
            std::cerr<<"<"<<tag<<"> Invalid value of the loglevel: ("<<
                                    loglevel<<")"<<std::endl;
            return -1;
        }
    }
    //size
    std::string size_s;
    daemon_section.lookupValue("Size", size_s);
    unsigned long logsize=0;
    ret=size_parse(size_s,logsize);
    if (ret)
    {//была какая-то ошибка
        return ret;
    }
    
    //max_ram
    size_s="";
    daemon_section.lookupValue("MaxRamUse", size_s);
    ret=size_parse(size_s,ramsize);
    if (ret)
    {//была какая-то ошибка
        return ret;
    }
    //создаем логгер, если все было успешно
    sink = std::make_shared<::spdlog::sinks::rotating_file_sink_mt> 
                                (logname, "log", logsize, 1, true);
    log  = std::make_shared<::spdlog::logger>(tag+"_logger",sink);
    log->set_pattern(std::string("[%T] <%L> (")+tag+") %v");
    log->set_level(lev);
    log->debug("Log config loaded");
    if(log_to_file)
    {
        log->info("===========================LOADED OPTIONS============================");
        log->info("{} using these options:",PACKAGE_NAME);
        log->info("Configuration file: {}", CONFIG_FILE);
        log->info("Count of the threads: {}", thread_count);
        log->info("LogName: {}.log", logname);
        log->info("Max Log Size: {}", size_s);
        log->info("Log level: {}", loglevel);
        log->info("Max RAM Size: {}", ramsize);
    }
    else
    {
        std::cout<<"===========================LOADED OPTIONS============================\r\n";
        std::cout<<PACKAGE_NAME<<" using these options:\r\n";
        std::cout<<"Configuration file: "<< CONFIG_FILE<<"\r\n";
        std::cout<<"Count of the threads: "<< thread_count<<"\r\n";
        std::cout<<"LogName: "<< logname<<".log\r\n";
        std::cout<<"Max Log Size: "<< size_s<<"\r\n";
        std::cout<<"Log level: "<< loglevel<<"\r\n";
        std::cout<<"Max RAM Size: "<< ramsize<<"\r\n";
    }
    ret=load_slave_config<slave...>(root);
    if(log_to_file)
    {
        log->info("=====================================================================");
    }
    else
    {
        std::cout<<"=====================================================================\n";
    }
    //больше не логгируем в консоль. теперь только в файл
    //строчка важна!
    log_to_file=true;
    return ret;
}

#endif
