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

#include "ExodusOutput.h"

#include "Moose.h"
#include "MooseApp.h"
#include "ActionWarehouse.h"
#include "Problem.h"
#include "ActionFactory.h"
#include "MooseObjectAction.h"
#include "MooseInit.h"
#include "ExodusFormatter.h"

// libMesh
#include "libmesh/exodusII.h"
#include "libmesh/exodusII_io.h"

#include <sstream>
#include <iomanip>

ExodusOutput::ExodusOutput(MooseApp & app, EquationSystems & es, bool output_input, SubProblem & sub_problem, std::string name) :
    Outputter(es, sub_problem, name),
    _app(app),
    _out(NULL),
    _seq(declareRecoverableData<bool>("seq", false)),
    _file_num(declareRecoverableData<int>("file_num", 0)),
    _num(declareRecoverableData<int>("num", 0)),
    _output_input(output_input),
    _first(declareRecoverableData<bool>("first",true)),
    _mesh_just_changed(declareRecoverableData<bool>("mesh_just_changed",false))
{
}

ExodusOutput::~ExodusOutput()
{
  delete _out;
}

std::string
ExodusOutput::getFileName(const std::string & file_base)
{
  std::ostringstream exodus_stream_file_base;

  exodus_stream_file_base << file_base << ".e";
  if (_seq)
  {
    /** Legacy output format
     * exodus_stream_file_base << "_";
     * OSSRealzeroright(exodus_stream_file_base, 4, 0, _file_num);
     * return exodus_stream_file_base.str() + ".e";
     */
    if (_file_num > 1)
    {
      exodus_stream_file_base << "-s"
                              << std::setw(3)
                              << std::setprecision(0)
                              << std::setfill('0')
                              << std::right
                              << _file_num;
    }
  }

  return exodus_stream_file_base.str();
}


void
ExodusOutput::output(const std::string & file_base, Real time, unsigned int /*t_step*/)
{
  if (_out == NULL)
    allocateExodusObject();
  _num++;

  _out->write_timestep(getFileName(file_base), _es, _num, time + _app.getGlobalTimeOffset());
  _out->write_element_data(_es);
}

void
ExodusOutput::outputPps(const std::string & /*file_base*/, const FormattedTable & table, Real time)
{
  if (_out == NULL)
    return;     // do nothing and safely return - we can write global vars (i.e. PPS only when output() occured)

  // Check to see if the FormattedTable is empty, if so, return
  if (table.getData().empty())
    return;

  // Search through the map, find a time in the table which matches the input time.
  // Note: search in reverse, since the input time is most likely to be the most recent time.
  const Real time_tol = 1.e-12;

  std::map<Real, std::map<std::string, Real> >::const_reverse_iterator
    rit  = table.getData().rbegin(),
    rend = table.getData().rend();

  for (; rit != rend; ++rit)
    {
      // Difference between input time and the time stored in the table
      Real time_diff = std::abs((time - (*rit).first));

      // Get relative difference, but don't divide by zero!
      if ( std::abs(time) > 0.)
        time_diff /= std::abs(time);

      // Break out of the loop if we found the right time
      if (time_diff < time_tol)
        break;
    }

  // If we didn't find anything, print an error message
  if ( rit == rend )
    {
      Moose::err << "Input time: " << time
                 << "\nLatest Table time: " << (*(table.getData().rbegin())).first << std::endl;
      mooseError("Time mismatch in outputting Nemesis global variables\n"
                 "Have the postprocessor values been computed with the correct time?");
    }

  // Otherwise, fill local vectors with name/value information and write to file.
  const std::map<std::string, Real> & tmp = (*rit).second;

  std::vector<Real> global_vars;
  std::vector<std::string> global_var_names;
  global_vars.reserve(tmp.size());
  global_var_names.reserve(tmp.size());

  for (std::map<std::string, Real>::const_iterator ii = tmp.begin();
       ii != tmp.end(); ++ii)
  {
    // Push back even though we know the exact size, saves us keeping
    // track of one more index.
    global_var_names.push_back( (*ii).first );
    global_vars.push_back( (*ii).second );
  }
  _out->write_global_data( global_vars, global_var_names );
}


void
ExodusOutput::meshChanged()
{
  _mesh_just_changed = true;

  _append = false;
  _num = 0;

  delete _out;
  _out = NULL;
}

void
ExodusOutput::outputInput()
{
  // parser/action system are not mandatory subsystems to use, thus empty action system -> no input output
  if (_app.actionWarehouse().empty())
    return;

  if (_out == NULL)
    allocateExodusObject();

  ExodusFormatter syntax_formatter;
  syntax_formatter.printInputFile(_app.actionWarehouse());
  syntax_formatter.format();

  _out->write_information_records(syntax_formatter.getInputFileRecord());
}

void
ExodusOutput::setOutputPosition(const Point & /* p */)
{
  if (_file_num == 0) // This might happen in the case of a MultiApp reset
    _file_num = 1;

  sequence(true);
  meshChanged();
}

void
ExodusOutput::allocateExodusObject()
{
  // Create the output object and set the variables
  _out = new ExodusII_IO(_es.get_mesh());
  _out->set_output_variables(_output_variables, /*allow_empty=*/false);

  // Skip output of z coordinates for 2D meshes, but still
  // write 1D meshes as 3D, otherwise Paraview has trouble
  // viewing them for some reason.
  if (_es.get_mesh().mesh_dimension() != 1)
    _out->use_mesh_dimension_instead_of_spatial_dimension(true);

  if (_app.hasOutputPosition())
    _out->set_coordinate_offset(_app.getOutputPosition());

  if (_first || _mesh_just_changed)
    _file_num++;

  _first = false;

  _mesh_just_changed = false;

  // Set the append flag on the underlying ExodusII_IO object
  if (!_mesh_just_changed)
    _out->append(_append);
}
