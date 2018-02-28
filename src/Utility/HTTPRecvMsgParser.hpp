#ifndef HTTPRecvMsgParser_hpp
#define HTTPRecvMsgParser_hpp

#include <deque>
#include "../HTTP/HTTPResponse.hpp"
#include "Util.hpp"

class HTTPRecvMsgParser
{
public:
  HTTPRecvMsgParser();
  ~HTTPRecvMsgParser();

  bool is_parse_msg();
  HTTPResponse * msg2res();

private:
  bool parse_line();
  bool parse_header();
  bool parse_body();

public:
  std::deque<Util::byte> *cache;

  std::string version;
  std::string status_code;
  std::string reason;

  std::map<std::string, std::string> header;

  std::string method;

private:
  HTTPMessageParseState state;
  long content_length;
  bool isChunk;
};

#endif
