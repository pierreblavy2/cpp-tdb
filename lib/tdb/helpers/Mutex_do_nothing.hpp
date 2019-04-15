#ifndef LIB_TDB_MUTEX_DO_NOTHING_HPP_
#define LIB_TDB_MUTEX_DO_NOTHING_HPP_

namespace tdb{

struct Mutex_do_nothing{
	void lock(){}
	void unlock(){}
};


}



#endif /* LIB_TDB_MUTEX_DO_NOTHING_HPP_ */
