[Mesh]
  file = constraint_test.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
#  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
#  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Constraints]
  [./value]
    type = TiedValueConstraint
    variable = u
    slave = 2
    master = 3
  [../]
[]

[Preconditioning]
#  active = 'FDP'
  active = ' '

  [./FDP]
    type = FDP
#    full = true
#    off_diag_row    = 'v'
#    off_diag_column = 'u'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  l_max_its = 100
#  l_tol = 1e-1
#  l_tol = 1e-
#  nl_rel_tol = 1e-14
[]

#[Debug]
#  show_top_residuals = 5
#[]

[Outputs]
  file_base = out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
