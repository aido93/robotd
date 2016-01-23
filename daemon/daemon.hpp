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
#ifndef DAEMON_HPP
#define DAEMON_HPP

#include <stdint.h>
#include <signal.h>
#include <string>
#include <memory>
#include "config.h"
#include "protocol.hpp"
#include "master.hpp"
#include "spdlog/tweakme.h"
#include "spdlog/spdlog.h"
#include "loader.hpp"

/**
 * Для демона будет использоваться пространство имен robot, 
 * а для слэйвов - slaves
 * */
namespace robot
{
    /**
     * \brief Основной класс, который предоставляет функции демонизации, 
     * создания PID-файла, перезагрузки файла конфигурации в процессе демона,
     * запуска и наблюдения за 'Master'-процессом.
     * 
     * Является шаблонным классом. В параметрах шаблона необходимо указать 
     * список классов-наследников robot::service, которые должны работать 
     * в 'Master'-процессе асинхронно.
     * 
     * */
    template<class... slave> class daemon : private robot::loader
    {
        private:
        
            /**\brief Полное имя файла, в котором содержится номер процесса (PID)*/
            const std::string   pid_file;
            
            /**\brief Полное имя файла конфигурации*/
            std::string         config;
            
            /**\brief Результат работы Master-процесса*/
            int                 status;
            
            /**\brief PID процесса*/
            int                 pid=0;
            
            /**\brief Функция демонизации
             * \return errno в случае ошибки, 0 - в случае успеха
             */
            int daemonize();
            
            /**\brief Функция создания PID-файла*/
            void set_pid_file();
        protected:
        
            /**\brief Функция перезагрузки файла конфигурации
             * \return Результат выполнения loader::daemon_load_config*/
            int reload_config();
        public:
        
            /**\brief Конструктор по умолчанию удален, так как в любом случае 
             * необходимо указать файл конфигурации при запуске программы.
             * Поведение по умолчанию лучше перенести на скрипты запуска.
             * */
            daemon()                      = delete;
            
            /**\brief Конструктор копирования удален, так как неясно,
             * куда и зачем копировать экземпляр класса daemon, 
             * который запускается в единственном экземпляре при запуске
             * программы.
             * */
            daemon(const daemon &)        = delete;
            
            /**\brief Конструктор перемещения удален, так как неясно,
             * куда и зачем перемещать экземпляр класса daemon, 
             * который запускается в единственном экземпляре при запуске
             * программы.
             * */
            daemon(daemon &&)             = delete;
            
            /**\brief Оператор присваивания удален, так как 
             * удалены конструкторы копирования и присваивания.
             * */
            daemon& operator=(daemon &)   = delete;
            
            /**\brief Конструктор
             * \details Конструктор класса демонизации. Загружает 
             * файл конфигурации (управление также передается 
             * в функции static slave::load_config для загрузки 
             * своих секций конфигурации) и демонизируется.
             * \param config_name - имя файла конфигурации
             * \param pid_dir - полный путь до директории, где будет находится PID-файл
             * по умолчанию '/var/run'*/
            explicit daemon(const std::string & config_name,
                             const std::string & pid_dir="/var/run");
                             
            /**\brief Деструктор.
             * 
             * Удаляет PID-файл.
             * */
            ~daemon();
            
            /**\brief Функция наблюдения за 'Master'-процессом.
            \return -1 в случае ошибки, 0 - в случае успеха
            */
            int monitor_proc() throw();
    };
}//robot
#include "daemon_impl.hpp"
#endif
