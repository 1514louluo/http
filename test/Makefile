all:
	g++ -Wall test_perf_protocol_single_thread.cpp ../http_parser.c ../httpparser.cpp -I../ -o test_perf_protocol_single_thread -std=c++11 -DHTTP_DEBUG -O3 #-Ofast #-ggdb 
clean:
	rm -f test_perf_protocol_single_thread
