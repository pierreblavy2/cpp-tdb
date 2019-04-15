#ifndef LIB_TDB_FUNCTORS_FN_VALUE_OPTIONAL_HPP_
#define LIB_TDB_FUNCTORS_FN_VALUE_OPTIONAL_HPP_

//tdb::Fn_get_value_optional< Tag_xxx, std::tuple<int> , std::tuple<double,double>, true> fn(connection, "select i from test where d1=? and d2=?");
//std::optional<int> r = fn(1.0, 2.0);

//0  row  => return empty optional
//1  row  => return value in optional
//2+ rows => throw


#include "../tdb.hpp"

namespace tdb{

template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_get_value_optional;

	template<typename Tag_t,  typename Return_tt, typename... Bind_a>
	struct Fn_get_value_optional<Tag_t, Return_tt, std::tuple<Bind_a...> , false >{
		static_assert(std::tuple_size<Return_tt>::value == 1, "Fn_get_value_optional expect a single value in Return_tt" );

		typedef typename std::tuple_element<0, Return_tt>::type el0_t;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_value_optional(Fn_get_value_optional&&)=default;
		Fn_get_value_optional(const Fn_get_value_optional&)=delete;
		Fn_get_value_optional& operator=(const Fn_get_value_optional&)=delete;
		Fn_get_value_optional()=delete;

		template<typename... A>
		Fn_get_value_optional(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		std::optional<el0_t> operator()( const Bind_a&... bind_me){
			auto result = tdb::get_result_a(q, bind_me...);
			auto l1 = tdb::try_fetch(result);
			auto l2 = tdb::try_fetch(result);

			if(l2.has_value()){throw std::runtime_error("Error in Fn_get_value_optional : more than 1 line"); }
			if(l1.has_value()){
				return std::get<0>(l1.value());
			}else{
				return std::optional<el0_t>();
			}
		}

		Query<Tag_t,Return_tt,Bind_tt > q;
	};

	template<typename Tag_t,  typename Return_tt, typename... Bind_a>
	struct Fn_get_value_optional<Tag_t, Return_tt, std::tuple<Bind_a...> , true >{
		static_assert(std::tuple_size<Return_tt>::value == 1, "Fn_get_value_optional expect a single value in Return_tt" );

		typedef typename std::tuple_element<0, Return_tt>::type el0_t;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_value_optional(Fn_get_value_optional&&)=default;
		Fn_get_value_optional(const Fn_get_value_optional&)=delete;
		Fn_get_value_optional& operator=(const Fn_get_value_optional&)=delete;
		Fn_get_value_optional()=delete;

		template<typename... A>
		Fn_get_value_optional(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto l = impl::connection_lock_guard (db);
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (q, db,s);
		}

		std::optional<el0_t> operator()( const Bind_a&... bind_me){
			auto result = tdb::get_result_a(q, bind_me...);
			auto l1 = tdb::try_fetch(result);
			auto l2 = tdb::try_fetch(result);

			if(l2.has_value()){throw std::runtime_error("Error in Fn_get_value_optional : more than 1 line"); }
			if(l1.has_value()){
				return std::get<0>(l1.value());
			}else{
				return std::optional<el0_t>();
			}
		}

		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;
	};


}//end namespace tdb




#endif /* LIB_TDB_FUNCTORS_FN_VALUE_OPTIONAL_HPP_ */
