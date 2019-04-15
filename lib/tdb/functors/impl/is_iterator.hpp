#ifndef LIB_TDB_FUNCTORS_IMPL_IS_ITERATOR_HPP_
#define LIB_TDB_FUNCTORS_IMPL_IS_ITERATOR_HPP_


//tdb::impl::is_iterator_of_type<T,tag>
//  false : T is not an iterator
//  false : T is an iterator, T::iterator_category does NOT match tag
//  true  : T is an iterator, T::iterator_category matches tag

//tdb::impl::is_iterator_<T>
//  false : T is not an iterator
//  true  : T is an iterator

//a iterator is any T with typename T::iterator_category .


#include <iterator>


//--- is_iterator_of_type_t ---

namespace tdb::impl{

	template<typename T, typename tag, typename is_enabled=void >//tag = std::output_iterator_tag
	struct is_iterator_of_type_t{
		static constexpr bool value = false ;
	};

	template<typename T, typename tag>
	struct is_iterator_of_type_t<
		T,
		tag ,
		typename std::enable_if<std::is_same<typename std::remove_reference<typename std::remove_cv<T>::type>::type::iterator_category,tag>::value,void>::type  >{
		static constexpr bool value = true;
	};

	template<typename T, typename tag>
	inline constexpr bool is_iterator_of_type =is_iterator_of_type_t<T,tag>::value;





	//--- is_iterator_t ---


	template<typename T, typename is_enabled=void >//tag = std::output_iterator_tag
	struct is_iterator_t{
		static constexpr bool value = false ;
	};

	template<typename T>
	struct is_iterator_t<
		T,
		typename std::enable_if<std::is_same<
			typename std::remove_reference<typename std::remove_cv<T>::type>::type::iterator_category,
			typename std::remove_reference<typename std::remove_cv<T>::type>::type::iterator_category
		>::value,void>::type  >
	{
		static constexpr bool value = true;
	};


	template<typename T>
	inline constexpr bool is_iterator =is_iterator_t<T>::value;



}




#endif /* LIB_TDB_FUNCTORS_IMPL_IS_ITERATOR_HPP_ */
