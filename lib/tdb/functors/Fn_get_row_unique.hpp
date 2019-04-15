#ifndef LIB_TDB_FUNCTORS_FN_ROW_UNIQUE_HPP_
#define LIB_TDB_FUNCTORS_FN_ROW_UNIQUE_HPP_
/*
    tdb::Fn_get_row_unique<
	  tdb::Tag_psql ,
	  std::tuple<int,int>,
	  std::tuple<double,double>,
      true
	> fn_row_unique(connection, "select max(i1), max(i2) from test where d1 != $1 and d2 != $2");
    std::tuple<int,int> fn_row_unique_r =  fn_row_unique(1.0,1.0);
	std::cout << "Fn_get_row_unique ok"<<std::endl;

 */
#include "../tdb.hpp"

namespace tdb{

template<typename Tag_t,  typename Return_tt, typename Bind_tt, bool Multi_thread>
	struct Fn_get_row_unique;

	template<typename Tag_t,  typename Return_tt_, typename... Bind_a>
	struct Fn_get_row_unique<Tag_t, Return_tt_, std::tuple<Bind_a...> , false >{

		typedef Return_tt_ Return_tt;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_row_unique(Fn_get_row_unique&&)=default;
		Fn_get_row_unique(const Fn_get_row_unique&)=delete;
		Fn_get_row_unique& operator=(const Fn_get_row_unique&)=delete;
		Fn_get_row_unique()=delete;

		template<typename... A>
		Fn_get_row_unique(Connection_t<Tag_t>& db, A&& ... a ){
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (db,q,s);
		}

		Return_tt operator()( const Bind_a&... bind_me){
			Return_tt r;
			get_unique(q,r,std::tie(bind_me...));
			return r;
		}

		Query<Tag_t,Return_tt,Bind_tt > q;
	};

	template<typename Tag_t,  typename Return_tt_, typename... Bind_a>
	struct Fn_get_row_unique<Tag_t, Return_tt_, std::tuple<Bind_a...> , true >{

		typedef Return_tt_ Return_tt;
		typedef std::tuple<Bind_a...> Bind_tt;

		//movable, NOT copiable
		Fn_get_row_unique(Fn_get_row_unique&&)=default;
		Fn_get_row_unique(const Fn_get_row_unique&)=delete;
		Fn_get_row_unique& operator=(const Fn_get_row_unique&)=delete;
		Fn_get_row_unique()=delete;

		template<typename... A>
		Fn_get_row_unique(Connection_t<Tag_t>& db_, A&& ... a ):db(db_){
			auto l = impl::connection_lock_guard (db);
			auto s = tdb::sql<Tag_t>(std::forward<A>(a)...);
			prepare_here<Return_tt,Bind_tt> (q, db,s);
		}

		Return_tt operator()( const Bind_a&... bind_me){
			auto l = impl::connection_lock_guard (db);
			Return_tt r;
			get_unique(q,r,std::tie(bind_me...));
			return r;
		}

		Connection_t<Tag_t>& db;
		Query<Tag_t,Return_tt,Bind_tt > q;
	};


}//end namespace tdb


#endif /* LIB_TDB_FUNCTORS_FN_ROW_UNIQUE_HPP_ */
