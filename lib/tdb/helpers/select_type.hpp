#ifndef LIB_TDB_HELPERS_SELECT_TYPE_HPP_
#define LIB_TDB_HELPERS_SELECT_TYPE_HPP_


namespace tdb::impl{
	template<bool b, typename T, typename U>
	struct select_type;

	template<typename T, typename U>
	struct select_type<true, T,U>{typedef T type;};

	template<typename T, typename U>
	struct select_type<false, T,U>{typedef U type;};
}



#endif /* LIB_TDB_HELPERS_SELECT_TYPE_HPP_ */
