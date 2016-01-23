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
#include "config.h"
#include <string>

/**
 * \file
 * \brief Файл с указанием основных рабочих директорий программы
 * 
 * \todo FHS соответствует?
 * */
 
/**
 * \brief Директория, в которой будет находится PID-файл демон-процесса.
 * */
std::string RUN_DIR="/var/run";

/**
 * \brief Директория, где находится файл конфигурации демона.
 * */
std::string ETC_CONF_DIR="/etc";

/**
 * \brief Директория, в которой будут находится настройки программы, 
 * используемые в течении текущей сессии.
 * 
 * */
std::string RUN_CONF_DIR=RUN_DIR+"/"+PACKAGE_NAME;

/**
 * \brief Полный путь к файлу, в котором сохранен полный путь 
 * до загруженного файла конфигурации.
 * */
std::string CONFIG_LOAD=RUN_DIR+"/"+PACKAGE_NAME+"/"+"configuration";

/**
 * \brief Полный путь к PID-файлу.
 * */
std::string PID_FILE=RUN_DIR+"/"+PACKAGE_NAME+".pid";

/**
 * \brief Полный путь к файлу конфигурации по умолчанию.
 * */
std::string CONFIG_FILE=ETC_CONF_DIR+"/"+PACKAGE_NAME+"/"+"General.conf";


