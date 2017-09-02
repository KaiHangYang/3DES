#ifndef MTIME_COUNT
#define MTIME_COUNT

#include <ctime>
#include <iostream>

namespace mTimeCount {
    static double _my_time_start;
    static double _my_time_end;
};
#define TIME_COUNT_START() mTimeCount::_my_time_start = clock();
#define TIME_COUNT_END(process) mTimeCount::_my_time_end = clock(); std::cout << #process << ": time comsume is " << ((mTimeCount::_my_time_end - mTimeCount::_my_time_start)/CLOCKS_PER_SEC) << std::endl;



#endif
