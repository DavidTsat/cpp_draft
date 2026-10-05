#include <vector>
#include <memory>
#include <variant>
#include <time.h>

#include <iostream>

using namespace std;

class Circle;
class Square;


class ShapeVisitor
{
public:
	void visit(const Circle& c) const
	{
		doVisit(c);
	}

	void visit(const Square& s) const
	{
		doVisit(s);
	}
private:
	virtual void doVisit(const Circle& c) const = 0;
	virtual void doVisit(const Square& s) const = 0;
};

class Draw : public ShapeVisitor
{
private:
	void doVisit(const Circle& c) const override
	{
		cout << "Draw circle\n";
	}

	void doVisit(const Square& s) const override
	{
		cout << "Draw square\n";
	}
};

class Rotate : public ShapeVisitor
{
private:
	void doVisit(const Circle& c) const override
	{
		cout << "Rotate Circle\n";
	}

	void doVisit(const Square& s) const override
	{
		cout << "Rotate Square\n";
	}
};


class Shape
{
public:
	virtual void accept(const ShapeVisitor& s) const = 0;

	virtual ~Shape() = default;
};

class Circle : public Shape
{
public:
	explicit Circle(double r) : r_(r) {}
	
	void accept(const ShapeVisitor& s) const override
	{
		s.visit(*this);
	}
private:
	double r_;
};

class Square : public Shape
{
public:
	explicit Square(double s) : s_(s) {}

	void accept(const ShapeVisitor& s) const override
	{
		s.visit(*this);
	}
private:
	double s_;
};

class circle 
{
public:
	explicit circle(double r) : r_(r) {}
	
private:
	double r_;
};

class square 
{
public:
	explicit square(double s) : s_(s) {}

private:
	double s_;
};

struct draw
{
	void operator()(const circle& c) const
	{
		cout << "draw circle\n";
	}

	void operator()(const square& s) const
	{
		cout << "draw square\n";
	}
};

using shapes = vector<variant<circle, square>>;
using Shapes = vector<unique_ptr<Shape>>;

void testPolym()
{
	Shapes sp;

	sp.emplace_back(make_unique<Circle>(4));
	sp.emplace_back(make_unique<Circle>(5));
	sp.emplace_back(make_unique<Square>(6));

	struct timespec ts1, ts2;
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts1);

	for (const auto& s : sp)
		s->accept(Draw{});
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts2);

	long long secs = ts2.tv_sec - ts1.tv_sec;
	long long nsecs = ts2.tv_nsec - ts1.tv_nsec;

	long long elapsedns = (secs * 1000000000LL) + nsecs;

	cout << "elapsed ns polym: " << elapsedns << endl;
}

void testVariant()
{
	shapes sp;

	sp.emplace_back(circle(4));
	sp.emplace_back(circle(5));
	sp.emplace_back(square(6));
	
	struct timespec ts1, ts2;
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts1);

	for (const auto& s : sp)
		visit(draw{}, s);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts2);

	long long secs = ts2.tv_sec - ts1.tv_sec;
	long long nsecs = ts2.tv_nsec - ts1.tv_nsec;

	long long elapsedns = (secs * 1000000000LL) + nsecs;

	cout << "elapsed ns variant: " << elapsedns << endl;

}

// TO IMPLEMENT NVI 
//
int main()
{
	testPolym();
	cout << '\n';
	testVariant();

	return 0;
}
