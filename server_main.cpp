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
* \b robotd - cross-platform UNIX-daemon, that can be used as a base 
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
* 1) Last version of the robotd software can be downloaded using git:
* 
*   <em>$ git clone https://github.com/aido93/robotd</em>
* 
* 2) Change directory to ./robotd:
* 
*   <em>$ cd ./robotd</em>
* 
* 3) Configure robotd:
* 
*   <em>$ cmake .</em>
* 
* 4) Build the project:
* 
*   <em>$ make </em>
* 
* \section run Run
* 
* By default, robotd use directories defined by the FHS:
* for log-files (/var/log/) and for temporary files, required for correct 
* work (/var/run). Thereat, robotd needs to be run as root. 
* If you want, you can modify the robotd code for using other directories
* and don't run it as root.
* How to do that is showed in the \ref struct.
* Below, it assumed that you prefer to run program as root.
* 
* To run robotd type this command in terminal:
* 
* <em>$ sudo ./robotd load cfg/daemon.cfg</em>
* 
* This command tells to the program to load standard configuration file, 
* that placed in the ./cfg/ directory.
* 
* To stop robotd:
* 
* <em>$ sudo ./robotd stop</em>
* 
* If you want to see other options, you can type:
* 
* <em>$ ./robotd help</em>
* 
* \htmlonly &ltoption&gt
* robotd 1.0 - cross-platform UNIX daemonized server. <br>
* License GNU GPLv2 &lthttp://www.gnu.org/licenses/old-licenses/gpl-2.0.html&gt<br>
* This program can be used for development your own daemon.<br>
* This is free software: you are free to change and redistribute it.<br>
* There is NO WARRANTY, to the extent permitted by law.<br>
* Written by Igor Diakonov. Compiled 23.01.2016.<br>
* <br><br>
* Usage: robotd <option><br><br>
* 
* Options:<br>
*	help                  show this message<br>
*	load [filename.cfg]   run daemon with configuration loaded from filename.cfg.<br>
*	                      Note: It can be only one copy of the daemon run in the system.<br>
*	reload                reload configuration file without stopping daemon.<br>
*	stop                  terminate daemon.<br>
*	restart               restart daemon with the same configuration.<br>
*	status                show status of the daemon. If daemon is run you will see its PID.
* \endhtmlonly
* 
* As we can see, 'load' option requires one parameter - the name of the configuration file.
* It supports as full filenames or relative filenames.
* 
* \author Diakonov Igor
* \date 23.01.2016
*/

/**\page struct robotd structure
 * Program is divided on the six logical parts: 
 * \htmlonly
 * <ul>
 * <li>Command line arguments parser. 
 * It is located in daemon/cli_parser.hpp, daemon/cli_parser.cpp</li>
 * 
 * <li>Configuration loader class robot::loader. 
 * It is located in loader/loader.hpp, loader/loader_impl.hpp</li>
 * 
 * <li>Daemon class robot::daemon. 
 * It is located in daemon/daemon.hpp, daemon/daemon_mpl.hpp</li>
 * 
 * <li>'Master'-process class robot::master. 
 * It is located in daemon/master.hpp, daemon/master_impl.hpp</li>
 * 
 * <li>Base class for all slaves robot::service. 
 * It is located in daemon/service.hpp</li>
 * 
 * <li>Asynchronous handlers for the incomming messages. 
 * It is located in slaves directory</li>
 * </ul>
 * \endhtmlonly
 * 
 * The order of class creation:
 * 
 * First occurs parsing of the command line arguments and, if parsing was successful,
 * then the robot::daemon class is created. This class reads configuration file and 
 * transfers control to the load_config functions of the 'slave' classes and 
 * creates robot::master class in the child process.
 * 
 * The robot::master class runs number of threads, specified in the configuration file
 * and asyncronously runs all handlers of incoming messages using the boost::asio::io_service.
 * 
 * The robot::service class is the extension over boost::asio::io_service::service
 * and allows 'slave' classes to log operations in the log-file specified in configuration.
 * 
 * The robot::master class and the robot::daemon class is inherited from the robot::loader class,
 * because loading and reloading configuration file in fact is the same operation.
 * 
 * <b>Information for developers and used agreements</b>
 * 
 * Because it is impossible to debug daemon using gdb debugger 
 * (parent process terminates earlier than child process),
 * I recommend to turn on full logging mode including all error messages from the OS.
 * For this it needs to add -DTRACE flag in the CMAKE_CXX_FLAGS option 
 * of the CMakeLists.txt file.
 * If you want the program terminates when any severe error occurs, 
 * just add the -DDEBUG flag in the CMAKE_CXX_FLAGS option.
 * It is worth noting, that messages with severe level 'trace' is logged 
 * only if the program is build with -DTRACE flag.
 * 
 * Each class must have its own \b tag - name of the class using for logging.
 * 
 * All slave classes must be inherited from slaves::service.
 * To log your message into the sink you shalluse this syntax:
 * 
 * \code
 * //For full logging
 * #ifdef TRACE
        log->trace("Acceptor {} is ready to async accept",num);
   #endif
 * 
 * //for debug messages.
 * log->debug("Debug message");
 * 
 * //for info messages.
 * log->info("Info message");
 * 
 * //for warning messages.
 * log->warn("Warning message");
 * 
 * //for error messages.
 * log->error("Error message");
 * \endcode
 * 
 * For more information, please visit https://github.com/gabime/spdlog/wiki .
 * 
 * For including your 'slave' classes in the load list just add them
 * in the template parameters as showed in the server_main.cpp file:
 * \code
 * robot::daemon<robot::tcp_server> robot_daemon(CONFIG_FILE);
 * \endcode
 * 
 *In the example above only the robot::tcp_server class will be daemonized.
 * 
 * <b>Agreement 1</b>:
 * 
 * Each slave must have static load_config member in the public field:
 * 
 * \code
 * public:
 * static int load_config( const libconfig::Setting& settings,
                            const std::shared_ptr<::spdlog::logger>& l,
                            bool to_file);
 * \endcode
 * 
 * In order to write your own load_config class member,read the documentation on the
 * robot::tcp_server::load_config class member and look at its realization 
 * in the nets/tcp_server.cpp file.
 * For more information on syntax of the configuration file, 
 * please visit http://www.hyperrealm.com/libconfig/libconfig_manual.html#The-C_002b_002b-API
 * 
 * For most tasks, that your 'slave' class solves, event-driven model is perfectly suitable, 
 * that is realized in the boost::asio library. 
 * For more information on this library, please visit
 * http://www.boost.org/doc/libs/1_59_0/doc/html/boost_asio.html . 
 * Also you can see, how it is realized in the robot::tcp_server class.
 * 
 * <b>Agreement 2</b>:
 * 
 * Code must conform the SEI CERT standard.
 * For more information on this standard, please visit: https://securecoding.cert.org/confluence/pages/viewpage.action?pageId=637
 * 
 * */
 
/**
 * \file
 * \brief Main file of the robotd daemon.
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
    // if something was wrong, then exit
    if(ret<0)
    {
        return EXIT_FAILURE;
    }
    if(ret>0)
    {
        return EXIT_SUCCESS;
    }

    robot::daemon<slaves::tcp_server> robot_daemon(CONFIG_FILE);
    robot_daemon.monitor_proc();
}
