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
#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "system_messages.hpp"
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio/io_service.hpp>
#include "spdlog/spdlog.h"
#include "spdlog/tweakme.h"

namespace slaves
{
/**\brief Абстрактный класс, расширяющий возможности boost::asio::io_service::service
 * 
 * Добавлена поддержка логгирования.
 * */
class service : public boost::asio::io_service::service
{
    private:
    
        /**\brief Ссылка на тэг дочернего класса, необходимый для логгирования*/
        const std::string &tag;
        
        /**\brief Уровень логгирования*/
        spdlog::level::level_enum lev;
        
        /**\brief Слив в лог-файл*/
        std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> &sink;
    protected:
    
        /**\brief Объект, осуществляющий логгирование в слив*/
        std::shared_ptr<spdlog::logger> log;
        
        /**\brief Ссылка на основной объект, предназначенный для получения 
         * сетевых пакетов и их обработки*/
        boost::asio::io_service & io_service_;
    public:
    
        /**\brief Конструктор интерфейса
         * \param [in] t - ссылка на тэг класса
         * \param [in] l - ссылка на уровень логгирования
         * \param [in] s - ссылка на умный указатель на объект, связанный с лог-файлом
         * \param [in] io_serv - ссылка на основной объект обработки всех входящих сообщений*/
        service(const std::string & t,
                const spdlog::level::level_enum & l,
                std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> &s,
                boost::asio::io_service & io_serv):
            boost::asio::io_service::service(io_serv),
            tag(t),
            lev(l),
            sink(s),
            log(std::make_shared<spdlog::logger>(tag+"_logger",sink)),
            io_service_(io_serv)
            {
                log->set_pattern(std::string("[%T] <%L> (")+tag+") %v");
                log->set_level(lev);
            }
            
        /**\brief Деструктор интерфейса*/
        virtual ~service(){};
        
        /**\brief Функция запуска работы сервиса*/
        virtual void run() =0;
        
        /**\brief Функция для выставления уровня логгирования
         * \param [in] l - ссылка на уровень логгирования, принятый в программе*/
        void set_level(const spdlog::level::level_enum & l)
        {
            lev=l;
            log->set_level(lev);
        }
};
}
#endif
