/**
 * @Author: Patrick K. Albers <pkalbers>
 * @Date:   2017-06-21T23:06:55+01:00
 * @Email:  pkalbers@gmail.com
 * @Filename: progress.hpp
 * @Last modified by:   pkalbers
 * @Last modified time: 2017-06-21T23:23:28+01:00
 */


#ifndef progress_hpp
#define progress_hpp

// C
#include <stdio.h>

// C++
#include <chrono>
#include <mutex>
#include <iomanip>
#include <iostream>
#include <ratio>
#include <string>
#include <sstream>


//
// Progress bar
//
class Progress
{
public:

	// construct
	Progress(const size_t = 0); // print progress bar, or update rate if target unknown

	// destruct
	~Progress(); // checks for regular exit

	// provide prefix string
	void show_prefix(const std::string &);

	// show progress indicators
	void show_percent(const bool);
	void show_progbar(const bool);
	void show_progress_time(const bool);
	void show_rate_per_time(const bool);
	void show_expected_time(const bool);

	// update counter
	void operator ++ (); // prefix
	void operator ++ (int); // postfix
	void operator += (const size_t); // value

	// enforce finish of progress (optional)
	void finish(const bool = false); // indicate completeness (true) even if target not reached
	void remove(); // finish and remove line

private:

	using time_clock_t = std::chrono::steady_clock;
	using time_point_t = std::chrono::time_point< time_clock_t >;
	using time_scale_t = std::chrono::milliseconds::period;

	using guard_t = std::lock_guard< std::mutex >;

	using period_d = std::ratio_multiply<std::chrono::hours::period, std::ratio<24> >::type; // days
	using period_h = std::chrono::hours::period;
	using period_m = std::chrono::minutes::period;
	using period_s = std::chrono::seconds::period;
	using period_t = std::chrono::milliseconds::period;

	template < class T_PERIOD >
	struct Period
	{
		using floating_t = double;
		using duration_t = std::chrono::duration< floating_t, T_PERIOD >;

		// construct
		Period()
		: progress_time(0)
		, rate_per_time(0)
		, expected_time(0)
		, good(false)
		{}

		// measure
		void operator () (const Progress & p)
		{
			this->good = false;

			this->progress_time = std::chrono::duration_cast< duration_t >(time_clock_t::now() - p.time_start).count();
			if (std::isnormal(this->progress_time))
			{
				this->rate_per_time = static_cast< floating_t >(p.count) / this->progress_time;
				if (std::isnormal(this->rate_per_time))
				{
					this->expected_time = static_cast< floating_t >(p.seek_count - p.count) / this->rate_per_time;
					if (std::isnormal(this->expected_time))
					{
						this->good = true;
					}
				}
			}
		}

		floating_t progress_time;
		floating_t rate_per_time;
		floating_t expected_time;
		bool good;
	};

	Period< period_d > d;
	Period< period_h > h;
	Period< period_m > m;
	Period< period_s > s;
	Period< period_t > t;

	// take progress measurements for relevant time periods
	void measure_period();

	// get duration since last print
	size_t ticks_since_print() const;

	// count update and decide whether to print
	bool decide_print() const;

	// print functions
	void   print_percent() const; // percent done
	void   print_progbar() const; // progress bar
	size_t print_progress_time() const; // counts over time since start
	size_t print_rate_per_time() const; // relative rate per time unit
	size_t print_expected_time() const; // expected time until finish

	// update progress and print
	void print(const bool);
	void print_update();

	// timestamps
	const time_point_t time_start; // time of process start
	time_point_t       time_print; // time of last print

	// counters
	const size_t seek_count;
	size_t       last_count;
	size_t       wait_count;
	size_t       count;

	size_t wait_interval; // wait a while until next print

	size_t max_print_length; // maximum length printed

	bool finished; // indicate if finished was enforced

	bool has_percent;
	bool has_progbar;
	bool has_progress_time;
	bool has_rate_per_time;
	bool has_expected_time;

	std::string prefix;
	size_t prefix_size;

	std::mutex guard;
};

#endif /* progress_hpp */
