//modify from libtnet
#pragma once

#include <string>
#include "noncopyable.h"

extern "C"
{
#include "http_parser.h"    
}


namespace HTTP
{

    class HttpError
    {
	    public:
		    HttpError(int code = 200, const std::string& m = std::string())
			    : statusCode(code)
			      , message(m){}

		    //200 for no error
		    int statusCode;
		    std::string message;
    };


    class HttpParser : public noncopyable
    {
    public:
        friend class HttpParserSettings;

        HttpParser(enum http_parser_type type);
        virtual ~HttpParser();

        enum http_parser_type getType() { return (http_parser_type)m_parser.type; }

        enum Event
        {
            Parser_MessageBegin,    
            Parser_Url,
            Parser_StatusComplete,
            Parser_HeaderField,
            Parser_HeaderValue,
            Parser_HeadersComplete,
            Parser_Body,
            Parser_MessageComplete,
        };

        int execute(const char* buf, size_t count);

	const char *getMethod() { return http_method_str((enum http_method)(m_parser.method)); }
	unsigned short getMajor() { return m_parser.http_major; }
	unsigned short getMinor() { return m_parser.http_minor; }
	unsigned int getStatusCode() { return m_parser.status_code; }

    protected:
        virtual int onMessageBegin() { return 0; }
        virtual int onUrl(const char*, size_t) { return 0; }
        virtual int onHeader(const std::string& field, const std::string& value) { return 0; }
        virtual int onHeadersComplete() { return 0; }
        virtual int onBody(const char*, size_t) { return 0; }
        virtual int onMessageComplete() { return 0; }
        virtual int onUpgrade(const char*, size_t) { return 0; }
        virtual int onError(const HttpError& error) { return 0; }

    private:
        int onParser(Event, const char*, size_t);
    
        int handleMessageBegin();
        int handleHeaderField(const char*, size_t);
        int handleHeaderValue(const char*, size_t);
        int handleHeadersComplete();

    protected:
        struct http_parser m_parser;

        std::string m_curField;
        std::string m_curValue;
        bool m_lastWasValue; 
        
        int m_errorCode;    
    };   
}
