vtk_module(vtkIOExodus
  GROUPS
    StandAlone
  TEST_DEPENDS
    vtkTestingRendering
    vtkInteractionStyle
    vtkRendering${VTK_RENDERING_BACKEND}
  KIT
    vtkIO
  DEPENDS
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkIOCore
    vtkIOXMLParser
    vtkexodusII
  PRIVATE_DEPENDS
    vtkFiltersCore
    vtksys
  )