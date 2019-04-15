#ifndef LIB_TDB_HELPERS_TUPLE_REF_HPP_
#define LIB_TDB_HELPERS_TUPLE_REF_HPP_

#include <tuple>

namespace tdb::impl{
	template<typename T>
	struct tuple_ref_t;

	template<typename... A>
	struct tuple_ref_t<std::tuple <A...> >{
		typedef std::tuple<A&... > type;
	};

	template<typename T>
	using tuple_ref = typename tuple_ref_t<T>::type;



	template<typename T>
	struct tuple_cref_t;

	template<typename... A>
	struct tuple_cref_t<std::tuple <A...> >{
		typedef std::tuple<const typename std::remove_reference<A>::type &... > type;
	};

	template<typename T>
	using tuple_cref = typename tuple_cref_t<T>::type;




	template<typename T>
	struct tuple_val_t;

	template<typename... A>
	struct tuple_val_t<std::tuple <A...> >{
		typedef std::tuple<typename  std::remove_const<typename std::remove_reference<A>::type>::type ... > type;
	};

	template<typename T>
	using tuple_val = typename tuple_val_t<T>::type;


	struct tuple_xxx_test{
		static_assert(std::is_same<std::tuple<double&,int&, char&>                  , tuple_ref <std::tuple<double&,int,char&&>> >::value, "");
		static_assert(std::is_same<std::tuple<const double&,const int&, const char&>, tuple_cref<std::tuple<double&,int,char&&>> >::value, "");
		static_assert(std::is_same<std::tuple<double,int, char>                     , tuple_val <std::tuple<double&,int,char&&>> >::value, "");
	};


}




#endif /* LIB_TDB_HELPERS_TUPLE_REF_HPP_ */
