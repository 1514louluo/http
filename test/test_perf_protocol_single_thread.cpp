#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include "http.h"
using namespace std;
using namespace HTTP;

#define TIMEDIFF(s, e) ((e.tv_sec - s.tv_sec)*1000 + (e.tv_usec - s.tv_usec)/1000) 

int main(int argc, char **argv)
{

	if(3> argc) 
	{
		cout<<"useage: "<<argv[0]<<" len num"<<endl;
		return 0;
	}

	int len = atoi(argv[1]);
	if(0 >= len)
	{
		cout<<"len must gt 0!"<<endl;
		return 0;
	}

	int num = atoi(argv[2]);
	if(0 >= num)
	{
		cout<<"num must gt 0!"<<endl;
		return 0;
	}


	const char * http_nobody = "POST /recognize.cgi?uid=ff,jjf2,jk:2fef,j2f&vid=&appkey=fjf?jj.fj;fjfj&token=fjfjjfjfjj&fmt=1&rate=16000&bits=16&decoder=1&decoder_idx=0&channel=1&lan=2&debug=1 HTTP/1.1\r\n"
				    "Host: 0.0.0.0=5000\r\n"
				    "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
				    "Content-Length: %d\r\n"
				    "Accept-Language: en-us,en;q=0.5\r\n"
				    "Accept-Encoding: gzip,deflate\r\n"
				    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
				    "Keep-Alive: 300\r\n"
				    "Connection: keep-alive\r\n"
				    "\r\n";

	char tempbuf[4096];
	snprintf(tempbuf, 4096, http_nobody, len);

	string httpbuf(tempbuf); 
	for(int i=0; i<len; ++i)
	{
		httpbuf.append("a");
	}


//	cout<<httpbuf<<endl;

	struct timeval start;
        struct timeval end;

	gettimeofday(&start, NULL);
	

	HttpReq httpreq;
	for(int i=0; i<num; ++i)
	{
//	HttpReq httpreq;
		int ret = httpreq.parse(httpbuf.c_str(), httpbuf.size());	
		if(ret) continue;
/*
		cout<<"+++++++++++++++++++++++++++: "<<ret<<endl;
		httpreq.print_method();
		httpreq.print_path();
		httpreq.print_querystring();
		httpreq.print_body();
		httpreq.print_header_map();
		httpreq.print_query_map();
		httpreq.print_ifcopy();
		httpreq.print_body_offset();
		httpreq.print_body_size();
		httpreq.print_path_offset();
		httpreq.print_path_size();
		httpreq.print_querystring_offset();
		httpreq.print_querystring_size();
cout<<"999999999999999999999999999"<<endl;
	cout<<httpbuf.substr(httpreq.path_offset(), httpreq.path_size())<<endl;
	cout<<httpbuf.substr(httpreq.querystring_offset(), httpreq.querystring_size())<<endl;
	cout<<httpbuf.substr(httpreq.body_offset(), httpreq.body_size())<<endl;
cout<<"999999999999999999999999999"<<endl;
*/
	}

	gettimeofday(&end, NULL);

	cout<<"parse "<<num<<" package, body size "<<len<<", use time: "<<TIMEDIFF(start, end)<<" ms"<<endl; 	


	gettimeofday(&start, NULL);

	HttpRsp httprsp;
//	httprsp.print_status(atoi(argv[3]));

	for(int i=0; i<num; ++i)
	{
//	HttpRsp httprsp;
		string rsp;
//		httprsp.build(201, rsp);
//		cout<<rsp<<endl;

		unordered_map<string, string> header_map;
		header_map["232"] = "232";
		header_map["231"] = "231";
		header_map["x231"] = "231";
		header_map["y231"] = "231";
		header_map["c231"] = "231";
		header_map["t231"] = "231";
		header_map["content"] = "content1";
		header_map["woqu"] = "woqu1";
		header_map["212fff31"] = "231";
		header_map["jfjjfjfjfjf"] = "jfj2j3232=32k32233f232f3f3232f23";

//		httprsp.build(501, header_map, rsp);
//		cout<<rsp<<endl;

//		httprsp.build(200, header_map, httpbuf, rsp);
		HttpRsp::build(200, header_map, httpbuf, rsp);
//		cout<<rsp<<endl;
/*
        cout<<"------------------------------------------"<<endl;
	httprsp.parse(rsp.c_str(), rsp.size());
	httprsp.print_code();
	httprsp.print_httpmajor();
	httprsp.print_httpminor();
	httprsp.print_header_map();
	httprsp.print_body();
	httprsp.print_ifcopy();
	httprsp.print_body_offset();
	httprsp.print_body_size();


cout<<"999999999999999999999999999"<<endl;
	cout<<rsp.substr(httprsp.body_offset(), httprsp.body_size())<<endl;
cout<<"999999999999999999999999999"<<endl;
*/
	}

	gettimeofday(&end, NULL);

	cout<<"build "<<num<<" package, body size "<<len<<", use time: "<<TIMEDIFF(start, end)<<" ms"<<endl; 	
//
//

	return 0;
}
