#ifndef LIB_TDB_FUNCTORS_FN_FUNCTION_CPP_
#define LIB_TDB_FUNCTORS_FN_FUNCTION_CPP_

#include "../Fn_function.hpp"

#include <iostream>
#include <tdb/tdb_sqlite.hpp>

//test code
namespace{
    [[maybe_unused]] void example(){
    	typedef tdb::Tag_sqlite Tag_xxx;
    	tdb::Connection_t<Tag_xxx> connection("/tmp/test.sqlite");

    	//--- User defined functors ---
		struct Fn_void{
			Fn_void(const std::string & prefix_):prefix(prefix_){}
			void operator()(int i){std::cout<< prefix <<i<<std::endl;}
			std::string prefix;
		};

		struct Fn_bool{
			Fn_bool(const int & threshold_):threshold(threshold_){}
			bool operator()(int i){if(i>threshold){return false;} std::cout<<i<<std::endl; return true;}
			int threshold;
		};

		//--- Fn_function (void) ---
		//multi thread
		tdb::Fn_function<
		  Tag_xxx ,
		  std::tuple<int>,
		  std::tuple<double,double>,
		  true,
		  Fn_void
		> fn_function1(
				connection,
				tdb::sql<Tag_xxx>("select i1 from test where d1 != $1 and d2 != $2"),
				"hello:" //args to construct Fn_void
		);
		fn_function1(1.0,1.0);

		//single thread
		tdb::Fn_function<
		  Tag_xxx ,
		  std::tuple<int>,
		  std::tuple<double,double>,
		  false,
		  Fn_void
		> fn_function2(
				connection,
				tdb::sql<Tag_xxx>("select i1 from test where d1 != $1 and d2 != $2"),
				"hello:" //args to construct Fn_void
		);
		fn_function2(1.0,1.0);




		//--- Fn_function (bool) ---
		//if function returns false, stop fetching
		//multi thread
		tdb::Fn_function<
		  Tag_xxx ,
		  std::tuple<int>,
		  std::tuple<double,double>,
		  true,
		  Fn_bool
		> fn_function3(
				connection,
				tdb::sql<Tag_xxx>("select i1 from test where d1 != $1 and d2 != $2"),
				5 //args to construct Fn_void
		);


		[[maybe_unused]] bool b1 = fn_function3(1.0,1.0);



		//single thread
		tdb::Fn_function<
		  Tag_xxx ,
		  std::tuple<int>,
		  std::tuple<double,double>,
		  false,
		  Fn_bool
		> fn_function4(
				connection,
				tdb::sql<Tag_xxx>("select i1 from test where d1 != $1 and d2 != $2"),
				5 //args to construct Fn_void
		);
		[[maybe_unused]] bool b2 = fn_function4(1.0,1.0);



		//--- check return type ---
		static_assert(tdb::Fn_function<
						  Tag_xxx ,
						  std::tuple<int>,
						  std::tuple<double,double>,
						  true,
						  Fn_bool
						>::fn_returns_bool,"");

		static_assert(!tdb::Fn_function<
						  Tag_xxx ,
						  std::tuple<int>,
						  std::tuple<double,double>,
						  true,
						  Fn_void
						>::fn_returns_bool,"");


    }

}



#endif /* LIB_TDB_FUNCTORS_FN_FUNCTION_CPP_ */
