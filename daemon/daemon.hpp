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
#include "master.hpp"
#include "spdlog/tweakme.h"
#include "spdlog/spdlog.h"
#include "loader.hpp"

namespace robot
{
    /**
     * \brief Main class, that offers daemonizating method, 
     * creating PID-file method, reloading configuration file method in the daemon process,
     * running and monitoring of the 'Master' process method.
     * 
     * It is template class. In the template parameters you must specify
     * a list of inheritors of the robot::service class, that you want to run
     * asyncronously.
     * 
     * */
    template<class... slave> class daemon : private robot::loader
    {
        private:
        
            /**\brief Full filename of the PID-file*/
            const std::string   pid_file;
            
            /**\brief Full name of the configuration file*/
            std::string         config;
            
            /**\brief Returned value of the 'Master' process*/
            int                 status;
            
            /**\brief Process ID*/
            int                 pid=0;
            
            /**\brief Daemonization function
             * \return in the case of error errno returns, in the case of success - 0.
             */
            int daemonize();
            
            /**\brief Create PID-file*/
            void set_pid_file();
        protected:
        
            /**\brief Reloading configuration
             * \return The result of the loader::daemon_load_config*/
            int reload_config();
        public:

            daemon()                      = delete;
            
            daemon(const daemon &)        = delete;
            
            daemon(daemon &&)             = delete;
            
            daemon& operator=(daemon &)   = delete;
            
            /**\brief Constructor
             * \details Load configuration (control is also passed
             * in the static members slave::load_config of your slave classes)
             * and demonize.
             * \param config_name - configuration file name
             * \param pid_dir - full path to the directory, where PID-file 
             * will be placed '/var/run'*/
            explicit daemon(const std::string & config_name,
                             const std::string & pid_dir="/var/run");
                             
            /**\brief Destructor.
             * 
             * Deletes PID-file.
             * */
            ~daemon();
            
            /**\brief Monitor the 'Master' process.
            \return in the case of error -1 returns, in the case of success - 0.
            */
            int monitor_proc() throw();
    };
}//robot
#include "daemon_impl.hpp"
#endif
