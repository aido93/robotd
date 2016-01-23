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
 * \brief Parsing command line arguments
 * 
 * \return In the case of success 0 is returned.
 * In the case of error errno is returned. 
 * In the case of no error, but it doesn't need to continue process 
 * positive value is returned.
 * 
 * Examples of positive value result:
 * \htmlonly
 * <ul>
 * <li>WRONG_OPTIONS</li>
 * <li>HELP_OPTION</li>
 * <li>DONT_DO_ANYTHING</li>
 * </ul>
 * \endhtmlonly
 * */
int parse_options(int argc, char**argv);

#endif
