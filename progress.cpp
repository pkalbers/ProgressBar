/**
 * @Author: Patrick K. Albers <pkalbers>
 * @Date:   2017-06-21T23:07:04+01:00
 * @Email:  pkalbers@gmail.com
 * @Filename: progress.cpp
 * @Last modified by:   pkalbers
 * @Last modified time: 2017-06-21T23:14:13+01:00
 */
 

#include "Progress.hpp"


#define PROGRESS_INTERVAL 1000 // milliseconds
#define PROGRESS_INCREASE 100  // milliseconds
#define PROGRESS_BARWIDTH 50   // length of progress bar
#define PROGRESS_BAR_LEFT '['
#define PROGRESS_BAR_DONE '='
#define PROGRESS_BAR_HEAD '>'
#define PROGRESS_BAR_REST ' '
#define PROGRESS_BAR_RGHT ']'


// construct

Progress::Progress(const size_t target)
: time_start(Progress::time_clock_t::now())
, time_print(Progress::time_clock_t::now())
, seek_count(target)
, last_count(0)
, wait_count(0)
, count(0)
, wait_interval(PROGRESS_INTERVAL)
, max_print_length(0)
, finished(false)
, has_percent(true)
, has_progbar(true)
, has_progress_time(true)
, has_rate_per_time(true)
, has_expected_time(true)
, prefix_size(0)
{
	std::cout << std::endl;
	std::cout << " ...\r" << std::flush;
}


// destruct

Progress::~Progress()
{
	if (!this->finished)
		this->print(true);
}


// provide prefix string

void Progress::show_prefix(const std::string & str)
{
	this->prefix = str;
	this->prefix_size = this->prefix.size();
}


// show progress indicators

void Progress::show_percent(const bool flag) { this->has_percent = flag; }
void Progress::show_progbar(const bool flag) { this->has_progbar = flag; }
void Progress::show_progress_time(const bool flag) { this->has_progress_time = flag; }
void Progress::show_rate_per_time(const bool flag) { this->has_rate_per_time = flag; }
void Progress::show_expected_time(const bool flag) { this->has_expected_time = flag; }


// update counter

void Progress::operator ++ () // prefix
{
	guard_t lock(guard);
	++this->count;
	this->print_update();
}

void Progress::operator ++ (int) // postfix
{
	guard_t lock(guard);
	++this->count;
	this->print_update();
}

void Progress::operator += (const size_t n) // value
{
	guard_t lock(guard);
	this->count += n;
	this->print_update();
}


// enforce finish of progress (optional)

void Progress::finish(const bool enforce)
{
	if (!this->finished)
	{
		if (enforce)
		{
			if (this->count != this->seek_count)
			{
				this->has_progress_time = false;
				this->has_rate_per_time = false;
			}
			this->count = this->seek_count;
		}

		this->print(true);
		this->finished = true;
		this->wait_count = std::numeric_limits<size_t>::max();
	}
}

void Progress::remove()
{
	if (!this->finished)
	{
		std::cout << std::setfill(' ') << std::setw(static_cast<int>(this->max_print_length)) << '\r' << std::flush;
		this->finished = true;
		this->wait_count = std::numeric_limits<size_t>::max();
	}
}


// take progress measurements for relevant time periods

void Progress::measure_period()
{
	this->d(*this);
	this->h(*this);
	this->m(*this);
	this->s(*this);
	this->t(*this);
}


// get duration since last print

size_t Progress::ticks_since_print() const
{
	return std::chrono::duration_cast< std::chrono::duration<size_t, time_scale_t> >(time_clock_t::now() - this->time_print).count();
}


// count update and decide whether to print

bool Progress::decide_print() const
{
	if (this->count > this->wait_count)
	{
		if (this->ticks_since_print() > this->wait_interval) // only if after wait interval
		{
			if (this->seek_count > 0)
			{
				const float ratio_increase = static_cast<float>(this->count - this->last_count) / static_cast<float>(this->seek_count);

				if (ratio_increase >= static_cast<float>(0.001)) // if increase by at least 0.1%  => max 1000 prints
				{
					return true;
				}
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}


// print functions

void Progress::print_percent() const  //  constant length of 6
{
	const float percent = (static_cast<float>(this->count) / static_cast<float>(this->seek_count)) * static_cast<float>(100);

	if (percent > static_cast<float>(100))
	{
		std::cout << ">100% ";
		return;
	}

	if      (percent < static_cast<float>(10))  std::cout << std::setw(4) << std::right << std::setprecision(2) << std::fixed << percent << "% ";
	else if (percent < static_cast<float>(100)) std::cout << std::setw(4) << std::right << std::setprecision(1) << std::fixed << percent << "% ";
	else                                        std::cout << std::setw(4) << std::right << std::setprecision(0) << std::fixed << percent << "% ";
}

void Progress::print_progbar() const  //  constant length of 'PROGRESS_BARWIDTH'
{
	static constexpr float width = static_cast<float>(PROGRESS_BARWIDTH);

	const float ratio      = (this->count < this->seek_count) ? static_cast<float>(this->count) / static_cast<float>(this->seek_count): 1.0;
	const int   width_done = static_cast<int>((width * ratio) + 0.5);
	const int   width_rest = static_cast<int>(width - width_done);

	std::cout << PROGRESS_BAR_LEFT;

	if (width_done > 0)
		std::cout << std::setfill(PROGRESS_BAR_DONE) << std::setw(width_done);

	if (width_rest > 0)
		std::cout << PROGRESS_BAR_HEAD << std::setfill(PROGRESS_BAR_REST) << std::setw(width_rest - 1);

	std::cout << PROGRESS_BAR_RGHT;
}

size_t Progress::print_progress_time() const
{
	std::string        str;
	std::ostringstream oss;

	oss << " " << this->count << " in ";

	if      (this->d.good && this->d.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->d.progress_time << " days";
	else if (this->h.good && this->h.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->h.progress_time << " hours";
	else if (this->m.good && this->m.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->m.progress_time << " min";
	else if (this->s.good && this->s.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->s.progress_time << " sec";
	else if (this->t.good && this->t.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->t.progress_time << " ms";
	else
	{
		return 0;
	}

	str = oss.str();
	std::cout << str;
	return str.size();
}

size_t Progress::print_rate_per_time() const
{
	std::string        str;
	std::ostringstream oss;

	oss << " (~";

	if      (this->t.good && this->t.rate_per_time > 1) oss << std::setprecision(1) << std::fixed << this->t.rate_per_time << " / ms)";
	else if (this->s.good && this->s.rate_per_time > 1) oss << std::setprecision(1) << std::fixed << this->s.rate_per_time << " / sec)";
	else if (this->m.good && this->m.rate_per_time > 1) oss << std::setprecision(1) << std::fixed << this->m.rate_per_time << " / min)";
	else if (this->h.good && this->h.rate_per_time > 1) oss << std::setprecision(1) << std::fixed << this->h.rate_per_time << " / hour)";
	else if (this->d.good && this->d.rate_per_time > 1) oss << std::setprecision(1) << std::fixed << this->d.rate_per_time << " / day)";
	else
	{
		return 0;
	}

	str = oss.str();
	std::cout << str;
	return str.size();
}

size_t Progress::print_expected_time() const
{
	std::string        str;
	std::ostringstream oss;

	oss << ", expected time: ~";

	if      (this->d.good && this->d.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->d.expected_time << " days";
	else if (this->h.good && this->h.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->h.expected_time << " hours";
	else if (this->m.good && this->m.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->m.expected_time << " min";
	else if (this->s.good && this->s.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->s.expected_time << " sec";
	else if (this->t.good && this->t.progress_time > 1) oss << std::setprecision(1) << std::fixed << this->t.expected_time << " ms";
	else
	{
		return 0;
	}

	str = oss.str();
	std::cout << str;
	return str.size();
}


// update progress and print

void Progress::print(const bool end)
{
	size_t len = 0;

	this->measure_period();

	if (this->prefix_size > 0)
	{
		std::cout << prefix << " ";
		len += this->prefix_size;
	}

	if (this->seek_count > 0)
	{
		if (this->has_percent)
		{
			this->print_percent();
			len += 6;
		}
		if (this->has_progbar)
		{
			this->print_progbar();
			len += PROGRESS_BARWIDTH;
		}
	}

	if (this->has_progress_time)
		len += this->print_progress_time();

	if (this->has_rate_per_time)
		len += this->print_rate_per_time();

	if (this->has_expected_time && (this->count < this->seek_count) && !end)
		len += this->print_expected_time();


	if (this->max_print_length < len)
		this->max_print_length = len;

	if (this->max_print_length > len)
	{
		std::cout << std::setfill(' ') << std::setw(static_cast<int>(this->max_print_length - len));
	}

	if (end)
		std::cout << std::endl;
	else
		std::cout << '\r' << std::flush;
}

void Progress::print_update()
{
	if (this->decide_print())
	{
		this->print(false);

		this->time_print = time_clock_t::now();
		this->wait_count = this->count + (this->count - this->last_count) - 1;
		this->last_count = this->count;
		this->wait_interval += PROGRESS_INCREASE;
	}
}
