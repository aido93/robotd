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
#include "tcp_server.hpp"
#include <boost/make_shared.hpp>
#include <iostream>
#include <memory>
#include <strings.h>

const std::string slaves::tcp_server::tag="Server(Slave)";
uint16_t slaves::tcp_server::port=3333;
std::string slaves::tcp_server::address="127.0.0.1";

int slaves::tcp_server::load_config( const libconfig::Setting& settings,
                                    const std::shared_ptr<::spdlog::logger>& l,
                                    bool to_file)
{
    const libconfig::Setting& robot_server = settings["TCPServer"];
    robot_server.lookupValue("Address", address);
    uint32_t tmp=port;
    robot_server.lookupValue("Port",tmp);
        if(tmp==0)
        {
            if(to_file)
            {
                l->warn("<{}> port number must not be zero.",tag);
                return CHILD_NEED_WORK;
            }
            else
            {
                std::cerr<<tag<<" port number must not be zero."<<std::endl;
                return -1;
            }
        }
        else if(tmp>65535)
        {
            if(to_file)
            {
                l->warn("<{}> {} port number must be less than 65535.",tag);
                return CHILD_NEED_WORK;
            }
            else
            {
                std::cerr<<tag<<" port number must be less than 65535."<<std::endl;
                return -1;
            }
        }
        port=tmp;
        if(to_file)
        {
            l->info("<{}> TCP Server address: {}",tag, address);
            l->info("<{}> TCP Server port: {}",tag, port);
        }
        else
        {
            std::cout<<tag<<" TCP Server address: "<<address<<std::endl;
            std::cout<<tag<<" TCP Server port: "<<port<<std::endl;
        }
    return 0;
}

slaves::tcp_server::tcp_server(
            const spdlog::level::level_enum & l,
            std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> &s,
            boost::asio::io_service & io_serv): 
        slaves::service(tag, l,s,io_serv),
        acceptor_(io_service_),
        new_connection_(new connection(io_service_, request_handler_,log)),
        request_handler_()
{
    log->debug("Started");
    // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::system::error_code ec;
    #ifdef TRACE
        log->trace("{} adding...", port);
    #endif
    boost::asio::ip::tcp::resolver::query query(address, std::to_string(port));
    #ifdef TRACE
        log->trace("Query connected to {}", port);
    #endif
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    #ifdef TRACE
        log->trace("Endpoint created");
    #endif
    acceptor_.open(endpoint.protocol(),ec);
    if(ec)
    {
        log->error("Can not open endpoint! {}", ec.value());
        // завершим процесс с кодом требующим перезапуска
        #ifdef DEBUG
            exit(CHILD_NEED_TERMINATE);
        #else
            //в надежде, что была случайность...
            exit(CHILD_NEED_WORK);
        #endif
    }
    #ifdef TRACE
        log->trace("Endpoint opened");
    #endif
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    #ifdef TRACE
        log->trace("Option REUSE_ADDRESS set");
    #endif
    acceptor_.bind(endpoint,ec);
    if(ec)
    {
        log->error("Can not bind to endpoint! {}", ec.value());
        // завершим процесс с кодом требующим перезапуска
        #ifdef DEBUG
            exit(CHILD_NEED_TERMINATE);
        #else
            //в надежде, что была случайность...
            exit(CHILD_NEED_WORK);
        #endif
    }
    #ifdef TRACE
        log->trace("Acceptor is binded to endpoint");
    #endif
    log->debug("{} added",port);
    acceptor_.listen();
    log->debug("Acceptor is listening");
}

void slaves::tcp_server::run()
{
    start_accept();
}

slaves::tcp_server::~tcp_server()
{
    log->debug("Stopped");
}

void slaves::tcp_server::shutdown_service()
{
    log->debug("Server is off.");
    //обязательно дописать
}

void slaves::tcp_server::start_accept()
{
    new_connection_.reset(new connection(io_service_, request_handler_,log));
    acceptor_.async_accept(new_connection_->socket(),
        boost::bind(&::slaves::tcp_server::handle_accept, this,
        boost::asio::placeholders::error));
    #ifdef TRACE
        log->trace("Acceptor is ready to async accept");
    #endif
}

void slaves::tcp_server::handle_accept(const boost::system::error_code& e)
{
    if (!e)
    {
        #ifdef TRACE
            log->trace("New connection is about to start");
        #endif
        new_connection_->start();
    }

    start_accept();
}

void slaves::tcp_server::handle_stop()
{
    io_service_.stop();
    #ifdef TRACE
        log->trace("io_service stopped");
    #endif
}
