#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include "reply.hpp"

namespace slaves
{
struct request
{
    std::string data;
    uint32_t len;
};
/// The common handler for all incoming requests.
class request_handler
  : private boost::noncopyable
{
public:

    /// Handle a request and produce a reply.
    void handle_request(const request& req, reply& rep);

private:
    
};
}

#endif
