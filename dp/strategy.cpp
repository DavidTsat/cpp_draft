#include <vector>
#include <memory>
#include <iostream>

using namespace std;

class Shape
{
public:
	virtual ~Shape() = 0;

	virtual void draw() const = 0;
};

Shape::~Shape() {}

class Circle;
class Square;
class Rectangle;

class DrawStrategy
{
public:
	virtual ~DrawStrategy() = 0;
	virtual void draw(const Circle&) const = 0;
	virtual void draw(const Square&) const = 0;
	virtual void draw(const Rectangle&) const = 0;
};

DrawStrategy::~DrawStrategy() {}

class OpenGLDrawStrategy : public DrawStrategy
{
public:
	~OpenGLDrawStrategy() = default;

	virtual void draw(const Circle&) const
	{
		cout << "OpenGLDrawStrategy circle draw\n";
	}

	virtual void draw(const Square&) const
	{
		cout << "OpenGLDrawStrategy square draw\n";
	}
	
	virtual void draw(const Rectangle&) const
	{
		cout << "OpenGLDrawStrategy rectangle draw\n";
	}
};

class TestDrawStrategy : public DrawStrategy
{
public:
	~TestDrawStrategy() = default;

	virtual void draw(const Circle&) const
	{
		cout << "TestDrawStraregy circle draw\n";
	}

	virtual void draw(const Square&) const
	{
		cout << "TestDrawStraregy square draw\n";
	}
	
	virtual void draw(const Rectangle&) const
	{
		cout << "TestDrawStrategy rectangle draw\n";
	}

};

class Circle : public Shape
{
public:
	~Circle() = default;

	Circle(unsigned r, unique_ptr<DrawStrategy> ds) : r_(r), ds_(move(ds))
	{
	}

	virtual void draw() const
	{
		ds_->draw(*this);
	}
private:
	unsigned r_;
	unique_ptr<DrawStrategy> ds_;
};


class Square : public Shape
{
public:
	~Square() = default;

	Square(unsigned s, unique_ptr<DrawStrategy> ds) : s_(s), ds_(move(ds))
	{
	}

	virtual void draw() const
	{
		ds_->draw(*this);
	}
private:
	unsigned s_;
	unique_ptr<DrawStrategy> ds_;
};

class Rectangle : public Shape
{
public:
	~Rectangle() = default;

	Rectangle(unsigned a, unsigned b, unique_ptr<DrawStrategy> ds) : a_(a), b_(b), ds_(move(ds))
	{
	}

	virtual void draw() const
	{
		ds_->draw(*this);
	}
private:
	unsigned a_;
	unsigned b_;
	unique_ptr<DrawStrategy> ds_;

};

class circle;
class square;

template <typename ShapeT>
class drawShapeStrategy
{
public:
	virtual ~drawShapeStrategy() {}
	virtual void draw(const ShapeT&) const = 0;
};


class drawCircleStrategy : public drawShapeStrategy<circle>
{
public:
	~drawCircleStrategy() = default;

	void draw(const circle&) const override = 0;
};

class drawSquareStrategy : public drawShapeStrategy<square>
{
public:
	~drawSquareStrategy() = default;
	
	void draw(const square&) const override = 0;
};

class openGLCircleStrategy : public drawCircleStrategy
{
public:
	~openGLCircleStrategy() = default;

	void draw(const circle&) const override
	{
		cout << "openGLCircleStrategy draw()\n";
	}
};

class openGLSquareStrategy : public drawSquareStrategy
{
public:
	~openGLSquareStrategy() = default;

	void draw(const square&) const override
	{
		cout << "openGLSquareStrategy draw()\n";
	}

};

class testCircleStrategy : public drawCircleStrategy
{
public:
	~testCircleStrategy() = default;

	void draw(const circle&) const override
	{
		cout << "testCircleStrategy draw()\n";
	}
};

class testSquareStrategy : public drawSquareStrategy
{
public:
	~testSquareStrategy() = default;

	void draw(const square&) const override
	{
		cout << "testSquareStrategy draw()\n";
	}

};


class shape
{
public:
	virtual ~shape() = 0;
	virtual void draw() const = 0;
};

shape::~shape() {}

class circle : public shape
{
public:
	~circle() = default;
	circle(unsigned r, unique_ptr<drawCircleStrategy> ds) : r_(r), ds_(move(ds)) {}

	void draw() const override
	{
		ds_->draw(*this);
	}

private:
	unsigned r_;
	unique_ptr<drawCircleStrategy> ds_;
};

class square : public shape
{
public:
	~square() = default;
	square(unsigned s, unique_ptr<drawSquareStrategy> ds) : s_(s), ds_(move(ds)) {}

	void draw() const override
	{
		ds_->draw(*this);
	}

private:
	unsigned s_;
	unique_ptr<drawSquareStrategy> ds_;
};

void test1()
{
	using Shapes = vector<unique_ptr<Shape>>;
	Shapes sp;

	sp.emplace_back(make_unique<Circle>(4, make_unique<OpenGLDrawStrategy>()));
	sp.emplace_back(make_unique<Circle>(5, make_unique<TestDrawStrategy>()));
	sp.emplace_back(make_unique<Square>(6, make_unique<TestDrawStrategy>()));
	sp.emplace_back(make_unique<Square>(7, make_unique<OpenGLDrawStrategy>()));
	sp.emplace_back(make_unique<Rectangle>(6, 4, make_unique<TestDrawStrategy>()));
	sp.emplace_back(make_unique<Rectangle>(7, 5, make_unique<OpenGLDrawStrategy>()));


	for (const auto& s : sp)
		s->draw();
}

void test2()
{
	using shapes = vector<unique_ptr<shape>>;
	shapes sp;

	sp.emplace_back(make_unique<circle>(4, make_unique<openGLCircleStrategy>()));
	sp.emplace_back(make_unique<circle>(5, make_unique<testCircleStrategy>()));
	sp.emplace_back(make_unique<square>(6, make_unique<testSquareStrategy>()));
	sp.emplace_back(make_unique<square>(7, make_unique<openGLSquareStrategy>()));
//	sp.emplace_back(make_unique<rectangle>(6, 4, make_unique<testRectangleStrategy>()));
//	sp.emplace_back(make_unique<rectangle>(7, 5, make_unique<openGLRectangleStrategy>()));


	for (const auto& s : sp)
		s->draw();
}


int main()
{
	test1();
	cout << '\n';
	test2();

	return 0;
}
