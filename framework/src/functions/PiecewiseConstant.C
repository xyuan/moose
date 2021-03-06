/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "PiecewiseConstant.h"

template<>
InputParameters validParams<PiecewiseConstant>()
{
  InputParameters params = validParams<Piecewise>();
  MooseEnum direction("left, right", "left");
  params.addParam<MooseEnum>("direction", direction, "Direction to look to find value: " + direction.getRawNames());
  return params;
}

PiecewiseConstant::DirectionEnum
PiecewiseConstant::getDirection(const std::string & direction)
{
  DirectionEnum dir = PiecewiseConstant::UNDEFINED;
  if (direction == "left")
  {
    dir = PiecewiseConstant::LEFT;
  }
  else if (direction == "right")
  {
    dir = PiecewiseConstant::RIGHT;
  }
  else
  {
    mooseError("Unknown direction in PiecewiseConstant");
  }
  return dir;
}


PiecewiseConstant::PiecewiseConstant(const std::string & name, InputParameters parameters) :
  Piecewise(name, parameters),
  _direction(getDirection(getParam<MooseEnum>("direction")))
{
}

PiecewiseConstant::~PiecewiseConstant()
{
}

Real
PiecewiseConstant::value(Real t, const Point & p)
{
  Real func_value(0);
  Real x = t;
  if (_has_axis)
  {
    x = p(_axis);
  }

  unsigned i = 1;
  const unsigned len = functionSize();
  const Real toler = 1e-14;

  // endpoint case
  if ( (_direction == LEFT  && x > (1+toler) * domain(len-1)) ||
       (_direction == RIGHT && x > (1-toler) * domain(len-1)) )
  {
    func_value = range(len-1);
    i = len;
  }

  for ( ; i < len; ++i )
  {
    if ( _direction == LEFT &&
         x < (1+toler) * domain(i) )
    {
      func_value = range(i-1);
      break;
    }
    else if ( (_direction == RIGHT &&
               x < (1-toler) * domain(i)) )
    {
      func_value = range(i);
      break;
    }
  }

  return _scale_factor * func_value;
}

Real
PiecewiseConstant::timeDerivative(Real /*t*/, const Point & /*p*/)
{
  return 0;
}

Real
PiecewiseConstant::integral()
{
  const unsigned len = functionSize();
  Real sum(0);
  unsigned offset = 0;
  if (_direction == RIGHT)
  {
    offset = 1;
  }
  for (unsigned i(0); i < len-1; ++i)
  {
    sum += range(i+offset) * (domain(i+1) - domain(i));
  }
  return _scale_factor * sum;
}

Real
PiecewiseConstant::average()
{
  return integral()/(domain(functionSize()-1) - domain(0));
}
