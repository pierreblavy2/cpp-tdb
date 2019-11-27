#Introduction to conversions problems
C++ provides rules for converting between types, trough default conversion operators, static_cast, 
boost::lexical_cast, boost_numeric cast and a bunch of function like std::to_string or atoi or the .c_str() member function. 
Such design leads to the following problems
- This design is highly heterogeneous which makes guessing how to convert between things impossible for a user.
- Members functions for conversion makes this design unextensible to user defined types, as you cannot add
a member to a class you don't own
- There's no clear way to choose how things should be converted based on the context. For example, I want to convert 
a bool to "0" or "1" string when I want to store them in a flat file, but to "true" or "false" when I want to display them in
the console, so a unique bool to string conversion will never do the job.

#Design of the convert library
In order to solve the following problems, the convert library works by defining the following template
```cpp
template<typename To_tt, typename From_tt, typename Context_tag_tt>
struct Convert_t{
	typedef To_tt   To_t;
	typedef From_tt From_t;
	typedef Context_tag_tt Context_tag_t;

	static To_t run(const From_t &f){
		return static_cast<To_t>(f);
	}
};
```
```xxx``` 





