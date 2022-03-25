colex: colex.cpp InfInt.h json.hpp
	c++ colex.cpp -std=c++2a -o colex -lpthread

json.hpp:
	wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp