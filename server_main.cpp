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
/**
* \mainpage General
* 
* \b robot_daemon - cross-platform UNIX-daemon, that can be used as a base 
* for developing other daemons. Written on C++11.
* 
* \section license License
* \htmlonly 
* Distributed under GNU GPL v2 license, 
* contains code, distributed under <a href=https://github.com/gabime/spdlog/blob/master/LICENSE>MIT license</a> (spdlog)
* and under <a href=http://www.boost.org/LICENSE_1_0.txt>Boost Software License - Version 1.0</a>
* \endhtmlonly 
* 
* \section arch Supported CPU architectures
* \htmlonly 
* <ul>
* <li>amd64</li>
* <li>x86 - unknown</li>
* <li>arm - unknown</li>
* </ul>
* \endhtmlonly 
* 
* \section deps Dependencies
* \htmlonly 
* <ul>
* <li>git         >= 2.1.4</li>
* <li>libconfig++ >= 9</li>
* <li>libboost    >= 1.59</li>
* <li>cmake       >= 3.0.2</li>
* <li>g++         >= 4.8.2 (support C++11 standard)</li>
* </ul>
* \endhtmlonly 
* Also it may be compiled with clang++ >= 3.6.0.
* 
* 
* These programs need to be installed to generate documentation:
* 
* \htmlonly 
* <ul>
* <li>doxygen     >= 1.8.9.1 </li>
* <li>mscgen      >= 0.20 </li>
* <li>graphviz    >= 2.38.0 </li>
* </ul>
* \endhtmlonly 
* 
* To generate documentation in the pdf-format:
* 
* \htmlonly
* <ul>
* <li>pdflatex    >= 3.14159265-2.6-1.40.15</li>
* </ul>
* \endhtmlonly 
* 
* Older versions of the programs and libraries is not tested.
* 
* \section make Compile and Install 
* 
* 1) Самая новая версия программы может быть загружена при помощи команды:
* 
*   <em>$ git clone https://github.com/aido93/robotd</em>
* 
* 2) Перейдите в директорию ./server:
* 
*   <em>$ cd ./server</em>
* 
* 3) Сконфигурируйте проект:
* 
*   <em>$ cmake .</em>
* 
* 4) Соберите проект:
* 
*   <em>$ make </em>
* 
* \section run Запуск программы
* 
* По умолчанию, программа использует директории, определенные стандартом FHS
* для сохранения логов (/var/log/) и создания временных файлов, необходимых
* для корректной работы программы (/var/run). По этой причине программе 
* требуются права суперпользователя. При желании, Вы можете скомпилировать
* программу так, чтобы она не требовала доступ к вышеперечисленным директориям.
* Как это сделать - будет рассказано в главе \ref struct.
* 
* Для запуска программы наберите в терминале:
* 
* <em>$ sudo ./robot_daemon load cfg/daemon.cfg</em>
* 
* Будет загружен стандартный файл конфигурации, который находится в директории ./cfg/
* 
* Для остановки программы:
* 
* <em>$ sudo ./robot_daemon stop</em>
* 
* Запускать программу напрямую от суперпользователя или через sudo - 
* личный выбор каждого. Более интересны на данный момент поддерживаемые 
* опции запуска. Их можно вывести при помощи команды help:
* 
* 
* <em>$ sudo ./robot_daemon help</em>
* \htmlonly
* robot_daemon 1.0 <br>
* Usage: robot_daemon  &ltoption&gt <br><br>
* Options: <br>
* load [filename.cfg] <br>
* reload <br>
* restart <br>
* stop <br>
* status <br>
* help
* \endhtmlonly
* 
* Как видно, только опция 'load' имеет параметр - имя файла конфигурации.
* Поддерживаются как полные имена, так и относительные.
* 
* Остальные опции служат:
* \htmlonly
* <ul>
* <li>reload  - для перезагрузки файла конфигурации без остановки работы демона.</li>
* <li>restart - для перезапуска демона и перезагрузки файла конфигурации.</li>
* <li>stop    - для завершения работы демона.</li>
* <li>status  - для получения статуса (работает/не работает) демона. В случае,
* если демон запущен, команда выведет на экран PID процесса демона.</li>
* 
* <li>help    - для вывода справки.</li>
* </ul>
* \endhtmlonly
* 
* \author Дьяконов Игорь
* \date 20.01.2016
*/

/**\page struct Структура программы
 * Программа разделена на несколько логических частей
 * \htmlonly
 * <ul>
 * <li>Парсер аргументов командной строки. 
 * Расположен в файлах daemon/cli_parser.hpp, daemon/cli_parser.cpp</li>
 * 
 * <li>Класс загрузчика конфигурации robot::loader. 
 * Распложен в loader/loader.hpp, loader/loader_impl.hpp</li>
 * 
 * <li>Класс демона robot::daemon. 
 * Расположен в файлах daemon/daemon.hpp, daemon/daemon_mpl.hpp</li>
 * 
 * <li>Класс мастер-процесса robot::master. 
 * Расположен в файлах daemon/master.hpp, daemon/master_impl.hpp</li>
 * 
 * <li>Абстрактный класс robot::service. 
 * Расположен в файле daemon/service.hpp</li>
 * 
 * <li>Запускаемые асинхронно обработчики входящих сообщений. 
 * Пока поддерживается только сетевой протокол TCP и 
 * соответствующие файлы находятся в директории nets</li>
 * </ul>
 * \endhtmlonly
 * 
 * Порядок создания классов следующий:
 * Сначала происходит парсинг аргументов командной строки и если все прошло успешно,
 * то создается класс robot::daemon, который занимается чтением файла конфигурации,
 * передает управление в функции load_config классов обработчиков входных сообщений
 * и создает в дочернем процессе класс master.
 * 
 * Класс robot::master создает указанное в файле конфигурации количество потоков,
 * запускает асинхронно при помощи boost::asio::io_service все обработчики 
 * входящих сообщений, именуемые в дальнейшем слэйвами (от англ. slave).
 * 
 * Класс robot::service является расширением над boost::asio::io_service::service
 * и позволяет слэйвам логгировать свои операции в указанный в файле конфигурации лог-файл.
 * 
 * Классы robot::master и robot::daemon являются наследниками класса robot::loader,
 * так как загрузка и перезагрузка файла конфигурации - по факту почти одна и та же задача.
 * 
 * <b>Информация для разработчиков и используемые соглашения</b>
 * 
 * Так как невозможно отлаживать демона в gdb (родительский процесс сразу завершается),
 * то выход остается один - включить полное логгирование действий, 
 * выполняемых программой. Для этого в файле CMakeLists.txt служит 
 * опция CMAKE_CXX_FLAGS. Для включения ПОЛНОГО логгирования добавьте в нее
 * опцию -DTRACE. Для того, чтобы программа завершалась при любой серьезной ошибке,
 * добавьте в CMAKE_CXX_FLAGS опцию -DDEBUG. Стоит отметить, что сообщения 
 * с уровнем логгирования trace будут выводиться только в том случае, если
 * программа скомпилирована с флагом -DTRACE.
 * 
 * Каждый класс должен иметь свой \b тэг - имя класса, используемое при логгировании.
 * 
 * Логгирование происходит в т.н. \b слив (от англ. sink).
 * 
 * В случае, если Вы хотите использовать логгирование в собственном slave-классе, 
 * отнаследуйтесь от robot::service и используйте следующий синтаксис:
 * 
 * \code
 * //Для подробного логгирования.
 * //Помните, функция log->trace должна использоваться ТОЛЬКО в таком виде.
 * #ifdef TRACE
        log->trace("Acceptor {} is ready to async accept",num);
   #endif
 * 
 * //для debug-сообщений.
 * log->debug("Debug message");
 * 
 * //для info-сообщений.
 * log->info("Info message");
 * 
 * //для warning-сообщений.
 * log->warn("Warning message");
 * 
 * //для error-сообщений.
 * log->error("Error message");
 * \endcode
 * 
 * Более подробную информацию вы найдете на сайте https://github.com/gabime/spdlog/wiki .
 * 
 * Для включения ваших слэйвов в список загружаемых, просто добавьте их в параметры
 * шаблона daemon через запятую, как показано в файле server_main.cpp:
 * \code
 * robot::daemon<robot::tcp_server> robot_daemon(CONFIG_FILE);
 * \endcode
 * 
 * В примере выше только класс robot::tcp_server требуется демонизировать.
 * 
 * <b>СОГЛАШЕНИЕ 1</b>:
 * 
 * Каждый слэйв должен иметь статическую функцию load_config в public-области:
 * 
 * \code
 * public:
 * static int load_config( const libconfig::Setting& settings,
                            const std::shared_ptr<::spdlog::logger>& l,
                            bool to_file);
 * \endcode
 * 
 * Для того, чтобы написать свою load_config-функцию, прочтите документацию на
 * robot::tcp_server::load_config и посмотрите ее реализацию в файле nets/tcp_server.cpp.
 * Более подробная информация по синтаксису файла конфигурации находится 
 * на сайте http://www.hyperrealm.com/libconfig/libconfig_manual.html#The-C_002b_002b-API
 * 
 * Для большинства задач, которые будет решать Ваш слэйв, прекрасно подходит
 * event-driven модель, которая реализована в библиотеке boost::asio. 
 * Подробную информацию об этой библиотеке вы найдете здесь:
 * http://www.boost.org/doc/libs/1_59_0/doc/html/boost_asio.html . 
 * Также Вы можете посмотреть, как реализована ее поддержка в классе robot::tcp_server.
 * 
 * <b>СОГЛАШЕНИЕ 2.1</b>:
 * 
 * Пишите имена всех классов, функций и переменных без использования заглавных букв.
 * Разрешается использовать подчеркивания, цифры используйте в самом крайнем случае.
 * 
 * <b>СОГЛАШЕНИЕ 2.2</b>:
 * 
 * Отступ слева должен быть равен 4 пробелам. Старайтесь писать код так, 
 * чтобы он умещался в строки, длиной 100 символов.
 * 
 * <b>СОГЛАШЕНИЕ 2.3</b>:
 * 
 * В файлах с объявлениями классов, функций, переменных, директив 
 * комментарии оформляются в doxygen-формате,
 * перед объявлением соответствующей сущности. Перед комментарием должна быть
 * одна пустая строка. В файлах с реализациями классов и функций используйте 
 * комментарии в не-doxygen-формате только там, где без них тяжело понять код.
 * Старайтесь писать понятный без комментариев код. Не используйте в именах
 * транслитерированные названия.
 * 
 * <b>СОГЛАШЕНИЕ 2.4</b>:
 * 
 * Код должен быть написан в соответствии со стандартами SEI CERT, POSIX.
 * Его описание Вы найдете здесь: https://securecoding.cert.org/confluence/pages/viewpage.action?pageId=637
 * 
 * */
 
/**
 * \file
 * \brief Основной файл сервера.
 * \details Содержит функцию main, в которой вызывается ParseOptions 
 * для распознания опций командной строки. Если опции были распознаны 
 * и все было успешно, то дальше создается демонизированный сервер.
 * */
#include <boost/asio.hpp>
#include "daemon.hpp"
#include "tcp_server.hpp"
#include "directories.h"
#include "cli_parser.hpp"
#include <unistd.h>


int main(int argc, char** argv)
{
    int ret=parse_options(argc, argv);
    // если что-то не так, то выходим.
    if(ret<0)
    {
        return EXIT_FAILURE;
    }
    if(ret>0)
    {
        return EXIT_SUCCESS;
    }
    // все так
    // каждый класс в параметрах шаблона должен содержать static load_config
    robot::daemon<slaves::tcp_server> robot_daemon(CONFIG_FILE);
    robot_daemon.monitor_proc();
}
