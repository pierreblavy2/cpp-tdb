#ifndef LIB_TDB_FUNCTORS_FN_LAMBDA_HPP_
#define LIB_TDB_FUNCTORS_FN_LAMBDA_HPP_


//--- apply a functor to all/some lines ---
//The applied function IS encapsulated and owned by Fn_function


/*
    //--- Fn_function (void, custom) ---
	struct Fn_void{
		Fn_void(const std::string & prefix_):prefix(prefix_){}
		void operator()(int i){std::cout<< prefix <<i<<std::endl;}
		std::string prefix;
	};

    tdb::Fn_function<
	  tdb::Tag_psql, std::tuple<int>, std::tuple<double,double>, true,
	  Fn_void  //The function to construct
	> fn_function1(
			connection,
			tdb::sql<tdb::Tag_psql>("select i1 from test where d1 != $1 and d2 != $2"), //SqlData_t is expected
			"fn_function1:" //args... to construct Fn_void
	);
    fn_function1(1.0,1.0);
	std::cout << "Fn_function (void) ok"<<std::endl;
	//--- Fn_function (bool, std::function) ---
    tdb::Fn_function<
      tdb::Tag_psql , std::tuple<int>, std::tuple<double,double>, true,
      std::function<bool(int)>
	> fn_function2(
			connection,
			tdb::sql<tdb::Tag_psql>("select i1 from test where d1 != $1 and d2 != $2"),
			[](int i){if(i>100){return false;}std::cout <<i<<std::endl; return true; }
	);
    bool b = fn_function2(1.0,1.0);
	std::cout << "Fn_function2 (void) ok"<<std::endl;
*/

#include "../tdb.hpp"
#include <type_traits>
#include <utility>
#include <functional>

#include "impl/Foreach.hpp"


namespace tdb{


	template<
	    typename Tag_t,
		typename Return_tt,
		typename Bind_tt,
		bool Multi_thread,
		typename Fn_t>
	struct Fn_function;

	template<typename Tag_t , typename... Return_a, typename... Bind_a,typename Fn_t >
	struct Fn_function<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , false , Fn_t>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;

		typedef typename std::invoke_result<Fn_t, Return_a...>::type fn_return_t;
		static constexpr bool fn_returns_bool = std::is_convertible<fn_return_t,bool>::value;


		//movable, NOT copiable
		Fn_function(Fn_function&&)=default;
		Fn_function(const Fn_function&)=delete;
		Fn_function& operator=(const Fn_function&)=delete;
		Fn_function()=delete;

		template<typename... A>
		Fn_function(
				Connection_t<Tag_t>& db,
				const tdb::SqlData_t<Tag_t> &sql,
				A... a
		):fn(std::forward<A>(a)...)		{
			prepare_here<Return_tt,Bind_tt> (db,q,sql);
		}

		Fn_t fn;
		Query<Tag_t,Return_tt,Bind_tt > q;


		//dispatch on  Fn_t type (returns bool, v.s. no return)
		//TRICK : make this function template so compiler doesn't complain for no return
		template<typename T = void>
		auto operator()( const Bind_a&... bind_me){
			if constexpr(fn_returns_bool){
				return impl::Foreach<false>::foreach_bool(q,fn,bind_me... );
			}else{
				impl::Foreach<false>::foreach_void(q,fn,bind_me... );
			}
		}
	};


	template<typename Tag_t,  typename... Return_a, typename... Bind_a,typename Fn_t >
	struct Fn_function<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , true, Fn_t>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;

		typedef typename std::invoke_result<Fn_t, Return_a...>::type fn_return_t;
		static constexpr bool fn_returns_bool = std::is_convertible<fn_return_t,bool>::value;

		//movable, NOT copiable
		Fn_function(Fn_function&&)=default;
		Fn_function(const Fn_function&)=delete;
		Fn_function& operator=(const Fn_function&)=delete;
		Fn_function()=delete;

		template<typename... A>
		Fn_function(
				Connection_t<Tag_t>& db_,
				const tdb::SqlData_t<Tag_t> &sql,
				A... a
		):fn(std::forward<A>(a)...),db(db_){
			auto l = impl::connection_lock_guard (db);
			prepare_here<Return_tt,Bind_tt> (q, db,sql);
		}

		Fn_t fn;
		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;

		//dispatch on  Fn_t type (returns bool, v.s. no return)
		template<typename T = void>
		auto operator()(const Bind_a&... bind_me){
			if constexpr(fn_returns_bool){
				return impl::Foreach<true>::foreach_bool(db,q,fn,bind_me... );
			}else{
				impl::Foreach<true>::foreach_void(db,q,fn,bind_me... );
			}
		}




	};


}//end namespace tdb



#endif /* LIB_TDB_FUNCTORS_FN_FOREACH_HPP_ */
