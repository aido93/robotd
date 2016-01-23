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

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

slaves::connection::connection(  boost::asio::io_service& io_service,
                                request_handler& handler, 
                                std::shared_ptr<::spdlog::logger>&l)
  : log(l),
    strand_(io_service),
    socket_(io_service),
    request_handler_(handler)
{
    #ifdef TRACE
        log->trace("New connection created");
    #endif
}

boost::asio::ip::tcp::socket& slaves::connection::socket()
{
  return socket_;
}

void slaves::connection::start()
{
    #ifdef TRACE
        log->trace("New connection started");
    #endif
    socket_.async_read_some(boost::asio::buffer(buffer_),
        strand_.wrap(
        boost::bind(&connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));
}

void slaves::connection::handle_read(const boost::system::error_code& e,
                            std::size_t bytes_transferred)
{
    if (!e)
    {
        boost::tribool result;
        boost::tie(result, boost::tuples::ignore)/* = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred)*/;
        #ifdef TRACE
            log->trace("Message in buffer: {}, {}",buffer_.data(), bytes_transferred);
        #endif
        if (result)
        {
            #ifdef TRACE
                log->trace("Client message parsed succesfully");
            #endif
            request_handler_.handle_request(request_, reply_);
            boost::asio::async_write(socket_, reply_.to_buffers(),
              strand_.wrap(
                boost::bind(&connection::handle_write, shared_from_this(),
                  boost::asio::placeholders::error)));
        }
        else if (!result)
        {
            #ifdef TRACE
                log->trace("Client message isn't parsed succesfully");
            #endif
            reply_ = reply::stock_reply(reply::bad_request);
            boost::asio::async_write(socket_, reply_.to_buffers(),
              strand_.wrap(
                boost::bind(&connection::handle_write, shared_from_this(),
                  boost::asio::placeholders::error)));
        }
        else
        {
            #ifdef TRACE
                log->trace("Client message is undefined");
            #endif
            socket_.async_read_some(boost::asio::buffer(buffer_),
              strand_.wrap(
                boost::bind(&connection::handle_read, shared_from_this(),
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred)));
        }
    }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

void slaves::connection::handle_write(const boost::system::error_code& e)
{
    if (!e)
    {
        // Initiate graceful connection closure.
        boost::system::error_code ignored_ec;
        #ifdef TRACE
            log->trace("Write handler is invoked");
        #endif
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}
