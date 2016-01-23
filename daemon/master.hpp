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
#ifndef CHILD_HPP
#define CHILD_HPP

#include <string>
#include <stdarg.h>
#include <signal.h>
#include <boost/lockfree/queue.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include "loader.hpp"
#include "spdlog/tweakme.h"
#include "spdlog/spdlog.h"
#include "tcp_server.hpp"
#include "system_messages.hpp"

namespace robot
{
/**\brief Class, that is created in the child to daemon 'Master' process,
* manage behaviour of the classes, specified in the template parameters. 
* */
template<class... slave> class master : private robot::loader
{
    private:
    
        /**\brief Thread group. All the threads are equal*/
        boost::thread_group tg;
        
        /**\brief io_service*/
        boost::asio::io_service io_service;
        
        /**\brief Set maximum count of the opened file descriptors
         * 
         * \param [in] max_fd - maximum count of the opened file descriptors
         * \return The result of setrlimit function.
         * */
        int set_fd_limit(const uint16_t& max_fd) const throw();
        
        /**\brief Reload configuration file without restarting the 'Master' process*/
        int reload_config();
        
        /**\brief Static pointer to this. 
         * Needs to transfer as option in the static self_debug_wrapper method
         * */
        static master<slave...>* me;
        
        /**\brief wrapper for the self_debug method. 
         * */
        static void self_debug_wrapper(int sig, siginfo_t *si, void *ptr);
        
        /**\brief Log critical errors of the program.
         * 
         * If -DDEBUG flag is set, then 'Master' process returns 
         * CHILD_NEED_TERMINATE in the case 
         * of critical error of any kind, else - CHILD_NEED_WORK
         * \param [in] sig - number of the caught signal
         * \param [in] si  - information about the caught signal
         * \param [in] ptr - instruction address, that caused the error
         * */
        void self_debug(int sig, siginfo_t *si, void *ptr);
        
        /**\brief Array of the unique pointers on slaves*/
        std::unique_ptr<slaves::service> slaves[sizeof...(slave)];
        
        /**\brief Register slave T*/
        template<std::size_t I, class T>
            void register_service();
        
        /**\brief Register all slaves*/
        template<std::size_t I = 0, class Head, class... Tail>
            typename std::enable_if<I < sizeof...(slave), void>::type
            register_all_services();
        
        /**\brief Terminal declaration of the registration method*/
        template<std::size_t I = 0, class... Tail>
            typename std::enable_if<I == sizeof...(slave), void>::type
            register_all_services()
        {}
        
        /**\brief Run all slaves*/
        void run_all_services();
        
        /**\brief Terminal declaration of the update_all_loggers method*/
        template<std::size_t I = 0>
        typename std::enable_if<I == sizeof...(slave), void>::type
            update_all_loggers() {}
        
        /**\brief Update loggers of all the slaves*/
        template<std::size_t I = 0>
            typename std::enable_if<I < sizeof...(slave), void>::type
            update_all_loggers();
        
        /**\brief Update logger of the I-th slave*/
        template<std::size_t I> 
            void update_logger();
    protected:
        
    public:

        master()                      = delete;

        master(const master &)        = delete;

        master(master &&)             = delete;

        master& operator=( master& )  = delete;
        
        /**\brief Constructor
         * \param [in] t - Class tag (name). 
         * */
        explicit master(const std::string & t);
        
        /**\brief Destructor
         * */
        ~master(){}
        
        /**\brief Main working class method. 
         * 
         * Run all slaves, receive all messages from the OS and from the daemon.
         * \return Returns CHILD_NEED_TERMINATE if error occurs or if received
         * notice of termination.
         * */
        int work_proc() throw();
};

}//robot

#include "master_impl.hpp"
#endif
