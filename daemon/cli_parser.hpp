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
#ifndef CLI_PARSER_HPP
#define CLI_PARSER_HPP

/**
 * \brief Функция парсинга аргументов командной строки
 * 
 * Функция работает в несколько этапов:
 * \htmlonly
 * <ol>
 * <li> проверяет количество параметров</li>
 * <li> проверяет наличие временной директории конфигов</li>
 * <li> проверяет наличие PID-файла</li>
 * <li> запускает работу в соответствии с опциями</li>
 * </ol>
 * \endhtmlonly
 * 
 * \param [in] argc - Количество аргументов командной строки
 * \param [in] argv - указатель на массив аргументов командной строки
 * \return В случае успешного парсинга возвращает 0, 
 * иначе - errno в случае ошибки или положительное значение, которое 
 * свидетельствует, что ошибки не было, но продолжать работу не нужно.
 * 
 * Примеры:
 * \htmlonly
 * <ul>
 * <li>WRONG_OPTIONS - возвращается, если опции указаны некорректно</li>
 * <li>HELP_OPTION - возвращается, если была передана опция help</li>
 * <li>DONT_DO_ANYTHING - возвращается, если невозможно выполнить 
 * запрошенную операцию, например, остановить демона, который не запущен</li>
 * </ul>
 * \endhtmlonly
 * */
int parse_options(int argc, char**argv);

#endif
