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
/**\brief Класс, который запускается в дочернем к демону 'Master'-процессе,
* управляет поведением классов, указанных в параметрах шаблона 'slave'. 
* */
template<class... slave> class master : private robot::loader
{
    private:
    
        /**\brief Группа потоков. все потоки равноправны*/
        boost::thread_group tg;
        
        /**\brief Основной объект обработки сообщений*/
        boost::asio::io_service io_service;
        
        /**\brief Функция, которая устанавливает максимальное допустимое количество 
         * одновременно открытых файловых дескрипторов
         * 
         * \param [in] max_fd - максимальное количество открытых дескрипторов
         * \return Результат выполнения setrlimit.
         * */
        int set_fd_limit(const uint16_t& max_fd) const throw();
        
        /**\brief Функция перезагрузки файлов конфигурации без перезапуска процесса*/
        int reload_config();
        
        /**\brief Статический указатель на this. 
         * Нужен для передачи в статическую функцию self_debug_wrapper
         * */
        static master<slave...>* me;
        
        /**\brief Обертка для функции self_debug. 
         * Должна быть статической для того, чтобы можно было
         * ее зарегистрировать как обработчик сигналов
         * */
        static void self_debug_wrapper(int sig, siginfo_t *si, void *ptr);
        
        /**\brief Функция для логгирования информации о серьезных ошибках программы.
         * 
         * Если при компиляции указана опция -DDEBUG, то при любой произошедшей ошибке 
         * выходит с кодом CHILD_NEED_TERMINATE, иначе - CHILD_NEED_WORK
         * \param [in] sig - номер пришедшего сигнала
         * \param [in] si  - информация о пришедшем сигнале
         * \param [in] ptr - адрес инструкции, которая вызвала ошибку
         * */
        void self_debug(int sig, siginfo_t *si, void *ptr);
        
        /**\brief Уникальный указатель на массив исполнителей задач*/
        std::unique_ptr<slaves::service> slaves[sizeof...(slave)];
        
        /**\brief Функция регистрации исполнителя T под номером I*/
        template<std::size_t I, class T>
            void register_service();
        
        /**\brief Функция регистрации всех исполнителей*/
        template<std::size_t I = 0, class Head, class... Tail>
            typename std::enable_if<I < sizeof...(slave), void>::type
            register_all_services();
        
        /**\brief Терминирующее объявление функции регистрации всех исполнителей*/
        template<std::size_t I = 0, class... Tail>
            typename std::enable_if<I == sizeof...(slave), void>::type
            register_all_services()
        {}
        
        /**\brief Функция запуска всех исполнителей*/
        void run_all_services();
        
        /**\brief Терминирующее объявление функции обновления логгеров всех исполнителей*/
        template<std::size_t I = 0>
        typename std::enable_if<I == sizeof...(slave), void>::type
            update_all_loggers() {}
        
        /**\brief Функция обновления логгеров всех исполнителей*/
        template<std::size_t I = 0>
            typename std::enable_if<I < sizeof...(slave), void>::type
            update_all_loggers();
        
        /**\brief Функция обновления логгера I-го исполнителя*/
        template<std::size_t I> 
            void update_logger();
    protected:
        
    public:
        /**\brief Конструктор по умолчанию удален, так как в любом случае 
        * необходимо указать тэг 'Master'-класса.
        * */
        master()                      = delete;
        /**\brief Конструктор копирования удален, так как неясно,
        * куда и зачем копировать экземпляр класса daemon, 
        * который запускается в единственном экземпляре.
        * */
        master(const master &)        = delete;
        /**\brief Конструктор перемещения удален, так как неясно,
        * куда и зачем перемещать экземпляр класса daemon, 
        * который запускается в единственном экземпляре.
        * */
        master(master &&)             = delete;
        /**\brief Оператор присваивания удален, так как 
        * удалены конструкторы копирования и присваивания.
        * */
        master& operator=( master& )  = delete;
        
        /**\brief Единственный разрешенный конструктор
         * \param [in] t - тэг класса. 
         * */
        explicit master(const std::string & t);
        
        /**\brief Деструктор класса
         * 
         * ничего не делает
         * */
        ~master(){}
        
        /**\brief Основная рабочая функция класса. 
         * 
         * Запускает всех исполнителей, принимает все сообщения 
         * от системы и демона и уведомляет демона о статусе завершения 'Master'-процесса.
         * \return CHILD_NEED_TERMINATE при возникновении ошибки или приеме 
         * сообщения о прекращении работы
         * */
        int work_proc() throw();
};

}//robot

#include "master_impl.hpp"
#endif
