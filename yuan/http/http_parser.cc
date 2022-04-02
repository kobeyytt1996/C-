#include "http_parser.h"

namespace yuan {
namespace http {

/**
 * request解析器里各种回调函数的定义
 */
void on_request_method(void *data, const char *at, size_t length) {

}

void on_request_uri(void *data, const char *at, size_t length) {

}

void on_request_fragment(void *data, const char *at, size_t length) {

}

void on_request_path(void *data, const char *at, size_t length) {

}

void on_request_query(void *data, const char *at, size_t length) {

}

void on_request_version(void *data, const char *at, size_t length) {

}

void on_request_header_done(void *data, const char *at, size_t length) {

}

void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {

}

/**
 * HttpRequestParser方法的实现
 */
HttpRequestParser::HttpRequestParser()
    : m_data(std::make_shared<HttpRequest>())
    , m_error(0) {
    http_parser_init(&m_parser);
    // 以下是设置parser中的回调函数
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query;
    m_parser.http_version = on_request_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
}

size_t HttpRequestParser::execute(const char *data, size_t len, size_t off) {
    return 0;
}

int HttpRequestParser::isFinished() const {
    return 0;
}

int HttpRequestParser::hasError() const {
    return 0;
}

/**
 * request解析器里各种回调函数的定义
 */
void on_response_reason(void *data, const char *at, size_t length) {

}

void on_response_status(void *data, const char *at, size_t length) {

}

void on_response_chunk(void *data, const char *at, size_t length) {

}

void on_response_version(void *data, const char *at, size_t length) {

}

void on_response_header_done(void *data, const char *at, size_t length) {

}

void on_response_last_chunk(void *data, const char *at, size_t length) {

}

void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {

}

HttpResponseParser::HttpResponseParser()
    : m_data(std::make_shared<HttpResponse>())
    , m_error(0) {
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason;
    m_parser.status_code = on_response_status;
    m_parser.chunk_size = on_response_chunk;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
}

size_t HttpResponseParser::execute(const char *data, size_t len, size_t off) {
    return 0;
}

int HttpResponseParser::isFinished() const {
    return 0;
}

int HttpResponseParser::hasError() const {
    return 0;
}

}
}