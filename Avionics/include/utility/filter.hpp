#ifndef FILTER_HPP_
#define FILTER_HPP_

namespace utility {
	template<typename T>
	inline auto average(const T *buf, const size_t &size) -> T {
		T ave = 0.0;
		for(size_t i=0;i<size;i++){
			ave += buf[i];
		}
		return ave / size;
	}

	template<typename T, size_t size>
	class moving_average {
	public:
		moving_average() : current(0) {
			for(size_t i=0;i<size;i++){
				buf[i] = 0.0;
			}
		}

		inline auto update_current() -> void {
			current++;
			if(current == size)
				current = 0;
		}

		auto add_data(const T &val) -> void {
			update_current();
			buf[current] = val;
		}

		auto filtered() const -> T {
			return average<T>(this->buf, size);
		}
	protected:
		T buf[size];
		size_t current;
	};
}

/*
 * sample code
 *
 * #include "utility/filter.hpp"
 *
 * namespace global {
 *     utility::moving_average<float, 5> hoge_filter;
 * }
 *
 * bool open_by_hoge(){
 *     const auto hoge = sensor::hoge.get_value();
 *     hoge_filter.add_data(hoge);
 *
 *     if(hoge_filter.filtered() >= HOGE_LIMIT) return true;
 *     return false;
 * }
*/

#endif
