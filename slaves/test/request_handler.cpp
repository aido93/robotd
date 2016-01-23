#include "request_handler.hpp"
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>
namespace slaves
{
    
void request_handler::handle_request(const request& req, reply& rep)
{
  std::string full_path /*= doc_root_ + request_path*/;
  std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
  if (!is)
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }
  // Fill out the reply to be sent to the client.
  rep.status = reply::ok;
  char buf[512];
  while (is.read(buf, sizeof(buf)).gcount() > 0)
    rep.content.append(buf, is.gcount());
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = /*mime_types::extension_to_type(extension)*/1;
}
}
