#ifndef LIB_TDB_FUNCTORS_FN_EXECUTE_HPP_
#define LIB_TDB_FUNCTORS_FN_EXECUTE_HPP_


#include "../tdb.hpp"

namespace tdb{


template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_execute;

	template<typename Tag_t,  typename... Return_a, typename... Bind_a>
	struct Fn_execute<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , false>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;
		static_assert(std::tuple_size<Return_tt>::value==0,"Fn_execute Return_tt must be std::tuple<> (as execute returns nothing)");

		//movable, NOT copiable
		Fn_execute(Fn_execute&&)=default;
		Fn_execute(const Fn_execute&)=delete;
		Fn_execute& operator=(const Fn_execute&)=delete;
		Fn_execute()=delete;

		template<typename... A>
		Fn_execute(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		Query<Tag_t,Return_tt,Bind_tt > q;

		void operator()(const Bind_a&... bind_me){
			tdb::execute_a(q,bind_me...);
		}

	};



	template<typename Tag_t,  typename... Return_a, typename... Bind_a>
	struct Fn_execute<Tag_t,  std::tuple<Return_a...> , std::tuple<Bind_a...> , true>{

		typedef std::tuple<Return_a...> Return_tt;
		typedef std::tuple<Bind_a...>   Bind_tt;
		static_assert(std::tuple_size<Return_tt>::value==0,"Fn_execute Return_tt must be std::tuple<> (as insert returns nothing)");

		//movable, NOT copiable
		Fn_execute(Fn_execute&&)=default;
		Fn_execute(const Fn_execute&)=delete;
		Fn_execute& operator=(const Fn_execute&)=delete;
		Fn_execute()=delete;

		template<typename... A>
		Fn_execute(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;

		void operator()(const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
			tdb::execute_a(q,bind_me...);
		}

	};








}//end namesapce tdb




#endif /* LIB_TDB_FUNCTORS_FN_EXECUTE_HPP_ */
