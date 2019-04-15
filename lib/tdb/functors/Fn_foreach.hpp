#ifndef LIB_TDB_FUNCTORS_FN_FOREACH_HPP_
#define LIB_TDB_FUNCTORS_FN_FOREACH_HPP_

//The applied function is NOT encapsulated, and can change at every call
//tdb::Fn_foreach< Tag_xxx, std::tuple<int> , std::tuple<double,double>, true> fn(connection, "select i from test where d1=? and d2=?");
//auto fn_type1 = []( int)->void{...};
//auto fn_type2 = []( int)->bool{...};
//fn(fn_type1, 1.0, 2.0);
//bool b = fn(fn_type2, 1.0, 2.0);

//functor type :
// type1 : void f(const xxx...&) : apply to all lines, no return
// type2 : bool f(const xxx...&) : apply until f returns false (return false), or no more lines (return true)
// the xxx... arguments are the tuple types in Return_tt

// the multi thread version is blocking. The mutex is NOT unlocked while fn is running



#include "../tdb.hpp"
#include <type_traits>

#include "impl/Foreach.hpp"


namespace tdb{


template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_foreach;

	template<typename Tag_t,  typename... Return_a, typename... Bind_a>
	struct Fn_foreach<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , false>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;


		template<typename... A>
		Fn_foreach(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		//movable, NOT copiable
		Fn_foreach(Fn_foreach&&)=default;
		Fn_foreach(const Fn_foreach&)=delete;
		Fn_foreach& operator=(const Fn_foreach&)=delete;
		Fn_foreach()=delete;


		Query<Tag_t,Return_tt,Bind_tt > q;

		//dispatch on  Fn_t type (returns bool, v.s. no return)
		template<typename Fn_t>
		auto operator()(Fn_t fn,  const Bind_a&... bind_me){
			typedef typename std::invoke_result<Fn_t, Return_a...>::type return_t;
			static constexpr bool is_bool = std::is_convertible<return_t,bool>::value;

			if constexpr(is_bool){
				return impl::Foreach<false>::foreach_bool(q,std::forward<Fn_t>(fn),bind_me... );
			}else{
				impl::Foreach<false>::foreach_void(q,std::forward<Fn_t>(fn),bind_me... );
			}
		}


	};


	template<typename Tag_t,  typename... Return_a, typename... Bind_a>
	struct Fn_foreach<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , true>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;

		//movable, NOT copiable
		Fn_foreach(Fn_foreach&&)=default;
		Fn_foreach(const Fn_foreach&)=delete;
		Fn_foreach& operator=(const Fn_foreach&)=delete;
		Fn_foreach()=delete;

		template<typename... A>
		Fn_foreach(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto l = impl::connection_lock_guard (db);
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (q, db,s);
		}

		Query<Tag_t,Return_tt,Bind_tt > q;
		Connection_t<Tag_t>& db;

		//dispatch on  Fn_t type (returns bool, v.s. no return)
		template<typename Fn_t>
		auto operator()(Fn_t fn,  const Bind_a&... bind_me){
			typedef typename std::invoke_result<Fn_t, Return_a...>::type return_t;
			static constexpr bool is_bool = std::is_convertible<return_t,bool>::value;

			if constexpr(is_bool){
				return impl::Foreach<true>::foreach_bool(db,q,std::forward<Fn_t>(fn),bind_me... );
			}else{
				impl::Foreach<true>::foreach_void(db,q,std::forward<Fn_t>(fn),bind_me... );
			}
		}


	};


}//end namespace tdb



#endif /* LIB_TDB_FUNCTORS_FN_FOREACH_HPP_ */
