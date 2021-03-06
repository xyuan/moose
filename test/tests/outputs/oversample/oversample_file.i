[Mesh]
  type = FileMesh
  file = square.e
  dim = 2
  uniform_refine = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  console = true
  [./exodus]
    type = Exodus
    output_initial = false
    oversample = true
    refinements = 2
    file_base = exodus_oversample_custom_name
  [../]
[]
