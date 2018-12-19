/*
 * author: louluo
 * desc: http wrapper
 */

#pragma once
#include <bitset>
#include <algorithm>
#include <unordered_map>
#include "httpparser.h"

extern "C"
{
#include "http_parser.h"    
}

using namespace std;

namespace HTTP
{
//////////////////////////////////////////////////////////////////////////////////////
//                               common function                                    //
//            trim lower upper urldecode urlencode query2map map2query              //
//////////////////////////////////////////////////////////////////////////////////////
/// Url的字符类别                                                               
	enum                                                                            
	{                                                                               
		E_URL_CHR_RESERVED = 1,                                                     
		E_URL_UNSAFE = 2                                                            
	};   
/// Shorthands for the table                                                    
#define R   E_URL_CHR_RESERVED                                                  
#define U   E_URL_UNSAFE                                                        
#define RU  R|U 
	/// Characters defined by RFC 3986                                                  
	const unsigned char URL_CHR_TABLE[256] =                                        
	{                                                                              
		U,  U,  U,  U,   U,  U,  U,  U,   /* NUL SOH STX ETX  EOT ENQ ACK BEL */    
		U,  U,  U,  U,   U,  U,  U,  U,   /* BS  HT  LF  VT   FF  CR  SO  SI  */    
		U,  U,  U,  U,   U,  U,  U,  U,   /* DLE DC1 DC2 DC3  DC4 NAK SYN ETB */    
		U,  U,  U,  U,   U,  U,  U,  U,   /* CAN EM  SUB ESC  FS  GS  RS  US  */    
		U,  R,  U, RU,   R,  U,  R,  R,   /* SP  !   "   #    $   %   &   '   */    
		R,  R,  R,  R,   R,  0,  0,  R,   /* (   )   *   +    ,   -   .   /   */    
		0,  0,  0,  0,   0,  0,  0,  0,   /* 0   1   2   3    4   5   6   7   */    
		0,  0, RU,  R,   U,  R,  U,  R,   /* 8   9   :   ;    <   =   >   ?   */    
		RU, 0,  0,  0,   0,  0,  0,  0,   /* @   A   B   C    D   E   F   G   */    
		0,  0,  0,  0,   0,  0,  0,  0,   /* H   I   J   K    L   M   N   O   */    
		0,  0,  0,  0,   0,  0,  0,  0,   /* P   Q   R   S    T   U   V   W   */    
		0,  0,  0, RU,   U, RU,  U,  0,   /* X   Y   Z   [    \   ]   ^   _   */    
		U,  0,  0,  0,   0,  0,  0,  0,   /* `   a   b   c    d   e   f   g   */    
		0,  0,  0,  0,   0,  0,  0,  0,   /* h   i   j   k    l   m   n   o   */    
		0,  0,  0,  0,   0,  0,  0,  0,   /* p   q   r   s    t   u   v   w   */    
		0,  0,  0,  U,   U,  U,  0,  U,   /* x   y   z   {    |   }   ~   DEL */    

		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          

		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
		U, U, U, U,  U, U, U, U,  U, U, U, U,  U, U, U, U,                          
	};     
#undef RU                                                                       
#undef U                                                                        
#undef R 
   /// 对URL进行编码，默认不编码保留字和百分号字符 
	enum                                                                           
	{                                                                              
		E_ENCODE_RESERVED_CHAR = 0x01,          ///< 编码保留字                    
		E_NOT_ENCODE_EXTENDED_CHAR_SET = 0x02   ///< 不编码扩展字符集(ASCII值大于127的字符)
	};                                                                             
                                                                                  
   /// 对URL进行解码，默认不解码保留字和百分号字符                                
	enum                                                                           
	{                                                                              
		E_DECODE_RESERVED_CHAR = 0x10,          ///< 解码保留字                    
		E_DECODE_PERCENT_SIGN_CHAR = 0x20       ///< 解码百分号字符
        };      
#define URL_CHR_TEST(c, mask)   (URL_CHR_TABLE[(unsigned char)(c)] & (mask))    
#define URL_RESERVED_CHAR(c)    URL_CHR_TEST(c, E_URL_CHR_RESERVED)             
#define URL_UNSAFE_CHAR(c)      URL_CHR_TEST(c, E_URL_UNSAFE)  
#define XNUM_TO_DIGIT(x)        ("0123456789ABCDEF"[x] + 0)
#define XDIGIT_TO_NUM(h)        ((h) < 'A' ? (h) - '0' : toupper(h) - 'A' + 10) 
#define X2DIGITS_TO_NUM(h1, h2) ((XDIGIT_TO_NUM (h1) << 4) + XDIGIT_TO_NUM (h2))

	string urlEncode(const string & url, int flag=0)                       
	{                                                                                   
		std::string encoded_url;                                                        
		std::string::size_type url_len = url.length();                                  
		std::string::size_type pos;                                                     
		unsigned char ch;                                                               

		for (pos = 0; pos < url_len; pos++)                                             
		{                                                                               
			ch = url.at(pos);                                                           

			if (ch == '%')                                                              
			{                                                                           
				if (((pos + 2) < url_len)                                               
						&& isxdigit((unsigned char)url.at(pos + 1))                         
						&& isxdigit((unsigned char)url.at(pos + 2)))                        
				{                                                                       
					encoded_url += ch;                                                  
				}                                                                       
				else                                                                    
				{                                                                       
					encoded_url += '%';                                                 
					encoded_url += XNUM_TO_DIGIT(ch >> 4);                              
					encoded_url += XNUM_TO_DIGIT(ch & 0xf);                             
				}                                                                       
			}                                                                           
			else if (URL_UNSAFE_CHAR (ch))                                              
			{                                                                           
				if ((!(flag & E_ENCODE_RESERVED_CHAR) && URL_RESERVED_CHAR (ch))                                                                                                                           
						|| ((flag & E_NOT_ENCODE_EXTENDED_CHAR_SET) && (ch > 127)))         
				{                                                                       
					encoded_url += ch;                                                  
				}                                                                       
				else                                                                    
				{                                                                       
					encoded_url += '%';                                                 
					encoded_url += XNUM_TO_DIGIT(ch >> 4);                              
					encoded_url += XNUM_TO_DIGIT(ch & 0xf);                             
				}                                                                       
			}                                                                           
			else                                                                        
			{                                                                           
				if ((flag & E_ENCODE_RESERVED_CHAR) && URL_RESERVED_CHAR(ch))           
				{                                                                       
					encoded_url += '%';                                                 
					encoded_url += XNUM_TO_DIGIT(ch >> 4);                              
					encoded_url += XNUM_TO_DIGIT(ch & 0xf);                             
				}                                                                       
				else                                                                    
				{                                                                       
					encoded_url += ch;                                                  
				}                                                                       
			}                                                                           
		}                                                                               

		return encoded_url;                                                             
	} 
        string urlDecode(const string &url, int flag=0)                   
	{                                                                               
		unsigned char ch;                                                           
		std::string::size_type pos;                                                 
		std::string decoded_url;                                                    
		const char * url_string = url.c_str();                                      
		unsigned int url_len = url.length();                                        

		for (pos = 0; pos < url_len; pos++)                                         
		{                                                                           
			if ((url_string[pos] == '%') && (pos + 2 < url_len)                     
					&& isxdigit((unsigned char)(url_string[pos + 1]))                   
					&& isxdigit((unsigned char)(url_string[pos + 2])))                  
			{                                                                       
				ch = X2DIGITS_TO_NUM(url_string[pos + 1], url_string[pos + 2]);     

				if ((!(flag & E_DECODE_RESERVED_CHAR) && URL_RESERVED_CHAR(ch))     
						|| (!(flag & E_DECODE_PERCENT_SIGN_CHAR) && (ch == '%')))       
				{                                                                   
					decoded_url.push_back(url_string[pos]);                         
					decoded_url.push_back(url_string[pos + 1]);                     
					decoded_url.push_back(url_string[pos + 2]);                     
				}                                                                   
				else                                                                
				{                                                                   
					decoded_url.push_back(ch);                                      
				}                                                                   

				pos += 2;                                                           
			}                                                                       
			else                                                                    
			{                                                                       
				decoded_url.push_back(url_string[pos]);                             
			}                                                                       
		}                                                                           

		return decoded_url;                                                         
	}      

	string lower(const string &s)
	{
		string ls = s;
		transform(ls.begin(),ls.end(),ls.begin(),::tolower);
		return ls;
	}
	string upper(const string &s)
	{
		string us = s;
		transform(us.begin(),us.end(),us.begin(),::toupper);
		return us;
	}
	void rtrim(string &s, const string& delimiters=" \f\n\r\t\v")
	{
		s.erase(s.find_last_not_of(delimiters) + 1);
	}

	void ltrim(string &s, const string& delimiters=" \f\n\r\t\v")
	{
		s.erase(0, s.find_first_not_of(delimiters));
	}

	void trim(string &s, const string& delimiters=" \f\n\r\t\v")
	{
		s.erase(s.find_last_not_of(delimiters) + 1)
			.erase(0, s.erase(s.find_last_not_of(delimiters) + 1)
					.find_first_not_of(delimiters));
	}
	int query2map(const string& str, unordered_map<string, string>& result, const string& sep="&")
	{
		string::size_type pos1, pos2;
		pos2 = str.find(sep);
		pos1 = 0;
		while(string::npos != pos2)
		{
			string sub = str.substr(pos1, pos2 - pos1);
			string::size_type pos3 = sub.find("=");
			if(string::npos != pos3)
			{
				string key = sub.substr(0, pos3);
				string value = sub.substr(pos3 + 1);
				result[key] = value;
			}
			else
			{
				return -1;
			}
			pos1 = pos2 + sep.size();
			pos2 = str.find(sep, pos1);
		}
		if(pos1 != str.length())
		{
			string sub = str.substr(pos1);
			string::size_type pos3 = sub.find("=");
			if(string::npos != pos3)
			{
				string key = sub.substr(0, pos3);
				string value = sub.substr(pos3 + 1);
				result[key] = value;
			}
		}
		return 0;
	}
	string map2query(unordered_map<string, string>& querymap, const string& sep="&")
	{
		string query;
		unordered_map<string, string>::iterator it;
		for(it = querymap.begin(); it != querymap.end(); ++it)
		{
			if(querymap.begin() != it) query.append(sep);	
			query.append(it->first);	
			query.append("=");	
			query.append(it->second);	
		}
		return query;
	}

//////////////////////////////////////////////////////////////////////////////////////
//                               HttpReq                                            //
//                   public interface:  parse/build                                 //
//////////////////////////////////////////////////////////////////////////////////////
	class HttpReq : public HttpParser
	{
		public:
			HttpReq(bool ifcopy=false):HttpParser(HTTP_REQUEST),m_ifcopy(ifcopy)
			{
				reset();
			}
			virtual ~HttpReq(){}

			// parse
			int parse(const char* buffer, size_t count)					
			{
				m_header_map.clear();
				if(execute(buffer, count)) return -1;

				if(0 < m_body_size)
				{
				    if(NULL == m_pbody) return -1;
				    m_body_offset = m_pbody - buffer;
				}
				else
				{
				    m_body_offset = count;
				}

				m_method = getMethod();
				m_http_major = getMajor();
				m_http_minor = getMinor();
				
				m_path_offset += m_method.size()+1;
				m_querystring_offset += m_method.size()+1;

				return 0;
			}

			static int build(const string &method, const string &path, const string &query, const string &body,
					unordered_map<string, string> &header_map, 
					string &strreq, int major=1, int minor=1)
			{
				build_query_prefix(method, path, query, header_map, strreq, major, minor);

				char reqbuf[64];
				snprintf(reqbuf, 64, "Content-Length: %d\r\n\r\n", int(body.size()));
				strreq.append(reqbuf);
				strreq.append(body);

				return 0;
			}

			static int build(const string &method, const string &path, const string &body,
					unordered_map<string, string> &query_map, 
					unordered_map<string, string> &header_map, 
					string &strreq, int major=1, int minor=1)
			{
				build_querymap_prefix(method, path, query_map, header_map, strreq, major, minor);

				char reqbuf[64];
				snprintf(reqbuf, 64, "Content-Length: %d\r\n\r\n", int(body.size()));
				strreq.append(reqbuf);
				strreq.append(body);

				return 0;
			}

			// parse
			// get var interface
			bool ifcopy() { return m_ifcopy; }
			int body_offset() { return m_body_offset; }
			int body_size() { return m_body_size; }
			int path_offset() { return m_path_offset; }
			int path_size() { return m_path_size; }
			int querystring_offset() { return m_querystring_offset; }
			int querystring_size() { return m_querystring_size; }
			unsigned short httpmajor() { return m_http_major; }
			unsigned short httpminor() { return m_http_minor; }
			string &method() { return m_method; }
			string &path() { return m_path; }
			string &querystring() { return m_querystring;}
			string &body() { return m_body; }               
			unordered_map<string, string> &headerMap() { return m_header_map; }
			unordered_map<string, string> &queryMap() { return m_query_map; }
			// get var interface

		private:
			void reset()
			{
				m_body_offset = 0;
				m_body_size = 0;
				m_path_offset = 0;
				m_path_size = 0;
				m_querystring_offset = 0;
				m_querystring_size = 0;
 				m_pbody = NULL;
				////
				m_http_major = 0;
				m_http_minor = 0;
				m_method.clear();
				m_path.clear();
				m_querystring.clear();
				m_body.clear();                
				m_header_map.clear();
				m_query_map.clear();
			}
			int onUrl(const char* at, size_t length)
			{
				struct http_parser_url u;
				if(http_parser_parse_url(at, length, 0, &u)) return -1;
				if(u.field_set & (1 << UF_PATH))   // get path
				{
					m_path_offset = u.field_data[UF_PATH].off;
					m_path_size = u.field_data[UF_PATH].len;
					if(m_ifcopy)
					{
						m_path.clear();
						m_path.append(at + m_path_offset, m_path_size);
					}
				}

				if(u.field_set & (1 << UF_QUERY))  // get querystring
				{
					m_querystring_offset = u.field_data[UF_QUERY].off;
					m_querystring_size = u.field_data[UF_QUERY].len;
					if(m_ifcopy)
					{
						m_querystring.clear();
						m_querystring.append(at + m_querystring_offset, m_querystring_size);
						m_query_map.clear();
						if(query2map(m_querystring, m_query_map)) return -1;
					}
				}
				return 0;
			}
			int onHeader(const std::string& field, const std::string& value)
			{
				// get header
				m_header_map[field] = value;
				return 0;
			}
			int onBody(const char* at, size_t length)
			{
                                m_pbody = at;
				m_body_size = length;
				// get body
				if(m_ifcopy)
				{
					m_body.clear();
					m_body.append(at, length);
				}

				return 0;
			}

			static int build_query_prefix(const string &method, const string &path, const string &query,
					unordered_map<string, string> &header_map, 
					string &strreq, int major=1, int minor=1)
			{
				strreq=upper(method);
				strreq.append(" ");
				strreq.append(path);
				strreq.append("?");
				strreq.append(query);
				char reqbuf[16];
				snprintf(reqbuf, 16, " HTTP/%d.%d\r\n", major, minor);
				strreq.append(reqbuf);

				unordered_map<string, string>::iterator it;
				for(it=header_map.begin(); it != header_map.end(); ++it)
				{
					string key = it->first;
					string &value = it->second;
					trim(key);
					trim(value);

					string lowerkey = lower(key);
					if(!lowerkey.compare("content-length")) continue;

					strreq.append(key);
					strreq.append(": ");
					strreq.append(value);
					strreq.append("\r\n");
				}

				return 0;

			}
			static int build_querymap_prefix(const string &method, const string &path,
					unordered_map<string, string> &query_map, 
					unordered_map<string, string> &header_map, 
					string &strreq, int major=1, int minor=1)
			{
				strreq=upper(method);
				strreq.append(" ");
				strreq.append(path);
				strreq.append("?");
				strreq.append(map2query(query_map));
				char reqbuf[16];
				snprintf(reqbuf, 16, " HTTP/%d.%d\r\n", major, minor);
				strreq.append(reqbuf);

				unordered_map<string, string>::iterator it;
				for(it=header_map.begin(); it != header_map.end(); ++it)
				{
					string key = it->first;
					string &value = it->second;
					trim(key);
					trim(value);

					string lowerkey = lower(key);
					if(!lowerkey.compare("content-length")) continue;

					strreq.append(key);
					strreq.append(": ");
					strreq.append(value);
					strreq.append("\r\n");
				}

				return 0;

			}

#ifdef HTTP_DEBUG
		public:
			// for debug

			void print_ifcopy() { cout<<"ifcopy: "<<ifcopy()<<endl; }
			void print_body_offset() { cout<<"body_offset: "<<body_offset()<<endl; }
			void print_body_size() { cout<<"body_size: "<<body_size()<<endl; }
			void print_path_offset() { cout<<"path_offset: "<<path_offset()<<endl; }
			void print_path_size() { cout<<"path_size: "<<path_size()<<endl; }
			void print_querystring_offset() { cout<<"querystring_offset: "<<querystring_offset()<<endl; }
			void print_querystring_size() { cout<<"querystring_size: "<<querystring_size()<<endl; }

			void print_httpmajor(){cout<<"http major: "<<httpmajor()<<endl;}
			void print_httpminor(){cout<<"http minor: "<<httpminor()<<endl;}

			void print_method(){cout<<"method: "<<method()<<endl;}
			void print_path(){cout<<"path: "<<path()<<endl;}
			void print_querystring(){cout<<"querystring: "<<querystring()<<endl;}
			void print_body(){cout<<"body: "<<body()<<endl;}
			void print_header_map()
			{
				unordered_map<string, string>::iterator it = headerMap().begin();
				for(;it != headerMap().end(); ++it)
				{
					cout<<"header_map- kv: "<<it->first<<", "<<it->second<<endl;
				}
			}
			void print_query_map()
			{
				unordered_map<string, string>::iterator it = queryMap().begin();
				for(;it != queryMap().end(); ++it)
				{
					cout<<"query_map- kv: "<<it->first<<", "<<it->second<<endl;
				}
			}
			// for debug

#endif

		private:
			bool             m_ifcopy;
			int              m_body_offset;
			int              m_body_size;
			int              m_path_offset;
			int              m_path_size;
			int              m_querystring_offset;
			int              m_querystring_size;
			const  char  	*m_pbody;
                        /////////
			unsigned short   m_http_major;
			unsigned short   m_http_minor;
			string           m_method;
			string           m_path;
			string           m_querystring;
			string           m_body;                
			unordered_map<string, string>    m_header_map;
			unordered_map<string, string>    m_query_map;
	};
//////////////////////////////////////////////////////////////////////////////////////
//                               HttpRsp                                            //
//                   public interface:  parse/build                                 //
//////////////////////////////////////////////////////////////////////////////////////
	static const char *status_strings[] = \
	{
		"Continue",
			"Switching Protocols",
			"Processing",
			"OK",
			"Created",
			"Accepted",
			"Non-Authoritative Information",
			"No Content",
			"Reset Content",
			"Partial Content",
			"Multi-Status",
			"Already Reported",
			"IM Used",
			"Multiple Choices",
			"Moved Permanently",
			"Found",
			"See Other",
			"Not Modified",
			"Use Proxy",
			"Temporary Redirect",
			"Permanent Redirect",
			"Bad Request",
			"Unauthorized",
			"Payment Required",
			"Forbidden",
			"Not Found",
			"Method Not Allowed",
			"Not Acceptable",
			"Proxy Authentication Required",
			"Request Timeout",
			"Conflict",
			"Gone",
			"Length Required",
			"Precondition Failed",
			"Payload Too Large",
			"URI Too Long",
			"Unsupported Media Type",
			"Range Not Satisfiable",
			"Expectation Failed",
			"Misdirected Request",
			"Unprocessable Entity",
			"Locked",
			"Failed Dependency",
			"Upgrade Required",
			"Precondition Required",
			"Too Many Requests",
			"Request Header Fields Too Large",
			"Unavailable For Legal Reasons",
			"Internal Server Error",
			"Bad Gateway",
			"Service Unavailable",
			"Gateway Timeout",
			"HTTP Version Not Supported",
			"Variant Also Negotiates",
			"Insufficient Storage",
			"Loop Detected",
			"Not Extended",
			"Network Authentication Required"
	};

	static const string status1x_str = "0111";
	static const string status2x_str = "0100000000000000000111111111";
	static const string status3x_str = "0110111111";
	static const string status4x_str = "01000000000000000000010110101111000111111111111111111";
	static const string status5x_str = "0110111111111";


	class HttpRsp : public HttpParser
	{
		public:
			HttpRsp(bool ifcopy=false):HttpParser(HTTP_RESPONSE),m_ifcopy(ifcopy)
			{
				reset();
			}
			virtual ~HttpRsp(){}

			int parse(const char* buffer, size_t count)					
			{
				m_header_map.clear();
				if(execute(buffer, count)) return -1;

				if(0 < m_body_size)
				{
				    if(NULL == m_pbody) return -1;
				    m_body_offset = m_pbody - buffer;
				}
				else
				{
				    m_body_offset = count;
				}

				m_code = getStatusCode();
				m_http_major = getMajor();
				m_http_minor = getMinor();

				return 0;
			}

			static int build(int code, string &strrsp, int major=1, int minor=1, bool disconnect=true)
			{
				const char *status = getStatusStr(code);
				if(NULL == status) return -1;

				char rspbuf[512];
				snprintf(rspbuf, 512, "HTTP/%d.%d %d %s\r\nContent-Length: 0\r\nConnection: %s\r\n\r\n",
						major, minor, code, status, disconnect?"Close":"keep-alive");
				strrsp = rspbuf;

				return 0;
			}

			static int build(int code, unordered_map<string, string> &header_map, const string &body, string &strrsp, int major=1, int minor=1, bool disconnect=true)
			{
				const char *status = getStatusStr(code);
				if(NULL == status) return -1;

				char rspbuf[256];
				snprintf(rspbuf, 256, "HTTP/%d.%d %d %s\r\n",
						major, minor, code, status);
				strrsp = rspbuf;

				unordered_map<string, string>::iterator it;
				for(it=header_map.begin(); it != header_map.end(); ++it)
				{
					string key = it->first;
					string &value = it->second;
					trim(key);
					trim(value);

					string lowerkey = lower(key);
					if(!lowerkey.compare("connection")) continue;
					if(!lowerkey.compare("content-length")) continue;

					strrsp.append(key);
					strrsp.append(": ");
					strrsp.append(value);
					strrsp.append("\r\n");				
				}

				strrsp.append("Connection: ");
				if(disconnect)
					strrsp.append("Close\r\n");
				else
					strrsp.append("keep-alive\r\n");


				snprintf(rspbuf, 256, "Content-Length: %d\r\n\r\n", int(body.size()));
				strrsp.append(rspbuf);

				strrsp.append(body);

				return 0;
			}
			// get var interface
			bool ifcopy() { return m_ifcopy; }
			int body_offset() { return m_body_offset; }
			int body_size() { return m_body_size; }
			unsigned short httpmajor() { return m_http_major; }
			unsigned short httpminor() { return m_http_minor; }
			unsigned int code() { return m_code; }
			string &body() { return m_body; }               
			unordered_map<string, string> &headerMap() { return m_header_map; }
			// get var interface


		private:
			void reset()
			{
				m_body_offset = 0;
				m_body_size = 0;
				m_pbody = NULL;
				m_http_major = 0;
				m_http_minor = 0;
				m_code = 0;
				m_body.clear();                
				m_header_map.clear();
			}
			int onHeader(const std::string& field, const std::string& value)
			{
				// get header
				m_header_map[field] = value;
				return 0;
			}
			int onBody(const char* at, size_t length)
			{
                                m_pbody = at;
				m_body_size = length;
				// get body
				if(m_ifcopy)
				{
					m_body.clear();
					m_body.append(at, length);
				}

				return 0;
			}

			static const char *getStatusStr(int code)
			{
				int index = -1;
				switch(code/100)
				{
					case 1:
						if(status1x_bits[code%100])
							index = (status1x_bits<<(100-(code%100))).count();
						break;
					case 2:
						if(status2x_bits[code%100])
							index = status1x_bits_count + 
								(status2x_bits<<(100-(code%100))).count();
						break;
					case 3:
						if(status3x_bits[code%100])
							index = status1x_bits_count + 
								status2x_bits_count + 
								(status3x_bits<<(100-(code%100))).count();
						break;
					case 4:
						if(status4x_bits[code%100])
							index = status1x_bits_count + 
								status2x_bits_count + 
								status3x_bits_count + 
								(status4x_bits<<(100-(code%100))).count();
						break;
					case 5:
						if(status5x_bits[code%100])
							index = status1x_bits_count + 
								status2x_bits_count + 
								status3x_bits_count + 
								status4x_bits_count + 
								(status5x_bits<<(100-(code%100))).count();
						break;
					default:
						break;
				}
				if(0 > index || index > (status1x_bits_count + 
							status2x_bits_count + 
							status3x_bits_count + 
							status4x_bits_count + 
							status5x_bits_count)) 
					return NULL;

				return status_strings[index];
			}

#ifdef HTTP_DEBUG			
		public:
			void print_status(int code)
			{
				cout<<getStatusStr(code)<<endl;
			}
			void print_ifcopy() { cout<<"ifcopy: "<<ifcopy()<<endl; }
			void print_body_offset() { cout<<"body_offset: "<<body_offset()<<endl; }
			void print_body_size() { cout<<"body_size: "<<body_size()<<endl; }
			void print_httpmajor(){cout<<"http major: "<<httpmajor()<<endl;}
			void print_httpminor(){cout<<"http minor: "<<httpminor()<<endl;}
			void print_code(){cout<<"status code: "<<code()<<endl;}
			void print_body(){cout<<"body: "<<body()<<endl;}
			void print_header_map()
			{
				unordered_map<string, string>::iterator it = headerMap().begin();
				for(;it != headerMap().end(); ++it)
				{
					cout<<"header_map- kv: "<<it->first<<", "<<it->second<<endl;
				}
			}
#endif
		private:
			bool             m_ifcopy;
			int              m_body_offset;
			int              m_body_size;
			const  char  	*m_pbody;
			/////
			unsigned short   m_http_major;
			unsigned short   m_http_minor;
			unsigned int     m_code;
			string           m_body;                
			unordered_map<string, string>    m_header_map;

			static bitset<100> status1x_bits;
			static bitset<100> status2x_bits;
			static bitset<100> status3x_bits;
			static bitset<100> status4x_bits;
			static bitset<100> status5x_bits;
			static int status1x_bits_count;
			static int status2x_bits_count;
			static int status3x_bits_count;
			static int status4x_bits_count;
			static int status5x_bits_count;

	};
	bitset<100> HttpRsp::status1x_bits(status1x_str);
	bitset<100> HttpRsp::status2x_bits(status2x_str);
	bitset<100> HttpRsp::status3x_bits(status3x_str);
	bitset<100> HttpRsp::status4x_bits(status4x_str);
	bitset<100> HttpRsp::status5x_bits(status5x_str);

	int HttpRsp::status1x_bits_count = status1x_bits.count();
	int HttpRsp::status2x_bits_count = status2x_bits.count();
	int HttpRsp::status3x_bits_count = status3x_bits.count();
	int HttpRsp::status4x_bits_count = status4x_bits.count();
	int HttpRsp::status5x_bits_count = status5x_bits.count();
}


