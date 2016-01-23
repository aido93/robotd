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
#ifndef TCP_SERVER_HPP
#define TCP_SERVER_HPP

#include <stdint.h>
#include <libconfig.h++>
#include "service.hpp"
#include "definitions.hpp"
#include <boost/asio/ip/tcp.hpp>
#include "connection.hpp"
#include "request_handler.hpp"

namespace slaves
{
/**\brief Класс, предоставляющий функционал TCP-сервера.
 * 
 * Запускается асинхронно. 
 * 
 * \todo Добавить авторизацию и Kerberos
 * \todo Продлить сессию соединения
 * \todo Написать shutdown_service
 * */
class tcp_server : public service
{
private:

    /**\brief Тэг класса, необходимый для логгирования.*/
    static const std::string tag;
    
    /**\brief Используемый порт для сервера. 
     * Изначально был массив портов, но мне показалось странным - 
     * использовать для одного и того же сервера с одним и тем же содержимым
     * несколько портов.*/
    static uint16_t port;
    
    /**\brief Адрес сервера*/
    static std::string address;
    
    /**\brief Функция инициирует асинхронную операцию приема*/
    void start_accept();

    /**\brief Функция обрабатывает завершение асинхронной операции приема*/
    void handle_accept(const boost::system::error_code& e);

    /**\brief Функция обрабатывает операцию остановки сервера*/
    void handle_stop();
    
    /**\brief acceptor_ - это массив объектов, принимающих входящие соединения
     * по указанным портам. Одному порту соответствует один элемент этого массива
     * */
    boost::asio::ip::tcp::acceptor acceptor_;

    /**\brief Следующее соединение, которое будет принято*/
    connection_ptr new_connection_;
    
    /**\brief Обработчик запросов*/
    request_handler request_handler_;
    
    /**\brief Функция, вызываемая при завершении работы сервера
     * необходима для корректной очистки памяти
     * 
     * */
    virtual void shutdown_service() final;
protected:
    
    /**\brief Функция, запускающая операцию приема*/
    virtual void run() final;
public:

    /**\brief Конструктор класса
     * \param l - ссылка на уровень логгирования, принятый в программе
     * \param s - умный указатель на логгирующий объект
     * \param io_serv - ссылка на основной объект обработки всех входящих сообщений
     * */
    tcp_server(const spdlog::level::level_enum & l,
               std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> &s,
               boost::asio::io_service & io_serv);
               
    /**\brief Деструктор
     * */
    ~tcp_server();
    
    /**\brief Функция загрузки параметров класса из файла конфигурации.
     * 
     * \param [in] settings - ссылка на объект настроек, прочитанных 
     *                        из файла конфигурации
     * \param [in] l - ссылка на объект логгирования
     * \param [in] to_file - переменная, которая указывает, 
     * куда выводить информацию о возможных ошибках
     * \return  
     * \htmlonly
     * <ul>
     * <li>-1 - при ошибке в случае первичного запуска и 
     * логгирования ошибок на экран</li>
     * <li>CHILD_NEED_WORK - при ошибке в случае уже запущенного демона 
     * и логгирования в файл</li>
     * <li>0 - если ошибок не было</li>
     * </ul>
     * \endhtmlonly
     * */
    static int load_config( const libconfig::Setting& settings,
                            const std::shared_ptr<::spdlog::logger>& l,
                            bool to_file);
};
}//robot
#endif
