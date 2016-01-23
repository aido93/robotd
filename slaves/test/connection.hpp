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

#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "reply.hpp"
#include "request_handler.hpp"
#include "spdlog/tweakme.h"
#include "spdlog/spdlog.h"

namespace slaves {

/**\brief Класс представляющий единственное соединение с клиентом
 * 
 * \todo Написать документацию к сетевой части
 * \todo Разобраться, что делать, если сообщение больше буфера
 * \todo Написать парсер входных данных
 * \todo Сделать закрытие соединений более редкими или опционально
 * */
class connection
  : public boost::enable_shared_from_this<connection>,
    private boost::noncopyable
{
    private:
        /**\brief Ссылка на логгер*/
        std::shared_ptr<::spdlog::logger> &log;
        
        /**\brief Обработчик завершения операции чтения*/
        void handle_read(const boost::system::error_code& e,
            std::size_t bytes_transferred);

        /**\brief Обработчик завершения операции записи*/
        void handle_write(const boost::system::error_code& e);

        /**\brief Strand для того, чтобы обработчики чтения и записи 
         * не испытывали конкуренцию за ресурсы*/
        boost::asio::io_service::strand strand_;

        /**\brief Сокет соединения*/
        boost::asio::ip::tcp::socket socket_;

        /**\brief Буфер входных данных*/
        boost::array<char, 8192> buffer_;

        /**\brief Входящий запрос*/
        request request_;

        /**\brief Обработчик входящего запроса*/
        request_handler & request_handler_;

        /**\brief Ответ, который будет отослан клиенту*/
        reply reply_;
    
    public:
        connection()                          = delete;
        connection(const connection &)        = delete;
        connection(connection &&)             = delete;
        connection& operator=( connection& )  = delete;
        
        /**\brief Конструктор
         * \param [in] io_service - ссылка на основной объект обработки всех сообщений
         * \param [in] handler - ссылка на обработчик запроса
         * \param [in] l - ссылка на логгер
         * */
        connection( boost::asio::io_service& io_service,
                    request_handler& handler, 
                    std::shared_ptr<::spdlog::logger>& l);

        /**\brief Функция, возвращающая сокет, ассоциированный с соединением*/
        boost::asio::ip::tcp::socket& socket();

        /**\brief Запуск первой асинхронной операции соединения*/
        void start();
};

typedef boost::shared_ptr<connection> connection_ptr;

}//robot

#endif //CONNECTION_HPP
