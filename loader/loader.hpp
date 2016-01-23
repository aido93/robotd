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
#ifndef LOADER_HPP
#define LOADER_HPP

#include <libconfig.h++>
#include "spdlog/tweakme.h"
#include "spdlog/spdlog.h"

namespace robot
{
/**\brief Загрузчик конфигурации демона. От него наследуются robot::daemon и robot::master
 * */
class loader
{
    private:
        /**\brief Индикатор записи в лог-файл(true) или вывода на экран (false)*/
        static bool log_to_file;
        
    protected:
        /**\brief Тэг класса, необходим для логгирования*/
        const std::string                   tag;
        
        /**\brief Количество потоков в программе*/
        static uint16_t                     thread_count;
        
        /**\brief Уровень логгирования в программе*/
        static ::spdlog::level::level_enum  lev;
        
        /**\brief Слив в лог-файл*/
        static std::shared_ptr<::spdlog::sinks::rotating_file_sink_mt> sink;
        
        /**\brief Объект, занимающийся логгированием в файл*/
               std::shared_ptr<::spdlog::logger> log;
        
        /**\brief Максимальное количество памяти, выделяемое потомку*/
        static unsigned long ramsize;
        
        /**\brief Конструктор по умолчанию удален, так как в любом случае 
        * необходимо указать тэг для логгера.
        * */
        loader()                      = delete;
            
        /**\brief Конструктор копирования удален, так как неясно,
        * куда и зачем копировать экземпляр класса loader.
        * */
        loader(const loader &)        = delete;
            
        /**\brief Конструктор перемещения удален, так как неясно,
        * куда и зачем перемещать экземпляр класса loader.
        * */
        loader(loader &&)             = delete;
            
        /**\brief Оператор присваивания удален, так как 
        * удалены конструкторы копирования и присваивания.
        * */
        loader& operator=(loader &)   = delete;
        
        /**\brief Конструктор
         * 
         * Просто устанавливает тэг класса для логгирования при загрузке
         * \param [in] t - ссылка на тэг класса
         * */
        explicit loader(const std::string &t) :
               tag(t){};
               
        /**\brief Терминальное определение функции загрузки файла конфигурации
         * 
         * \param [in] settings - ссылка на открытый файл конфигурации 
         * в формате libconfig++
         * 
         * \param [in] T1 - slave-класс, который будет выполняться в потоке 
         * 'Master'-процесса
         * 
         * \return \htmlonly
         * <ul>
         * <li>-1 - при ошибке в случае первичного запуска и 
         * логгирования ошибок на экран</li>
         * <li>CHILD_NEED_WORK - при ошибке в случае уже запущенного демона 
         * и логгирования в файл</li>
         * <li>0 - если ошибок не было</li>
         * </ul>
         * \endhtmlonly
         * */
        template <typename T1>
        int load_slave_config ( const libconfig::Setting& settings);
        
        /**\brief Функция загрузки файла конфигурации
         * 
         * \param [in] settings - ссылка на открытый файл конфигурации 
         * в формате libconfig++
         * \param [in] T1, T2, Args - slave-классы, которые будут выполняться в потоке 
         * 'Master'-процесса
         * 
         * \return \htmlonly
         * <ul>
         * <li>-1 - при ошибке в случае первичного запуска и 
         * логгирования ошибок на экран</li>
         * <li>CHILD_NEED_WORK - при ошибке в случае уже запущенного демона 
         * и логгирования в файл</li>
         * <li>0 - если ошибок не было</li>
         * </ul>
         * \endhtmlonly
         * */
        template <typename T1, typename T2, typename... Args>
        int load_slave_config ( const libconfig::Setting& settings);
        
        /**\brief Функция распознавания размера лог-файла
         * 
         * \param [in] size_s - строка, содержащая максимальный размер 
         * лог-файла
         * \param [in] logsize - ссылка на переменную, в которую 
         * будет занесен распознанный размер
         * \return \htmlonly
         * <ul>
         * <li>-1 - при ошибке в случае первичного запуска и 
         * логгирования ошибок на экран</li>
         * <li>CHILD_NEED_WORK - при ошибке в случае уже запущенного демона 
         * и логгирования в файл</li>
         * <li>0 - если ошибок не было</li>
         * </ul>
         * \endhtmlonly
         * */
        int size_parse(const std::string & size_s, 
                       unsigned long & logsize);
        /**\brief Функция загрузки основной конфигурации демона и 
         * конфигурации slave-классов
         * 
         * \param [in] filename - имя файла конфигурации
         * \param [in] slave - список классов, конфигурацию которых нужно прочитать
         * \return \htmlonly
         * <ul>
         * <li>-1 - при ошибке в случае первичного запуска и 
         * логгирования ошибок на экран</li>
         * <li>CHILD_NEED_WORK - при ошибке в случае уже запущенного демона 
         * и логгирования в файл</li>
         * <li>0 - если ошибок не было</li>
         * </ul>
         * \endhtmlonly
         * */
        template<class... slave> 
        int daemon_load_config(const std::string & filename);
};

}

#include "loader_impl.hpp"
#endif
