#ifndef LIB_TDB_FUNCTORS_FN_VALUE_UNIQUE_HPP_
#define LIB_TDB_FUNCTORS_FN_VALUE_UNIQUE_HPP_

//example :
//tdb::Fn_get_value_unique< Tag_xxx, std::tuple<int> , std::tuple<double,double>, true> fn(connection, "select i from test where d1=? and d2=?");
//int r = fn(1.0, 2.0);

//0  row  => throw
//1  row  => return single row value
//2+ rows => throw


#include "../tdb.hpp"

namespace tdb{

template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_get_value_unique;

	template<typename Tag_t,  typename Return_tt, typename... Bind_a>
	struct Fn_get_value_unique<Tag_t, Return_tt, std::tuple<Bind_a...> , false >{
		static_assert(std::tuple_size<Return_tt>::value == 1, "Fn_get_value_unique expect a single value in Return_tt" );

		typedef typename std::tuple_element<0, Return_tt>::type el0_t;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_value_unique(Fn_get_value_unique&&)=default;
		Fn_get_value_unique(const Fn_get_value_unique&)=delete;
		Fn_get_value_unique& operator=(const Fn_get_value_unique&)=delete;
		Fn_get_value_unique()=delete;

		template<typename... A>
		Fn_get_value_unique(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		el0_t operator()( const Bind_a&... bind_me){
			el0_t r;
			auto t = std::tie(r);
			get_unique(q,t,std::tie(bind_me...));
			return r;
		}

		Query<Tag_t,Return_tt,Bind_tt > q;
	};

	template<typename Tag_t,  typename Return_tt, typename... Bind_a>
	struct Fn_get_value_unique<Tag_t, Return_tt, std::tuple<Bind_a...> , true >{
		static_assert(std::tuple_size<Return_tt>::value == 1, "Fn_get_value_unique expect a single value in Return_tt" );

		typedef typename std::tuple_element<0, Return_tt>::type return_type;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_value_unique(Fn_get_value_unique&&)=default;
		Fn_get_value_unique(const Fn_get_value_unique&)=delete;
		Fn_get_value_unique& operator=(const Fn_get_value_unique&)=delete;
		Fn_get_value_unique()=delete;

		template<typename... A>
		Fn_get_value_unique(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto l = impl::connection_lock_guard (db);
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (q, db,s);
		}

		return_type operator()( const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
			return_type r;
			auto t = std::tie(r);
			get_unique(q,t,std::tie(bind_me...));
			return r;
		}

		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;
	};


}//end namespace tdb





#endif /* LIB_TDB_FUNCTORS_FN_VALUE_UNIQUE_TPP_ */
