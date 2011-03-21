#ifndef EXODUSOUTPUTTER_H_
#define EXODUSOUTPUTTER_H_

#include "Outputter.h"

// libMesh
#include "libmesh_common.h"
#include "exodusII_io.h"


namespace Moose {

class ExodusOutput : public Outputter {
public:
  ExodusOutput(SubProblem & problem);
  virtual ~ExodusOutput();

  virtual void output(const std::string & file_base);

protected:
  ExodusII_IO _out;

  int _num;                             // the number of timestep in the file
  // FIXME: later on, this will be a reference to transient executioner
  Real & _time;

  std::string getFileName(const std::string & file_base);
};

} // namespace

#endif /* OUTPUTTER_H_ */
